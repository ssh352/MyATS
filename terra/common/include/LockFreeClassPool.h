#ifndef _LOCK_FREE_CLASSPOOL_COMMON_H
#define _LOCK_FREE_CLASSPOOL_COMMON_H

#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>


namespace terra
{
	namespace common
	{

		template <typename Data>
		class SpscLockFreeClassPool
		{
		public:
			SpscLockFreeClassPool(){};
			virtual ~SpscLockFreeClassPool(){};

			template <typename ...Args>
			void init(size_t len, Args&&... arg)
			{
				if (len < 16)
					len = 16;
				mlen = len;
				for (size_t i = 0; i < len; ++i)
				{
					m_queue.push(std::shared_ptr<Data>(new Data(std::forward<Args>(arg)...)));/*, [this](Data *p)
					{
						while(!m_queue.push(std::shared_ptr<Data>(p)))
							;
					}*/
					
				}
			}

			template <typename ...Args>
			bool Push(std::shared_ptr<Data> ptr)
			{
				if (ptr != nullptr)
				{
					while (!m_queue.push(ptr))
						;
					return true;
				}
				return false;
			}

			/*template <typename ...Args>
			void free_mem(std::shared_ptr<Data> ptr)
			{
				Push(ptr);
			}*/

			template <typename ...Args>
			std::shared_ptr<Data> Pop_wait()
			{
				std::shared_ptr<Data> ptr;
				while (!m_queue.pop(ptr))
				{
					;
				}
				return ptr;
			}

			template <typename ...Args>
			std::shared_ptr<Data> Pop()
			{
				std::shared_ptr<Data> ptr;
				if (m_queue.pop(ptr))
					return std::move(ptr);
				else
					return nullptr;
			}

			template <typename ...Args>
			std::shared_ptr<Data> get_mem()
			{
				auto ptr = Pop();
				auto p = std::shared_ptr<Data>(new Data(std::move(*ptr.get())), [this](Data *p)
				{
					m_queue.push(std::shared_ptr<Data>(p));
				}
				);
				return p;
			}

			boost::lockfree::spsc_queue<std::shared_ptr<Data>, boost::lockfree::capacity<32> > m_queue;
			size_t mlen;
		};


		template <typename Data>
		class SingleLockFreeClassPool
		{
		public:
			SingleLockFreeClassPool(){};
			~SingleLockFreeClassPool()
			{
				for (unsigned int i = 0; i < mlen; ++i)
				{
					if (m_data[i] != nullptr)
					{
						delete(m_data[i]);
						m_data[i] = nullptr;
					}
				}
				delete(m_data);
			}
			template <typename ...Args>
			void init(size_t len, Args&&... arg)
			{
				if (len < 16)
					len = 16;
				mlen = len;

				m_data = (Data **)malloc(sizeof(Data *)*mlen);

				for (unsigned int i = 0; i < mlen; ++i)
				{
					m_data[i] = new Data(std::forward<Args>(arg)...);
					m_queue.push(m_data[i]);
				}
			}
			bool Push(Data* _pObject);
			Data* Pop_wait();
			Data* Pop();


			std::atomic<bool> m_bRead_Available;
			boost::lockfree::spsc_queue<Data *, boost::lockfree::capacity<16*2> > m_queue;
			Data* get_mem();
			void free_mem(Data*);

			Data **m_data;
			unsigned int mlen;
		};
		

		template <class Data>
		bool SingleLockFreeClassPool<Data>::Push(Data* ptr)
		{
			if (ptr != nullptr)
			{
				while (!m_queue.push(ptr))
					;
				return true;
			}
			return false;
		}

		template <class Data>
		Data* SingleLockFreeClassPool<Data>::Pop_wait()
		{
			Data* ptr;
			while (!m_queue.pop(ptr))
			{
				;
			}
			return ptr;
		}

		template <class Data>
		Data* SingleLockFreeClassPool<Data>::get_mem()
		{
			Data* ptr;
			ptr = Pop();
			if (ptr == nullptr)
			{
				ptr = new Data;
			}
			return ptr;
		}

		template <class Data>
		void SingleLockFreeClassPool<Data>::free_mem(Data *ptr)
		{
			Push(ptr);
		}

		template <class Data>
		Data* SingleLockFreeClassPool<Data>::Pop()
		{
			Data* ptr;
			if (m_queue.pop(ptr))
				return ptr;
			else
				return nullptr;
		}
	}

}
#endif
