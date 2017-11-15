#ifndef __LTSL2_SOURCE_V2_H__
#define __LTSL2_SOURCE_V2_H__
#include "LTS_ns.h"

#include "ltsl2_decoder.h"
#include "SecurityFtdcL2MDUserApiStruct.h"
#include "feedsource.h"

using namespace terra::common;
using namespace terra::feedcommon;
_USING_LTS_NS_
namespace feed
{
	namespace ltsl2
	{
		typedef terra::common::lockfree_classpool_workqueue<CSecurityFtdcL2MarketDataField> outbound_queue;

		class ltsl2_source : public feed_source
		{
		public:
			ltsl2_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & upd, string pub = "", string url = "", string req_url = "");
			//virtual ~ltsl2_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_msg(CSecurityFtdcL2MarketDataField* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			bool isUdp() { return m_sUdp == "UDP"; }
			//
			void start_receiver();
			//
		protected:
			virtual void process();
			//virtual int  process_out_bound_msg_handler();
			void process_msg(CSecurityFtdcL2MarketDataField* pMsg, feed_item * feed_item);
		protected:
			//ltsl2_decoder     m_decoder;
			std::string m_sUdp;
			outbound_queue m_queue;
		};
	}
}
#endif //__ltsl2_source_V2_H__

