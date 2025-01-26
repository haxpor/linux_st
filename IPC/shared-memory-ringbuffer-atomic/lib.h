#pragma once

#include <pthread.h>
#include <stdexcept>
#include <ostream>
#include <cassert>
#include <mutex>
#include <type_traits>
#include <atomic>

namespace lib
{

// NOTE: byte-alignment won't make difference (even if apply where situation doesn't need it makes it worse) in case of no modification of the consuming data.
// So there is no risk of false sharing. No cacheline boundary alignment allows CPU to read more data per one fetch thus less latency.
// This is from benchmark.
struct ElementData
{
	char name[255];
	int id;

	friend std::ostream& operator<<(std::ostream& os, const ElementData& obj)
	{
		os << "ID: " << obj.id << ", Name: " << obj.name;
		return os;
	}
};

struct RingBufferCtrlFields
{
	alignas(64) std::atomic<int> head;
	alignas(64) std::atomic<int> tail;
};

const int sElementSize = 500;
struct SharedData
{
	alignas(64) std::atomic<bool> operational;
	RingBufferCtrlFields rb_ctrl_fields;
	ElementData elems[sElementSize];
};

// RAII of pthread_rwlock_wrlock
struct RWLock
{
	RWLock(pthread_rwlock_t* rwlock) :
		m_rwlock(rwlock)
	{
		assert(m_rwlock != nullptr);

		if (pthread_rwlock_wrlock(&(*m_rwlock)) != 0)
			throw std::runtime_error("Error: RWLock error in locking");
	}

	~RWLock()
	{
		assert(m_rwlock != nullptr);

		if (pthread_rwlock_unlock(&(*m_rwlock)) != 0)
			std::cerr << "Error: unlocking RWLock\n";
	}
private:
	pthread_rwlock_t* m_rwlock = nullptr;
};

// RAII of pthread_rwlock_rdlock
struct RWSharedLock
{
	RWSharedLock(pthread_rwlock_t* rwlock) :
		m_rwlock(rwlock)
	{
		assert(m_rwlock != nullptr);

		if (pthread_rwlock_rdlock(&(*m_rwlock)) != 0)
			throw std::runtime_error("Error: RWSharedLock error in locking");
	}

	~RWSharedLock()
	{
		assert(m_rwlock != nullptr);

		if (pthread_rwlock_unlock(&(*m_rwlock)) != 0)
			std::cerr << "Error: unlocking RWSharedLock\n";
	}
private:
	pthread_rwlock_t* m_rwlock = nullptr;
};

struct RWUniqueLock
{	
	RWUniqueLock(pthread_rwlock_t* rwlock) :
		m_rwlock(rwlock)
	{
		if (rwlock == nullptr)
			throw std::runtime_error("pthread_rwlock_t cannot be nullptr");

		lock();
	}
	
	RWUniqueLock(pthread_rwlock_t* rwlock, std::defer_lock_t t) :
		m_rwlock(rwlock)
	{
		if (rwlock == nullptr)
			throw std::runtime_error("pthread_rwlock_t cannot be nullptr");
	}

	void lock()
	{
		assert(m_rwlock != nullptr);

		if (m_own)
			throw std::runtime_error("Error: RWUniqueLock already locked. This is double lock.");
		if (pthread_rwlock_rdlock(&(*m_rwlock)) != 0)
			throw std::runtime_error("Error: RWUniqueLock error in locking");
		m_own = true;
	}

	void unlock()
	{
		assert(m_rwlock != nullptr);

		if (!m_own)
			throw std::runtime_error("Error: RWUniqueLock isn't locked yet");
		if (pthread_rwlock_unlock(&(*m_rwlock)) != 0)
			std::cerr << "Error: unlocking RWUniqueLock\n";
		m_own = false;
	}

	~RWUniqueLock()
	{
		assert(m_rwlock != nullptr);

		// only unlock if owns
		if (m_own)
			if (pthread_rwlock_unlock(&(*m_rwlock)) != 0)
				std::cerr << "Error: unlocking RWUniqueLock\n";
	}
private:
	pthread_rwlock_t* m_rwlock = nullptr;
	bool m_own = false;
};

struct ShmFd
{
	// require name to be null-terminated string
	ShmFd(int fd, std::string_view name):
		m_fd(fd),
		m_name(name)
	{}

	~ShmFd()
	{
		if (m_fd != -1)
		{
			std::cout << "ShmFd - releases resource" << std::endl;
			close(m_fd);
			shm_unlink(m_name.data());
		}
	}

	int get_id() const
	{
		return m_fd;
	}

	const char* get_name() const
	{
		// UNSAFE: we require that name shall be null-termianted string
		return m_name.data();
	}

private:
	int m_fd = -1;
	std::string_view m_name;
};

// client side, won't call shm_unlink()
struct ShmFdClient
{
	// require name to be null-terminated string
	ShmFdClient(int fd, std::string_view name):
		m_fd(fd),
		m_name(name)
	{}

	~ShmFdClient()
	{
		if (m_fd != -1)
		{
			std::cout << "ShmFdClient - releases resource" << std::endl;
			close(m_fd);
		}
	}

	int get_id() const
	{
		return m_fd;
	}

	const char* get_name() const
	{
		// UNSAFE: we require that name shall be null-termianted string
		return m_name.data();
	}

private:
	int m_fd = -1;
	std::string_view m_name;
};

struct MMap
{
	MMap(void* ptr, int size):
		m_ptr(ptr),
		m_size(size)
	{
	}

	~MMap()
	{
		if (m_ptr != nullptr)
		{
			std::cout << "MMap - releases resource" << std::endl;
			if (munmap(m_ptr, m_size) == -1)
				std::cerr << "MMap - munmap() error\n";
		}
	}

private:
	void* m_ptr;
	int m_size;
};

};
