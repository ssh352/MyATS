#ifndef __OP_sf_CONNECTION_H__
#define __OP_sf_CONNECTION_H__
#include <string>
#include "connection.h"
#include "sf_order_aux.h"
#include "LockFreeWorkQueue.h"
#include "LockFreeClassPool.h"
#include "ctpbase_connection.h"
#include <lockfree_classpool_workqueue.h>
#include "SimulationOperation.h"
#include "zmqserver.h"
#include "Simulation_types.h"
using namespace zmq;
using namespace Simulation;
using namespace terra::marketaccess::orderpassing;
namespace sf
{
	class user_info
	{
	public:
		int    OrderRef;
		string UserID;				
	};
   class sf_connection : public ctpbase_connection
   {
   public:
	   typedef terra::common::LockFreeWorkQueue<OrderTradeRtnMsg> trade_rtn_outbound_queue;
   public:
      sf_connection(bool checkSecurities = true);
      	  
      // connection methods
      virtual void init_connection();
	  virtual void release();

	  virtual void connect();
	  virtual void disconnect();

	  virtual void request_trading_account();

	  void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
	  void request_investor_full_positions();


	  void OnRtnTradeAsync(OrderTradeRtnMsg* pTrade);

	  void OnRspOrderActionAsync(int* nOrdRef);


      int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;

	 

	  int get_order_id(const char*pszOrderRef);
	  void get_user_info(const char* pszOrderRef, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType);
	  bool get_userId(const char* pszUserName, char* userID, int n);
	  void create_user_info(order * o, int orderRef, string userID);

	  void init_user_info(char * user_info_file);
	  void append(user_info * info);
	  
	  
	  void init_api();
	  void release_api();

	  bool connect_api();
	  bool disconnect_api();

	  virtual void OnFrontConnected();
	  virtual void OnFrontDisconnected(int nReason);

	  virtual void OnHeartBeatWarning(int nTimeLapse);

	  inline trade_rtn_outbound_queue* get_trade_queue() { return &m_trade_rtn_queue; }

	  void request_instruments();
	  void Process_api();
	  //end add on 20160929
	  void process_data(uint8_t* buffer, size_t len);
	  bool setUDPSockect(const char * pBroadcastIP, int nBroadcastPort);
	  void start_receiver();
   private:
	  string m_user_info_file_name;
   protected:
      // connection methods
      virtual bool init_config(const std::string &name, const std::string &ini);
	  void request_login();	  
	  //end add on 20160929
   protected:      
	   void process() override;
	   //begin add on 20160929,from fstech::CThostFtdcTraderSpi
	   bool m_connectionStatus;
	   bool m_isAlive;
	   int m_nRequestId;
	   int m_nCurrentOrderRef;


	   SimulationOperationClient *m_pUserApi = nullptr;
	   zmq_client     *m_zmq_receiver=nullptr;
	   std::thread    m_receiver_thread;
	   string         m_zmq_server;
	   string         m_zmq_port;
	   trade_rtn_outbound_queue m_trade_rtn_queue;

	   //end add on 20160929
#ifdef Linux
	   int efd;
	   void  init_epoll_eventfd();
#endif
   private:
	  bool m_bTsession;
      friend class sf_order_aux;	    
	  tbb::concurrent_unordered_map<int, user_info*> m_user_info_map;	  
   };
}
#endif // __OP_sf_CONNECTION_H__

