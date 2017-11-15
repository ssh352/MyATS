#ifndef __LTS_FEED_V2_H__
#define __LTS_FEED_V2_H__
#include "lts_source.h"
#include "SecurityFtdcMdApi.h"
namespace feed
{
	namespace lts
	{
		class lts_connection : public CSecurityFtdcMdSpi, public feed_connection
		{
		public:
			lts_connection(lts_source* pSource);
			virtual ~lts_connection();

			virtual void init();
			virtual void cleanup();
			virtual void create()
			{
				
			}
			virtual bool subscribe_item(feed_item* item) override;
			virtual bool unsubscribe_item(feed_item* item) override;

			// lts callbacks
			virtual void OnRspError(CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);

			virtual void OnHeartBeatWarning(int nTimeLapse);

			virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField* pRspUserLogin, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField* pDepthMarketData);
			//


			virtual void Process();


		private:
			bool request_login();

		private:
			lts_source* m_pSource;

			CSecurityFtdcMdApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}
#endif // __LTS_FEED_V2_H__

