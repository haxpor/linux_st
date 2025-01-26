#pragma once

#include <pthread.h>
#include <stdexcept>
#include <ostream>
#include <cassert>
#include <mutex>
#include <type_traits>

namespace lib
{

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
	alignas(64) pthread_rwlock_t rwlock;
	alignas(64) int head;
	alignas(64) int tail;
};

const int sElementSize = 500;
struct SharedData
{
	alignas(64) pthread_rwlock_t rwlock;
	alignas(64) bool operational;

	RingBufferCtrlFields rb_ctrl_fields;
	ElementData elems[sElementSize];
};

// this section will be hidden from visiblity
namespace
{

#if 0
	struct CheckElementDataAlignment
	{
		using BaseType = ElementData;
		using Name = typename std::remove_all_extents<decltype(BaseType::name)>::type;
		static constexpr bool is_name_aligned = (alignof(BaseType) == 64) && (alignof(Name) == 1);
		static constexpr bool is_id_aligned = alignof(BaseType::id) == alignof(int);

		static_assert(is_name_aligned, "ElementData::name must align on cacheline size(64 bytes) and its own natural size(1)");
		static_assert(is_id_aligned, "ElementData::id must align on its natural size(4)");
	};

	struct CheckSharedDataAlignment
	{
		static constexpr bool is_struct_aligned = alignof(SharedData) == 64;
		static constexpr bool is_rb_ctrl_fields_aligned = alignof(SharedData::rb_ctrl_fields) == 64;
		static constexpr bool is_elems_aligned = alignof(SharedData::elems) == 64;

		static_assert(is_struct_aligned, "SharedData must align on cacheline size(64)");
		static_assert(is_rb_ctrl_fields_aligned, "SharedData::rb_ctrl_fields must align on cacheline size(64)");
		static_assert(is_elems_aligned, "SharedData::elems must align on cacheline size(64)");
	};
#endif
}

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
