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
#include <chrono>
#include <random>

#include "lib.h"
#include "ringbuffer.h"

using namespace lib;

static bool s_still_operate = true;

ShmFd* s_shm_fd_obj = nullptr;
MMap* s_mmap = nullptr;
SharedData* s_ptr = nullptr;

// shared memory won't be unlinked automatically and it still exists on the machine if signal comes
// so we handle them here.
void signal_handler(int signal)
{
	s_still_operate = false;

	if (s_ptr != nullptr)
	{
		// destroy RingBuffer's rwlock
		{
			// due to RAII, we dont need to worry about unlocking
			//pthread_rwlock_destroy(&s_ptr->rb_ctrl_fields.rwlock);
		}

		// destroy SharedData's rwlock
		{
			//RWUniqueLock _lock(&s_ptr->rwlock, std::defer_lock);
			//_lock.lock();
			s_ptr->operational.store(false, std::memory_order_release);	// to signal other processes that writer process has down
			//_lock.unlock();
			//pthread_rwlock_destroy(&s_ptr->rwlock);
		}
	}

	// just make a copy
	if (s_shm_fd_obj != nullptr)
		ShmFd stack_value = *s_shm_fd_obj;

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
	std::uniform_int_distribution<> dis(20, 40);

	// recommended to use slash prefixed from manpage
	const char* name = "/osimhen";
	const int SIZE = sizeof(SharedData);

	int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1)
	{
		std::cerr << "shm_open() failed\n";
		return 1;
	}

	if (ftruncate(shm_fd, SIZE) != 0)
		std::cerr << "ftruncate error\n";

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

	// initialize rwlock for SharedData's control fields
	//{
	//	pthread_rwlockattr_t attr;
	//	pthread_rwlockattr_init(&attr);
	//	pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	//	pthread_rwlock_init(&ptr->rwlock, &attr);
	//}

	// initialize rwlock for RingBuffer's control fields
	//{
	//	pthread_rwlockattr_t attr;
	//	pthread_rwlockattr_init(&attr);
	//	pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	//	pthread_rwlock_init(&ptr->rb_ctrl_fields.rwlock, &attr);
	//}

	RingBuffer rb(ptr->elems, sElementSize, &ptr->rb_ctrl_fields.head, &ptr->rb_ctrl_fields.tail);

	int increment_id = 0;
	while (s_still_operate)
	{
		using namespace std::chrono_literals;

		if (!s_still_operate)
			break;

		// prepare ElementData
		ElementData elem_data;
		elem_data.id = increment_id++;
		const char* message = "hello world";
		std::strcpy(elem_data.name, message);

		if (!s_still_operate)
			break;

		rb.put(elem_data);
		rb.printAllElements();
		std::cout << "---------" << std::endl;

		if (!ptr->operational.load(std::memory_order_acquire))
		{
			ptr->operational.store(true, std::memory_order_release);
		}

		// random delay time in ms
		int delay_ms = dis(gen);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	}

	return 0;
}
