#ifndef _LOCK_FREE_CLASSPOOL_WORKQUEUE_H
#define _LOCK_FREE_CLASSPOOL_WORKQUEUE_H

#include <list>
#include <functional>
#include <atomic>
#ifdef Linux
#include <errno.h>
#include<unistd.h>
#include "terra_logger.h"
#endif

#include "FastMemcpy.h"
#include "tbb/concurrent_queue.h"
namespace terra
{
	namespace common
	{
		//template <typename Data>

		//typedef std::function<void()> PushHandler;
		template <typename Data>
		class lockfree_classpool_workqueue
		{
			typedef std::function<void(Data*)> ProcessHandler;
		public:
			lockfree_classpool_workqueue();
			virtual ~lockfree_classpool_workqueue();
			//bool Push(Data* _pObject);
			bool CopyPush(Data* _pObject);
			//Data* Pop();
			bool Pop_Handle();
			void setHandler(ProcessHandler handler);
			void clear();
			void on_handle(Data*);
			int Pops_Handle(int max_pop = 0);

			void set_fd(int _fd){ fd = _fd; }
			bool read_available(){ return m_queue.empty() == false; }
			static const int MAX_QUEUE = 2048;
			static const int MAX_POOL_SIZE = 1024;
			//
			int size()
			{
				return m_queue.unsafe_size();
			}
			//
		protected:
			
		//	boost::lockfree::spsc_queue<Data *, boost::lockfree::capacity<MAX_QUEUE> > m_queue;
		//	boost::lockfree::spsc_queue<Data *, boost::lockfree::capacity<MAX_POOL_SIZE> > m_class_pool_queue;

			tbb::concurrent_queue<Data *> m_queue;
			tbb::concurrent_queue<Data *> m_class_pool_queue;
			ProcessHandler m_handler;
			Data* get_mem();
			void frem_mem(Data* ptr);
			int fd;
		};

		template <class Data>
		lockfree_classpool_workqueue<Data>::lockfree_classpool_workqueue()
		{
			//if (len < 16)
			//	len = 16;
			fd = -1;
			Data **m_data;
			m_data = (Data **)malloc(sizeof(Data *)*MAX_POOL_SIZE);

			for (unsigned int i = 0; i < MAX_POOL_SIZE; ++i)
			{
				m_data[i] = new Data;
				m_class_pool_queue.push(m_data[i]);
			}
			delete m_data;
		}

		template <class Data>
		lockfree_classpool_workqueue<Data>::~lockfree_classpool_workqueue()
		{
		}

		/*template <class Data>
		bool lockfree_classpool_workqueue<Data>::Push(Data* ptr)
		{
			if (ptr != nullptr)
			{
				while (!m_queue.push(ptr))
					;
				return true;
			}
			return false;
		}*/

		template <typename Data>
		bool lockfree_classpool_workqueue<Data>::CopyPush(Data* _pObject)
		{
			if (_pObject != nullptr)
			{
				Data* ptr = get_mem();
				memcpy_lw(ptr, _pObject, sizeof(Data));
				//while (!m_queue.push(ptr))
				//{
				//}
				m_queue.push(ptr);
#ifdef Linux
				if (fd != -1)
				{
					uint64_t buf = 1;
					int wlen = 0;
					while(1)
					{
						wlen = write(fd, &buf, sizeof(buf));
						if(wlen>0)
							return true;
						else
						{
							if (errno == EAGAIN||errno==EINTR)
							{
								continue;
							}
							else
							{
								loggerv2::error("write efd fail");
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

		//template <class Data>
		//Data* lockfree_classpool_workqueue<Data>::Pop()
		//{
		//	Data* ptr;
		//	if (m_queue.pop(ptr))
		//		return ptr;
		//	else
		//		return nullptr;
		//}
		template <class Data>
		bool lockfree_classpool_workqueue<Data>::Pop_Handle()
		{
			Data* ptr;
			if (m_queue.try_pop(ptr))
			{
				on_handle(ptr);
				frem_mem(ptr);
				return true;
			}
			return false;
		}

		template <class Data>
		int lockfree_classpool_workqueue<Data>::Pops_Handle(int max_pop)
		{
			int i = 0;
			Data* ptr;
			while (m_queue.try_pop(ptr))
			{
				on_handle(ptr);

				frem_mem(ptr);
				++i;
				if (max_pop>0 && i>=max_pop)
				{
					return i;
				}
				
			}
			return i;
		}

		template <typename Data>
		Data* lockfree_classpool_workqueue<Data>::get_mem()
		{
			Data* ptr;
			if (m_class_pool_queue.try_pop(ptr))
				return ptr;
			ptr = new Data;
			return ptr;
		}

		template <typename Data>
		void lockfree_classpool_workqueue<Data>::frem_mem(Data* ptr)
		{
			if (ptr != nullptr)
			{
				//while (!m_class_pool_queue.push(ptr))
				//	;
				m_class_pool_queue.push(ptr);
			}
		}

		template <class Data>
		void lockfree_classpool_workqueue<Data>::setHandler(ProcessHandler handler)
		{
			m_handler=handler;
		}

		template <class Data>
		void lockfree_classpool_workqueue<Data>::clear()
		{
			m_handler.clear();
		}

		template <typename Data>
		void lockfree_classpool_workqueue<Data>::on_handle(Data* ptr)
		{
			if (ptr!=nullptr && m_handler!= nullptr)
				m_handler(ptr);
		}
	}

}
#endif