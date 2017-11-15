#ifndef __OP_FS_CONNECTION_H__
#define __OP_FS_CONNECTION_H__

#include <string>
#include "connection.h"

//#include "fs_api.h"
#include "fs_order_aux.h"
#include "SgitFtdcTraderApi.h"

#include "LockFreeWorkQueue.h"

#include "LockFreeClassPool.h"
#include "ctpbase_connection.h"
//#define CFFEX_CONNECTION_THREADED
//#define POLL_NUM 10
#include <lockfree_classpool_workqueue.h>
using namespace terra::marketaccess::orderpassing;

namespace fs
{
	class user_info
	{
	public:
		int    OrderRef;
		string UserID;				
#if 0
		int    InternalRef;
		int    AccountNum;
		int    UserOrderID;		
		int    TradeType;
		int    Portfolio;
#endif
	};

	class fs_connection : public ctpbase_connection, public fstech::CThostFtdcTraderSpi

   {

   public:

	   terra::common::SingleLockFreeClassPool<fstech::CThostFtdcInputOrderField> xs_create_pool;
	   terra::common::SingleLockFreeClassPool<fstech::CThostFtdcInputOrderActionField> xs_cancel_pool;

	   //begin add on 20160929,from fstech::CThostFtdcTraderSpi
	   typedef terra::common::lockfree_classpool_workqueue<fstech::CThostFtdcInputOrderField> fs_input_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<fstech::CThostFtdcOrderField> fs_order_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<fstech::CThostFtdcTradeField> fs_trade_inbound_queue;
	   typedef terra::common::LockFreeWorkQueue<int> fs_input_action_inbound_queue;
	   //end add on 20160929
   public:
      fs_connection(bool checkSecurities = true);
      //virtual ~fs_connection();


      // connection methods
      virtual void init_connection();
	  virtual void release();

	  virtual void connect();
	  virtual void disconnect();

	  virtual void request_trading_account();

	  void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
	  void request_investor_full_positions();

#if 0
	  void request_instruments();
#endif
	  //void request_investor_position()
	  
	  //TThostFtdcOffsetFlagType compute_open_close(order* ord);


	  void OnRspOrderInsertAsync(fstech::CThostFtdcInputOrderField* pOrder);
	  void OnRtnOrderAsync(fstech::CThostFtdcOrderField* pOrder);
	  void OnRtnTradeAsync(fstech::CThostFtdcTradeField* pTrade);
	  void OnRspOrderActionAsync(int* nOrdRef);


      int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;

	 

	  int get_order_id(const char*pszOrderRef);
	  void get_user_info(const char* pszOrderRef, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType);
	  bool get_userId(const char* pszUserName, char* userID, int n);
	  void create_user_info(order * o, int orderRef, string userID);

	  void init_user_info(char * user_info_file);
	  void append(user_info * info);
	  //int  get_order_ref(int order_id);

	  //begin add on 20160929,from fstech::CThostFtdcTraderSpi
	  void init_api();
	  void release_api();

	  bool connect_api();
	  bool disconnect_api();


	  bool ReqOrderInsert(fstech::CThostFtdcInputOrderField* pOrder);
	  bool ReqOrderAction(fstech::CThostFtdcInputOrderActionField* pOrder);
	  bool ReqQryInvestorPosition(fstech::CThostFtdcQryInvestorPositionField *pQryInvestorPosition);
	  bool ReqQryTradingAccount(fstech::CThostFtdcQryTradingAccountField *pQryTradingAccount);


#if 0
	  void OnRspOrderInsertAsync(fstech::CThostFtdcInputOrderField* pInput);
	  void OnRtnOrderAsync(fstech::CThostFtdcOrderField* pOrder);
	  void OnRtnTradeAsync(fstech::CThostFtdcTradeField* pTrade);
	  void OnRspOrderActionAsync(int nRequest);
#endif


	  // cffex callbacks
	  virtual void OnRspError(fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	  virtual void OnFrontConnected();
	  virtual void OnFrontDisconnected(int nReason);

	  virtual void OnHeartBeatWarning(int nTimeLapse);

	  virtual void OnRspUserLogin(fstech::CThostFtdcRspUserLoginField* pRspUserLogin, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspUserLogout(fstech::CThostFtdcUserLogoutField* pUserLogout, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspSettlementInfoConfirm(fstech::CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspOrderInsert(fstech::CThostFtdcInputOrderField* pInputOrder, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspOrderAction(fstech::CThostFtdcInputOrderActionField* pInputOrderAction, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnErrRtnOrderInsert(fstech::CThostFtdcInputOrderField* pInputOrder, fstech::CThostFtdcRspInfoField* pRspInfo);
	  virtual void OnErrRtnOrderAction(fstech::CThostFtdcInputOrderActionField* pInputOrderAction, fstech::CThostFtdcRspInfoField* pRspInfo);

	  //for futures only
	  virtual void OnRspQryInvestorPosition(fstech::CThostFtdcInvestorPositionField *pInvestorPosition, fstech::CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  virtual void OnRspQryTradingAccount(fstech::CThostFtdcTradingAccountField *pTradingAccount, fstech::CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	  virtual void OnRtnOrder(fstech::CThostFtdcOrderField* pOrder);
	  virtual void OnRtnTrade(fstech::CThostFtdcTradeField* pTrade);

	  inline fs_input_inbound_queue* get_input_queue() { return &m_inputQueue; }
	  inline fs_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
	  inline fs_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }
	  inline fs_input_action_inbound_queue* get_input_action_queue() { return &m_inputActionQueue; }

	  void OnRspQryInstrument(fstech::CThostFtdcInstrumentField *pInstrument, fstech::CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  void request_instruments();

	  //int process_inbound_input_cb();
	  //int process_inbound_input_action_cb();
	  //int process_inbound_order_cb();
	  //int process_inbound_trade_cb();
	  void Process_api();
	  //end add on 20160929
   private:
	  string m_user_info_file_name;
   protected:
      // connection methods
	   //virtual bool init_config(const std::string &ini);
	   virtual bool init_config(const std::string &name, const std::string &ini);
      //virtual order* create_order() { return new fs_order(this); }
	   //virtual void cancel_num_warning(tradeitem* i) override;
	   //virtual void cancel_num_ban(tradeitem* i) override;
#if 0
	  std::string m_sName;
	  std::string GetName(){ return m_sName; }
	  inline bool is_alive() { return m_isAlive; }
	  inline void is_alive(bool b) { m_isAlive = b; }	  
#endif
	  void request_login();	  
	  //end add on 20160929
   protected:
      
	   void process() override;

	   //std::thread m_thread;
	   //boost::asio::io_service io;
	   //void Process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
	   //void set_kernel_timer_thread();


	   //begin add on 20160929,from fstech::CThostFtdcTraderSpi
	   fstech::CThostFtdcTraderApi* m_pUserApi;
	   bool m_connectionStatus;
	   bool m_isAlive;
	   int m_nRequestId;
	   int m_nCurrentOrderRef;
	   fs_input_inbound_queue m_inputQueue;
	   fs_order_inbound_queue m_orderQueue;
	   fs_trade_inbound_queue m_tradeQueue;
	   fs_input_action_inbound_queue m_inputActionQueue;
	   //end add on 20160929
#ifdef Linux
	   int efd;
	   void  init_epoll_eventfd();
#endif

   private:

	   bool m_bTsession;
	   //friend class fs_api;
      friend class fs_order_aux;

  
	  tbb::concurrent_unordered_map<int, user_info*> m_user_info_map;
	  //tbb::concurrent_unordered_map<int, int> m_order_id_map;
   };
}

#endif // __OP_FS_CONNECTION_H__

