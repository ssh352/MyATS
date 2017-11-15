#ifndef __SL_SOURCE_V2_H__
#define __SL_SOURCE_V2_H__
#include "feedsource.h"
#include "EESQuoteApi.h"
using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace sl
	{
		typedef terra::common::lockfree_classpool_workqueue<EESMarketDepthQuoteData> outbound_queue;

		class sl_source : public feed_source
		{
		public:
			sl_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & upd, string pub = "", string url = "", string req_url = "");
			//virtual ~sl_source();
		public:
			virtual void init_source();			
			void process_msg(EESMarketDepthQuoteData* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			bool isUdp() { return m_sUdp == "UDP"; }
			//
			void start_receiver();
			//
		protected:
			virtual void process();			
			void process_msg(EESMarketDepthQuoteData* pMsg, feed_item * feed_item);
		protected:			
			std::string m_sUdp;
			outbound_queue m_queue;
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
		};
	}
}
#endif //__SL_source_V2_H__

