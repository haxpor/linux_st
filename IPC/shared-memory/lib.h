#pragma once

namespace lib
{

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
