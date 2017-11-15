#ifndef _ORDER_DATA_COLLECTION_V2_H_
#define _ORDER_DATA_COLLECTION_V2_H_
#pragma once

#include "exec_book_notifier.h"
#include "order.h"
#include "quote.h"
#include "LockFreeWorkQueue.h"

using namespace terra::marketaccess::orderpassing;
using namespace std;

namespace terra
{
	namespace atsserver
	{
		class order_data_collection
		{
		public:
			order_data_collection();
			~order_data_collection();

			void collect_add_order(order *);
			void collect_add_quote(quote *);
			void collect_exe_order(exec *);
			terra::common::LockFreeWorkQueue<order>*get_add_order();
			terra::common::LockFreeWorkQueue<exec>* get_exe_order();
			terra::common::LockFreeWorkQueue<quote>*get_add_quote();
		public:
			void init(){}
			terra::common::LockFreeWorkQueue<order> m_add_order;
			terra::common::LockFreeWorkQueue<quote> m_add_quote;
			terra::common::LockFreeWorkQueue<exec> m_exe_order;

		};
	}
}
#endif //_ORDER_DATA_COLLECTION_V2_H_


