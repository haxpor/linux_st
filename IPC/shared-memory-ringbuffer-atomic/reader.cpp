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
#include <chrono>

#ifdef BENCH_LATENCY
#include <fstream>
#endif

#include "lib.h"
#include "ringbuffer.h"

using namespace lib;

ShmFdClient* s_shm_fd_obj = nullptr;
MMap* s_mmap = nullptr;
SharedData *s_ptr = nullptr;

#ifdef BENCH_LATENCY
std::ofstream *s_ts_output_file = nullptr;
#endif

// shared memory won't be unlinked automatically and it still exists on the machine if signal comes
// so we handle them here.
void signal_handler(int signal)
{
#ifdef BENCH_LATENCY
	// close the file if not yet
	// this happens when signal caught after opening the file and we are not done yet
	if (s_ts_output_file != nullptr && s_ts_output_file->is_open())
		s_ts_output_file->close();
#endif

	// just make a copy
	if (s_shm_fd_obj != nullptr)
		ShmFdClient stack_value = *s_shm_fd_obj;

	if (s_mmap != nullptr)
		MMap stack_value2 = *s_mmap;

	// force exit to avoid double destructor call otherwise it will return back to normal flow within main()
	std::exit(1);
}

int main(int argc, char* argv[])
{
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	// random for ms to delay each iteration of writing into shared memory
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(50, 120);

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

	RingBuffer rb(ptr->elems, sElementSize, &ptr->rb_ctrl_fields.head, &ptr->rb_ctrl_fields.tail);

	bool operational = true;
	ElementData data;	// reuse holding data structure

#ifdef BENCH_LATENCY
	const char* ts_output_filename = "ts-input.txt";
	if (argc > 1)
		ts_output_filename = argv[1];

	// ignore the current file content, open for writing and truncate them
	std::ofstream ts_output_file(ts_output_filename, std::ios::out | std::ios::trunc);
	if (!ts_output_file.is_open())
	{
		std::cerr << "Error opening file for output benchmark of cache latency access\n";
		return 1;
	}

	s_ts_output_file = &ts_output_file;

	// turn off buffering of file
	ts_output_file.rdbuf()->pubsetbuf(nullptr, 0);
	// output header column names
	ts_output_file << "Timestamp,Latency\n" << std::flush;
#endif

	while (operational)
	{
#ifdef BENCH_LATENCY
		auto start = std::chrono::steady_clock::now();
		bool res = rb.get(data);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double, std::micro> elapsed = end - start;
		double elapsed_value = elapsed.count();

		if (res)
		{
			std::cout << data << std::endl;

			// current milli
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count();
			// output to the file for time series
			ts_output_file << ms << "," << elapsed_value << "\n" << std::flush;
		}
		else
			std::cerr << "not available data\n";
#else
		if (rb.get(data))
			std::cout << data << std::endl;
		else
			std::cerr << "not available data\n";
#endif

		//RWUniqueLock _opt_lock(&ptr->rwlock);
		operational = ptr->operational.load(std::memory_order_acquire);
		//_opt_lock.unlock();

		// random delay time in ms
		int delay_ms = dis(gen);
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	}

	return 0;
}
