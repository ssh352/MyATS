#ifndef __LTS_TRDAPI_H__
#define __LTS_TRDAPI_H__

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "SecurityFtdcUserApiStruct.h"
#include "SecurityFtdcUserApiDataType.h"
#include "SecurityFtdcTraderApi.h"
//#include "LTSSecWrapper.h"

#include "lockfree_classpool_workqueue.h"

namespace lts
{
   class lts_connection;
   class lts_trdapi :public CSecurityFtdcTraderSpi//CSecurityFtdcTraderSpiBase//CSecurityFtdcTraderSpi
   {

   public:
	   typedef terra::common::lockfree_classpool_workqueue<CSecurityFtdcInputOrderField> lts_input_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<CSecurityFtdcOrderField> lts_order_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<CSecurityFtdcTradeField> lts_trade_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<CSecurityFtdcInputOrderActionField> lts_input_action_inbound_queue;
   
   public:
	   lts_trdapi();
	   lts_trdapi(lts_connection* pConnection);
	   virtual ~lts_trdapi();

   public:
	   void init();
	   void release();
	   bool connect();
	   bool disconnect();
	   bool get_status() { return m_isAlive; }
	   void set_status(bool stat) { m_isAlive = stat; }

	   bool ReqOrderInsert(CSecurityFtdcInputOrderField* pOrder);
	   bool ReqOrderAction(CSecurityFtdcInputOrderActionField* pOrder);
	   bool ReqQryInvestorPosition(CSecurityFtdcQryInvestorPositionField *pQryInvestorPosition);
	   bool ReqQryTradingAccount(CSecurityFtdcQryTradingAccountField *pQryTradingAccount);

	   void OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pInput);
	   void OnRtnOrderAsync(CSecurityFtdcOrderField* pOrder);
	   void OnRtnTradeAsync(CSecurityFtdcTradeField* pTrade);
	   void OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pInputOrderAction);
	   
	    void OnFrontConnected();
	    void OnFrontDisconnected(int nReason);
	    void OnHeartBeatWarning(int nTimeLapse);
	    void OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    virtual void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    void OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	    void OnRspOrderInsert(CSecurityFtdcInputOrderField *pInputOrder, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    void OnRspOrderAction(CSecurityFtdcInputOrderActionField *pInputOrderAction, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    //void OnRspUserPasswordUpdate(CSecurityFtdcUserPasswordUpdateField *pUserPasswordUpdate, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    //void OnRspTradingAccountPasswordUpdate(CSecurityFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    void OnRtnOrder(CSecurityFtdcOrderField *pOrder) ;
	    void OnRtnTrade(CSecurityFtdcTradeField *pTrade) ;
	    void OnErrRtnOrderInsert(CSecurityFtdcInputOrderField *pInputOrder, CSecurityFtdcRspInfoField *pRspInfo) ;
		void OnErrRtnOrderAction(CSecurityFtdcOrderActionField *pOrderAction, CSecurityFtdcRspInfoField *pRspInfo) ;
	    //void OnRspFundOutByLiber(CSecurityFtdcInputFundTransferField *pInputFundTransfer, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
		//void OnRtnFundOutByLiber(CSecurityFtdcFundTransferField *pFundTransfer) ;
	    //void OnErrRtnFundOutByLiber(CSecurityFtdcInputFundTransferField *pInputFundTransfer, CSecurityFtdcRspInfoField *pRspInfo) ;
	    //void OnRtnFundInByBank(CSecurityFtdcFundTransferField *pFundTransfer) ;
	    //void OnRspFundInterTransfer(CSecurityFtdcFundInterTransferField *pFundInterTransfer, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
	    //void OnRtnFundInterTransferSerial(CSecurityFtdcFundInterTransferSerialField *pFundInterTransferSerial) ;
	    //void OnErrRtnFundInterTransfer(CSecurityFtdcFundInterTransferField *pFundInterTransfer, CSecurityFtdcRspInfoField *pRspInfo) ;
	   
	   inline lts_input_inbound_queue* get_input_queue() { return &m_inputQueue; }
	   inline lts_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
	   inline lts_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }
	   inline lts_input_action_inbound_queue* get_input_action_queue() { return &m_inputActionQueue; }
	   void Process();

	   //int process_inbound_input_cb();
	   //int process_inbound_input_action_cb();
	   //int process_inbound_order_cb();
	   //int process_inbound_trade_cb();

   protected:
	   
	   inline bool is_alive() { return m_isAlive; }
	   inline void is_alive(bool b) { m_isAlive = b; }
	   void request_login(CSecurityFtdcAuthRandCodeField *pAuthRandCode);
	   void request_auth();
	   void request_instruments();

   protected:
	   lts_connection* m_pConnection;

	   CSecurityFtdcTraderApi* m_pTraderApi;

	   //bool m_connectionStatus;

	   bool m_isAlive=false;

	   int m_nRequestId;
	   int m_nCurrentOrderRef;

	   lts_input_inbound_queue m_inputQueue;
	   lts_order_inbound_queue m_orderQueue;
	   lts_trade_inbound_queue m_tradeQueue;
	   lts_input_action_inbound_queue m_inputActionQueue;

	   bool m_bUserReqDiscon;

	   //friend class lts_connection;

   };

}

#endif // __LTS_API_H__
