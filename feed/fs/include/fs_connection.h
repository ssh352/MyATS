#ifndef __FS_FEED_V2_H__
#define __FS_FEED_V2_H__
#include "fs_source.h"
#include "SgitFtdcMdApi.h"
using namespace terra::feedcommon;
using namespace fstech;
namespace feed
{
	namespace fs
	{
		class fs_connection : public CThostFtdcMdSpi, public feed_connection
		{
		public:
			fs_connection(fs_source* pSource);
			virtual ~fs_connection();

			void init() override;
			void cleanup() override;
			void create() override
			{
				
			}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

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


			virtual void Process();


		private:
			bool request_login();


		private:
			fs_source* m_pSource;

			CThostFtdcMdApi* m_pUserApi;

			int m_nRequestId;
		};
	}
}
#endif // __CFFEX_CONNECTION_V2_H__

