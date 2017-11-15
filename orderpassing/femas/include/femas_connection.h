#ifndef __FEMAS_CONNECTION_H_OP_
#define __FEMAS_CONNECTION_H_OP_
#include <string>
#include <vector>
#include "femas_api.h"
#include "femas_order_aux.h"
#include "USTPFtdcTraderApi.h"
#include "ctpbase_connection.h"
using namespace terra::marketaccess::orderpassing;
namespace femas
{	
   class femas_connection : public ctpbase_connection
   {
   public:
	   SingleLockFreeClassPool<CUstpFtdcInputOrderField>        femas_create_pool;
	   SingleLockFreeClassPool<CUstpFtdcOrderActionField>       femas_cancel_pool;
   public:
      femas_connection(bool checkSecurities = true);
      virtual ~femas_connection();
	  // connection methods
      virtual void init_connection();
      virtual void release();

      virtual void connect();
      virtual void disconnect();
	  
	  virtual void request_trading_account();
	  //
	  void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
	  void request_investor_full_positions();
	  //
	  void req_RiskDegree() override;
	  	  
	  //
	  //void request_instruments();
      std::string getMaturity(std::string& sMat);
	  void OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	  //	  
	  void OnRspOrderInsertAsync(CUstpFtdcInputOrderField* pOrder);
      void OnRtnOrderAsync(CUstpFtdcOrderField* pOrder);
      void OnRtnTradeAsync(CUstpFtdcTradeField* pTrade);
	  void OnRspOrderActionAsync(int* nOrdId);
	  //	  
	  int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;
	  string get_user_id(string userOrderLocalID)
	  {
		  return m_pFemasApi->get_user_id(userOrderLocalID);
	  }
       void init_user_info(char * user_info_file)
	  {
		  return m_pFemasApi->init_user_info(user_info_file);
	  }
   protected:         
	   bool init_config(const std::string &name, const std::string &ini) override;
       void process() override;
#ifdef Linux
	   int efd;
	   void  init_epoll_eventfd();
#endif
   private:
      //string   m_strInvestorID;
      femas_api* m_pFemasApi;     
	  bool m_bTsession;
      friend class femas_api;
      friend class femas_order_aux;	  
   };
}
#endif // __FEMAS_CONNECTION_H_OP_

