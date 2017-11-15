#ifndef __XS_OF_CONNECTION2_H__
#define __XS_OF_CONNECTION2_H__

#include "AtsType_types.h"
#include "xs_of_order_aux.h"
#include "DFITCTraderApi.h"
#include "tbb/concurrent_hash_map.h"

#include "lockfree_classpool_workqueue.h"

#include <ctpbase_connection.h>

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;
using namespace DFITCXSPEEDAPI;
#include "LockFreeClassPool.h"

#ifdef _WIN32
#define OSD '\\'
#else
#define OSD '/'
#endif



namespace xs_of
{
	class xs_of_connection : public ctpbase_connection, public DFITCTraderSpi
   {
   public:
      //typedef terra::common::LockFreeWorkQueue<order> xs_outbound_queue;
	  typedef terra::common::lockfree_classpool_workqueue<DFITCOrderRtnField> xs_order_rtn_queue;//交易所回执
	  typedef terra::common::lockfree_classpool_workqueue<DFITCOrderRspDataRtnField> xs_order_rsp_queue;//券商回执
	  typedef terra::common::lockfree_classpool_workqueue<DFITCOrderCanceledRtnField> xs_order_can_rtn_queue;
	  typedef terra::common::lockfree_classpool_workqueue<DFITCMatchRtnField> xs_trade_queue;

	  //terra::common::SingleLockFreeClassPool<DFITCOrderRtnField> xs_order_rtn_pool;
	  //terra::common::SingleLockFreeClassPool<DFITCOrderRspDataRtnField> xs_order_rsp_pool;
	  //terra::common::SingleLockFreeClassPool<DFITCOrderCanceledRtnField> xs_order_can_rtn_pool;
	  //terra::common::SingleLockFreeClassPool<DFITCMatchRtnField> xs_trade_pool;

	  terra::common::SingleLockFreeClassPool<DFITCInsertOrderField> xs_create_pool;
	  terra::common::SingleLockFreeClassPool<DFITCCancelOrderField> xs_cancel_pool;
	  terra::common::SingleLockFreeClassPool<DFITCQuoteInsertField> quote_create_pool;
	  terra::common::SingleLockFreeClassPool<DFITCCancelOrderField> quote_cancel_pool;
   public:
      xs_of_connection(bool checkSecurities = true);
      //virtual ~xs_of_connection();

      // connection methods
      virtual void init_connection();
	  virtual void release();
	  virtual void connect();
	  virtual void disconnect();
	  //virtual void process_idle();
	  virtual void OnFrontConnected();
	  virtual void OnFrontDisconnected(int nReason);
	  virtual void OnRtnErrorMsg(struct DFITCErrorRtnField * pErrorInfo);

	  //history query
	  void OnQryOpOrders();
	  void OnQryFutureOrders();
	  void OnQryOpMatches();
	  void OnQryFutureMatches();
	  virtual void OnRspQryOrderInfo(struct DFITCOrderCommRtnField *pRtnOrderData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);
	  virtual void OnRspQryMatchInfo(struct DFITCMatchedRtnField *pRtnMatchData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast);

	  //Sec Trade Ack
	  virtual void OnRspInsertOrder(struct DFITCOrderRspDataRtnField * pOrderRtn, struct DFITCErrorRtnField * pErrorInfo);//qs
	  virtual void OnRspCancelOrder(struct DFITCOrderRspDataRtnField * pOrderCanceledRtn, struct DFITCErrorRtnField * pErrorInfo); //qs
	  virtual void OnRspError(DFITCErrorRtnField *pRspInfo);

	  //Exchange Trade Ack
	  virtual void OnRtnOrder(struct DFITCOrderRtnField * pRtnOrderData);//T
	  virtual void OnRtnCancelOrder(struct DFITCOrderCanceledRtnField * pCancelOrderData);//T
	  virtual void OnRtnMatchedInfo(struct DFITCMatchRtnField * pRtnMatchData);//T
	  
	  //Handler
	  //两个下单ack谁先收到就先处理谁。
	  void OnRtnOrderAsyn(DFITCOrderRtnField * pData);//T
	  void OnRspInsertOrderAsyn(DFITCOrderRspDataRtnField * pData);//qs
	  void OnRtnCancelOrderAsyn(DFITCOrderCanceledRtnField * pData);
	  void OnRtnMatchedInfoAsyn(DFITCMatchRtnField * pData);

	  //Login,LogOut
	  virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pUserLoginInfoRtn, struct DFITCErrorRtnField * pErrorInfo);
	  virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pUserLogoutInfoRtn, struct DFITCErrorRtnField * pErrorInfo);

	  //send order
	  int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;

	  //quote
	  int market_create_quote_async(quote* q, char* pszReason) override;
	  int market_cancel_quote_async(quote* q, char* pszReason) override;
	  void OnRspQuoteInsert(struct DFITCQuoteRspField * pRspQuote, struct DFITCErrorRtnField * pErrorInfo) override;
	  void OnRspQuoteCancel(struct DFITCQuoteRspField * pRspQuoteCancel, struct DFITCErrorRtnField * pErrorInfo) override;

	  void OnRtnQuoteInsert(struct DFITCQuoteRtnField * pRtnQuote) override;
	  void OnRtnQuoteCancel(struct DFITCQuoteCanceledRtnField * pRtnQuoteCanceled) override;
	  void OnRtnQuoteMatchedInfo(struct DFITCQuoteMatchRtnField * pRtnQuoteMatched) override;

	  //Query position,instr
	  void requset_op_instruments();
	  void request_op_positions();
	  void requset_future_instruments();
	  void request_future_positions();

	  void req_RiskDegree() override;
	  void OnRspCustomerCapital(struct DFITCCapitalInfoRtnField * pCapitalInfo, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast) override;

	  void request_investor_full_positions();
	  virtual void OnRspQryExchangeInstrument(struct DFITCExchangeInstrumentRtnField *pInstrumentData, struct DFITCErrorRtnField *pErrorInfo, bool bIsLast);
	  virtual void OnRspQryPosition(struct DFITCPositionInfoRtnField *pPositionInfoRtn, struct DFITCErrorRtnField *pErrorInfo, bool bIsLast);

	  std::string getMaturity(std::string sMat);

	  //callback
	  //int process_outbound_msg_cb();
	  //int process_order_rtn_cb();
	  //int process_order_rsp_cb();
	  //int process_order_cancel_rtn_cb();
	  //int process_trade_cb();

	  //get
	  //xs_outbound_queue*  get_outbound_queue() { return &m_outboundQueue; }
	  xs_order_rtn_queue* get_order_rtn_queue() { return &m_orderRtnQueue; }
	  xs_order_rsp_queue* get_order_rsp_queue() { return &m_orderRspQueue; }
	  xs_order_can_rtn_queue* get_order_can_rtn_queue() { return &m_ordCanRtnQueue; }
	  xs_trade_queue* get_trade_queue() { return &m_tradeQueue; }

	  void insert_localId2order(int id, order* o);
	  order *get_localId2order(int id);
	  void insert_used_locId(int id);
	  bool contain_used_locId(int id);
	  void insert_localId2quote(int id, quote* o);
	  quote *get_localId2quote(int id);

	  void insert_spId2quote(long id, quote* o);
	  quote *get_spId2quote(long id);

	  void insert_spId2order(long id, order* o);
	  order *get_spId2order(long id);
	  
   protected:
      // connection methods

	   bool init_config(const std::string &name, const std::string &ini) override;
      //virtual order* create_order() override;

	   void process() override;
	   //virtual void cancel_num_warning(tradeitem* i) override;
	   //virtual void cancel_num_ban(tradeitem* i) override;
	  // Thread methods
	  //std::thread m_thread;
	  //std::thread m_thread_rw;
	  //boost::asio::io_service io;
	  //void Process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
	  //void set_kernel_timer_thread();


   private:
	  int get_xs_instype(AtsType::InstrType::type _type);
	  void ackBackup();
	  void readLocal2Portfolio();

   protected:
	  void request_op_login();
#ifdef Linux
	  int efd;
	  void  init_epoll_eventfd();
#endif

   private:
	  
	  DFITCTraderApi* m_pUserApi;

	  
	  xs_order_rtn_queue m_orderRtnQueue;
	  xs_order_rsp_queue m_orderRspQueue;
	  xs_order_can_rtn_queue m_ordCanRtnQueue;
	  xs_trade_queue m_tradeQueue;


	  std::vector<std::string> m_etfName;

	  lwtp m_startTime;
	  
	  //abstract_database* m_database;
	  //bool m_bIsDicoRdy = false;
	  bool m_bRequestPosition = false;
	  //bool m_bRequestdico = false;

	  int m_nRequestId;
	  int m_nCurrentOrderRef;

	  tbb::concurrent_hash_map<int, order*> m_localId2order;
	  tbb::concurrent_hash_map<int, quote*> m_localId2quote;
	  tbb::concurrent_hash_map<int, order*> m_spId2order;
	  tbb::concurrent_hash_map<int, quote*> m_spId2quote;
	  tbb::concurrent_hash_map<int, int> m_used_locId;
	  friend class xs_of_order_aux;
	  //boost::shared_mutex m_rwmutex;
	  std::unordered_map<int, std::string> m_localId2Portfolio;//acked but untrade

	  bool m_bOpOrder = true; 
	  bool m_bFuOrder = false;
	  bool m_bOpTrade = false;
	  bool m_bFuTrade = false;
	  bool m_bTsession;
	  //bool m_bCloseToday;
	  int m_begin_Id;

	  
	  bool m_bRequestFuture = false;	  
   };
}
#endif // __XS_CONNECTION_H__

