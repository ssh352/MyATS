#ifndef __CFFEX_CONNECTION_H__
#define __CFFEX_CONNECTION_H__

#include "cffex_api.h"
#include "cffex_order_aux.h"
#include "ThostFtdcTraderApi.h"
#include "ctpbase_connection.h"


//#define POLL_NUM 10
using namespace terra::marketaccess::orderpassing;

namespace cffex
{

   class cffex_connection : public ctpbase_connection

   {

   public:

	   SingleLockFreeClassPool<CThostFtdcInputOrderField> xs_create_pool;
	   SingleLockFreeClassPool<CThostFtdcInputOrderActionField> xs_cancel_pool;

	   SingleLockFreeClassPool<CThostFtdcInputQuoteField> quote_create_pool;
	   SingleLockFreeClassPool<CThostFtdcInputQuoteActionField> quote_cancel_pool;

   public:
      cffex_connection(bool checkSecurities = true);
      virtual ~cffex_connection();


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
	  void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  void OnRspQryInstrument_Future(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


      void OnRspOrderInsertAsync(CThostFtdcInputOrderField* pOrder);
	  void OnRspQuoteInsertAsync(CThostFtdcInputQuoteField* pOrder);

      void OnRtnOrderAsync(CThostFtdcOrderField* pOrder);
	  void OnRtnQuoteAsync(CThostFtdcQuoteField* pQuote);


      void OnRtnTradeAsync(CThostFtdcTradeField* pTrade);

	  void OnRspOrderActionAsync(int* nOrdRef);
	  void OnRspQuoteActionAsync(int* nOrdRef);

	  
	  void OnRtnExecOrder(CThostFtdcExecOrderField *pOrder);
      int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;

	  int market_create_quote_async(quote* q, char* pszReason) override;
	  int market_cancel_quote_async(quote* q, char* pszReason) override;
   protected:
	   void process() override;
	   virtual bool init_config(const std::string &name, const std::string &ini);
      //virtual order* create_order() { return new cffex_order(this); }

	   //virtual void cancel_num_warning(tradeitem* i) override;
	   //virtual void cancel_num_ban(tradeitem* i) override;
	   void OnRspExecOrderInsert(CThostFtdcInputExecOrderField * pInputExecOrder, TThostFtdcErrorIDType ErrorID);
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


      cffex_api* m_pCffexApi;

	   bool m_bTsession;
	   friend class cffex_api;
	   friend class cffex_order_aux;

   };
}

#endif // __CFFEX_CONNECTION_H__

