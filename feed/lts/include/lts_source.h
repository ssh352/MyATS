#ifndef __LTS_SOURCE_V2_H__
#define __LTS_SOURCE_V2_H__
#include "lts_decoder.h"
#include "SecurityFtdcUserApiStruct.h"
#include "feedsource.h"
#ifdef Linux
#include <sys/eventfd.h>
#endif
using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace lts
	{
		typedef terra::common::lockfree_classpool_workqueue<CSecurityFtdcDepthMarketDataField> outbound_queue;

		class lts_source : public feed_source
		{
		public:
			lts_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, string pub = "", string url = "", string req_url = "");
			//virtual ~lts_source();
		public:
			void process_msg(CSecurityFtdcDepthMarketDataField* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
		public:
			virtual void init_source();
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
			void process_msg(CSecurityFtdcDepthMarketDataField* pMsg, feed_item * feed_item);
		protected:
			//lts_decoder     m_decoder;
			outbound_queue  m_queue;
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
		
		};
	}
}
#endif //__lts_SOURCE_V2_H__

