#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string_view>
#include <csignal>

#include "lib.h"

using namespace lib;

ShmFd* s_shm_fd_obj = nullptr;
MMap* s_mmap = nullptr;

// shared memory won't be unlinked automatically and it still exists on the machine if signal comes
// so we handle them here.
void signal_handler(int signal)
{
	// just make a copy
	if (s_shm_fd_obj != nullptr)
		ShmFd stack_value = *s_shm_fd_obj;

	if (s_mmap != nullptr)
		MMap stack_value2 = *s_mmap;

	// force exit to avoid double destructor call for those two objects
	std::exit(1);
}

int main()
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	// recommended to use slash prefixed from manpage
	const char* name = "/osimhen";
	const int SIZE = 4096;

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

	void *ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, shm_fd, 0);
	if (ptr == MAP_FAILED)
	{
		std::cerr << "mmap() failed\n";
		return 1;
	}

	// for RAII obj
	MMap mmap(ptr, SIZE);
	s_mmap = &mmap;

	// write into shard memory region
	const char* message = "hello world";
	std::strcpy(static_cast<char*>(ptr), message);

	std::cout << "Wrote: " << message << std::endl;

	std::cout << "Press enter to finish" << std::endl;
	std::cin.ignore();

	return 0;
}
