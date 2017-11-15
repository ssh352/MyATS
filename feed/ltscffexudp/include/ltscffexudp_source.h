#ifndef __LTSCFFEXUDP_SOURCE_V2_H__
#define __LTSCFFEXUDP_SOURCE_V2_H__

//#include "ltscffexudp_decoder.h"
#include "cffexpdu.h"
#include "feedsource.h"

using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace ltscffexudp
	{
		typedef terra::common::LockFreeWorkQueue<CDepthMarketDataField> outbound_queue;

		class ltscffexudp_source : public feed_source
		{
		public:
			ltscffexudp_source(const string & sourceName, const string & hostname, const string & service, const string & db, string pub = "", string url = "", string req_url = "");
			virtual ~ltscffexudp_source();
		public:
			virtual void init_source();
			void release_source() override;
			void process_depthMarketDataField(CDepthMarketDataField* pMsg);
			// get/set
			inline outbound_queue* get_queue() { return &m_queue; }
			bool setUDPSockect(const char * pBroadcastIP, int nBroadcastPort);
			void listhen2udp();
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
			void process_depthMarketDataField(CDepthMarketDataField* pMsg, feed_item * feed_item);
		protected:
			std::thread    m_tListhen2Udp;
			outbound_queue m_queue;
			int			   m_UDPSockID;
			//ltscffexudp_decoder m_decoder;
		};
	}
}
#endif //__ltscffexudp_SOURCE_V2_H__

