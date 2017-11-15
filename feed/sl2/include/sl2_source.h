#ifndef __SL2_SOURCE_V2_H__
#define __SL2_SOURCE_V2_H__
#include "feedsource.h"
extern "C"
{
#include "efh_sf_api.h"
}
using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace sl2
	{
		typedef terra::common::lockfree_classpool_workqueue<guava_udp_normal> outbound_queue;
		class sl2_source : public feed_source
		{
		public:
			sl2_source(const string & sourceName, const string & hostname, const string & service, const string & db, const string & ethName, string pub = "", string url = "", string req_url = "");
			virtual ~sl2_source();
		public:
			virtual void init_source();
			void release_source() override;
			void process_msg(guava_udp_normal* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			bool setUDPSockect(const char * pBroadcastIP, int nBroadcastPort);
			virtual bool subscribe(feed_item * feed_item);
			//
			void start_receiver();
			//
		protected:
			void process() override;			
		protected:
			string         m_strEthName;
			struct sl_efh_quote* m_pSlEfh=nullptr;
			outbound_queue m_queue;
			void process_msg(guava_udp_normal* pMsg, feed_item * feed_item);
#ifdef Linux
                        int efd;
                        void  init_epoll_eventfd();
#endif
		};
	}
}
#endif //__SL2_SOURCE_V2_H__

