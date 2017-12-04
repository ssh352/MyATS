#ifndef __CFFEX_SOURCE_V2_H__
#define __CFFEX_SOURCE_V2_H__

#include "cffex_decoder.h"
#include "ThostFtdcUserApiStruct.h"
#include "feedsource.h"


using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace cffex
	{
		typedef terra::common::lockfree_classpool_workqueue<feed_item*> outbound_queue;

		class cffex_source : public feed_source
		{
		public:
			cffex_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, const string& is_FQR = "", string pub = "", string url = "", string req_url = "");
			virtual ~cffex_source();
		public:
			virtual void init_source();

			void process_msg(feed_item** pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			bool FQR;
			void receive_FQR(CThostFtdcForQuoteRspField * pForQuoteRsp);
			
			void start_receiver();
			
			void update_item(CThostFtdcDepthMarketDataField* pMsg, feed_item * feed_item);

			void process_sqsc_msg();
			void push_feed_ptr(feed_item * feed);
			tbb::concurrent_queue<feed_item *> m_sqsc_queue;

		protected:
			void process() override;
		protected:
			outbound_queue    m_queue;

			
#ifdef Linux
			int efd;
			bool is_feed_engine_bind_cpu = true;
			void  init_epoll_eventfd();
#endif
			
		};
	}
}
#endif //__CFFEX_SOURCE_V2_H__

