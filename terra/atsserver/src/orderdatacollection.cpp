#include "orderdatacollection.h"
#include "boost/bind.hpp"
#include <order_book_notifier.h>

namespace terra
{
	namespace atsserver
	{
		order_data_collection::order_data_collection()
		{
			order_book_notifier::get_instance().add(boost::bind(&order_data_collection::collect_add_order, this, _1));
			exec_book_notifier::get_instance().add(boost::bind(&order_data_collection::collect_exe_order, this, _1));

			quote_book_notifier::get_instance().add(boost::bind(&order_data_collection::collect_add_quote, this, _1));
		}		
		order_data_collection::~order_data_collection()
		{

		}

		void order_data_collection::collect_add_order(order *order_ptr)
		{
			//readLock rlock(m_rwmutex_add);
			//ptime time_point = order_ptr->get_last_time();
			//ptime tnow = date_time_publisher_gh::get_instance()->now();
			//boost::posix_time::time_duration ts(boost::posix_time::minutes(10));
			//auto status = order_ptr->get_status();

			//if ((tnow - time_point) > ts && (status == OrderStatus::Cancel || status == OrderStatus::Exec || status == OrderStatus::Reject))
			//	return;

			m_add_order.Push(order_ptr);
		}
		void order_data_collection::collect_add_quote(quote *order_ptr)
		{
			//readLock rlock(m_rwmutex_add);
			//ptime time_point = order_ptr->get_last_time();
			//ptime tnow = date_time_publisher_gh::get_instance()->now();
			//boost::posix_time::time_duration ts(boost::posix_time::minutes(10));
			//auto status = order_ptr->get_status();

			//if ((tnow - time_point) > ts && (status == OrderStatus::Cancel || status == OrderStatus::Exec || status == OrderStatus::Reject))
			//	return;

			m_add_quote.Push(order_ptr);
		}

		void order_data_collection::collect_exe_order(exec *order_ptr)
		{
			//readLock rlock(m_rwmutex_exe);
			m_exe_order.Push(order_ptr);
		}

		terra::common::LockFreeWorkQueue<order>* order_data_collection::get_add_order()
		{
			return &m_add_order;
		}
		terra::common::LockFreeWorkQueue<quote>* order_data_collection::get_add_quote()
		{
			return &m_add_quote;
		}

		terra::common::LockFreeWorkQueue<exec>* order_data_collection::get_exe_order()
		{
			return &m_exe_order;
		}
	}
}