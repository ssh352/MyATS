#ifndef __xsl2_FEED_V2_H__
#define __xsl2_FEED_V2_H__
#include "xsl2_source.h"
#include "DFITCL2Api.h"
using namespace terra::feedcommon;
using namespace DFITC_L2;
namespace feed
{
	namespace xsl2
	{
		class xsl2_connection : public DFITCL2Spi, public feed_connection
		{
		public:
			xsl2_connection(xsl2_source* pSource);
			virtual ~xsl2_connection();

			void init();
			void cleanup();
			virtual void create()
			{				
			}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			virtual void OnConnected();
			virtual void OnDisconnected(int pReason);
			virtual void OnRspUserLogin(struct ErrorRtnField * pErrorField);
			virtual void OnRspUserLogout(struct ErrorRtnField * pErrorField);
			virtual void OnRspSubscribeMarketData(struct ErrorRtnField * pErrorField);
			virtual void OnRspUnSubscribeMarketData(ErrorRtnField * pErrorField);
			virtual void OnRspSubscribeAll(struct ErrorRtnField * pErrorField);
			virtual void OnRspUnSubscribeAll(struct ErrorRtnField * pErrorField);
			virtual void OnBestAndDeep(MDBestAndDeep * const pQuote);
			virtual void OnTenEntrust(MDTenEntrust * const pQuote);
			virtual void OnHeartBeatLost();

			virtual void Process();
		private:
			bool request_login();
		private:
			xsl2_source* m_pSource;
			DFITCL2Api* m_pUserApi;
			int m_nRequestId;
		};
	}
}
#endif // __xsl2_feed_V2_H__

