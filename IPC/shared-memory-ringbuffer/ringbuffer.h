#pragma once

#include <pthread.h>
#include <cassert>
#include <sched.h>

#include "lib.h"

using namespace lib;

// Ring buffer operating through pointer
class RingBuffer
{
public:
	// accept the pointer to the shared data to mgmt fields
    RingBuffer(ElementData* buffer_ptr, int buffer_size, pthread_rwlock_t* rwlock_ptr, int* head_ptr, int* tail_ptr) :
        m_buffer(buffer_ptr),
		m_buffer_size(buffer_size),
        m_head(head_ptr),
        m_tail(tail_ptr),
		m_rwLock(rwlock_ptr)
    {
		assert(m_buffer != nullptr);
		assert(m_buffer_size > 0);
		assert(m_rwLock != nullptr);
		assert(m_head != nullptr);
		assert(m_tail != nullptr);
    }

    bool isFull()
    {
		RWSharedLock _shared_lock(m_rwLock);
        return _isFull_nolock();
    }

private:
    bool _isFull_nolock()
    {
        const size_t s = m_buffer_size;
        return ((s + *m_head - *m_tail) % s) + 1 >= s;  
    }

	// disable copy-construct, and assignment operator
	RingBuffer(const RingBuffer&);
	RingBuffer(RingBuffer&&);
	RingBuffer& operator=(RingBuffer&);
	RingBuffer& operator=(RingBuffer&&);

public:
    bool isEmpty()
    {
		RWSharedLock _shared_lock(m_rwLock);
        return !_isFull_nolock() && (*m_head == *m_tail);
    }

    void put(const ElementData& obj)
    {
        while (isFull())
		{
			// relinquish the current thread to the back of the queue
			// allow other threads even on other processes to execute, so more chance that reader process will read something from the queue.
			sched_yield();

			// there might be better option to do this
		}

		RWLock _lock(m_rwLock);
        m_buffer[*m_head] = obj;
        *m_head = (*m_head + 1) % m_buffer_size;

		std::cout << "Head: " << *m_head << ", Tail: " << *m_tail << std::endl;
    }

	bool get(ElementData& rdata)
	{
		return getImpl(rdata);
	}

    void reset()
    {
		RWLock _lock(m_rwLock);
        *m_head = 0;
        *m_tail = 0;
        // TODO: shall we reset value of all element as well ?
    }

    size_t size()
    {
        if (isFull())
			return m_buffer_size;
        else
        {
            // head can be at index less than tail
			RWSharedLock _lock(m_rwLock);
            return (m_buffer_size + *m_head - *m_tail) % m_buffer_size;
        }
    }

    void printAllElements()
    {
        // isEmpty() already locked
        if (!isEmpty())
        {
            std::size_t optSize = size();

			RWUniqueLock _lock(m_rwLock, std::defer_lock);
			_lock.lock();
            int tailCopy = *m_tail;
            for (std::size_t i=0; i<optSize-1; i++)
            {
                std::cout << m_buffer[tailCopy] << "\n";
                tailCopy = (tailCopy + 1) % m_buffer_size;
            }
			_lock.unlock();
        }
    }

private:
	
	// Return -1 if there is no more element to return
	bool getImpl(ElementData& rdata)
	{
		if (isEmpty())
			return false;

		RWLock _lock(m_rwLock);
		rdata = m_buffer[*m_tail];
		*m_tail = (*m_tail + 1) % m_buffer_size;

		return true;
	}

	// possibly need to implement return by batch

private:
    ElementData* m_buffer = nullptr;
	const int m_buffer_size = 0;
    int* m_head = nullptr;
    int* m_tail = nullptr;
    pthread_rwlock_t* m_rwLock = nullptr;
};
