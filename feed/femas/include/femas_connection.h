#ifndef __FEMAS_FEED_V2_H__
#define __FEMAS_FEED_V2_H__


#include "femas_source.h"
#include "USTPFtdcMduserApi.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace femas
	{
		class femas_connection : public CUstpFtdcMduserSpi, public feed_connection
		{
		public:
			femas_connection(femas_source* pSource);
			virtual ~femas_connection();

			void init() override;
			void cleanup() override;
			void create() override
			{				
			}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			// femas callbacks
			virtual void OnRspError(CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);

			virtual void OnHeartBeatWarning(int nTimeLapse);

			virtual void OnRspUserLogin(CUstpFtdcRspUserLoginField* pRspUserLogin, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRspSubMarketData(CUstpFtdcSpecificInstrumentField* pSpecificInstrument, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubMarketData(CUstpFtdcSpecificInstrumentField* pSpecificInstrument, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField* pDepthMarketData);
			//


			virtual void Process();


		private:
			bool request_login();


		private:
			femas_source* m_pSource;

			CUstpFtdcMduserApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}
#endif // __FEMAS_FEED_V2_H__

