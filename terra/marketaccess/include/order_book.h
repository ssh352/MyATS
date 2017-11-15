#ifndef TERRA_ORDER_BOOK_H__
#define TERRA_ORDER_BOOK_H__
/*
#include "IOrderBook.h"
#include <unordered_map>
#include "order.h"
#include "exec.h"
#include "AtsType_types.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
typedef boost::shared_lock<boost::shared_mutex> readLock;
typedef boost::unique_lock<boost::shared_mutex> writeLock;

namespace Terra
{
	namespace MarketAccess
	{
		namespace OrderPassing
		{
			class order_book:public IOrderBook
			{
			public:
				order_book(){}
				virtual ~order_book(){}
				virtual bool Add(order * order);
				virtual bool Remove(order * order);
				virtual bool Contains(order * order);

				virtual order * GetById(int id);

				virtual int Count();

				virtual int GetNbOpenOrders();

				virtual void KillAll();
				virtual void KillAll(std::string portfolio);
				virtual void KillAll(std::string portfolio, AtsType::TradingType::type Type);
				void Clear();
				void setOrderRecord(order *order)
				{
					m_order = order;
				}
				order *m_order;
			private:
				std::unordered_map<int, order *>m_container;
				boost::shared_mutex m_rwmutex;
			};

		}
	}
}
*/

#endif