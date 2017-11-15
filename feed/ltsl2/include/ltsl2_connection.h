#ifndef __LTSL2_FEED_V2_H__
#define __LTSL2_FEED_V2_H__
#include "LTS_ns.h"
#include "ltsl2_source.h"
#include "SecurityFtdcL2MDUserApi.h"
#include "SecurityFtdcL2MDUserApiStruct.h"
using namespace terra::feedcommon;
//_USING_LTS_NS_
namespace feed
{
	namespace ltsl2
	{
		class ltsl2_connection : /*public RTThread, */public _LTS_::CSecurityFtdcL2MDUserSpi, public feed_connection
		{
		public:
			ltsl2_connection(ltsl2_source* pSource);
			virtual ~ltsl2_connection();

			void init();
			void cleanup();
			virtual void create()
			{
				/*RTThread::Create();*/
			}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			// lts callbacks		  
			virtual void OnRspError(_LTS_::CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);
			virtual void OnHeartBeatWarning(int nTimeLapse);
			virtual void OnRspUserLogin(_LTS_::CSecurityFtdcUserLoginField* pRspUserLogin, _LTS_::CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

			virtual void OnRspSubL2MarketData(_LTS_::CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, _LTS_::CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubL2MarketData(_LTS_::CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, _LTS_::CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


			virtual void OnRtnL2MarketData(_LTS_::CSecurityFtdcL2MarketDataField *pL2MarketData);
			virtual void Process();

		private:
			bool request_login();

		private:
			ltsl2_source* m_pSource;
			_LTS_::CSecurityFtdcL2MDUserApi* m_pUserApi;
			int m_nRequestId;
		};
	}
}
#endif // __ltsl2_feed_V2_H__

