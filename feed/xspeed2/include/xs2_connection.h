#ifndef __xs2_connection_H__
#define __xs2_connection_H__
#include "xs2_source.h"
#include "DFITCMdApi.h"
using namespace DFITCXSPEEDMDAPI;
namespace feed
{
	namespace xs2
	{
		class xs2_connection :public DFITCMdSpi,public feed_connection
		{
		public:
			xs2_connection(xs2_source* pSource);
			virtual ~xs2_connection();

			void init() override;
			void cleanup() override;
			void create() override{};
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			virtual void OnFrontConnected();

			virtual void OnFrontDisconnected(int nReason);

			virtual void OnHeartBeatWarning(int nTimeLapse);

			virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField *pRspUserLogin, struct DFITCErrorRtnField *pRspInfo);

			virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField *pRspUsrLogout, struct DFITCErrorRtnField *pRspInfo) {}

			virtual void OnRspError(struct DFITCErrorRtnField *pRspInfo);

			virtual void OnRspSubMarketData(struct DFITCSpecificInstrumentField *pSpecificInstrument, struct DFITCErrorRtnField *pRspInfo);

			virtual void OnRspUnSubMarketData(struct DFITCSpecificInstrumentField *pSpecificInstrument, struct DFITCErrorRtnField *pRspInfo);

			virtual void OnMarketData(struct DFITCDepthMarketDataField *pMarketDataField);

			void OnRspSubForQuoteRsp(struct DFITCSpecificInstrumentField * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo) override;
			void OnRspUnSubForQuoteRsp(struct DFITCSpecificInstrumentField * pSpecificInstrument, struct DFITCErrorRtnField * pRspInfo) override;
			void OnRtnForQuoteRsp(struct DFITCQuoteSubscribeRtnField * pForQuoteField) override;

			void Process();

		private:
			bool request_login();

		private:
			xs2_source* m_pSource;

			DFITCMdApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}

#endif // __xs2_connection_H__

