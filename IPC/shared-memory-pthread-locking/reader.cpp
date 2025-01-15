/**
 * Consumer process will continue reading data as assigned from the writer process with delay in each iteraion of reading.
 * It will automatically break out from the loop if the writer process has terminated via checking 'operational' flag.
 *
 * Notice that there is no logic to avoid reading the old data as written into shared memory.
 */
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <csignal>
#include <random>
#include <thread>

#include "lib.h"

using namespace lib;

struct SharedData
{
	pthread_rwlock_t rwlock;
	char name[255];
	int id;
	bool operational;
};

ShmFdClient* s_shm_fd_obj = nullptr;
MMap* s_mmap = nullptr;
SharedData *s_ptr = nullptr;
bool s_is_unlock = false;	// require to not block writer process

// shared memory won't be unlinked automatically and it still exists on the machine if signal comes
// so we handle them here.
void signal_handler(int signal)
{
	if (s_ptr != nullptr && !s_is_unlock)
		pthread_rwlock_unlock(&s_ptr->rwlock);

	// just make a copy
	if (s_shm_fd_obj != nullptr)
		ShmFdClient stack_value = *s_shm_fd_obj;

	if (s_mmap != nullptr)
		MMap stack_value2 = *s_mmap;

	// force exit to avoid double destructor call otherwise it will return back to normal flow within main()
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

	int shm_id = shm_open(name, O_RDWR, 0666);
	if (shm_id == -1)
	{
		std::cerr << "shm_open() failed\n";
		return 1;
	}

	// for RAII
	ShmFdClient shm_fd_obj(shm_id, name);
	s_shm_fd_obj = &shm_fd_obj;

	SharedData *ptr = static_cast<SharedData*>(mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_id, 0));
	if (ptr == MAP_FAILED)
	{
		std::cerr << "mmap() failed\n";
		return 1;
	}

	s_ptr = ptr;

	// for RAII
	MMap mmap(ptr, SIZE);
	s_mmap = &mmap;

	bool operational = true;
	while (operational)
	{
		pthread_rwlock_rdlock(&ptr->rwlock);
		s_is_unlock = false;

		operational = ptr->operational;
		std::cout << "read - ID: " << ptr->id << ", name: " << ptr->name << ", operational: " << std::boolalpha << ptr->operational << std::endl;
		pthread_rwlock_unlock(&ptr->rwlock);

		s_is_unlock = true;

		// random delay time in ms
		int delay_ms = dis(gen);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	}

	return 0;
}
