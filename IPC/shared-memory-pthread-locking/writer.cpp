/**
 * Write process will write some data into shared memory.
 * Each iteration will delay some short duration of time in ms.
 * User can quit the writer process by pressing Ctrl+C then it will clear resource as well as setting
 * 'operional' data member of SharedData to notify other processes that it has terminated.
 */
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string_view>
#include <csignal>
#include <pthread.h>
#include <chrono>
#include <random>

#include "lib.h"

using namespace lib;

struct SharedData
{
	pthread_rwlock_t rwlock;
	char name[255];
	int id;
	bool operational;
};

static bool s_still_operate = true;

ShmFd* s_shm_fd_obj = nullptr;
MMap* s_mmap = nullptr;
SharedData* s_ptr = nullptr;
bool s_is_unlock = false;

// shared memory won't be unlinked automatically and it still exists on the machine if signal comes
// so we handle them here.
void signal_handler(int signal)
{
	s_still_operate = false;

	if (s_ptr != nullptr)
	{
		if (s_is_unlock)
			// need lock first
			pthread_rwlock_wrlock(&s_ptr->rwlock);

		s_ptr->operational = false;	// to signal other processes that writer process has down
		pthread_rwlock_unlock(&s_ptr->rwlock);
		pthread_rwlock_destroy(&s_ptr->rwlock);
	}

	// destroy the lock
	if (s_ptr != nullptr && !s_is_unlock)

	// just make a copy
	if (s_shm_fd_obj != nullptr)
		ShmFd stack_value = *s_shm_fd_obj;

	if (s_mmap != nullptr)
		MMap stack_value2 = *s_mmap;

	// force exit to avoid double destructor call as it will return back to normal flow within main()
	std::exit(1);
}

int main()
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	// random for ms to delay each iteration of writing into shared memory
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(100, 300);

	// recommended to use slash prefixed from manpage
	const char* name = "/osimhen";
	const int SIZE = sizeof(SharedData);

	int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1)
	{
		std::cerr << "shm_open() failed\n";
		return 1;
	}

	ftruncate(shm_fd, SIZE);

	// for RAII obj
	ShmFd shm_fd_obj(shm_fd, name);
	s_shm_fd_obj = &shm_fd_obj;

	SharedData *ptr = static_cast<SharedData*>(mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0));
	if (ptr == MAP_FAILED)
	{
		std::cerr << "mmap() failed\n";
		return 1;
	}

	s_ptr = ptr;

	// for RAII obj
	MMap mmap(ptr, SIZE);
	s_mmap = &mmap;

	// initialize rwlock for inter-process (only initialize in writer process)
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
	pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	pthread_rwlock_init(&ptr->rwlock, &attr);

	int increment_id = 0;
	while (s_still_operate)
	{
		using namespace std::chrono_literals;

		if (!s_still_operate)
			break;

		pthread_rwlock_wrlock(&ptr->rwlock);
		s_is_unlock = false;

		if (!s_still_operate)
			break;

		// write into shard memory region
		ptr->id = increment_id++;
		const char* message = "hello world";
		std::strcpy(ptr->name, message);
		ptr->operational = true;
		pthread_rwlock_unlock(&ptr->rwlock);
		s_is_unlock = true;

		std::cout << "wrote - ID:" << ptr->id << ", name: " << ptr->name << std::endl;

		// random delay time in ms
		int delay_ms = dis(gen);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	}

	return 0;
}
