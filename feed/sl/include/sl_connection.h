#ifndef __SL_FEED_V2_H__
#define __SL_FEED_V2_H__
#include "sl_source.h"
#include "EESQuoteApi.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace sl
	{
		class sl_connection : /*public RTThread, */public EESQuoteEvent, public feed_connection
		{
		public:
			sl_connection(sl_source* pSource);
			virtual ~sl_connection();

			virtual void init();
			virtual void cleanup();
			virtual void create(){}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			// lts callbacks		 
#if 0
			virtual void OnRspError(_LTS_::CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);
			virtual void OnHeartBeatWarning(int nTimeLapse);
			virtual void OnRspUserLogin(_LTS_::CSecurityFtdcUserLoginField* pRspUserLogin, _LTS_::CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspSubL2MarketData(_LTS_::CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, _LTS_::CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubL2MarketData(_LTS_::CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, _LTS_::CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRtnL2MarketData(_LTS_::CSecurityFtdcL2MarketDataField *pL2MarketData);
			virtual void Process();
#else
			/// \brief 当服务器连接成功，登录前调用, 如果是组播模式不会发生, 只需判断InitMulticast返回值即可
			virtual void OnEqsConnected();

			/// \brief 当服务器曾经连接成功，被断开时调用，组播模式不会发生该事件
			virtual void OnEqsDisconnected();

			/// \brief 当登录成功或者失败时调用，组播模式不会发生
			/// \param bSuccess 登陆是否成功标志  
			/// \param pReason  登陆失败原因  
			virtual void OnLoginResponse(bool bSuccess, const char* pReason);

			/// \brief 收到行情时调用,具体格式根据instrument_type不同而不同
			/// \param chInstrumentType  EES行情类型
			/// \param pDepthQuoteData   EES统一行情指针  
			virtual void OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* pDepthQuoteData);

			/// \brief 日志接口，让使用者帮助写日志。
			/// \param nlevel    日志级别
			/// \param pLogText  日志内容
			/// \param nLogLen   日志长度
			virtual void OnWriteTextLog(EesEqsLogLevel nlevel, const char* pLogText, int nLogLen);

			/// \brief 注册symbol响应消息来时调用，组播模式不支持行情注册
			/// \param chInstrumentType  EES行情类型
			/// \param pSymbol           合约名称
			/// \param bSuccess          注册是否成功标志
			virtual void OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);

			/// \brief  注销symbol响应消息来时调用，组播模式不支持行情注册
			/// \param chInstrumentType  EES行情类型
			/// \param pSymbol           合约名称
			/// \param bSuccess          注册是否成功标志
			virtual void OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);

			/// \brief 查询symbol列表响应消息来时调用，组播模式不支持合约列表查询
			/// \param chInstrumentType  EES行情类型
			/// \param pSymbol           合约名称
			/// \param bLast             最后一条查询合约列表消息的标识
			/// \remark 查询合约列表响应, last = true时，本条数据是无效数据。
			virtual void OnSymbolListResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bLast);

		private:
			bool request_login();
#endif
		private:
			sl_source*   m_pSource;
			EESQuoteApi* m_pUserApi;
			int m_nRequestId;
		};
	}
}
#endif // __sl_feed_V2_H__

