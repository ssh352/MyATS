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
	void push(T &data,bool is_single_p=false);
	bool pop(T &data,bool is_single_c=false);

public:
	/** Ring producer status. */
	struct prod
	{
		volatile uint32_t head;  /**< Producer head. */
		volatile uint32_t tail;  /**< Producer tail. */
	} prod;

	/** Ring consumer status. */
	struct cons
	{
		volatile uint32_t head;  /**< Consumer head. */
		volatile uint32_t tail;  /**< Consumer tail. */
	} cons;

	uint32_t size;
	T ring[0];
};

template<typename T>
void LockFreeRingBuffer<T>::init(int len)
{
	this->size = len;
	this->prod.head = 0;
	this->prod.tail = 0;

	this->cons.head = 0;
	this->cons.tail = 0;
}

template<typename T>
void LockFreeRingBuffer<T>::push(T &data,bool is_single_p)
{
	uint32_t prod_head, prod_next;
	//uint32_t cons_tail, free_entries;
	bool success = false;
	do
	{
		prod_head = prod.head;

		prod_next = (prod_head + 1) % size;

		if(is_single_p)
		{
		   prod.head = prod_next;
		   break;
		}
		//success = std::atomic_compare_exchange_weak(&(prod.head), prod_head, prod_next);
		success = CAS(&(prod.head), prod_head, prod_next);
	} while (success == false);
	//copy
	this->ring[prod_next] = data;
	/*
	* If there are other enqueues in progress that preceded us,
	* we need to wait for them to complete
	*/
	if(!is_single_p)
	{
		while (prod.tail != prod_head)
			;
	}
	this->prod.tail = prod_next;
}

template<typename T>
bool LockFreeRingBuffer<T>::pop(T &data,bool is_single_c)
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

		cons_next = (cons_head + 1) % size;
		if(is_single_c)
		{
			cons.head = cons_next;
			break;
		}
		//success = std::atomic_compare_exchange_weak(&(cons.head), &(cons_head), cons_next);
		success = CAS(&(cons.head), cons_head, cons_next);
	} while (success == false);

	data = ring[cons_next];
	
	if(!is_single_c)
	{
		while (cons.tail != cons_head)
			;
	}
	cons.tail = cons_next;
	return true;

}
#endif
