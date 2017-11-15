#ifndef __xsl2_SOURCE_V2_H__
#define __xsl2_SOURCE_V2_H__
#include "feedsource.h"
#include "DFITCL2Api.h"
using namespace terra::common;
using namespace terra::feedcommon;
using namespace DFITC_L2;
namespace feed
{
	namespace xsl2
	{
		typedef terra::common::lockfree_classpool_workqueue<MDBestAndDeep> outbound_5_queue;
		typedef terra::common::lockfree_classpool_workqueue<MDTenEntrust>  outbound_10_queue;
		class xsl2_source : public feed_source
		{
		public:
			xsl2_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & upd);			
		public:
			virtual void init_source();			
			void process_5_msg(MDBestAndDeep* pMsg);
			void process_10_msg(MDTenEntrust* pMsg);
			inline outbound_5_queue* get_5_queue() { return &m_5_queue; }
			inline outbound_10_queue* get_10_queue() { return &m_10_queue; }
			bool isUdp() { return m_sUdp == "UDP"; }
		protected:
			virtual void process();			
			void process_5_msg(MDBestAndDeep* pMsg, feed_item * feed_item);
			void process_10_msg(MDTenEntrust* pMsg, feed_item * feed_item);
		protected:			
			std::string       m_sUdp;
			outbound_5_queue  m_5_queue;
			outbound_10_queue m_10_queue;
		};
	}
}
#endif //__xsl2_source_V2_H__

