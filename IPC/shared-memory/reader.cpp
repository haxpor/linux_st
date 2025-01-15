#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib.h"

using namespace lib;

int main()
{
	// recommended to use slash prefixed from manpage
	const char* name = "/osimhen";
	const int SIZE = 4096;

	int shm_id = shm_open(name, O_RDONLY, 0666);
	if (shm_id == -1)
	{
		std::cerr << "shm_open() failed\n";
		return 1;
	}

	// for RAII
	ShmFdClient shm_fd_obj(shm_id, name);

	void *ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED_VALIDATE, shm_id, 0);
	if (ptr == MAP_FAILED)
	{
		std::cerr << "mmap() failed\n";
		return 1;
	}

	// for RAII
	MMap mmap(ptr, SIZE);

	std::cout << "Read: " << static_cast<char*>(ptr) << std::endl;

	return 0;
}
