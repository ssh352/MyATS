#include "femas_api.h"
#include "femas_connection.h"
#include "string_tokenizer.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
using namespace terra::common;
namespace femas
{
   femas_api::femas_api(femas_connection* pConnection)
   {
      m_pConnection = pConnection;
      m_connectionStatus = false;
      m_isAlive = true;      
      m_nRequestId = 0;
      m_nCurrentOrderRef = 0;
	  m_begin_id = order_reference_provider::get_instance().get_current_int();
   }

   femas_api::~femas_api()
   {
      //delete m_pUserApi;
   }

   void femas_api::init()
   {
     m_pUserApi = CUstpFtdcTraderApi::CreateFtdcTraderApi();

	 m_inputQueue.setHandler(boost::bind(&femas_connection::OnRspOrderInsertAsync, m_pConnection, _1));
	 //m_inputQuoteQueue.setHandler(boost::bind(&femas_connection::OnRspQuoteInsertAsync, m_pConnection, _1));

	 m_orderQueue.setHandler(boost::bind(&femas_connection::OnRtnOrderAsync, m_pConnection, _1));
	 //m_quoteQueue.setHandler(boost::bind(&femas_connection::OnRtnQuoteAsync, m_pConnection, _1));

	 m_tradeQueue.setHandler(boost::bind(&femas_connection::OnRtnTradeAsync, m_pConnection, _1));

	 m_inputActionQueue.setHandler(boost::bind(&femas_connection::OnRspOrderActionAsync, m_pConnection, _1));
	 //m_inputActionQuoteQueue.setHandler(boost::bind(&femas_connection::OnRspQuoteActionAsync, m_pConnection, _1));

	 m_userInfoQueue.setHandler(boost::bind(&femas_api::OnUserInfoAsync, this, _1));

   }
   
   void femas_api::release()
   {     
      m_pUserApi->Release();
   }
     
   void femas_api::Process()
   {
	   m_inputQueue.Pops_Handle(0);
	   //m_inputQuoteQueue.Pops_Handle(0);

	   m_orderQueue.Pops_Handle(0);
	   //m_quoteQueue.Pops_Handle(0);

	   m_tradeQueue.Pops_Handle(0);
	   m_inputActionQueue.Pops_Handle(0);
	   //m_inputActionQuoteQueue.Pops_Handle(0);
	   m_userInfoQueue.Pops_Handle(0);
   }

   int femas_api::get_ord_ref_from_reqid(int nReqId)
   {
	   auto search = m_ordInputActiondRefMap.find(nReqId);
	   if (search != m_ordInputActiondRefMap.end()) 
	   {
		   int i = search->second;
		   m_ordInputActiondRefMap.erase(search);
		   return i;
	   }
	   else 
	   {
		   return 0;
	   }
   }

   bool femas_api::connect()
   {
	   //loggerv2::info("calling femas_api::connect");
      // For first connection, we are disconnected so we need to connect API first (RequestLogin will be done on API UP).
      // For later connections (disconnect / reconnect), API is already up so we just need to relogin.
      //
      if (m_connectionStatus == false)
      {
         char addr[1024 + 1];
         snprintf(addr, 1024, "%s:%s", m_pConnection->m_sHostname.c_str(), m_pConnection->m_sService.c_str());
         m_pUserApi->RegisterSpi(this);         
		 m_pUserApi->RegisterFront(addr);
    	 loggerv2::info("femas_api::connect connecting to %s", addr);
		 printf_ex("femas_api::connect connecting to %s\n", addr);
		 switch (m_pConnection->getResynchronizationMode())
         {
		 case ResynchronizationMode::None:
			 m_pUserApi->SubscribePrivateTopic(USTP_TERT_QUICK);
            break;
		 case ResynchronizationMode::Last:
            m_pUserApi->SubscribePrivateTopic(USTP_TERT_RESUME);
            break;
         default:
		 case ResynchronizationMode::Full:
            m_pUserApi->SubscribePrivateTopic(USTP_TERT_RESTART);
            break;
         }
		 loggerv2::info("femas_api::connect initializing api");
         m_pUserApi->Init();
		 loggerv2::info("femas_api::connect api intialized");
      }
      else
      {
         request_login();
      }
      return true;
   }

   bool femas_api::disconnect()
   {
	  CUstpFtdcReqUserLogoutField request;
      memset(&request, 0, sizeof(request));
      strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
	  strcpy(request.UserID, m_pConnection->get_login_id().c_str());
      int res = m_pUserApi->ReqUserLogout(&request, ++m_nRequestId);
      if (res != 0)
      {
         return false;
      }
      return true;
   }

   void femas_api::request_login()
   {
      loggerv2::info("femas_api::request_login");
      CUstpFtdcReqUserLoginField request;
      memset(&request, 0, sizeof(request));
      strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
	  strcpy(request.UserID, m_pConnection->get_login_id().c_str());
      strcpy(request.Password, m_pConnection->m_sPassword.c_str());
	  //strcpy(request.UserProductInfo, "Terra");
      int res = m_pUserApi->ReqUserLogin(&request, ++m_nRequestId);
      if (res != 0)
      {
		  m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "femas_api - ReqUserLogin failed");
      }
   }

   bool femas_api::ReqQryInvestorAccount(CUstpFtdcQryInvestorAccountField* pQryInvestorAccountField)
   {
	   int ret = m_pUserApi->ReqQryInvestorAccount(pQryInvestorAccountField, ++m_nRequestId);
	   loggerv2::info("femas_api::ReqQryInvestorAccount brokerid %s , requestId %d,ret:%d", pQryInvestorAccountField->BrokerID, m_nRequestId,ret);
	   if (ret != 0)
	   {
		   return false;
	   }
	   return true;   
   }

   void femas_api::OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pTradingAccount, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
   {
	   if (pTradingAccount == nullptr)
		   return;

	   loggerv2::info("femas_api::OnRspQryInvestorAccount,"

		   //"PreMortgage[%f]"
		   //"PreCredit[%f]"
		   //"PreDeposit[%f]"
		   "PreBalance[%f]"
		   //"PreMargin[%f]"
		   //"Interest[%f]"
		   "Deposit[%f]"
		   "Withdraw[%f]"
		   "FrozenMargin[%f]"
		   "FrozenPremium[%f]"
		   "FrozenFee[%f]"
		   "Margin[%f]"
		   "TodayInOut[%f]"
		   "Fee[%f]"
		   //"Balance[%f]"
		   "Available[%f]"
		   //"WithdrawQuota[%f]"
		   //"Reserve[%f]"
		   //"Premium[%f]"
		   //"Mortgage[%f]"
		   //"ExchangeMargin[%f]"
		   "Risk[%f]",

		   //pTradingAccount->PreMortgage,
		   //pTradingAccount->PreCredit,
		   //pTradingAccount->PreDeposit,
		   pTradingAccount->PreBalance,
		   //pTradingAccount->PreMargin,
		   //pTradingAccount->Interest,
		   pTradingAccount->Deposit,
		   pTradingAccount->Withdraw,
		   pTradingAccount->FrozenMargin,
		   pTradingAccount->FrozenPremium,
		   pTradingAccount->FrozenFee,
		   pTradingAccount->Margin,
		   pTradingAccount->TodayInOut,
		   pTradingAccount->Fee,
		   //pTradingAccount->Balance,
		   pTradingAccount->Available,
		   pTradingAccount->Risk/100
		   //pTradingAccount->WithdrawQuota,
		   //pTradingAccount->Reserve,
		   //pTradingAccount->Premium,
		   //pTradingAccount->Mortgage,
		   //pTradingAccount->ExchangeMargin
		   );

	   //
	   m_pConnection->set_RiskDegree(pTradingAccount->Risk/100);
	   //

	   tradingaccount* ta = new tradingaccount(
		   m_pConnection->getName(),
		   pTradingAccount->AccountID,
		   0,
		   0,
		   0,
		   pTradingAccount->PreBalance,
		   0,
		   0,
		   pTradingAccount->Deposit,
		   pTradingAccount->Withdraw,
		   pTradingAccount->FrozenMargin,
		   pTradingAccount->FrozenPremium,
		   pTradingAccount->FrozenFee,
		   pTradingAccount->Margin,
		   pTradingAccount->TodayInOut,
		   pTradingAccount->Fee,
		   0,//pTradingAccount->Balance,
		   pTradingAccount->Available,
		   0,//pTradingAccount->WithdrawQuota,
		   0,//pTradingAccount->Reserve,
		   0,//pTradingAccount->Credit,
		   0,//pTradingAccount->Mortgage,
		   0);//pTradingAccount->ExchangeMargin);	   

	   m_pConnection->on_trading_account_cb(ta);

	   delete ta;
   }

   void femas_api::OnRspError(CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
   {
      if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
      {
         loggerv2::error("femas_api::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
      }
      else
      {
         loggerv2::info("femas_api::OnRspError - ok");
      }
   }

   void femas_api::OnFrontConnected()
   {
      loggerv2::info("femas_api::femas_api is UP");
      m_connectionStatus = true;
	  if (m_pConnection->getStatus() == AtsType::ConnectionStatus::WaitConnect)
      {
        request_login();
      }
      else
      {
		  loggerv2::info("femas_api will asking for reconnect...");
		  request_login();
      }
   }

   bool femas_api::ReqQryInvestorPosition(CUstpFtdcQryInvestorPositionField *pQryInvestorPosition)
   {  
	   //loggerv2::info("calling femas_api::ReqQryInvestorPosition");
	   int ret = m_pUserApi->ReqQryInvestorPosition(pQryInvestorPosition, ++m_nRequestId);
	   loggerv2::info("femas_api::ReqQryInvestorPosition instr %s , requestId %d", pQryInvestorPosition->InstrumentID,m_nRequestId);
	   if (ret != 0)
	   {
		   return false;
	   }
	   return true;
	}

   void femas_api::OnFrontDisconnected(int nReason)
   {
      //loggerv2::info("femas_api is DOWN");
      m_connectionStatus = false;
      char* pszMessage;
	  /*
	    0x1001 网络读失败
		0x1002 网络写失败
		0x2001 接收心跳超时
		0x2002 发送心跳失败
		0x2003 收到错误报文
	  */
	  
	  switch (nReason)
      {
         // normal disconnection
      case 0:
         pszMessage = "";
         break;
         // error
	  case 0x1001:
		  pszMessage = "network reading failed";
		  break;
	  case 0x1002:
		  pszMessage = "network writing failed";
		  break;
	  case 0x2001:
		  pszMessage = "heartbeat timeout";
		  break;
	  case 0x2002:
		  pszMessage = "sending timeout";
		  break;
	  case 0x2003:
         //pszMessage = "ERROR MSG TO TRANSLATE [" + nReason + "];
         pszMessage = "ERROR MSG TO TRANSLATE";
         break;
      default:
         //pszMessage = "unknown error [" + nReason + "];
         pszMessage = "unknown error";
         break;
      }
	  loggerv2::warn("Front Disconnected ! Error Msg %s",pszMessage);
	  printf_ex("femas_api::OnFrontDisconnected Front Disconnected ! Error Msg %s", pszMessage);
	  m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage);
   }

   void femas_api::OnHeartBeatWarning(int nTimeLapse)
   {
      loggerv2::info("femas_api - heartbeat warning");
   }

   void femas_api::OnRspUserLogin(CUstpFtdcRspUserLoginField* pRspUserLogin, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
   {
      loggerv2::info("femas_api::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	  printf_ex("femas_api::OnRspUserLogin - (%d, %s)\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
      if (pRspInfo->ErrorID == 0)
      {
		 m_nCurrentOrderRef = atoi(pRspUserLogin->MaxOrderLocalID);
		 loggerv2::error("femas_api::OnRspUserLogin max order local id [%s],m_nCurrentOrderRef:%d", pRspUserLogin->MaxOrderLocalID, m_nCurrentOrderRef);
		 printf_ex("femas_api::OnRspUserLogin max order local id [%s],m_nCurrentOrderRef:%d\n", pRspUserLogin->MaxOrderLocalID, m_nCurrentOrderRef);
		 m_pConnection->on_status_changed(AtsType::ConnectionStatus::Connected, "femas_api - OnRspUserLogin with error id =0. ");

		 loggerv2::info("Going to request investor full position.");
		 if (m_pConnection->m_bRequestPosition == true)
		 {
		 m_pConnection->request_investor_full_positions();
		 }
		 else if (m_pConnection->m_bRequestInstruments == true)
		 {
			 request_instruments();
		 }
      }
      else
      {
		  m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pRspInfo->ErrorMsg);
      }
   }

   void femas_api::OnRspUserLogout(CUstpFtdcRspUserLogoutField* pUserLogout, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
   {
	  if (pRspInfo->ErrorID == 0)
		  m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "femas_api::OnRspUserLogout Receive Logout Msg Error Id 0");
	  else
		  loggerv2::error("femas_api::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);
   }
   /*
   ///合约查询应答
   struct CUstpFtdcRspInstrumentField
   {
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///品种代码
   TUstpFtdcProductIDType	ProductID;
   ///品种名称
   TUstpFtdcProductNameType	ProductName;
   ///合约代码
   TUstpFtdcInstrumentIDType	InstrumentID;
   ///合约名称
   TUstpFtdcInstrumentNameType	InstrumentName;
   ///交割年份
   TUstpFtdcYearType	DeliveryYear;
   ///交割月
   TUstpFtdcMonthType	DeliveryMonth;
   ///限价单最大下单量
   TUstpFtdcVolumeType	MaxLimitOrderVolume;
   ///限价单最小下单量
   TUstpFtdcVolumeType	MinLimitOrderVolume;
   ///市价单最大下单量
   TUstpFtdcVolumeType	MaxMarketOrderVolume;
   ///市价单最小下单量
   TUstpFtdcVolumeType	MinMarketOrderVolume;
   ///数量乘数
   TUstpFtdcVolumeMultipleType	VolumeMultiple;
   ///报价单位
   TUstpFtdcPriceTickType	PriceTick;
   ///币种
   TUstpFtdcCurrencyType	Currency;
   ///多头限仓
   TUstpFtdcVolumeType	LongPosLimit;
   ///空头限仓
   TUstpFtdcVolumeType	ShortPosLimit;
   ///跌停板价
   TUstpFtdcPriceType	LowerLimitPrice;
   ///涨停板价
   TUstpFtdcPriceType	UpperLimitPrice;
   ///昨结算
   TUstpFtdcPriceType	PreSettlementPrice;
   ///合约交易状态
   TUstpFtdcInstrumentStatusType	InstrumentStatus;
   ///创建日
   TUstpFtdcDateType	CreateDate;
   ///上市日
   TUstpFtdcDateType	OpenDate;
   ///到期日
   TUstpFtdcDateType	ExpireDate;
   ///开始交割日
   TUstpFtdcDateType	StartDelivDate;
   ///最后交割日
   TUstpFtdcDateType	EndDelivDate;
   ///挂牌基准价
   TUstpFtdcPriceType	BasisPrice;
   ///当前是否交易
   TUstpFtdcBoolType	IsTrading;
   ///基础商品代码
   TUstpFtdcInstrumentIDType	UnderlyingInstrID;
   ///基础商品乘数
   TUstpFtdcUnderlyingMultipleType	UnderlyingMultiple;
   ///持仓类型
   TUstpFtdcPositionTypeType	PositionType;
   ///执行价
   TUstpFtdcPriceType	StrikePrice;
   ///期权类型
   TUstpFtdcOptionsTypeType	OptionsType;
   ///币种代码
   TUstpFtdcCurrencyIDType	CurrencyID;
   ///策略类别
   TUstpFtdcArbiTypeType	ArbiType;
   ///第一腿合约代码
   TUstpFtdcInstrumentIDType	InstrumentID_1;
   ///第一腿买卖方向
   TUstpFtdcDirectionType	Direction_1;
   ///第一腿数量比例
   TUstpFtdcRatioType	Ratio_1;
   ///第二腿合约代码
   TUstpFtdcInstrumentIDType	InstrumentID_2;
   ///第二腿买卖方向
   TUstpFtdcDirectionType	Direction_2;
   ///第二腿数量比例
   TUstpFtdcRatioType	Ratio_2;
   };
   */
   void femas_api::OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
   {
	   m_pConnection->OnRspQryInstrument(pRspInstrument, pRspInfo, nRequestID, bIsLast);
   }	   
   /*
   ///投资者持仓查询应答
   struct CUstpFtdcRspInvestorPositionField
   {
   ///投资者编号
   TUstpFtdcInvestorIDType	InvestorID;
   ///经纪公司编号
   TUstpFtdcBrokerIDType	BrokerID;
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///客户代码
   TUstpFtdcClientIDType	ClientID;
   ///合约代码
   TUstpFtdcInstrumentIDType	InstrumentID;
   ///买卖方向
   TUstpFtdcDirectionType	Direction;
   ///投机套保标志
   TUstpFtdcHedgeFlagType	HedgeFlag;
   ///占用保证金
   TUstpFtdcMoneyType	UsedMargin;
   ///今总持仓量
   TUstpFtdcVolumeType	Position;
   ///今日持仓成本
   TUstpFtdcPriceType	PositionCost;
   ///昨持仓量
   TUstpFtdcVolumeType	YdPosition;
   ///昨日持仓成本
   TUstpFtdcMoneyType	YdPositionCost;
   ///冻结的保证金
   TUstpFtdcMoneyType	FrozenMargin;
   ///开仓冻结持仓
   TUstpFtdcVolumeType	FrozenPosition;
   ///平仓冻结持仓
   TUstpFtdcVolumeType	FrozenClosing;
   ///平昨仓冻结持仓
   TUstpFtdcVolumeType	YdFrozenClosing;
   ///冻结的权利金
   TUstpFtdcMoneyType	FrozenPremium;
   
   ///最后一笔成交编号
   TUstpFtdcTradeIDType	LastTradeID;
   ///最后一笔本地报单编号
   TUstpFtdcOrderLocalIDType	LastOrderLocalID;

   ///投机持仓量
   TUstpFtdcVolumeType	SpeculationPosition;
   ///套利持仓量
   TUstpFtdcVolumeType	ArbitragePosition;
   ///套保持仓量
   TUstpFtdcVolumeType	HedgePosition;
   ///投机平仓冻结量
   TUstpFtdcVolumeType	SpecFrozenClosing;
   ///套保平仓冻结量
   TUstpFtdcVolumeType	HedgeFrozenClosing;
   ///币种
   TUstpFtdcCurrencyIDType	Currency;
   };
   */
   void femas_api::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
   {
	   if (bIsLast == true)
	   {
		   if (m_pConnection->getRequestInstruments() == true)
		   {
			   sleep_by_milliseconds(2000);
			   this->request_instruments();
			   m_pConnection->setRequestInstruments(false);
		   }
		   else
		   {
			   sleep_by_milliseconds(2000);
			   m_pConnection->request_trading_account();
		   }
	   }
	   if (pInvestorPosition && strlen(pInvestorPosition->InstrumentID)>0)
	   {
	   loggerv2::info("Calling OnRspQryInvestorPosition "
		   "InstrumentID[%s]"
		   "ExchangeID[%s]"
		   "Direction[%c]"
		   "Position[%d]"
		   "YdPosition[%d]"
		   "FrozenPosition[%d]"
		   "FrozenClosing[%d]"
		   "FrozenMargin[%f]"
		   "HedgeFlag[%c]"
		   "UsedMargin[%f]",
		   pInvestorPosition->InstrumentID,
		   pInvestorPosition->ExchangeID,
		   pInvestorPosition->Direction,
			   pInvestorPosition->Position,   ///今总持仓量
			   pInvestorPosition->YdPosition, ///昨持仓量
			   pInvestorPosition->FrozenPosition,///开仓冻结持仓
			   pInvestorPosition->FrozenClosing, ///平仓冻结持仓
			   pInvestorPosition->FrozenMargin,  ///冻结的保证金
		   pInvestorPosition->HedgeFlag,
		   pInvestorPosition->UsedMargin
		   );

		   std::string sInstrCode = std::string(pInvestorPosition->InstrumentID) + "@" + m_pConnection->getName();
		   tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		   //bool islog = false;
		   if (i)
		   {
			   if (pInvestorPosition->Direction == USTP_FTDC_D_Buy)
			   {
				   if (i->get_today_long_position() != pInvestorPosition->Position - pInvestorPosition->YdPosition)
				   {
					   loggerv2::info("femas_api::OnRspQryInvestorPosition:today_long_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_today_long_position(pInvestorPosition->Position - pInvestorPosition->YdPosition);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();
						//islog = true;
				   }
				   if (i->get_yst_long_position() != pInvestorPosition->YdPosition)
				   {
					   loggerv2::info("femas_api::OnRspQryInvestorPosition:yst_long_position change,Code=%s", i->getCode().c_str());
					   i->dumpinfo();
					   i->set_yst_long_position(pInvestorPosition->YdPosition);
					   i->set_last_sychro_timepoint(get_lwtp_now());
					   i->dumpinfo();
					   //islog = true;
				   }
			   }
			   else if  (pInvestorPosition->Direction == USTP_FTDC_D_Sell)
			   {
				   if (i->get_today_short_position() != pInvestorPosition->Position - pInvestorPosition->YdPosition)
				   {
					   loggerv2::info("femas_api::OnRspQryInvestorPosition:today_short_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_today_short_position(pInvestorPosition->Position - pInvestorPosition->YdPosition);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();
						//islog = true;
				   }
				   if (i->get_yst_short_position() != pInvestorPosition->YdPosition)
				   {
						loggerv2::info("femas_api::OnRspQryInvestorPosition:yst_short_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
					    i->set_yst_short_position(pInvestorPosition->YdPosition);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();
						//islog = true;
					}
			   }
			   if (i->get_tot_long_position() != i->get_today_long_position() + i->get_yst_long_position())
			   {
				   loggerv2::info("femas_api::OnRspQryInvestorPosition:tot_long_position change,Code=%s", i->getCode().c_str());
				   i->dumpinfo();
				   i->set_tot_long_position(i->get_today_long_position() + i->get_yst_long_position());
				   i->set_last_sychro_timepoint(get_lwtp_now());
				   i->dumpinfo();
				   //islog = true;
			   }
			   if (i->get_tot_short_position() != i->get_today_short_position() + i->get_yst_short_position())
			   {
				   loggerv2::info("femas_api::OnRspQryInvestorPosition:tot_short_position change,Code=%s", i->getCode().c_str());
				   i->dumpinfo();
				   i->set_tot_short_position(i->get_today_short_position() + i->get_yst_short_position());
				   i->set_last_sychro_timepoint(get_lwtp_now());
				   i->dumpinfo();
				   //islog = true;
			   }

			   //if (islog)
				  // i->set_last_sychro_timepoint(get_lwtp_now());

		   }
		   else
		   {
			   loggerv2::info("femas_api::OnRspQryInvestorPosition cannot find tradeitem %s by second key", std::string(pInvestorPosition->InstrumentID).c_str());
		   }
	   }
	   if (bIsLast == true)
	   {
		   loggerv2::info("femas_api::OnRspQryInvestorPosition bIsLast == true");
#if 0
		   for (auto &it : tradeitem_gh::get_instance().container().get_map())
		   {
		   it.second->dumpinfo();
		   }
#endif
	   }
   }
   void femas_api::OnRspOrderInsert(CUstpFtdcInputOrderField* pOrder, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
   {
       // use unused field to store errorId
	   pOrder->IsAutoSuspend = pRspInfo->ErrorID;
	   m_inputQueue.CopyPush(pOrder);
   }
   void femas_api::OnErrRtnOrderInsert(CUstpFtdcInputOrderField* pInputOrder, CUstpFtdcRspInfoField* pRspInfo)
   {  
	   if (pInputOrder)
	   {
	   loggerv2::info("femas_api::OnErrRtnOrderInsert"
		   "OrderSysID[%s]"
		   "UserOrderLocalID[%s]"
			   "InstrumentID[%s]"
			   "UserCustom[%s]",
		   pInputOrder->OrderSysID,
		   pInputOrder->UserOrderLocalID,
			   pInputOrder->InstrumentID,
			   pInputOrder->UserCustom
		   );   
	   }
	   if (pRspInfo)
	   {
		   loggerv2::info("femas_api::OnErrRtnOrderInsert "
			   "ErrorID[%d]"
			   "ErrorMsg[%s]",
			   pRspInfo->ErrorID,
			   pRspInfo->ErrorMsg);
	   }
	   //to do ...

   }
   void femas_api::OnErrRtnOrderAction(CUstpFtdcOrderActionField* pInputOrderAction, CUstpFtdcRspInfoField* pRspInfo)
   {
	   loggerv2::info("femas_api::OnErrRtnOrderAction --> need to implement,UserOrderLocalID:%s,OrderSysID:%s", pInputOrderAction->UserOrderLocalID, pInputOrderAction->OrderSysID);
	   //to do ...

   }
   void femas_api::OnRspOrderAction(CUstpFtdcOrderActionField* pInputOrderAction, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
   {
	   //loggerv2::info("calling femas_api::OnRspOrderAction . request id=[%d] --> need to implement",nRequestID);
	   if (!bIsLast)
	   {
		   return;
	   }
	   loggerv2::info("calling femas_api::OnRspOrderAction");
	   loggerv2::info("femas_api::OnRspOrderAction pInputOrderAction - "
		   "UserOrderActionLocalID[%s]"
		   "UserOrderLocalID[%s]"
		   "ExchangeID[%s]"
		   "OrdSysID[%s]"
		   "ActionFlag[%c]"
		   "LimitPrice[%f]"
		   "VolumeChange[%d]"
		   "UserID[%s]"
		   "Investor[%s]",

		   pInputOrderAction->UserOrderActionLocalID,
		   pInputOrderAction->UserOrderLocalID,
		   pInputOrderAction->ExchangeID,
		   pInputOrderAction->OrderSysID,
		   pInputOrderAction->ActionFlag,
		   pInputOrderAction->LimitPrice,
		   pInputOrderAction->VolumeChange,
		   pInputOrderAction->UserID,
		   pInputOrderAction->InvestorID
		   );
	   loggerv2::info("femas_api::OnRspOrderActionpRspInfo - "
		   "ErrorID[%d]"
		   "ErrorMsg[%s]",
		   pRspInfo->ErrorID,
		   pRspInfo->ErrorMsg
		   );
	   /*
	   撤单请求有错
	   */
	   if (pRspInfo->ErrorID != 0)
	   {
		   int orderRef = get_ord_ref_from_reqid(nRequestID);
		   if (!orderRef)
		   {
			   loggerv2::error("could not find associated order for nRequestId %d", nRequestID);
			   return;
		   }
		   int* i = new int(orderRef);
		   m_inputActionQueue.Push(i);
	   }
   }  
   void femas_api::OnRtnOrder(CUstpFtdcOrderField* pOrder)
   {      
	  m_orderQueue.CopyPush(pOrder);
   }
   void femas_api::OnRtnTrade(CUstpFtdcTradeField* pTrade)
   {      
	  m_tradeQueue.CopyPush(pTrade);
   }   
   //
   // prder sending
   //
   bool femas_api::ReqOrderInsert(CUstpFtdcInputOrderField* pRequest)
   {
      //loggerv2::info("sending order to api.");
      int ret = m_pUserApi->ReqOrderInsert(pRequest, ++m_nRequestId);
      if (ret != 0)
      {
		  loggerv2::error("fail to femas_api::ReqOrderInsert,ret:%d,%s",ret, pRequest->UserCustom);
         return false;
      }
	  create_user_info(pRequest);
      return true;
   }
   
   bool femas_api::ReqOrderAction(CUstpFtdcOrderActionField* pRequest)
   {
   	  int ret = m_pUserApi->ReqOrderAction(pRequest, ++m_nRequestId);
	  if (ret != 0)
      {
		 loggerv2::error("fail to femas_api::ReqOrderAction,ret:%d",ret);
         return false;
      }	  
	  //need to found a way to deal with this field.	  
	  string userId = this->get_user_id(pRequest->UserOrderLocalID);
	  int nOrderId = m_pConnection->get_order_id(userId.c_str());
	  m_ordInputActiondRefMap.insert(std::pair<int, int>(m_nRequestId, nOrderId));
      return true;
   }   
   void femas_api::init_user_info(char * user_info_file)
   {
	   if (user_info_file == nullptr)
		   return;
	   boost::filesystem::path p;
	   p.clear();
	   p.append(user_info_file);
	   p.append("user_info.csv");
	   m_user_info_file_name = p.string();
	   printf_ex("femas_api::init_user_info filename:%s\n", m_user_info_file_name.c_str());
	   if (!boost::filesystem::exists(p))
		   return;
	   boost::filesystem::ifstream stream;
	   stream.open(m_user_info_file_name.c_str());
	   string_tokenizer<1024> tokenizer;
	   const char* szSeparators = ",";
	   std::string line;
	   while (stream.good())
	   {
		   std::getline(stream, line);
		   if (line.length() == 0 || line[0] == '#')
			   continue;
		   tokenizer.break_line(line.c_str(), szSeparators);
		   /*
		   UserOrderLocalID may be duplicate,update it
		   */
		   if (m_user_info_map.find(tokenizer[0]) == m_user_info_map.end())
		   {
		   user_info * info = new user_info();
		   info->UserOrderLocalID = tokenizer[0];
		   info->UserID           = tokenizer[1];
		   m_user_info_map.emplace(info->UserOrderLocalID, info);
	   }
		   else
		   {
			   user_info * info       = m_user_info_map[tokenizer[0]];
			   info->UserOrderLocalID = tokenizer[0];
			   info->UserID           = tokenizer[1];
		   }
	   }
	   stream.close();
   }
   void femas_api::append(user_info * info)
   {
	   if (info == nullptr)
		   return;
	   boost::filesystem::ofstream stream;
	   stream.open(m_user_info_file_name.c_str(), ios::app);
	   char buffer[256];
	   memset(buffer, 0, sizeof(buffer));
	   sprintf(buffer, "%s,%s\n", info->UserOrderLocalID.c_str(), info->UserID.c_str());
	   stream << buffer;
	   stream.close();
   }
   void femas_api::create_user_info(CUstpFtdcInputOrderField* pRequest)
   {
	   if (pRequest==nullptr)
	   {
		   return;
	   }
	   if (m_user_info_map.find(pRequest->UserOrderLocalID) == m_user_info_map.end())
	   {
		   user_info * info       = new user_info();
		   info->UserOrderLocalID = pRequest->UserOrderLocalID;
		   info->UserID           = pRequest->UserCustom;

		   m_user_info_map.emplace(info->UserOrderLocalID, info);

		   //to do ... append the file every day
		   m_userInfoQueue.CopyPush(info);
	   }
	   else
	   {
		   user_info * info       = m_user_info_map[pRequest->UserOrderLocalID];
		   printf_ex("warn:femas_api::create_user_info already include the UserOrderLocalID:%s,from [%s] to [%s]\n", pRequest->UserOrderLocalID,info->UserID.c_str(),pRequest->UserCustom);
		   loggerv2::warn("warn:femas_api::create_user_info already include the UserOrderLocalID:%s,from [%s] to [%s]\n", pRequest->UserOrderLocalID,info->UserID.c_str(),pRequest->UserCustom);
		   info->UserOrderLocalID = pRequest->UserOrderLocalID;
		   info->UserID           = pRequest->UserCustom;
		   m_userInfoQueue.CopyPush(info);		   
	   }
   }
   void femas_api::OnUserInfoAsync(user_info* pInfo)
   {
	   this->append(pInfo);
   }
   string femas_api::get_user_id(string userOrderLocalID)
   {
	   if (m_user_info_map.find(userOrderLocalID) != m_user_info_map.end())
	   {
		   user_info * info = m_user_info_map[userOrderLocalID];
		   return info->UserID;
	   }
	   return "";
   }
   void femas_api::request_instruments()
   {	   
	   CUstpFtdcQryInstrumentField QryInstrumentField;
	   if (m_pUserApi != nullptr)
	   {
		   memset(&QryInstrumentField, 0, sizeof(CUstpFtdcQryInstrumentField));
		   //strcpy(tapAPICommodity.ExchangeNo, "ZCE");
		   //tapAPICommodity.CommodityType = TAPI_COMMODITY_TYPE_OPTION;
		   int ret = m_pUserApi->ReqQryInstrument(&QryInstrumentField, ++m_nRequestId);
		   printf_ex("femas_api::request_instruments ReqQryInstrument ret:%d\n", ret);
		   loggerv2::error("femas_api::request_instruments ReqQryInstrument ret:%d\n", ret);
	   }
   }
}

