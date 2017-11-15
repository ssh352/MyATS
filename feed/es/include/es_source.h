#ifndef __ES_SOURCE_V2_H__
#define __ES_SOURCE_V2_H__
#include "feedsource.h"
#include "TapQuoteAPI.h"
#ifdef Linux
#include <sys/eventfd.h>
#endif
using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace es
	{
		typedef terra::common::lockfree_classpool_workqueue<TapAPIQuoteWhole> outbound_queue;

		class es_source : public feed_source
		{
		public:
			es_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & authCode, string pub = "", string url = "", string req_url = "");
		public:
			virtual void init_source();			
			void process_msg(TapAPIQuoteWhole* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }		
			string get_auth_code(){ return m_strAuthCode; }
			//
			void start_receiver();
			//
		protected:
			virtual void process();			
			void process_msg(TapAPIQuoteWhole* pMsg, feed_item * feed_item);
		protected:						
			outbound_queue m_queue;
			string m_strAuthCode;
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
		};
	}
}
#endif //__ES_SOURCE_V2_H__

