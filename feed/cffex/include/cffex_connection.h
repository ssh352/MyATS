#ifndef __CFFEX_FEED_V2_H__
#define __CFFEX_FEED_V2_H__
#include "cffex_source.h"
#include "ThostFtdcMdApi.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace cffex
	{
		class cffex_connection : public CThostFtdcMdSpi, public feed_connection
		{
		public:
			cffex_connection(cffex_source* pSource);
			virtual ~cffex_connection();

			void init() override;
			void cleanup() override;
			void create() override
			{
				
			}
			bool subscribe_item(feed_item * item) override;
			bool unsubscribe_item(feed_item *  item) override;

			// cffex callbacks
			virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);

			virtual void OnHeartBeatWarning(int nTimeLapse);

			virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);
			//
			virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

			virtual void Process();


		private:
			bool request_login();


		private:
			cffex_source* m_pSource;

			CThostFtdcMdApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}
#endif // __CFFEX_CONNECTION_V2_H__

