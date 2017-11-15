#ifndef __ZD_CONNECTION_H__
#define __ZD_CONNECTION_H__

#include "zd_api.h"
#include "zd_order_aux.h"
#include "ShZdFutureTradeApi.h"
#include "ctpbase_connection.h"


//#define POLL_NUM 10
using namespace terra::marketaccess::orderpassing;

namespace zd
{
	class user_info
	{
	public:
		string OrderLocalID;
		string UserID;
	};

   class zd_connection : public ctpbase_connection
   {
   public:

	   SingleLockFreeClassPool<CTShZdInputOrderField>  zd_create_pool;
	   SingleLockFreeClassPool<CTShZdOrderActionField> zd_cancel_pool;

	   //SingleLockFreeClassPool<CThostFtdcInputQuoteField> quote_create_pool;
	   //SingleLockFreeClassPool<CThostFtdcInputQuoteActionField> quote_cancel_pool;
	   typedef terra::common::lockfree_classpool_workqueue<user_info>       zd_user_info_inbound_queue;
   public:
      zd_connection(bool checkSecurities = true);
      virtual ~zd_connection();


      // connection methods
      virtual void init_connection();
	  virtual void release();

	  virtual void connect();
	  virtual void disconnect();


	  virtual void request_trading_account();


	  void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
	  void request_investor_full_positions();
	  //
	  void req_RiskDegree() override;

	  //
	  void request_instruments();
	  std::string getMaturity(std::string& sMat);
	  void OnRspQryInstrument(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  void OnRspQryInstrument_Future(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


	  void OnRspOrderInsertAsync(CTShZdInputOrderField* pOrder);

	  //void OnRspQuoteInsertAsync(CThostFtdcInputQuoteField* pOrder);

	  void OnRtnOrderAsync(CTShZdOrderField* pOrder);

	  //void OnRtnQuoteAsync(CThostFtdcQuoteField* pQuote);


	  void OnRtnTradeAsync(CTShZdTradeField* pTrade);

	  void OnRspOrderActionAsync(int* nOrdRef);
	  //void OnRspQuoteActionAsync(int* nOrdRef);

	  
	  //void OnRtnExecOrder(CThostFtdcExecOrderField *pOrder);
      int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;

	  //int market_create_quote_async(quote* q, char* pszReason) override;
	  //int market_cancel_quote_async(quote* q, char* pszReason) override;
	  string get_auth_code(){ return m_strAuthCode; }
	  string get_investor_id(){ return this->m_sUsername; }
	  string get_user_id(){ return this->get_login_id(); }
	  //
	  void init_user_info(char * user_info_file);
	  void append(user_info * info);
	  void OnUserInfoAsync(user_info* pInfo);//Òì²½¼ÇÂ¼userinfo
	  void create_user_info(string UserID,string orderLocalId);
	  string get_user_id_ex(string orderLocalId);
	  //
	  bool get_night_trade(){ return (m_bTsession==false); }
	  string get_futures(){ return m_strFutures; }
	  string getInsertTimeStart(){ return m_strInsertTimeStart; }
	  //
   protected:
	   void process() override;
	   virtual bool init_config(const std::string &name, const std::string &ini);
      //virtual order* create_order() { return new cffex_order(this); }

	   //virtual void cancel_num_warning(tradeitem* i) override;
	   //virtual void cancel_num_ban(tradeitem* i) override;
	   //void OnRspExecOrderInsert(CThostFtdcInputExecOrderField * pInputExecOrder, TThostFtdcErrorIDType ErrorID);
#ifdef Linux
	   int efd;
	   void  init_epoll_eventfd();
#endif
	   //  

	   //std::thread m_thread;

	   //boost::asio::io_service io;
	   //void Process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
	   //void set_kernel_timer_thread();

   private:
#if  0
      cffex_api* m_pCffexApi;
#else
	   zd_api* m_pZdApi;
	   string  m_strAuthCode;
	   string m_user_info_file_name;
	   zd_user_info_inbound_queue m_userInfoQueue;
	   tbb::concurrent_unordered_map<string, user_info*>   m_user_info_map;	   
	   string m_strFutures = "CME";
	   string m_strInsertTimeStart = "20170101";
#endif
	   bool m_bTsession;
	   friend class zd_api;
	   friend class zd_order_aux;
   };
}
#endif // __ZD_CONNECTION_H__

