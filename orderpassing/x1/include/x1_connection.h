#ifndef __X1_CONNECTION2_H__
#define __X1_CONNECTION2_H__

#include "AtsType_types.h"
#include "x1_order_aux.h"
#include "X1FtdcTraderApi.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_queue.h"
#include "lockfree_classpool_workqueue.h"

#include <ctpbase_connection.h>

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;
using namespace x1ftdcapi;
#include "LockFreeClassPool.h"

#ifdef _WIN32
#define OSD '\\'
#else
#define OSD '/'
#include <sys/eventfd.h>
#endif



namespace x1
{
	typedef struct _x1OrderMsg
	{
		int x1Id;
		int orderId;
		int accId;
		int nTradingType;
		std::string portfolio;

	}x1OrderMsg;

	typedef struct _localOrderMsg
	{
		int localId;
		int orderId;
		int accId;
		int nTradingType;
		std::string portfolio;

	}localOrderMsg;

	class x1_connection : public ctpbase_connection, public CX1FtdcTraderSpi
   {
   public:
      //typedef terra::common::LockFreeWorkQueue<order> xs_outbound_queue;
	  typedef terra::common::lockfree_classpool_workqueue<CX1FtdcRspPriOrderField> xs_order_rtn_queue;//交易所回执
	  typedef terra::common::lockfree_classpool_workqueue<CX1FtdcRspOperOrderField> xs_order_rsp_queue;//券商回执
	  typedef terra::common::lockfree_classpool_workqueue<CX1FtdcRspPriCancelOrderField> xs_order_can_rtn_queue;
	  typedef terra::common::lockfree_classpool_workqueue<CX1FtdcRspPriMatchInfoField> xs_trade_queue;

	  terra::common::SingleLockFreeClassPool<CX1FtdcInsertOrderField> xs_create_pool;
	  terra::common::SingleLockFreeClassPool<CX1FtdcCancelOrderField> xs_cancel_pool;
	  terra::common::SingleLockFreeClassPool<CX1FtdcQuoteInsertField> quote_create_pool;
	  terra::common::SingleLockFreeClassPool<CX1FtdcCancelOrderField> quote_cancel_pool;

   public:
	   x1_connection(const std::string &m_path,bool checkSecurities = true);
      //virtual ~x1_connection();

      // connection methods
      virtual void init_connection();
	  virtual void release();
	  virtual void connect();
	  virtual void disconnect();
	  //virtual void process_idle();
	  virtual void OnFrontConnected() override;
	  virtual void OnFrontDisconnected(int nReason) override;
	  virtual void OnRtnErrorMsg(struct CX1FtdcRspErrorField* pErrorInfo) override;


	  //history query
	  void OnQryOpOrders();
	  void OnQryFutureOrders();
	  void OnQryOpMatches();
	  void OnQryFutureMatches();
	  virtual void OnRspQryOrderInfo(struct CX1FtdcRspOrderField *pRtnOrderData, struct CX1FtdcRspErrorField * pErrorInfo, bool bIsLast) override;
	  virtual void OnRspQryMatchInfo(struct CX1FtdcRspMatchField *pRtnMatchData, struct CX1FtdcRspErrorField * pErrorInfo, bool bIsLast) override;

	  //Sec Trade Ack
	  virtual void OnRspInsertOrder(struct CX1FtdcRspOperOrderField * pOrderRtn, struct CX1FtdcRspErrorField * pErrorInfo);//qs
	  virtual void OnRspCancelOrder(struct CX1FtdcRspOperOrderField * pOrderCanceledRtn, struct CX1FtdcRspErrorField * pErrorInfo); //qs

	  //Exchange Trade Ack
	  virtual void OnRtnOrder(struct CX1FtdcRspPriOrderField * pRtnOrderData) override;//T
	  virtual void OnRtnCancelOrder(struct CX1FtdcRspPriCancelOrderField * pCancelOrderData) override;//T
	  virtual void OnRtnMatchedInfo(struct CX1FtdcRspPriMatchInfoField * pRtnMatchData) override;//T
	  
	  //Handler
	  //两个下单ack谁先收到就先处理谁。
	  void OnRtnOrderAsyn(CX1FtdcRspPriOrderField * pData);//T
	  void OnRspInsertOrderAsyn(CX1FtdcRspOperOrderField * pData);//qs
	  void OnRtnCancelOrderAsyn(CX1FtdcRspPriCancelOrderField * pData);
	  void OnRtnMatchedInfoAsyn(CX1FtdcRspPriMatchInfoField * pData);

	  //Login,LogOut
	  void OnRspUserLogin(struct CX1FtdcRspUserLoginField * pUserLoginInfoRtn, struct CX1FtdcRspErrorField * pErrorInfo) override;
	  void OnRspUserLogout(struct CX1FtdcRspUserLogoutInfoField * pUserLogoutInfoRtn, struct CX1FtdcRspErrorField * pErrorInfo) override;
	  
	  //send order
	  int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;

	  //quote
	  int market_create_quote_async(quote* q, char* pszReason) override;
	  int market_cancel_quote_async(quote* q, char* pszReason) override;
	  void OnRspQuoteInsert(struct CX1FtdcQuoteRspField * pRspQuoteData, struct CX1FtdcRspErrorField * pErrorInfo) override;

	  void OnRtnQuoteInsert(struct CX1FtdcQuoteRtnField * pRtnQuoteData);
	  void OnRtnQuoteCancel(struct CX1FtdcQuoteCanceledRtnField * pRtnQuoteCanceledData);
	  void OnRtnQuoteMatchedInfo(struct CX1FtdcQuoteMatchRtnField * pRtnQuoteMatchedData);


	  //Query position,instr,req_RiskDegree
	  void requset_op_instruments();
	  void request_op_positions();
	  void requset_future_instruments();
	  void request_future_positions();

	  void request_investor_full_positions();
	  void req_RiskDegree() override;

	  void OnRspCustomerCapital(struct CX1FtdcRspCapitalField* pCapitalInfoRtn, struct CX1FtdcRspErrorField* pErrorInfo, bool bIsLast) override;
	  virtual void OnRspQryExchangeInstrument(struct CX1FtdcRspExchangeInstrumentField *pInstrumentData, struct CX1FtdcRspErrorField *pErrorInfo, bool bIsLast);
	  virtual void OnRspQryPosition(struct CX1FtdcRspPositionField *pPositionInfoRtn, struct CX1FtdcRspErrorField *pErrorInfo, bool bIsLast);

	  std::string getMaturity(std::string sMat);

	  //get
	  //xs_outbound_queue*  get_outbound_queue() { return &m_outboundQueue; }
	  xs_order_rtn_queue* get_order_rtn_queue() { return &m_orderRtnQueue; }
	  xs_order_rsp_queue* get_order_rsp_queue() { return &m_orderRspQueue; }
	  xs_order_can_rtn_queue* get_order_can_rtn_queue() { return &m_ordCanRtnQueue; }
	  xs_trade_queue* get_trade_queue() { return &m_tradeQueue; }

	  void insert_localId2order(int id, order* o);
	  order *get_localId2order(int id);

	  void insert_spId2order(long id, order* o);
	  order *get_spId2order(long id);

	  void insert_used_locId(int id);
	  bool contain_used_locId(int id);

	  void insert_localId2quote(int id, quote* o);
	  quote *get_localId2quote(int id);

	  void insert_spId2quote(long id, quote* o);
	  quote *get_spId2quote(long id);
	  
   protected:
	   bool init_config(const std::string &name, const std::string &ini) override;

	   void process() override;
	   //virtual void cancel_num_warning(tradeitem* i) override;
	   //virtual void cancel_num_ban(tradeitem* i) override;

   private:
	  int get_xs_instype(AtsType::InstrType::type _type);
	  void ackBackup();
	  void readLocal2Portfolio();
	  void write_data2disk();

   protected:
	  void request_op_login();

#ifdef Linux
	  int efd;
	  void  init_epoll_eventfd();
#endif

   private:
	  
	   CX1FtdcTraderApi* m_pUserApi;
	   std::string m_path;
	  
	  xs_order_rtn_queue m_orderRtnQueue;
	  xs_order_rsp_queue m_orderRspQueue;
	  xs_order_can_rtn_queue m_ordCanRtnQueue;
	  xs_trade_queue m_tradeQueue;

	  std::vector<std::string> m_etfName;

	  lwtp m_startTime;

	  bool m_bRequestPosition = false;

	  int m_nRequestId;
	  int m_nCurrentOrderRef;

	  friend class x1_order_aux;
	  tbb::concurrent_hash_map<int, order*> m_localId2order;
	  tbb::concurrent_hash_map<int, quote*> m_localId2quote;
	  tbb::concurrent_hash_map<int, order*> m_spId2order;
	  tbb::concurrent_hash_map<int, quote*> m_spId2quote;
	  tbb::concurrent_hash_map<int,int> m_used_locId;

	  std::unordered_map<int, x1OrderMsg> m_x1Id2Portfolio;
	  std::unordered_map<int, localOrderMsg> m_localId2Portfolio;
	  tbb::concurrent_queue<std::shared_ptr<x1OrderMsg>> x1MsgQueue;
	  tbb::concurrent_queue<std::shared_ptr<localOrderMsg>> localMsgQueue;


	  bool m_bOpOrder = true; 
	  bool m_bFuOrder = false;
	  bool m_bOpTrade = false;
	  bool m_bFuTrade = false;
	  bool m_bTsession;
	  //bool m_bCloseToday;
	  int m_begin_Id;

	  bool m_OnFrontConnected = false;
	  bool m_bRequestFuture = false;	  
	  bool last_req_fut_pos = true;

	  int m_input_core;
	  int m_output_core;
	 lwtp m_timepoint;
   };
}

#endif // __XS_CONNECTION_H__

