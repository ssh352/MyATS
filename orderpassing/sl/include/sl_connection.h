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
	  void OnRspOrderInsertAsync(EES_OrderAcceptField* pField);//��̨����
	  void OnRtnOrderAsync(EES_OrderMarketAcceptField * pField);//�г�����
	  void OnRtnTradeAsync(EES_OrderExecutionField* pTrade);//�ɽ�
	  void OnRspOrderActionAsync(EES_OrderCxled* pCancel);//����cancel
	  void OnRspRejectActionAsync(EES_OrderRejectField* pReject);//��̨�ܾ�
	  void OnRspMarketRejectActionAsync(EES_OrderMarketRejectField* pReject);//�г��ܾ�
	  void OnUserInfoAsync(user_info* pInfo);//�첽��¼userinfo
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
	  /// ������Ϣ�Ļص�

	  ///	\brief	�����������¼�
	  ///	\param  errNo                   ���ӳɹ���������Ϣ
	  ///	\param  pErrStr                 ������Ϣ
	  ///	\return void  

	  virtual void OnConnection(ERR_NO errNo, const char* pErrStr);

	  /// ���ӶϿ���Ϣ�Ļص�

	  /// \brief	�����������Ͽ������յ������Ϣ
	  /// \param  ERR_NO errNo         ���ӳɹ���������Ϣ
	  /// \param  const char* pErrStr  ������Ϣ
	  /// \return void  

	  virtual void OnDisConnection(ERR_NO errNo, const char* pErrStr);

	  /// ��¼��Ϣ�Ļص�

	  /// \param  pLogon                  ��¼�ɹ�����ʧ�ܵĽṹ
	  /// \return void 

	  virtual void OnUserLogon(EES_LogonResponse* pLogon);

	  /// �޸�������Ӧ�ص�

	  /// \param  nResult                  ��������Ӧ�ĳɹ���񷵻���
	  /// \return void 

	  virtual void OnRspChangePassword(EES_ChangePasswordResult nResult);

	  /// ��ѯ�û������ʻ��ķ����¼�

	  /// \param  pAccountInfo	        �ʻ�����Ϣ
	  /// \param  bFinish	                ���û�д�����ɣ����ֵ�� false ���������ˣ��Ǹ����ֵΪ true 
	  /// \remark ������� bFinish == true����ô�Ǵ������������ pAccountInfoֵ��Ч��
	  /// \return void 

	  virtual void OnQueryUserAccount(EES_AccountInfo * pAccoutnInfo, bool bFinish);

	  /// ��ѯ�ʻ������λ��Ϣ�ķ����¼�

	  /// \param  pAccount	                �ʻ�ID 	
	  /// \param  pAccoutnPosition	        �ʻ��Ĳ�λ��Ϣ					   
	  /// \param  nReqId		                ����������Ϣʱ���ID�š�
	  /// \param  bFinish	                    ���û�д�����ɣ����ֵ��false���������ˣ��Ǹ����ֵΪ true 
	  /// \remark ������� bFinish == true����ô�Ǵ������������ pAccountInfoֵ��Ч��
	  /// \return void 

	  virtual void OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish);


	  /// ��ѯ�ʻ������ʽ���Ϣ�ķ����¼�

	  /// \param  pAccount	                �ʻ�ID 	
	  /// \param  pAccoutnPosition	        �ʻ��Ĳ�λ��Ϣ					   
	  /// \param  nReqId		                ����������Ϣʱ���ID��
	  /// \return void 

	  virtual void OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccoutnPosition, int nReqId);

	  /// ��ѯ��Լ�б�ķ����¼�

	  /// \param  pSymbol	                    ��Լ��Ϣ   
	  /// \param  bFinish	                    ���û�д�����ɣ����ֵ�� false���������ˣ��Ǹ����ֵΪ true   
	  /// \remark ������� bFinish == true����ô�Ǵ������������ pSymbol ֵ��Ч��
	  /// \return void 

	  virtual void OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish);

	  /// ��ѯ�ʻ����ױ�֤��ķ����¼�

	  /// \param  pAccount                    �ʻ�ID 
	  /// \param  pSymbolMargin               �ʻ��ı�֤����Ϣ 
	  /// \param  bFinish	                    ���û�д�����ɣ����ֵ�� false�������ɣ��Ǹ����ֵΪ true 
	  /// \remark ������� bFinish == true����ô�Ǵ������������ pSymbolMargin ֵ��Ч��
	  /// \return void 

	  virtual void OnQueryAccountTradeMargin(const char* pAccount, EES_AccountMargin* pSymbolMargin, bool bFinish);

	  /// ��ѯ�ʻ����׷��õķ����¼�

	  /// \param  pAccount                    �ʻ�ID 
	  /// \param  pSymbolFee	                �ʻ��ķ�����Ϣ	 
	  /// \param  bFinish	                    ���û�д�����ɣ����ֵ�� false���������ˣ��Ǹ����ֵΪ true    
	  /// \remark ������� bFinish == true ����ô�Ǵ������������ pSymbolFee ֵ��Ч��
	  /// \return void 

	  virtual void OnQueryAccountTradeFee(const char* pAccount, EES_AccountFee* pSymbolFee, bool bFinish);

	  /// �µ�����̨ϵͳ���ܵ��¼�

	  /// \brief ��ʾ��������Ѿ�����̨ϵͳ��ʽ�Ľ���
	  /// \param  pAccept	                    �����������Ժ����Ϣ��
	  /// \return void 

	  virtual void OnOrderAccept(EES_OrderAcceptField* pAccept);


	  /// �µ����г����ܵ��¼�

	  /// \brief ��ʾ��������Ѿ�����������ʽ�Ľ���
	  /// \param  pAccept	                    �����������Ժ����Ϣ�壬����������г�����ID
	  /// \return void 
	  virtual void OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept);


	  ///	�µ�����̨ϵͳ�ܾ����¼�

	  /// \brief	��������̨ϵͳ�ܾ������Բ鿴�﷨�����Ƿ�ؼ�顣 
	  /// \param  pReject	                    �����������Ժ����Ϣ��
	  /// \return void 

	  virtual void OnOrderReject(EES_OrderRejectField* pReject);


	  ///	�µ����г��ܾ����¼�

	  /// \brief	�������г��ܾ������Բ鿴�﷨�����Ƿ�ؼ�顣 
	  /// \param  pReject	                    �����������Ժ����Ϣ�壬����������г�����ID
	  /// \return void 

	  virtual void OnOrderMarketReject(EES_OrderMarketRejectField* pReject);


	  ///	�����ɽ�����Ϣ�¼�

	  /// \brief	�ɽ���������˶����г�ID�����������ID��ѯ��Ӧ�Ķ���
	  /// \param  pExec	                   �����������Ժ����Ϣ�壬����������г�����ID
	  /// \return void 

	  virtual void OnOrderExecution(EES_OrderExecutionField* pExec);

	  ///	�����ɹ������¼�

	  /// \brief	�ɽ���������˶����г�ID�����������ID��ѯ��Ӧ�Ķ���
	  /// \param  pCxled		               �����������Ժ����Ϣ�壬����������г�����ID
	  /// \return void 

	  virtual void OnOrderCxled(EES_OrderCxled* pCxled);

	  ///	�������ܾ�����Ϣ�¼�

	  /// \brief	һ����ڷ��ͳ����Ժ��յ������Ϣ����ʾ�������ܾ�
	  /// \param  pReject	                   �������ܾ���Ϣ��
	  /// \return void 

	  virtual void OnCxlOrderReject(EES_CxlOrderRej* pReject);

	  ///	��ѯ�����ķ����¼�

	  /// \brief	��ѯ������Ϣʱ��Ļص���������Ҳ���ܰ������ǵ�ǰ�û��µĶ���
	  /// \param  pAccount                 �ʻ�ID 
	  /// \param  pQueryOrder	             ��ѯ�����Ľṹ
	  /// \param  bFinish	                 ���û�д�����ɣ����ֵ�� false���������ˣ��Ǹ����ֵΪ true    
	  /// \remark ������� bFinish == true����ô�Ǵ������������ pQueryOrderֵ��Ч��
	  /// \return void 

	  virtual void OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish);

	  ///	��ѯ�����ķ����¼�

	  /// \brief	��ѯ������Ϣʱ��Ļص���������Ҳ���ܰ������ǵ�ǰ�û��µĶ����ɽ�
	  /// \param  pAccount                        �ʻ�ID 
	  /// \param  pQueryOrderExec	                ��ѯ�����ɽ��Ľṹ
	  /// \param  bFinish	                        ���û�д�����ɣ����ֵ��false���������ˣ��Ǹ����ֵΪ true    
	  /// \remark ������� bFinish == true����ô�Ǵ������������pQueryOrderExecֵ��Ч��
	  /// \return void 

	  virtual void OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish);

	  ///	�����ⲿ��������Ϣ

	  /// \brief	һ�����ϵͳ�������������˹�������ʱ���õ���
	  /// \param  pPostOrder	                    ��ѯ�����ɽ��Ľṹ
	  /// \return void 

	  virtual void OnPostOrder(EES_PostOrder* pPostOrder);

	  ///	�����ⲿ�����ɽ�����Ϣ

	  /// \brief	һ�����ϵͳ�������������˹�������ʱ���õ���
	  /// \param  pPostOrderExecution	             ��ѯ�����ɽ��Ľṹ
	  /// \return void 

	  virtual void OnPostOrderExecution(EES_PostOrderExecution* pPostOrderExecution);

	  ///	��ѯ�������������ӵ���Ӧ

	  /// \brief	ÿ����ǰϵͳ֧�ֵĻ㱨һ�Σ���bFinish= trueʱ����ʾ���н���������Ӧ���ѵ����������Ϣ�����������õ���Ϣ��
	  /// \param  pPostOrderExecution	             ��ѯ�����ɽ��Ľṹ
	  /// \return void 
	  virtual void OnQueryMarketSession(EES_ExchangeMarketSession* pMarketSession, bool bFinish);

	  ///	����������״̬�仯���棬

	  /// \brief	�����������ӷ�������/�Ͽ�ʱ�����״̬
	  /// \param  MarketSessionId: ���������Ӵ���
	  /// \param  ConnectionGood: true��ʾ����������������false��ʾ���������ӶϿ��ˡ�
	  /// \return void 
	  virtual void OnMarketSessionStatReport(EES_MarketSessionId MarketSessionId, bool ConnectionGood);

	  ///	��Լ״̬�仯����

	  /// \brief	����Լ״̬�����仯ʱ����
	  /// \param  pSymbolStatus: �μ�EES_SymbolStatus��Լ״̬�ṹ�嶨��
	  /// \return void 
	  virtual void OnSymbolStatusReport(EES_SymbolStatus* pSymbolStatus);


	  ///	��Լ״̬��ѯ��Ӧ

	  /// \brief  ��Ӧ��Լ״̬��ѯ����
	  /// \param  pSymbolStatus: �μ�EES_SymbolStatus��Լ״̬�ṹ�嶨��
	  /// \param	bFinish: ��Ϊtrueʱ����ʾ��ѯ���н�����ء���ʱpSymbolStatusΪ��ָ��NULL
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

