#ifndef __FEMAS_API_H_OP_
#define __FEMAS_API_H_OP_
#include "USTPFtdcTraderApi.h"
#include <map>
#include <string>
#include "ctpbase_connection.h"
#include "lockfree_classpool_workqueue.h"
using namespace terra::marketaccess::orderpassing;
namespace femas
{
   class femas_connection;
   class user_info
   {
   public:
	std::string UserOrderLocalID;
	std::string UserID;
   };
   class femas_api : public CUstpFtdcTraderSpi
   {
   public:
	  typedef terra::common::lockfree_classpool_workqueue<CUstpFtdcInputOrderField> femas_input_inbound_queue;
	  typedef terra::common::lockfree_classpool_workqueue<CUstpFtdcOrderField>      femas_order_inbound_queue;
	  typedef terra::common::lockfree_classpool_workqueue<CUstpFtdcTradeField>      femas_trade_inbound_queue;
	  typedef terra::common::LockFreeWorkQueue<int>                                 femas_input_action_inbound_queue;	  
	  //
	  typedef terra::common::lockfree_classpool_workqueue<user_info>                femas_user_info_inbound_queue;
   public:
      femas_api(femas_connection* pConnection);
      virtual ~femas_api();
      
      void init();
      void release();

      bool connect();
      bool disconnect();
      
      bool ReqOrderInsert(CUstpFtdcInputOrderField* pOrder);	  
	  bool ReqOrderAction(CUstpFtdcOrderActionField* pOrder);	  
	  bool ReqQryInvestorPosition(CUstpFtdcQryInvestorPositionField *pQryInvestorPosition);
	  bool ReqQryInvestorAccount(CUstpFtdcQryInvestorAccountField* pQryInvestorAccountField);	  

	  // femas callbacks
      virtual void OnRspError(CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
      virtual void OnFrontConnected();
      virtual void OnFrontDisconnected(int nReason);
      virtual void OnHeartBeatWarning(int nTimeLapse);

      virtual void OnRspUserLogin(CUstpFtdcRspUserLoginField* pRspUserLogin,	CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspUserLogout(CUstpFtdcRspUserLogoutField* pUserLogout, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
      virtual void OnRspOrderInsert(CUstpFtdcInputOrderField* pInputOrder, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspOrderAction(CUstpFtdcOrderActionField* pInputOrderAction, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnErrRtnOrderInsert(CUstpFtdcInputOrderField* pInputOrder, CUstpFtdcRspInfoField* pRspInfo);
	  virtual void OnErrRtnOrderAction(CUstpFtdcOrderActionField* pInputOrderAction, CUstpFtdcRspInfoField* pRspInfo);

	  //for futures only
	  virtual void OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pTradingAccount, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  
	  virtual void OnRtnOrder(CUstpFtdcOrderField* pOrder);
      virtual void OnRtnTrade(CUstpFtdcTradeField* pTrade);
	  //
      inline femas_input_inbound_queue* get_input_queue() { return &m_inputQueue; }
      inline femas_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
      inline femas_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }
	  inline femas_input_action_inbound_queue* get_input_action_queue() { return &m_inputActionQueue; }
          inline femas_user_info_inbound_queue* get_user_info_queue() { return &m_userInfoQueue; }
	  //Todolist
	  virtual void OnRtnInstrumentStatus(CUstpFtdcInstrumentStatusField *pInstrumentStatus) {};
	  virtual void OnRtnInvestorAccountDeposit(CUstpFtdcInvestorAccountDepositResField *pInvestorAccountDepositRes) {};
	  virtual void OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	  virtual void OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	  virtual void OnRspQryUserInvestor(CUstpFtdcRspUserInvestorField *pRspUserInvestor, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	  virtual void OnRspQryTradingCode(CUstpFtdcRspTradingCodeField *pRspTradingCode, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	  //virtual void OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pRspInvestorAccount, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	  virtual void OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspQryExchange(CUstpFtdcRspExchangeField *pRspExchange, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	  virtual void OnRspQryInvestorMargin(CUstpFtdcInvestorMarginField *pInvestorMargin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};      
   public:
      void Process();   
      inline bool is_alive() { return m_isAlive; }
      inline void is_alive(bool b) { m_isAlive = b; }
      void request_login();
	  int get_ord_ref_from_reqid(int nReqId);
	  int get_order_id(const char*);
   public:
	   void init_user_info(char * user_info_file);
	   void append(user_info * info);
	   void create_user_info(CUstpFtdcInputOrderField* pRequest);
	   void OnUserInfoAsync(user_info* pInfo);
	   string get_user_id(string userOrderLocalID);
	   void request_instruments();
	   int  get_begin_id(){ return m_begin_id; }
   public: 
	  int get_next_OrderRef(){ return ++m_nCurrentOrderRef; }
	  //int get_request_id(){ return m_nRequestId; }
   protected:
	  int m_begin_id = 0;
      femas_connection* m_pConnection;
      CUstpFtdcTraderApi* m_pUserApi;
      bool m_connectionStatus;
      bool m_isAlive;
      int m_nRequestId;
      int m_nCurrentOrderRef; //auxiliary int at the beginning of the userlocalorderref
      femas_input_inbound_queue m_inputQueue;
      femas_order_inbound_queue m_orderQueue;
      femas_trade_inbound_queue m_tradeQueue;
	  //
	  femas_input_action_inbound_queue m_inputActionQueue;
	  std::map<int, int> m_ordInputActiondRefMap; // contains <nRequestId,nOrderid>	
	  //
	  string m_user_info_file_name;
	  femas_user_info_inbound_queue m_userInfoQueue;
	  tbb::concurrent_unordered_map<string, user_info*>   m_user_info_map;
   };
}

#endif // __FEMAS_API_H_OP_
