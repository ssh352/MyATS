#ifndef __CFFEX_API_H__
#define __CFFEX_API_H__

#ifdef _WIN32
#include <WinSock2.h>
#endif


#include "ThostFtdcTraderApi.h"
#include <map>

#include <thread>
#include "LockFreeWorkQueue.h"
#include "lockfree_classpool_workqueue.h"
#include "boost/date_time/posix_time/posix_time.hpp"
namespace cffex
{
	class cffex_connection;

	class cffex_api : /*public RTThread,*/ public CThostFtdcTraderSpi
	{
	public:
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcInputOrderField> cffex_input_inbound_queue;
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcInputQuoteField> cffex_input_quote_inbound_queue;
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcOrderField> cffex_order_inbound_queue;
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcQuoteField> cffex_quote_inbound_queue;
		typedef terra::common::lockfree_classpool_workqueue<CThostFtdcTradeField> cffex_trade_inbound_queue;

		typedef terra::common::LockFreeWorkQueue<int> cffex_input_action_inbound_queue;
		typedef terra::common::LockFreeWorkQueue<int> cffex_input_action_quote_inbound_queue;

	public:
		cffex_api(cffex_connection* pConnection);
		virtual ~cffex_api();


		void init();
		void release();

		bool connect();
		bool disconnect();


		bool ReqOrderInsert(CThostFtdcInputOrderField* pOrder);
		bool ReqOrderAction(CThostFtdcInputOrderActionField* pOrder);
		bool ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition);
		bool ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount);

		bool ReqExecOrderInsert(CThostFtdcInputExecOrderField* pRequest);
		bool ReqExecOrderAction(CThostFtdcInputExecOrderActionField *pRequest);

		bool ReqQuoteInsert(CThostFtdcInputQuoteField * request);
		bool ReqQuoteAction(CThostFtdcInputQuoteActionField * request);

		//void OnRspOrderInsertAsync(CThostFtdcInputOrderField* pInput);
		//void OnRtnOrderAsync(CThostFtdcOrderField* pOrder);
		//void OnRtnTradeAsync(CThostFtdcTradeField* pTrade);
		//void OnRspOrderActionAsync(int* nRequest);



		// cffex callbacks
		virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

		virtual void OnFrontConnected();
		virtual void OnFrontDisconnected(int nReason);

		virtual void OnHeartBeatWarning(int nTimeLapse);

		virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

		virtual void OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder);
		virtual void OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		virtual void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo);
		virtual void OnErrRtnOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo);

		//for futures only
		virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);
		virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);

		virtual void OnRtnQuote(CThostFtdcQuoteField* pQuote);
		virtual void OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		virtual void OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

		inline cffex_input_inbound_queue* get_input_queue() { return &m_inputQueue; }
		inline cffex_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
		inline cffex_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }
		inline cffex_input_action_inbound_queue* get_input_action_queue() { return &m_inputActionQueue; }

		inline cffex_input_quote_inbound_queue* get_input_quote_queue() { return &m_inputQuoteQueue; }
		inline cffex_quote_inbound_queue* get_order_quote_queue() { return &m_quoteQueue; }
		inline cffex_input_action_quote_inbound_queue* get_input_action_quote_queue() { return &m_inputActionQuoteQueue; }

		void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		void request_instruments();

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
	protected:
		std::thread m_thread;
		std::string m_sName;

		std::string GetName(){ return m_sName; }
		inline bool is_alive() { return m_isAlive; }
		inline void is_alive(bool b) { m_isAlive = b; }


		void request_login();
		int get_ord_ref_from_reqid(int nReqId);
		void request_ack_order();
		void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	protected:
		cffex_connection* m_pConnection;

		CThostFtdcTraderApi* m_pUserApi;
		bool m_connectionStatus;

		bool m_isAlive;

		int m_nRequestId;
		int m_nCurrentOrderRef;

		cffex_input_inbound_queue m_inputQueue;
		cffex_input_quote_inbound_queue m_inputQuoteQueue;
		cffex_order_inbound_queue m_orderQueue;
		cffex_quote_inbound_queue m_quoteQueue;
		cffex_trade_inbound_queue m_tradeQueue;
		cffex_input_action_inbound_queue m_inputActionQueue;
		cffex_input_action_quote_inbound_queue m_inputActionQuoteQueue;
		std::map<int, int> m_ordInputActiondRefMap; // contains <nRequestId,nOrderid>
		std::map<std::string, std::string> unknown_pos;



	};
}

#endif // __CFFEX_API_H__
