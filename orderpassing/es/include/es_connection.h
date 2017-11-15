#ifndef __OP_ES_CONNECTION_H__
#define __OP_ES_CONNECTION_H__
#include <string>
#include "connection.h"
#include "es_order_aux.h"
#include "ctpbase_connection.h"
#include "TapTradeAPI.h"
#include "TapAPIError.h"
#include <lockfree_classpool_workqueue.h>
using namespace terra::marketaccess::orderpassing;
namespace es
{
	class user_info
	{
	public:		
		string OrderNo;
		string UserID;
	};

  class es_connection : public ctpbase_connection, public ITapTradeAPINotify
   {
   public:
	   terra::common::SingleLockFreeClassPool<TapAPINewOrder>       es_create_pool;
	   terra::common::SingleLockFreeClassPool<TapAPIOrderCancelReq> es_cancel_pool;

	   typedef terra::common::lockfree_classpool_workqueue<TapAPIOrderInfo> es_order_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<TapAPIFillInfo>  es_trade_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<user_info>       es_user_info_inbound_queue;
	   
	   typedef std::vector<exec*>  exec_vector;

   public:
      es_connection(bool checkSecurities = true);
      
      // connection methods
      virtual void init_connection();
	  virtual void release();

	  virtual void connect();
	  virtual void disconnect();

	  virtual void request_trading_account();

	  void request_investor_full_positions();  
	  //
	  void req_RiskDegree() override;

	  //
	  void OnRtnOrderAsync(TapAPIOrderInfo * pField);//市场接受
	  void OnRtnTradeAsync(TapAPIFillInfo  * pTrade);//成交
	  void OnUserInfoAsync(user_info* pInfo);//异步记录userinfo
	  //to do ...
	  void OnRtnOrderAsync_Quote(TapAPIOrderInfo * pField);

      int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;
	  	
	  //to do ...
	  int market_create_quote_async(quote* q, char* pszReason) override;
	  int market_cancel_quote_async(quote* q, char* pszReason) override;

	  void get_user_info_ex(string orderNo, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType);
	  void create_user_info(TapAPIOrderInfo * pField);
	  ptime get_time(const string & time_buffer);	  

	  void init_user_info(char * user_info_file);
	  void append(user_info * info);
	  	  
	  void init_api();
	  void release_api();

	  bool connect_api();
	  bool disconnect_api();


	  bool ReqOrderInsert(TapAPINewOrder* pRequest);
	  bool ReqOrderAction(TapAPIOrderCancelReq* pRequest);

	  virtual void TAP_CDECL OnConnect();
	  virtual void TAP_CDECL OnRspLogin(TAPIINT32 errorCode, const TapAPITradeLoginRspInfo *loginRspInfo);
	  virtual void TAP_CDECL OnAPIReady();
	  virtual void TAP_CDECL OnDisconnect(TAPIINT32 reasonCode);
	  virtual void TAP_CDECL OnRspChangePassword(TAPIUINT32 sessionID, TAPIINT32 errorCode);
	  virtual void TAP_CDECL OnRspSetReservedInfo(TAPIUINT32 sessionID, TAPIINT32 errorCode, const TAPISTR_50 info);
	  virtual void TAP_CDECL OnRspQryAccount(TAPIUINT32 sessionID, TAPIUINT32 errorCode, TAPIYNFLAG isLast, const TapAPIAccountInfo *info);
	  virtual void TAP_CDECL OnRspQryFund(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIFundData *info);
	  virtual void TAP_CDECL OnRtnFund(const TapAPIFundData *info);
	  virtual void TAP_CDECL OnRspQryExchange(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIExchangeInfo *info);
	  virtual void TAP_CDECL OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPICommodityInfo *info);
	  virtual void TAP_CDECL OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPITradeContractInfo *info);
	  virtual void TAP_CDECL OnRtnContract(const TapAPITradeContractInfo *info);
	  virtual void TAP_CDECL OnRtnOrder(const TapAPIOrderInfoNotice *info);
	  virtual void TAP_CDECL OnRspOrderAction(TAPIUINT32 sessionID, TAPIUINT32 errorCode, const TapAPIOrderActionRsp *info);
	  virtual void TAP_CDECL OnRspQryOrder(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIOrderInfo *info);
	  virtual void TAP_CDECL OnRspQryOrderProcess(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIOrderInfo *info);
	  virtual void TAP_CDECL OnRspQryFill(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIFillInfo *info);
	  virtual void TAP_CDECL OnRtnFill(const TapAPIFillInfo *info);
	  virtual void TAP_CDECL OnRspQryPosition(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIPositionInfo *info);
	  virtual void TAP_CDECL OnRtnPosition(const TapAPIPositionInfo *info);
	  virtual void TAP_CDECL OnRspQryClose(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPICloseInfo *info);
	  virtual void TAP_CDECL OnRtnClose(const TapAPICloseInfo *info);
	  virtual void TAP_CDECL OnRtnPositionProfit(const TapAPIPositionProfitNotice *info);
	  virtual void TAP_CDECL OnRspQryDeepQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIDeepQuoteQryRsp *info);
	  virtual void TAP_CDECL OnRspQryExchangeStateInfo(TAPIUINT32 sessionID,TAPIINT32 errorCode, TAPIYNFLAG isLast,const TapAPIExchangeStateInfo * info);
	  virtual void TAP_CDECL OnRtnExchangeStateInfo(const TapAPIExchangeStateInfoNotice * info);
	  virtual void TAP_CDECL OnRtnReqQuoteNotice(const TapAPIReqQuoteNotice *info); //V9.0.2.0 20150520
	  virtual void TAP_CDECL OnRspUpperChannelInfo(TAPIUINT32 sessionID,TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIUpperChannelInfo * info);
	  virtual void TAP_CDECL OnRspAccountRentInfo(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIAccountRentInfo * info);
	  //to do ...
	  void OnRspQryInstrument_Future(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPITradeContractInfo *info);

	  inline es_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
	  inline es_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }

	  void request_instruments();
	  void request_commodity();
	  void Process_api();
	  //end add on 20160929     
   protected:      
	  void process() override;
	  virtual bool init_config(const std::string &name, const std::string &ini);      
	  void request_login();
	  string get_broker_id(){ return m_sBrokerId; }	
	  string get_CommodityNo(string code);
	  string get_ContractNo(string code);
	  string get_Strike(string code);
   protected:      	   
	   ITapTradeAPI* m_pUserApi;
	   bool m_connectionStatus;
	   bool m_isAlive;
	   int m_nRequestId;
	   int m_nCurrentOrderRef;
	   string m_user_info_file_name;
	   bool m_bTsession;
	   string m_strAuthCode;	   
	   es_order_inbound_queue m_orderQueue;
	   es_trade_inbound_queue m_tradeQueue;
	   es_user_info_inbound_queue m_userInfoQueue;
	   bool m_bPosition = false;
	   string m_strApiLogPath;
#ifdef Linux
	   int efd;
	   void init_epoll_eventfd();
	   bool write_fd();
#endif
   private:	  
       friend class es_order_aux;
	   tbb::concurrent_unordered_map<string, user_info*>   m_user_info_map;	
#if 0
	   tbb::concurrent_hash_map<string, exec_vector*>      m_local_no_map;
#endif
	   //
	   tbb::concurrent_unordered_map<string,int>           m_order_local_no_id_map;
	   //
	   tbb::concurrent_unordered_map<string, tradeitem*>   m_trade_item_map;
   };
}

#endif // __OP_ES_CONNECTION_H__

