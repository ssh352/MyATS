#ifndef __OP_SL_CONNECTION_H__
#define __OP_SL_CONNECTION_H__
#include <string>
#include "connection.h"
#include "sl_order_aux.h"
#include "ctpbase_connection.h"
#include "EesTraderApi.h"
#include <lockfree_classpool_workqueue.h>
using namespace terra::marketaccess::orderpassing;
namespace sl
{
	class user_info
	{
	public:
		EES_ClientToken ClientToken;
		string UserID;
		/*
		int    InternalRef;
		int    AccountNum;
		int    UserOrderID;		
		int    TradeType;
		int    Portfolio;
		*/		
		string Symbol;
		EES_SideType SideType;
		int    OrderId;
	};
	//
	class user_info_ex
	{
	public:
		EES_MarketToken MarketToken;		
		string Symbol;
		EES_SideType SideType;
		int    OrderId;
	};
	//
  class sl_connection : public ctpbase_connection, public EESTraderEvent
   {
   public:
	   terra::common::SingleLockFreeClassPool<EES_EnterOrderField> sl_create_pool;
	   terra::common::SingleLockFreeClassPool<EES_CancelOrder>     sl_cancel_pool;
	   //begin add on 20160929,from fstech::CThostFtdcTraderSpi
	   typedef terra::common::lockfree_classpool_workqueue<EES_OrderAcceptField> sl_input_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<EES_OrderMarketAcceptField> sl_order_inbound_queue;	   
	   typedef terra::common::lockfree_classpool_workqueue<EES_OrderExecutionField> sl_trade_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<EES_OrderCxled> sl_input_action_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<EES_OrderRejectField> sl_order_reject_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<EES_OrderMarketRejectField> sl_market_order_reject_inbound_queue;
	   typedef terra::common::lockfree_classpool_workqueue<user_info> sl_user_info_inbound_queue;
	   //end add on 20160929
   public:
      sl_connection(bool checkSecurities = true);
      //virtual ~sl_connection();

      // connection methods
      virtual void init_connection();
	  virtual void release();

	  virtual void connect();
	  virtual void disconnect();

	  virtual void request_trading_account();

#if 0
	  void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
#endif

	  void request_investor_full_positions();
	  //
	  void req_RiskDegree() override;

	  //
#if 0
	  void request_instruments();
#endif
	  //void request_investor_position()
	  
	  //TThostFtdcOffsetFlagType compute_open_close(order* ord);

#if 1
	  /*

	  */
	  void OnRspOrderInsertAsync(EES_OrderAcceptField* pField);//柜台接受
	  void OnRtnOrderAsync(EES_OrderMarketAcceptField * pField);//市场接受
	  void OnRtnTradeAsync(EES_OrderExecutionField* pTrade);//成交
	  void OnRspOrderActionAsync(EES_OrderCxled* pCancel);//撤单cancel
	  void OnRspRejectActionAsync(EES_OrderRejectField* pReject);//柜台拒绝
	  void OnRspMarketRejectActionAsync(EES_OrderMarketRejectField* pReject);//市场拒绝
	  void OnUserInfoAsync(user_info* pInfo);//异步记录userinfo
#endif

      int market_create_order_async(order* o, char* pszReason) override;
	  int market_cancel_order_async(order* o, char* pszReason) override;
	  	
	  //to do ...
	  //
	  int get_local_user_id(){ return m_UserId; }
	  //
	  int get_order_id(EES_ClientToken clientToken);
	  bool get_symbol(EES_ClientToken clientToken,string & symbol);
	  EES_SideType get_ess_side_type(EES_ClientToken clientToken);
	  void get_user_info(EES_ClientToken clientToken, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType);
	  //	
	  int get_external_order_id(EES_MarketToken marketToken);
	  bool get_external_symbol(EES_MarketToken marketToken, string & symbol);
	  EES_SideType get_ess_external_side_type(EES_MarketToken marketToken);	  
	  //
#if 0
	  bool get_userId(const char* pszUserName, char* userID, int n);
#endif
	  void create_user_info(order * o, EES_ClientToken clientTocken, string userID, string symbol,EES_SideType type);
	  void init_user_info(char * user_info_file);
	  void append(user_info * info);
	  //int  get_order_ref(int order_id);

	  //begin add on 20160929,from fstech::CThostFtdcTraderSpi
	  void init_api();
	  void release_api();

	  bool connect_api();
	  bool disconnect_api();


	  bool ReqOrderInsert(EES_EnterOrderField* pRequest);
	  bool ReqOrderAction(EES_CancelOrder* pRequest);
#if 0
	  bool ReqQryInvestorPosition(fstech::CThostFtdcQryInvestorPositionField *pQryInvestorPosition);
#endif
#if 0
	  bool ReqQryTradingAccount(fstech::CThostFtdcQryTradingAccountField *pQryTradingAccount);
#endif
#if 0
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
	  void OnRspQryInstrument(fstech::CThostFtdcInstrumentField *pInstrument, fstech::CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
#else
	  /// 连接消息的回调

	  ///	\brief	服务器连接事件
	  ///	\param  errNo                   连接成功能与否的消息
	  ///	\param  pErrStr                 错误信息
	  ///	\return void  

	  virtual void OnConnection(ERR_NO errNo, const char* pErrStr);

	  /// 连接断开消息的回调

	  /// \brief	服务器主动断开，会收到这个消息
	  /// \param  ERR_NO errNo         连接成功能与否的消息
	  /// \param  const char* pErrStr  错误信息
	  /// \return void  

	  virtual void OnDisConnection(ERR_NO errNo, const char* pErrStr);

	  /// 登录消息的回调

	  /// \param  pLogon                  登录成功或是失败的结构
	  /// \return void 

	  virtual void OnUserLogon(EES_LogonResponse* pLogon);

	  /// 修改密码响应回调

	  /// \param  nResult                  服务器响应的成功与否返回码
	  /// \return void 

	  virtual void OnRspChangePassword(EES_ChangePasswordResult nResult);

	  /// 查询用户下面帐户的返回事件

	  /// \param  pAccountInfo	        帐户的信息
	  /// \param  bFinish	                如果没有传输完成，这个值是 false ，如果完成了，那个这个值为 true 
	  /// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pAccountInfo值无效。
	  /// \return void 

	  virtual void OnQueryUserAccount(EES_AccountInfo * pAccoutnInfo, bool bFinish);

	  /// 查询帐户下面仓位信息的返回事件

	  /// \param  pAccount	                帐户ID 	
	  /// \param  pAccoutnPosition	        帐户的仓位信息					   
	  /// \param  nReqId		                发送请求消息时候的ID号。
	  /// \param  bFinish	                    如果没有传输完成，这个值是false，如果完成了，那个这个值为 true 
	  /// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pAccountInfo值无效。
	  /// \return void 

	  virtual void OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish);


	  /// 查询帐户下面资金信息的返回事件

	  /// \param  pAccount	                帐户ID 	
	  /// \param  pAccoutnPosition	        帐户的仓位信息					   
	  /// \param  nReqId		                发送请求消息时候的ID号
	  /// \return void 

	  virtual void OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccoutnPosition, int nReqId);

	  /// 查询合约列表的返回事件

	  /// \param  pSymbol	                    合约信息   
	  /// \param  bFinish	                    如果没有传输完成，这个值是 false，如果完成了，那个这个值为 true   
	  /// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pSymbol 值无效。
	  /// \return void 

	  virtual void OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish);

	  /// 查询帐户交易保证金的返回事件

	  /// \param  pAccount                    帐户ID 
	  /// \param  pSymbolMargin               帐户的保证金信息 
	  /// \param  bFinish	                    如果没有传输完成，这个值是 false，如果完成，那个这个值为 true 
	  /// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pSymbolMargin 值无效。
	  /// \return void 

	  virtual void OnQueryAccountTradeMargin(const char* pAccount, EES_AccountMargin* pSymbolMargin, bool bFinish);

	  /// 查询帐户交易费用的返回事件

	  /// \param  pAccount                    帐户ID 
	  /// \param  pSymbolFee	                帐户的费率信息	 
	  /// \param  bFinish	                    如果没有传输完成，这个值是 false，如果完成了，那个这个值为 true    
	  /// \remark 如果碰到 bFinish == true ，那么是传输结束，并且 pSymbolFee 值无效。
	  /// \return void 

	  virtual void OnQueryAccountTradeFee(const char* pAccount, EES_AccountFee* pSymbolFee, bool bFinish);

	  /// 下单被柜台系统接受的事件

	  /// \brief 表示这个订单已经被柜台系统正式的接受
	  /// \param  pAccept	                    订单被接受以后的消息体
	  /// \return void 

	  virtual void OnOrderAccept(EES_OrderAcceptField* pAccept);


	  /// 下单被市场接受的事件

	  /// \brief 表示这个订单已经被交易所正式的接受
	  /// \param  pAccept	                    订单被接受以后的消息体，里面包含了市场订单ID
	  /// \return void 
	  virtual void OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept);


	  ///	下单被柜台系统拒绝的事件

	  /// \brief	订单被柜台系统拒绝，可以查看语法检查或是风控检查。 
	  /// \param  pReject	                    订单被接受以后的消息体
	  /// \return void 

	  virtual void OnOrderReject(EES_OrderRejectField* pReject);


	  ///	下单被市场拒绝的事件

	  /// \brief	订单被市场拒绝，可以查看语法检查或是风控检查。 
	  /// \param  pReject	                    订单被接受以后的消息体，里面包含了市场订单ID
	  /// \return void 

	  virtual void OnOrderMarketReject(EES_OrderMarketRejectField* pReject);


	  ///	订单成交的消息事件

	  /// \brief	成交里面包括了订单市场ID，建议用这个ID查询对应的订单
	  /// \param  pExec	                   订单被接受以后的消息体，里面包含了市场订单ID
	  /// \return void 

	  virtual void OnOrderExecution(EES_OrderExecutionField* pExec);

	  ///	订单成功撤销事件

	  /// \brief	成交里面包括了订单市场ID，建议用这个ID查询对应的订单
	  /// \param  pCxled		               订单被接受以后的消息体，里面包含了市场订单ID
	  /// \return void 

	  virtual void OnOrderCxled(EES_OrderCxled* pCxled);

	  ///	撤单被拒绝的消息事件

	  /// \brief	一般会在发送撤单以后，收到这个消息，表示撤单被拒绝
	  /// \param  pReject	                   撤单被拒绝消息体
	  /// \return void 

	  virtual void OnCxlOrderReject(EES_CxlOrderRej* pReject);

	  ///	查询订单的返回事件

	  /// \brief	查询订单信息时候的回调，这里面也可能包含不是当前用户下的订单
	  /// \param  pAccount                 帐户ID 
	  /// \param  pQueryOrder	             查询订单的结构
	  /// \param  bFinish	                 如果没有传输完成，这个值是 false，如果完成了，那个这个值为 true    
	  /// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pQueryOrder值无效。
	  /// \return void 

	  virtual void OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish);

	  ///	查询订单的返回事件

	  /// \brief	查询订单信息时候的回调，这里面也可能包含不是当前用户下的订单成交
	  /// \param  pAccount                        帐户ID 
	  /// \param  pQueryOrderExec	                查询订单成交的结构
	  /// \param  bFinish	                        如果没有传输完成，这个值是false，如果完成了，那个这个值为 true    
	  /// \remark 如果碰到 bFinish == true，那么是传输结束，并且pQueryOrderExec值无效。
	  /// \return void 

	  virtual void OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish);

	  ///	接收外部订单的消息

	  /// \brief	一般会在系统订单出错，进行人工调整的时候用到。
	  /// \param  pPostOrder	                    查询订单成交的结构
	  /// \return void 

	  virtual void OnPostOrder(EES_PostOrder* pPostOrder);

	  ///	接收外部订单成交的消息

	  /// \brief	一般会在系统订单出错，进行人工调整的时候用到。
	  /// \param  pPostOrderExecution	             查询订单成交的结构
	  /// \return void 

	  virtual void OnPostOrderExecution(EES_PostOrderExecution* pPostOrderExecution);

	  ///	查询交易所可用连接的响应

	  /// \brief	每个当前系统支持的汇报一次，当bFinish= true时，表示所有交易所的响应都已到达，但本条消息本身不包含有用的信息。
	  /// \param  pPostOrderExecution	             查询订单成交的结构
	  /// \return void 
	  virtual void OnQueryMarketSession(EES_ExchangeMarketSession* pMarketSession, bool bFinish);

	  ///	交易所连接状态变化报告，

	  /// \brief	当交易所连接发生连接/断开时报告此状态
	  /// \param  MarketSessionId: 交易所连接代码
	  /// \param  ConnectionGood: true表示交易所连接正常，false表示交易所连接断开了。
	  /// \return void 
	  virtual void OnMarketSessionStatReport(EES_MarketSessionId MarketSessionId, bool ConnectionGood);

	  ///	合约状态变化报告

	  /// \brief	当合约状态发生变化时报告
	  /// \param  pSymbolStatus: 参见EES_SymbolStatus合约状态结构体定义
	  /// \return void 
	  virtual void OnSymbolStatusReport(EES_SymbolStatus* pSymbolStatus);


	  ///	合约状态查询响应

	  /// \brief  响应合约状态查询请求
	  /// \param  pSymbolStatus: 参见EES_SymbolStatus合约状态结构体定义
	  /// \param	bFinish: 当为true时，表示查询所有结果返回。此时pSymbolStatus为空指针NULL
	  /// \return void 
	  virtual void OnQuerySymbolStatus(EES_SymbolStatus* pSymbolStatus, bool bFinish);
#endif

	  inline sl_input_inbound_queue* get_input_queue() { return &m_inputQueue; }
	  inline sl_order_inbound_queue* get_order_queue() { return &m_orderQueue; }
	  inline sl_trade_inbound_queue* get_trade_queue() { return &m_tradeQueue; }
	  inline sl_input_action_inbound_queue* get_input_action_queue() { return &m_inputActionQueue; }	  
	  std::string getMaturity(std::string& sMat);
	  void request_instruments();
	  void Process_api();
	  //end add on 20160929
   private:
	  string m_user_info_file_name;
	  bool m_bPosition = false;
	  string m_strQryServerIp;
	  string m_strQryServerPort;
   protected:      
	  virtual bool init_config(const std::string &name, const std::string &ini);      
	  void request_login();
	  string get_account()
	  {
		  return m_sUsername; 
	  }
   protected:      
	   void process() override;
	   //virtual void cancel_num_warning(tradeitem* i) override;
	   //virtual void cancel_num_ban(tradeitem* i) override;
	   EESTraderApi* m_pUserApi;
	   bool m_connectionStatus;
	   bool m_isAlive;
	   int m_nRequestId;
	   int m_nCurrentOrderRef;
	   sl_input_inbound_queue m_inputQueue;
	   sl_order_inbound_queue m_orderQueue;
	   sl_trade_inbound_queue m_tradeQueue;
	   sl_input_action_inbound_queue m_inputActionQueue;	   
	   sl_order_reject_inbound_queue m_rejectQueue;
	   sl_market_order_reject_inbound_queue m_marketRejectQueue;
	   sl_user_info_inbound_queue m_userInfoQueue;
	   //
	   int   MAX_RETRY_COUNT = 3;
	   int   m_retry_count;
	   //
#ifdef Linux
	   int efd;
	   void  init_epoll_eventfd();
#endif
   private:
	   //
	   EES_UserID m_UserId = -1;
       //
	   bool m_bTsession;	   
       friend class sl_order_aux;
	   //EES_ClientToken map to user info
	   tbb::concurrent_unordered_map<EES_ClientToken, user_info*> m_local_user_info_map;
	   //EES_MarketToken map to order id
	   //for the OnRtnOrderAsync,OnRspMarketRejectActionAsync:not include userid and clientToken
	   //and OnRspOrderInsertAsync:not include userid
	   tbb::concurrent_unordered_map<EES_MarketToken,int> m_ees_local_market_token_map;
	   //EES_ClientToken map to order id
	   tbb::concurrent_unordered_map<EES_ClientToken,int> m_ees_local_client_token_map;
	   //external server
	   tbb::concurrent_unordered_map<EES_MarketToken,user_info_ex*> m_external_user_info_map;
	   //    
   };
}

#endif // __OP_SL_CONNECTION_H__

