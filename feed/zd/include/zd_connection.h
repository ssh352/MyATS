#ifndef __ZD_FEED_V2_H__
#define __ZD_FEED_V2_H__
#include "zd_source.h"
#include "ShZdFutureMarketApi.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace zd
	{
		class zd_connection : public CSHZdMarketSpi, public feed_connection
		{
		public:
			zd_connection(zd_source* pSource);
			virtual ~zd_connection();

			void init() override;
			void cleanup() override;
			void create() override
			{
				
			}
			bool subscribe_item(feed_item * item) override;
			bool unsubscribe_item(feed_item *  item) override;

			// zd callbacks
			virtual void OnRspError(CTShZdRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);

			//
			virtual void OnFrontLoginConnected();
            //
			virtual void OnHeartBeatWarning(int nTimeLapse);

			virtual void OnRspUserLogin(CTShZdRspUserLoginField *pRspUserLogin, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRspSubMarketData(CTShZdSpecificInstrumentField *pSpecificInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubMarketData(CTShZdSpecificInstrumentField *pSpecificInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRtnDepthMarketData(CTShZdDepthMarketDataField* pDepthMarketData);
#if 0			
			virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);
#endif
			virtual void Process();


		private:
			bool request_login();


		private:
			zd_source* m_pSource;

			CSHZdMarketApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}
#endif // __ZD_CONNECTION_V2_H__

