#ifndef __XS_SOURCE_V2_H__
#define __XS_SOURCE_V2_H__
#include <string>
#include <set>
#include "xs_decoder.h"
#include "DFITCSECApiStruct.h"
#include "feedsource.h"
#ifdef Linux
#include <sys/eventfd.h>
#endif

using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace xs
	{
		typedef terra::common::lockfree_classpool_workqueue<DFITCSOPDepthMarketDataField> op_outbound_queue;
		typedef terra::common::lockfree_classpool_workqueue<DFITCStockDepthMarketDataField> stock_outbound_queue;

		class xs_source : public feed_source
		{
		public:
			xs_source(const string &sourceName, const string &hostname, const string &service, const string &user, const string &password, const string &db, string pub = "", string url = "", string req_url = "");
			//virtual ~xs_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_sop_msg(DFITCSOPDepthMarketDataField* pMsg);
			void process_stock_msg(DFITCStockDepthMarketDataField*  pMsg);
			//virtual bool subscribe(feed_item * feed_item);
			//virtual bool unSubscribe(feed_item * feed_item);
			// get/set
			inline op_outbound_queue* get_op_queue() { return &m_op_queue; }
			inline stock_outbound_queue* get_stock_queue() { return &m_stock_queue; }
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
			//int          process_stock_out_bound_msg_handler();
			void process_msg(DFITCSOPDepthMarketDataField* pMsg, feed_item * feed_item);
			void process_msg(DFITCStockDepthMarketDataField* pMsg, feed_item * feed_item);
		protected:
			op_outbound_queue     m_op_queue;
			stock_outbound_queue  m_stock_queue;
			
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif

		};
	}
}
#endif //__xs_source_V2_H__

