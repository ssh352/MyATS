#ifndef __LOCK_FREE_RINGBUFFER_H__
#define __LOCK_FREE_RINGBUFFER_H__
#include <cstdint>
#include <atomic>

#ifdef __linux__
#define CAS(v,old_v,new_v) __sync_bool_compare_and_swap (v,old_v,new_v)
#else
#include "windows.h"
#define CAS(des, old_v, new_v) _InterlockedCompareExchange((long *)des,new_v, old_v) == old_v
#endif
using namespace std;
template<typename T>
class LockFreeRingBuffer
{
public:
	void init(int len);

	void push(T &data, bool is_single_p = false);//将T push入队列，is_single_p用去区分写入场景是否是单线程写

	uint32_t get_next(bool is_single_p = false);//从队列里获取下个可用的成员下标，和publish配合使用。多线程模式下，请使用该方法后尽快调用publish方法，否则会阻塞
	//别的线程的publish方法
	void publish(uint32_t index, bool is_single_p = false);//和get_next方法配套使用，意思是：从get_next拿到的下标使用完毕，通过publish方法将prod.tail前移

	bool copy_pop(T &data, bool is_single_c = false);
	bool ref_pop(T* &data, uint32_t &index, bool is_single_c = false);

	T* get_by_sequence(const uint64_t sequence);
	void add_sequence(bool is_single_p = false);

public:
	/** Ring producer status. */
	struct prod
	{
		volatile uint32_t head;  /**< Producer head. */
		volatile uint32_t tail;  /**< Producer tail. */
		long long appending[7];
	} prod;

	/** Ring consumer status. */
	struct cons
	{
		volatile uint32_t head;  /**< Consumer head. */
		volatile uint32_t tail;  /**< Consumer tail. */
		long long appending[7];
	} cons;

	struct seq
	{
		volatile uint64_t m_sequence;
		long long appending[7];
	}sequence;

	int64_t ring_size;
	
	T ring[0];
};

template<typename T>
void LockFreeRingBuffer<T>::init(int len)
{
	this->ring_size = len;
	this->prod.head = 0;
	this->prod.tail = 0;

	this->cons.head = 0;
	this->cons.tail = 0;
	sequence.m_sequence = 0;
}

template<typename T>
void LockFreeRingBuffer<T>::add_sequence(bool is_single_p)
{
	uint64_t temp_sequence, next_sequence;
	bool success = false;
	do
	{
		temp_sequence = sequence.m_sequence;
		next_sequence = temp_sequence + 1;

		if (is_single_p)
		{
			++sequence.m_sequence;
			break;
		}

		success = CAS(&(sequence.m_sequence), temp_sequence, next_sequence);
	} while (success == false);
}

template<typename T>
void LockFreeRingBuffer<T>::push(T &data, bool is_single_p)
{
	uint32_t prod_head, prod_next;
	bool success = false;
	do
	{
		prod_head = prod.head;
		prod_next = (prod_head + 1) % ring_size;

		if (prod_next == prod.tail)
			continue;

		if (is_single_p)
		{
			prod.head = prod_next;
			break;
		}

		success = CAS(&(prod.head), prod_head, prod_next);
	} while (success == false);
	//copy
	this->ring[prod_next] = data;
	/*
	* If there are other enqueues in progress that preceded us,
	* we need to wait for them to complete
	*/
	if (!is_single_p)
	{
		while (prod.tail != prod_head)
			;
	}
	this->prod.tail = prod_next;

	add_sequence(is_single_p);
}

template<typename T>
uint32_t LockFreeRingBuffer<T>::get_next(bool is_single_p)
{
	uint32_t prod_head, prod_next;
	bool success = false;
	do
	{
		prod_head = prod.head;

		prod_next = (prod_head + 1) % ring_size;

		if (is_single_p)
		{
			prod.head = prod_next;
			break;
		}

		success = CAS(&(prod.head), prod_head, prod_next);
	} while (success == false);
	//copy
	return prod_next;
}

template<typename T>
void LockFreeRingBuffer<T>::publish(uint32_t index, bool is_single_p)
{
	uint32_t prod_tail, prod_tail_next;
	do
	{
		prod_tail = prod.tail;
		prod_tail_next = (prod_tail + 1) % ring_size;

		if (prod_tail_next == index || is_single_p)
		{
			prod.tail = prod_tail_next;
			break;
		}
		else
			continue;

	} while (1);

	add_sequence(is_single_p);
}

template<typename T>
bool LockFreeRingBuffer<T>::copy_pop(T &data, bool is_single_c)
{

	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	bool success = false;
	do
	{
		cons_head = this->cons.head;
		prod_tail = this->prod.tail;

		entries = (prod_tail - cons_head);

		/* Set the actual entries for dequeue */
		if (1 > entries)
		{
			return false;
		}

		cons_next = (cons_head + 1) % ring_size;
		if (is_single_c)
		{
			cons.head = cons_next;
			break;
		}
		success = CAS(&(cons.head), cons_head, cons_next);
	} while (success == false);

	data = ring[cons_next];

	if (!is_single_c)
	{
		while (cons.tail != cons_head)
			;
	}
	cons.tail = cons_next;
	return true;

}

template<typename T>
bool LockFreeRingBuffer<T>::ref_pop(T* &data, uint32_t &index, bool is_single_c)
{

	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	bool success = false;
	do
	{
		cons_head = this->cons.head;
		prod_tail = this->prod.tail;

		entries = (prod_tail - cons_head);

		/* Set the actual entries for dequeue */
		if (1 > entries)
		{
			return false;
		}

		cons_next = (cons_head + 1) % ring_size;
		if (is_single_c)
		{
			cons.head = cons_next;
			break;
		}
		success = CAS(&(cons.head), cons_head, cons_next);
	} while (success == false);

	data = &ring[cons_next];
	index = cons_next;

	if (!is_single_c)
	{
		while (cons.tail != cons_head)
			;
	}
	cons.tail = cons_next;
	return true;

}

template<typename T>
T* LockFreeRingBuffer<T>::get_by_sequence(const uint64_t sequence)
{
	return &ring[sequence%ring_size];
}

#endif
