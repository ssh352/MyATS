#ifndef _LOCK_FREE_WORKQUEUE_COMMON_H
#define _LOCK_FREE_WORKQUEUE_COMMON_H

#include <list>
#include <functional>
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <tbb/concurrent_queue.h>
#ifdef Linux
#include<unistd.h>
#include <errno.h>
#endif

namespace terra
{
	namespace common
	{
		//template <typename Data>
		
		//typedef std::function<void()> PushHandler;
		template <typename Data>
		class LockFreeWorkQueue
		{
			typedef std::function<void(Data*)> ProcessHandler;
		public:
			LockFreeWorkQueue();
			virtual ~LockFreeWorkQueue();
			bool Push(Data* _pObject);
			Data* Pop_wait();
			Data* Pop();
			bool Pop_Handle();
			int Pops_Handle(int max_pop = 0);
			bool Pop_Handle_Keep();
			int Pops_Handle_Keep(int max_pop = 0);
			void setHandler(ProcessHandler handler);
			void clear();
			void set_fd(int _fd){ fd = _fd; }
			bool read_available(){ return m_queue.empty()==false; }
		protected:
			tbb::concurrent_queue<Data*> m_queue;
			std::list<ProcessHandler> m_handler;
			int fd;
		protected:

		};

		template <class Data>
		LockFreeWorkQueue<Data>::LockFreeWorkQueue()
		{
			fd = -1;
		}

		template <class Data>
		LockFreeWorkQueue<Data>::~LockFreeWorkQueue()
		{
		}

		template <class Data>
		bool LockFreeWorkQueue<Data>::Push(Data* ptr)
		{
			if (ptr != nullptr)
			{
				/*while (!m_queue.push(ptr))
					;*/
				m_queue.push(ptr);
#ifdef Linux
				if(fd!=-1)
				{
					uint64_t buf = 1;
					int wlen = 0;
					while(1)
					{
						wlen = write(fd, &buf, sizeof(buf));
						if (wlen > 0)
							return true;
						else
						{
							if (errno == EAGAIN || errno == EINTR)
							{
								continue;
							}
							else
							{
								printf("write efd fail\n");
								break;
							}
						}
					}
				}
#endif
				return true;
			}
			return false;
		}

		template <class Data>
		Data* LockFreeWorkQueue<Data>::Pop_wait()
		{
			Data* ptr;
			//while (!m_queue.pop(ptr))
			while (!m_queue.try_pop(ptr))
			{
				;
			}
			return ptr;
		}

		template <class Data>
		Data* LockFreeWorkQueue<Data>::Pop()
		{
			Data* ptr = nullptr;
			int try_time = 0;
			while (!m_queue.try_pop(ptr)&&try_time<3)
			{
				++try_time;
			}
			return ptr;
		}

		template <class Data>
		bool LockFreeWorkQueue<Data>::Pop_Handle()
		{
			Data* ptr = Pop();
			if (ptr!=nullptr)
			{
				for (auto& fun : m_handler)
				{
					fun(ptr);
				}
				delete ptr;
				return true;
			}
			return false;
		}

		template <class Data>
		int LockFreeWorkQueue<Data>::Pops_Handle(int max_pop)
		{
			int i = 0;
			Data* ptr;

			while (1)//(m_queue.pop(ptr) )
			{
				ptr = Pop();
				if (ptr == nullptr)
					return i;

				for (auto& fun : m_handler)
				{
					fun(ptr);
				}
				delete ptr;

				++i;
				if (max_pop > 0 && i >= max_pop)
					return i;
			}
			return i;
		}
		
		template <class Data>
		bool LockFreeWorkQueue<Data>::Pop_Handle_Keep()
		{
			Data* ptr = Pop();
			if (ptr!=nullptr)
			{
				for (auto& fun : m_handler)
				{
					fun(ptr);
				}
				return true;
			}
			return false;
		}

		template <class Data>
		int LockFreeWorkQueue<Data>::Pops_Handle_Keep(int max_pop)
		{
			int i = 0;
			Data* ptr;
			while ((max_pop <= 0 || i < max_pop))
			{
				ptr = Pop();
				if (ptr == nullptr)
					break;

				for (auto& fun : m_handler)
				{
					fun(ptr);
				}
				++i;
			}
			return i;
		}
		template <class Data>
		void LockFreeWorkQueue<Data>::setHandler(ProcessHandler handler)
		{
			m_handler.push_back(handler);
		}

		//template <class Data>
		//void LockFreeWorkQueue<Data>::setPushHandler(PushHandler handler)
		//{
		//	m_pushFunc = handler;
		//}

		template <class Data>
		void LockFreeWorkQueue<Data>::clear()
		{
			m_handler.clear();
		}


		//template <typename Data>
		//class MutiLockFreeWorkQueue
		//{
		//	typedef std::function<void(Data*)> ProcessHandler;
		//public:
		//	MutiLockFreeWorkQueue();
		//	virtual ~MutiLockFreeWorkQueue();
		//	bool Push(Data* _pObject);
		//	Data* Pop_wait();
		//	Data* Pop();
		//	void setHandler(ProcessHandler handler);
		//	void clear();
		//	boost::lockfree::queue<Data *, boost::lockfree::capacity<2048>> m_queue;
		//	std::list<ProcessHandler> m_handler;

		//protected:
		//	std::atomic<bool> m_bRead_Available;

		//};

		//template <typename Data>
		//MutiLockFreeWorkQueue<Data>::MutiLockFreeWorkQueue()
		//{
		//	m_bRead_Available = false;
		//}

		//template <typename Data>
		//MutiLockFreeWorkQueue<Data>::~MutiLockFreeWorkQueue()
		//{
		//}

		//template <typename Data>
		//bool MutiLockFreeWorkQueue<Data>::Push(Data* ptr)
		//{
		//	if (ptr != nullptr)
		//	{
		//		while (!m_queue.push(ptr))
		//			;

		//		return true;
		//	}
		//	return false;
		//}

		//template <typename Data>
		//Data* MutiLockFreeWorkQueue<Data>::Pop_wait()
		//{
		//	Data* ptr;
		//	while (!m_queue.pop(ptr))
		//	{
		//		;
		//	}
		//	return ptr;
		//}

		//template <typename Data>
		//Data* MutiLockFreeWorkQueue<Data>::Pop()
		//{
		//	Data* ptr;
		//	if (m_queue.pop(ptr))
		//		return ptr;
		//	else
		//		return nullptr;
		//}

		//template <typename Data>
		//void MutiLockFreeWorkQueue<Data>::setHandler(ProcessHandler handler)
		//{
		//	m_handler.push_back(handler);
		//}

		//template <typename Data>
		//void MutiLockFreeWorkQueue<Data>::clear()
		//{
		//	m_handler.clear();
		//}
	}

}
#endif