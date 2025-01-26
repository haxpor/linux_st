#pragma once

#include <cassert>
#include <sched.h>

#include "lib.h"

using namespace lib;

// Ring buffer operating through pointer
class RingBuffer
{
public:
	// accept the pointer to the shared data to mgmt fields
    RingBuffer(ElementData* buffer_ptr, int buffer_size, std::atomic<int>* head_ptr, std::atomic<int>* tail_ptr) :
	    m_buffer(buffer_ptr),
	    m_buffer_size(buffer_size),
	    m_head(head_ptr),
	    m_tail(tail_ptr)
    {
		assert(m_buffer != nullptr);
		assert(m_buffer_size > 0);
		assert(m_head != nullptr);
		assert(m_tail != nullptr);
    }

    bool isFull()
    {
        return _isFull_nolock();
    }

private:
    bool _isFull_nolock()
    {
        const size_t s = m_buffer_size;
        return ((s + m_head->load(std::memory_order_acquire) - m_tail->load(std::memory_order_acquire)) % s) + 1 >= s;  
    }

	// disable copy-construct, and assignment operator
	RingBuffer(const RingBuffer&);
	RingBuffer(RingBuffer&&);
	RingBuffer& operator=(RingBuffer&);
	RingBuffer& operator=(RingBuffer&&);

public:
    bool isEmpty()
    {
        return !_isFull_nolock() && (m_head->load(std::memory_order_acquire) == m_tail->load(std::memory_order_acquire));
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

		int head = m_head->load(std::memory_order_acquire);

        m_buffer[head] = obj;
        m_head->store((head + 1) % m_buffer_size, std::memory_order_release);

		std::cout << "Head: " << m_head->load(std::memory_order_acquire) << ", Tail: " << m_tail->load(std::memory_order_acquire) << std::endl;
    }

	bool get(ElementData& rdata)
	{
		return getImpl(rdata);
	}

    void reset()
    {
        m_head->store(0, std::memory_order_release);
        m_tail->store(0, std::memory_order_release);
        // TODO: shall we reset value of all element as well ?
    }

    size_t size()
    {
        if (isFull())
			return m_buffer_size;
        else
        {
            // head can be at index less than tail
            return (m_buffer_size + m_head->load(std::memory_order_acquire) - m_tail->load(std::memory_order_acquire)) % m_buffer_size;
        }
    }

    void printAllElements()
    {
        // isEmpty() already locked
        if (!isEmpty())
        {
            std::size_t optSize = size();

            int tailCopy = m_tail->load(std::memory_order_acquire);
            for (std::size_t i=0; i<optSize-1; i++)
            {
                std::cout << m_buffer[tailCopy] << "\n";
                tailCopy = (tailCopy + 1) % m_buffer_size;
            }
        }
    }

private:
	
	// Return -1 if there is no more element to return
	bool getImpl(ElementData& rdata)
	{
		if (isEmpty())
			return false;

		// improved one refactoring isEmpty() but didn't see improvement much
		//int head = m_head->load(std::memory_order_acquire);
		int tail = m_tail->load(std::memory_order_acquire);

		//auto is_full = [s = this->m_buffer_size, head, tail]() -> bool {
		//	return ((s + head - tail) % s) + 1 >= s;  
		//};
		//auto is_empty = [head, tail, is_full]() -> bool {
		//	return !is_full() && (head == tail);
		//};

		//if (is_empty())
		//	return false;

		rdata = m_buffer[tail];
		m_tail->store((tail + 1) % m_buffer_size, std::memory_order_release);

		return true;
	}

	// possibly need to implement return by batch

private:
    ElementData* m_buffer = nullptr;
	const int m_buffer_size = 0;
	std::atomic<int>* m_head = nullptr;
	std::atomic<int>* m_tail = nullptr;
};
