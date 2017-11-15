#ifndef __ZD_API_H__
#define __ZD_API_H__

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "ShZdFutureTradeApi.h"
#include <map>
#include <string>

#include <thread>
#include "LockFreeWorkQueue.h"
#include "lockfree_classpool_workqueue.h"
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
namespace zd
{	
	class zd_connection;

	class zd_api : public CSHZdTraderSpi
	{
	public:
		typedef terra::common::lockfree_classpool_workqueue<CTShZdInputOrderField> cffex_input_inbound_queue;
#if 0
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcInputQuoteField> cffex_input_quote_inbound_queue;
#endif
		typedef terra::common::lockfree_classpool_workqueue<CTShZdOrderField> cffex_order_inbound_queue;
#if 0
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcQuoteField> cffex_quote_inbound_queue;
#endif
		typedef terra::common::lockfree_classpool_workqueue<CTShZdTradeField> cffex_trade_inbound_queue;

		typedef terra::common::LockFreeWorkQueue<int> cffex_input_action_inbound_queue;
		//typedef terra::common::LockFreeWorkQueue<int> cffex_input_action_quote_inbound_queue;

	public:
		zd_api(zd_connection* pConnection);
		virtual ~zd_api();


		void init();
		void release();

		bool connect();
		bool disconnect();


		bool ReqOrderInsert(CTShZdInputOrderField* pOrder);
		bool ReqOrderAction(CTShZdOrderActionField* pOrder);
		
		bool ReqQryInvestorPosition();

		bool ReqQryTradingAccount(CTShZdQryTradingAccountField *pQryTradingAccount);

#if 0
		bool ReqExecOrderInsert(CThostFtdcInputExecOrderField* pRequest);
		bool ReqExecOrderAction(CThostFtdcInputExecOrderActionField *pRequest);
		bool ReqQuoteInsert(CThostFtdcInputQuoteField * request);
		bool ReqQuoteAction(CThostFtdcInputQuoteActionField * request);
#endif
		//void OnRspOrderInsertAsync(CTShZdInputOrderField* pInput);
		//void OnRtnOrderAsync(CThostFtdcOrderField* pOrder);
		//void OnRtnTradeAsync(CThostFtdcTradeField* pTrade);
		//void OnRspOrderActionAsync(int* nRequest);



		// cffex callbacks
		virtual void OnRspError(CTShZdRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

		virtual void OnFrontConnected();
		virtual void OnFrontDisconnected(int nReason);

		virtual void OnHeartBeatWarning(int nTimeLapse);

		virtual void OnRspUserLogin(CTShZdRspUserLoginField* pRspUserLogin, CTShZdRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspUserLogout(CTShZdUserLogoutField *pUserLogout, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
#if 0
		virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
#endif
		virtual void OnRspOrderInsert(CTShZdInputOrderField *pInputOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
#if 0
		virtual void OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder);
		virtual void OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
#endif
		virtual void OnRspOrderAction(CTShZdInputOrderActionField *pInputOrderAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnErrRtnOrderInsert(CTShZdInputOrderField* pInputOrder, CTShZdRspInfoField* pRspInfo);
		//virtual void OnErrRtnOrderAction(CTShZdInputOrderActionField* pInputOrderAction, CTShZdRspInfoField* pRspInfo);

		//for futures only
		virtual void OnRspQryInvestorPosition(CTShZdInvestorPositionField *pInvestorPosition, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspQryTradingAccount(CTShZdTradingAccountField *pTradingAccount, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		virtual void OnRtnOrder(CTShZdOrderField* pOrder);
		virtual void OnRtnTrade(CTShZdTradeField* pTrade);
#if 0
		virtual void OnRtnQuote(CThostFtdcQuoteField* pQuote);
		virtual void OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
#endif
		inline cffex_input_inbound_queue* get_input_queue() { return &m_inputQueue; }
		inline cffex_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
		inline cffex_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }
		inline cffex_input_action_inbound_queue* get_input_action_queue() { return &m_inputActionQueue; }
#if 0
		inline cffex_input_quote_inbound_queue* get_input_quote_queue() { return &m_inputQuoteQueue; }
		inline cffex_quote_inbound_queue* get_order_quote_queue() { return &m_quoteQueue; }
		inline cffex_input_action_quote_inbound_queue* get_input_action_quote_queue() { return &m_inputActionQuoteQueue; }
#endif
		void OnRspQryInstrument(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);		
		void request_instruments();
		void OnRspQryExchange(CTShZdExchangeField *pExchange, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		void request_investor_full_positions();

		//int process_inbound_input_cb();
		//int process_inbound_input_action_cb();
		//int process_inbound_order_cb();
		//int process_inbound_trade_cb();
		void Process();
		/*boost::posix_time::time_duration CZCE_Time_dur = boost::posix_time::seconds(0);
		boost::posix_time::time_duration DCE_Time_dur = boost::posix_time::seconds(0);
		boost::posix_time::time_duration CFFEX_Time_dur = boost::posix_time::seconds(0);

		boost::posix_time::time_duration SHFE_Time_dur = boost::posix_time::seconds(0);*/

		int m_begin_Id = 0;
		int get_request_id(){ return m_nRequestId; }
		std::map<std::string, int> & getOrdInputActiondRefMap(){ return m_ordInputActiondRefMap; }
	protected:
		std::thread m_thread;
		std::string m_sName;

		std::string GetName(){ return m_sName; }
		inline bool is_alive() { return m_isAlive; }
		inline void is_alive(bool b) { m_isAlive = b; }


		void request_login();
		int get_ord_ref_from_reqid(string orderRef);
		void request_ack_order();
		void OnRspQryOrder(CTShZdOrderField *pOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		void OnRspQryTrade(CTShZdTradeField *pTrade, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	protected:		
#if 0
		CThostFtdcTraderApi* m_pUserApi;
		cffex_connection* m_pConnection;
#else
		CSHZdTraderApi*   m_pUserApi;
		zd_connection*    m_pConnection;
#endif
		bool m_connectionStatus;

		bool m_isAlive;

		int m_nRequestId;
		int m_nCurrentOrderRef;

		bool m_bQryOrder = false;
		bool m_bQryTrade = false;

		cffex_input_inbound_queue m_inputQueue;
#if 0
		cffex_input_quote_inbound_queue m_inputQuoteQueue;
#endif
		cffex_order_inbound_queue m_orderQueue;
#if 0
		cffex_quote_inbound_queue m_quoteQueue;
#endif
		cffex_trade_inbound_queue m_tradeQueue;
		cffex_input_action_inbound_queue m_inputActionQueue;
#if 0
		cffex_input_action_quote_inbound_queue m_inputActionQuoteQueue;
#endif
		std::map<std::string, int> m_ordInputActiondRefMap; // contains <orderRef,nOrderid>				
	};
}
#endif // __ZD_API_H__
