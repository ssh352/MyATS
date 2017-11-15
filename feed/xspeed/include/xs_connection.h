#ifndef __XS_FEED_V2_H__
#define __XS_FEED_V2_H__
/*#include "RTThread.h"*/
#include "xs_source.h"
#include "DFITCSECMdApi.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace xs
	{
		class xs_connection : public DFITCSECMdSpi, public feed_connection
		{
		public:
			xs_connection(xs_source* pSource);
			virtual ~xs_connection();

			void init();
			void cleanup();
			virtual void create()
			{				
			}
			bool subscribe_item(feed_item * item) override;
			bool unsubscribe_item(feed_item* item) override;

			/*bool subscribe_item(const char* item, AtsType::InstrType::type instype);
			bool unsubscribe_item(const char* item, AtsType::InstrType::type instype);*/

			// cffex callbacks

			virtual void OnRspError(struct DFITCSECRspInfoField *pRspInfo);
			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);
			virtual void OnHeartBeatWarning(int nTimeLapse);


			virtual void OnRspSOPUserLogin(struct DFITCSECRspUserLoginField * pRspUserLogin, struct DFITCSECRspInfoField * pRspInfo);
			virtual void OnRspSOPSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo);
			virtual void OnRspSOPUnSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo);

			virtual void OnRspStockUserLogin(struct DFITCSECRspUserLoginField * pRspUserLogin, struct DFITCSECRspInfoField * pRspInfo);
			virtual void OnRspStockSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo);
			virtual void OnRspStockUnSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo);

			virtual void OnSOPMarketData(struct DFITCSOPDepthMarketDataField * pMarketDataField);

			virtual void OnStockMarketData(struct DFITCStockDepthMarketDataField * pMarketDataField);

			virtual void Process();


		private:
			bool request_op_login();
			bool request_stock_login();


		private:
			xs_source* m_pSource;

			DFITCSECMdApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}
#endif // __xs_feed_V2_H__

