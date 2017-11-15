#ifndef __ORDERBOOKNOTIFIER_H__
#define __ORDERBOOKNOTIFIER_H__

#include "order.h"
#include "singleton.hpp"



namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class order_book_notifier :public SingletonBase<order_book_notifier>
			{
			public:
				typedef std::function<void(order *)>OrderUpdateEventHandler;

				void add_order_cb(order* order)
				{
					boost_read_lock rlock(m_rwmutex);
					auto it = m_func.begin();
					for (; it != m_func.end(); ++it)
					{
						(*it)(order);
					}

				}

				void update_order_cb(order* order)
				{
					boost_read_lock rlock(m_rwmutex);
					auto it = m_func.begin();
					for (; it != m_func.end(); ++it)
					{
						(*it)(order);
					}
				}

				void add(OrderUpdateEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.push_back(handle);
				}

				void insert(OrderUpdateEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.push_back(handle);
				}

				void del()
				{
					//boost_write_lock wlock(m_rwmutex);
					//m_func.erase(i);
				}

				void clear_all()
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.clear();
				}

			private:
				std::list<OrderUpdateEventHandler> m_func;
				boost::shared_mutex m_rwmutex;
			};


			class quote_book_notifier :public SingletonBase<quote_book_notifier>
			{
			public:
				typedef std::function<void(quote *)>QuoteUpdateEventHandler;

				void add_quote_cb(quote* quote)
				{
					boost_read_lock rlock(m_rwmutex);
					auto it = m_func.begin();
					for (; it != m_func.end(); ++it)
					{
						(*it)(quote);
					}

				}

				void update_quote_cb(quote* quote)
				{
					boost_read_lock rlock(m_rwmutex);
					auto it = m_func.begin();
					for (; it != m_func.end(); ++it)
					{
						(*it)(quote);
					}
				}

				void add(QuoteUpdateEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.push_back(handle);
				}

				void insert(QuoteUpdateEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.push_back(handle);
				}

				void del()
				{
					//boost_write_lock wlock(m_rwmutex);
					//m_func.erase(i);
				}

				void clear_all()
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.clear();
				}

			private:
				std::list<QuoteUpdateEventHandler> m_func;
				boost::shared_mutex m_rwmutex;
			};
		}
	}
}


#endif