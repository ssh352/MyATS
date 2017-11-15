#ifndef __IB_CONNECTION_H__
#define __IB_CONNECTION_H__

#if 1

#include "StdAfx.h"
#ifndef TWSAPIDLL
#ifndef TWSAPIDLLEXP
#define TWSAPIDLLEXP __declspec(dllimport)
#endif
#endif

#endif//endif --1

#include "ctpbase_connection.h"
#include "order.h"

#if 0
#include "../api/Shared/Order.h"
#include "../api/Shared/EWrapper.h"
#include "../api/Shared/Contract.h"
#include "../api/Shared/Execution.h"
#include "../api/Shared/OrderState.h"
#else
#include "Order.h"
#include "EWrapper.h"
#include "Contract.h"
#include "Execution.h"
#include "OrderState.h"
#endif

#if 0
class EPosixClientSocket;
#else
#include "EReader.h"
#include "EReaderOSSignal.h"
#include "EClientSocket.h"
#endif

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;


namespace ib
{
	class ib_connection : public ctpbase_connection, public EWrapper
   {
    public:
	  terra::common::SingleLockFreeClassPool<Order>       ib_create_pool;
	  

      ib_connection(bool checkSecurities = true);
      virtual ~ib_connection();

      // connection methods
      virtual void init_connection(/*RTListener* pListener*/);
      //virtual void release_connection();

      virtual void connect();
      virtual void disconnect();

	  //virtual void load_instruments(const std::string& name, const std::string &ini, const char* sqlfile);
	  //void load_instruments_type(const std::string& name, const std::string &strConfigFile, abstract_database* db, const std::string& instType);

	  void requestOpenOrders();
	  void requestExecs();
	  void requestAccountSummary();

	  void processMessages();
	  
	  void request_investor_full_positions(){}
	  
	  //virtual void release(){};

	  //virtual order* create_order(tradeitem *instrument, OrderWay::type way, int qty, double price);

   protected:
      // connection methods
	  //virtual bool init_config(const std::string &ini);
	  //virtual bool init_config(const string &name, const std::string &strConfigFile);
      //virtual order* create_order() { return new ib_order(this); }

	   virtual int market_create_order_async(order* o, char* pszReason) override;
	   virtual int market_cancel_order_async(order* o, char* pszReason) override;
	   void process() override;
	   
	   virtual void cancel_num_warning(tradeitem* i) override;
	   virtual void cancel_num_ban(tradeitem* i) override;
	   
   private:
      //int get_order_id(const char*);
	  //void get_user_info(const char* psz, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType);

   public:
	  // Ewrapper calls
	  void tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute) {};
	  void tickSize(TickerId tickerId, TickType field, int size) {};
	  void tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta, double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice) {};
	  void tickGeneric(TickerId tickerId, TickType tickType, double value) {};
	  void tickString(TickerId tickerId, TickType tickType, const std::string& value) {};
	  void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints, double totalDividends, int holdDays, const std::string& futureExpiry, double dividendImpact, double dividendsToExpiry) {};
	  void orderStatus(OrderId orderId, const std::string &status, int filled,  int remaining, double avgFillPrice, int permId, int parentId,  double lastFillPrice, int clientId, const std::string& whyHeld) ;
	  void openOrder(OrderId orderId, const Contract&, const Order&, const OrderState&);
	  void openOrderEnd();
	  void winError(const std::string &str, int lastError) {};
	  void connectionClosed() {};
	  void updateAccountValue(const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName) {};
	  void updatePortfolio(const Contract& contract, int position, double marketPrice, double marketValue, double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName) {};
	  void updateAccountTime(const std::string& timeStamp) {};
	  void accountDownloadEnd(const std::string& accountName) {};
	  void nextValidId(OrderId orderId);
	  void contractDetails(int reqId, const ContractDetails& contractDetails) {};
	  void bondContractDetails(int reqId, const ContractDetails& contractDetails) {};
	  void contractDetailsEnd(int reqId) {};
	  void execDetails(int reqId, const Contract& contract, const Execution& execution);
	  void execDetailsEnd(int reqId);
	  void error(const int id, const int errorCode, const std::string errorString) ;
	  void updateMktDepth(TickerId id, int position, int operation, int side, double price, int size) {};
	  void updateMktDepthL2(TickerId id, int position, std::string marketMaker, int operation, int side, double price, int size) {};
	  void updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {};
	  void managedAccounts(const std::string& accountsList) {};
	  void receiveFA(faDataType pFaDataType, const std::string& cxml) {};
	  void historicalData(TickerId reqId, const std::string& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps) {};
	  void scannerParameters(const std::string &xml) {};
	  void scannerData(int reqId, int rank, const ContractDetails &contractDetails, const std::string &distance, const std::string &benchmark, const std::string &projection, const std::string &legsStr) {};
	  void scannerDataEnd(int reqId) {};
	  void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count) {};
	  void currentTime(long time) {};
	  void fundamentalData(TickerId reqId, const std::string& data){};
	  void deltaNeutralValidation(int reqId, const UnderComp& underComp){};
	  void tickSnapshotEnd(int reqId){};
	  void marketDataType(TickerId reqId, int marketDataType){};
	  void commissionReport(const CommissionReport& commissionReport){};
	  void position(const std::string& account, const Contract& contract, int position, double avgCost){};
	  void positionEnd(){};
	  void accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency);
	  void accountSummaryEnd(int reqId) ;
	  void verifyMessageAPI(const std::string& apiData) {};
	  void verifyCompleted(bool isSuccessful, const std::string& errorText) {};
	  void displayGroupList(int reqId, const std::string& groups) {};
	  void displayGroupUpdated(int reqId, const std::string& contractInfo) {};
	  //
	  virtual void orderStatus(OrderId orderId, const std::string& status, double filled,
		  double remaining, double avgFillPrice, int permId, int parentId,
		  double lastFillPrice, int clientId, const std::string& whyHeld){}
	  virtual void updatePortfolio(const Contract& contract, double position,
		  double marketPrice, double marketValue, double averageCost,
		  double unrealizedPNL, double realizedPNL, const std::string& accountName){}
	  virtual void position(const std::string& account, const Contract& contract, double position, double avgCost){}
	  virtual void verifyAndAuthMessageAPI(const std::string& apiData, const std::string& xyzChallenge){}
	  virtual void verifyAndAuthCompleted(bool isSuccessful, const std::string& errorText){}
	  virtual void connectAck();
	  virtual void positionMulti(int reqId, const std::string& account, const std::string& modelCode, const Contract& contract, double pos, double avgCost){}
	  virtual void positionMultiEnd(int reqId){}
	  virtual void accountUpdateMulti(int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency){}
	  virtual void accountUpdateMultiEnd(int reqId){}
	  virtual void securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass, const std::string& multiplier, std::set<std::string> expirations, std::set<double> strikes){}
	  virtual void securityDefinitionOptionalParameterEnd(int reqId){}
	  virtual void softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers){}
	  //
	  bool get_night_trade(){ return (m_bTsession == false); }
	  //
   private:
#if 0
	  std::auto_ptr<EPosixClientSocket> m_pClient;
#else
	   EReaderOSSignal m_osSignal;
	   EClientSocket * m_pClient;
	   EReader *m_pReader;
	   bool m_extraAuth = false;
	   std::thread m_tListhen2IB;
	   //
	   bool m_bTsession;
	   //int  m_begin_Id = 0;
	   //
#endif
      //bool m_debug;
	  //std::string m_sName;
      //std::string m_sHostname;
      //int m_sService;
      //int m_sUserId;
	  int m_requestId;
	  //std::thread m_tListhen2IB;
	  //boost::thread m_tListhen2IB;
#ifdef Linux
	  int efd;
	  void  init_epoll_eventfd();
#endif
	  friend class ib_order_aux;
   };
}

#endif
