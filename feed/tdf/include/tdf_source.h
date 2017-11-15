#ifndef __TDF_SOURCE_H__
#define __TDF_SOURCE_H__

#include <string>
#include "TDFAPIStruct.h"
#include "feedsource.h"



using namespace terra::common;
using namespace terra::feedcommon;

namespace feed
{
	namespace tdf
	{
		//typedef terra::common::SingleLockFreeWorkQueue<TDF_MSG> outbound_queue;
		

		class tdf_source : public feed_source
		{
		protected:
			void process() override;
			//int process_out_bound_msg_handler() override;
		public:
			tdf_source(const string & sourceName, const string &  hostname, const string &  service, const string &  user, const string &  password, const string &  dbName, int date = 0, int time = 0);
			//virtual ~tdf_source();




			virtual void init_source();
			//void release() override;
			//void process_msg(TDF_MSG* pMsg);
			//inline outbound_queue* get_queue() { return &m_queue; }

			virtual bool subscribe(feed_item * feed_item);
			terra::common::lockfree_classpool_workqueue<TDF_MARKET_DATA> market_data_queue;
			terra::common::lockfree_classpool_workqueue<TDF_INDEX_DATA> index_queue;
			terra::common::lockfree_classpool_workqueue<TDF_TRANSACTION> transaction_queue;
			terra::common::lockfree_classpool_workqueue<TDF_ORDER_QUEUE> orderqueue_queue;
			terra::common::lockfree_classpool_workqueue<TDF_FUTURE_DATA> future_data_queue;
			terra::common::lockfree_classpool_workqueue<TDF_ORDER> order_queue;

		protected:
			
			//outbound_queue    m_queue;
	
			//virtual ifeed_item* find_item(const char* name);

			//std::thread m_thread;
			//boost::asio::io_service io;
			//void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			//void set_kernel_timer_thread();
			void process_market_data(TDF_MARKET_DATA* pMsg);
			void process_index(TDF_INDEX_DATA* pMsg);
			void process_transaction(TDF_TRANSACTION* pMsg);
			void process_orderqueue(TDF_ORDER_QUEUE* pMsg);
			void process_future(TDF_FUTURE_DATA* pMsg);
			void process_order(TDF_ORDER* pMsg);




			void process_msg(TDF_MARKET_DATA* pMsg, feed_item* pItem);
			void process_msg(TDF_INDEX_DATA* pMsg, feed_item* pItem);
			void process_msg(TDF_TRANSACTION* pMsg, feed_item* pItem);
			void process_msg(TDF_ORDER_QUEUE* pMsg, feed_item* pItem);
			void process_msg(TDF_FUTURE_DATA* pMsg, feed_item* pItem);
			const double m_factor = 10000.0;
			const int TDF_FEED_MAX_DEPTH = 10;
			unsigned int m_date;
			unsigned int m_time;
		};
	}
}

#endif