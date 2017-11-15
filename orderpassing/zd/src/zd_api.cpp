#include "zd_api.h"
#include "zd_connection.h"
#include "tradeItem_gh.h"
#include <boost/algorithm/string.hpp>
#include "terra_logger.h"
#include "FastMemcpy.h"
#include <thread>
#include "order_reference_provider.h"
using namespace terra::common;
namespace zd
{

	zd_api::zd_api(zd_connection* pConnection)
	{
		m_pConnection = pConnection;
		m_connectionStatus = false;

		m_isAlive = true;

		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		m_begin_Id = order_reference_provider::get_instance().get_current_int();

	}

	zd_api::~zd_api()
	{
		//delete m_pUserApi;
	}

	void zd_api::init()
	{
		m_pUserApi = CSHZdTraderApi::CreateSHZdTraderApi();

		m_inputQueue.setHandler(boost::bind(&zd_connection::OnRspOrderInsertAsync, m_pConnection, _1));
		//m_inputQuoteQueue.setHandler(boost::bind(&cffex_connection::OnRspQuoteInsertAsync, m_pConnection, _1));

		m_orderQueue.setHandler(boost::bind(&zd_connection::OnRtnOrderAsync, m_pConnection, _1));
		//m_quoteQueue.setHandler(boost::bind(&cffex_connection::OnRtnQuoteAsync, m_pConnection, _1));
		
		m_tradeQueue.setHandler(boost::bind(&zd_connection::OnRtnTradeAsync, m_pConnection, _1));

		m_inputActionQueue.setHandler(boost::bind(&zd_connection::OnRspOrderActionAsync, m_pConnection, _1));
		//m_inputActionQuoteQueue.setHandler(boost::bind(&cffex_connection::OnRspQuoteActionAsync, m_pConnection, _1));

		m_pConnection->m_userInfoQueue.setHandler(boost::bind(&zd_connection::OnUserInfoAsync,m_pConnection, _1));
	}

	void zd_api::release()
	{
		// removeFDFromList...

		m_pUserApi->Release();
	}

	void zd_api::Process()
	{
		
		m_inputQueue.Pops_Handle(0);
		//m_inputQuoteQueue.Pops_Handle(0);

		m_orderQueue.Pops_Handle(0);
		//m_quoteQueue.Pops_Handle(0);

		m_tradeQueue.Pops_Handle(0);
		m_inputActionQueue.Pops_Handle(0);
		//m_inputActionQuoteQueue.Pops_Handle(0);
	
		m_pConnection->m_userInfoQueue.Pops_Handle(0);
	}

	int zd_api::get_ord_ref_from_reqid(string orderRef)
	{
		auto search = m_ordInputActiondRefMap.find(orderRef);
		if (search != m_ordInputActiondRefMap.end()) {
			int i = search->second;
			m_ordInputActiondRefMap.erase(search);
			return i;
		}
		else
		{
			return 0;
		}
	}

	bool zd_api::connect()
	{
		if (m_connectionStatus == false)
		{
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", m_pConnection->m_sHostname.c_str(), m_pConnection->m_sService.c_str());
			m_pUserApi->RegisterSpi(this);

			m_pUserApi->Init();
			m_pUserApi->AuthonInfo((char*)m_pConnection->get_auth_code().c_str());

			m_pUserApi->RegisterFront(addr);

			loggerv2::info("zd_api::connect connecting to %s", addr);

			loggerv2::info("zd_api::connect api intialized");
		}
		else
		{
			request_login();
		}

		return true;
	}
	/*
	///直达用户登出请求
	struct CTShZdUserLogoutField
	{
	///经纪公司代码
	TShZdBrokerIDType	BrokerID;
	///用户代码  直达
	TShZdUserIDType	UserID;
	};
	*/
	bool zd_api::disconnect()
	{
		CTShZdUserLogoutField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->get_user_id().c_str());

		int res = m_pUserApi->ReqUserLogout(&request, ++m_nRequestId);
		//
		m_pUserApi->Release();
		m_connectionStatus = false;
		//
		if (res != 0)
		{
			return false;
		}
		return true;
	}
	/*
	///直达用户登录请求
	struct CTShZdReqUserLoginField
	{
	///交易日
	TShZdDateType	TradingDay;
	///经纪公司代码
	TShZdBrokerIDType	BrokerID;
	///用户代码  直达必须填写
	TShZdUserIDType	UserID;
	///密码  直达必须填写
	TShZdPasswordType	Password;
	///用户端产品信息
	TShZdProductInfoType	UserProductInfo;
	///接口端产品信息
	TShZdProductInfoType	InterfaceProductInfo;
	///协议信息
	TShZdProtocolInfoType	ProtocolInfo;
	///Mac地址
	TShZdMacAddressType	MacAddress;
	///动态密码
	TShZdPasswordType	OneTimePassword;
	///终端IP地址  直达必须填写
	TShZdIPAddressType	ClientIPAddress;
	};
	*/
	void zd_api::request_login()
	{
		loggerv2::info("zd_api::request_login");
		printf_ex("zd_api::request_login\n");

		CTShZdReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->get_user_id().c_str());
		strcpy(request.Password, m_pConnection->m_sPassword.c_str());

		int res = m_pUserApi->ReqUserLogin(&request, ++m_nRequestId);
		if (res != 0)
		{
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "zd_api - ReqUserLogin failed");
		}
		//else if (m_pConnection->getRequestInstruments() == true)
		//{
			//request_instruments();
		//}
	}

	void zd_api::request_ack_order()
	{
		loggerv2::info("zd_api::request_ack_order");
	}

	void  zd_api::OnRspQryOrder(CTShZdOrderField *pOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pOrder)
		{
			if ( bIsLast == false)
			{
				if (m_pConnection->get_night_trade() == true)
				{
					string ts = strlen(pOrder->InsertDate)>0 ? pOrder->InsertDate : pOrder->CancelDate;
#if 0
					ts.insert(10, " ");
#else
					char buffer[64];
					memset(buffer, 0, sizeof(buffer));
					if (strlen(pOrder->InsertDate) > 0)
					{
						sprintf(buffer, "%s %s", pOrder->InsertDate, pOrder->InsertTime);
					}
					else
					{
						sprintf(buffer, "%s %s", pOrder->CancelDate, pOrder->CancelTime);
					}
					ts = buffer;
#endif
					auto tp = string_to_lwtp(ts.c_str());
					int hour = get_hour_from_lwtp(tp);
					//if (hour >= 16 || hour <= 8)
					if (!(hour < 16 && hour > 8))
					{
						m_orderQueue.CopyPush(pOrder);
					}
				}
				else
				{
					m_orderQueue.CopyPush(pOrder);
				}
			}
			else
			{
				//
				if (m_bQryTrade == false)
				{
					CTShZdQryTradeField QryTradeField;
					memset(&QryTradeField, 0, sizeof(CTShZdQryTradeField));
					strcpy(QryTradeField.InvestorID, m_pConnection->get_investor_id().c_str());
					strcpy(QryTradeField.UserID, m_pConnection->get_user_id().c_str());
					int ret = m_pUserApi->ReqQryTrade(&QryTradeField, ++m_nRequestId);
					loggerv2::info("zd_api::OnRspQryOrder call the ReqQryTrade,ret:%d", ret);
					printf_ex("zd_api::OnRspQryOrder call the ReqQryTrade,ret:%d\n", ret);
					m_bQryTrade = true;
				}
				//
			}
		}
	}

	void zd_api::OnRspQryTrade(CTShZdTradeField *pTrade, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pTrade)
		{
			if (bIsLast == false)
			{
				//m_tradeQueue.CopyPush(pTrade);
				if (m_pConnection->get_night_trade() == true)
				{
					string ts = pTrade->TradeDate;
#if 0
					ts.insert(10, " ");
#else
					char buffer[64];
					memset(buffer, 0, sizeof(buffer));
					if (strlen(pTrade->TradeDate) > 0)
					{
						sprintf(buffer, "%s %s", pTrade->TradeDate, pTrade->TradeTime);
					}
					ts = buffer;
#endif
					auto tp = string_to_lwtp(ts.c_str());
					int hour = get_hour_from_lwtp(tp);
					//if (hour >= 16 || hour <= 8)
					if (!(hour < 16 && hour > 8))
					{
						m_tradeQueue.CopyPush(pTrade);
					}
					
				}
				else
				{
					m_tradeQueue.CopyPush(pTrade);
				}
			}
		}
	}
	void zd_api::OnRspError(CTShZdRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
		{
			loggerv2::error("zd_api::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			printf_ex("zd_api::OnRspError - (%d, %s)\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
		else
		{
			loggerv2::info("zd_api::OnRspError - ok");
		}
	}

	void zd_api::OnFrontConnected()
	{
		loggerv2::info("zd_api is UP");

		m_connectionStatus = true;

		if (m_pConnection->getStatus() == AtsType::ConnectionStatus::WaitConnect)
		{
			request_login();
		}
		else
		{
			loggerv2::info("zd_api not asking for reconnect...");
		}
	}

	bool zd_api::ReqQryInvestorPosition()
	{
		CTShZdQryInvestorPositionField PositionField;
		memset(&PositionField, 0, sizeof(CTShZdQryInvestorPositionField));
		strcpy(PositionField.InvestorID, m_pConnection->get_investor_id().c_str());
		strcpy(PositionField.UserID, m_pConnection->get_user_id().c_str());
		int ret = m_pUserApi->ReqQryInvestorPosition(&PositionField, ++m_nRequestId);	
		return true;
	}
	/*
	//资金查询
	CTShZdQryTradingAccountField pQryTradingAccount;
	memset(&pQryTradingAccount,0,sizeof(CTShZdQryTradingAccountField));
	memcpy(pQryTradingAccount.UserID,"MN000301",13);
	apiTrade->ReqQryTradingAccount(&pQryTradingAccount,7);
	*/
	bool zd_api::ReqQryTradingAccount(CTShZdQryTradingAccountField *pQryTradingAccount)
	{
		if (m_pUserApi == nullptr)
			return false;
		int ret = m_pUserApi->ReqQryTradingAccount(pQryTradingAccount, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}
	void zd_api::OnFrontDisconnected(int nReason)
	{
		loggerv2::info("zd_api is DOWN");

		m_connectionStatus = false;

		std::string pszMessage;
		switch (nReason)
		{
			// normal disconnection
		case 0:
			break;

			// error
		case 1:
		case 2:
		case 3:
		case 4:
			//pszMessage = "ERROR MSG TO TRANSLATE [" + nReason + "];
			pszMessage = "ERROR MSG TO TRANSLATE";
			break;


		default:
			//pszMessage = "unknown error [" + nReason + "];
			pszMessage = "unknown error";
			break;
		}

		m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage.data());
	}

	void zd_api::OnHeartBeatWarning(int nTimeLapse)
	{
		loggerv2::info("zd_api - heartbeat warning,nTimeLapse:%d", nTimeLapse);
	}

	void zd_api::OnRspUserLogin(CTShZdRspUserLoginField* pRspUserLogin, CTShZdRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::info("zd_api::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		printf_ex("zd_api::OnRspUserLogin - (%d, %s)\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		if (pRspInfo->ErrorID == 0)
		{	
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Connected, pRspInfo->ErrorMsg);
			//
			if (m_pConnection->getRequestInstruments() == true)
			{
				request_instruments();
				m_pConnection->setRequestInstruments(false);
			}
			//
			/*
			///直达查询报单
			struct CTShZdQryOrderField
			{
			///用户代码  直达
			TShZdUserIDType	UserID;
			///资金代码 直达
			TShZdInvestorIDType	InvestorID;
			///合约代码  直达
			TShZdInstrumentIDType	InstrumentID;
			///交易所代码  直达
			TShZdExchangeIDType	ExchangeID;
			///系统号  直达
			TShZdOrderSysIDType	OrderSysID;
			///开始时间  直达
			TShZdTimeType	InsertTimeStart;
			///结束时间  直达
			TShZdTimeType	InsertTimeEnd;
			};
			*/
			if (m_bQryOrder == false)
			{
				CTShZdQryOrderField QryOrderField;
				memset(&QryOrderField, 0, sizeof(CTShZdQryOrderField));
				strcpy(QryOrderField.InvestorID, m_pConnection->get_investor_id().c_str());
				strcpy(QryOrderField.UserID, m_pConnection->get_user_id().c_str());
				int ret = m_pUserApi->ReqQryOrder(&QryOrderField, ++m_nRequestId);
				loggerv2::info("zd_api::OnRspUserLogin call the ReqQryOrder,ret:%d", ret);
				printf_ex("zd_api::OnRspUserLogin call the ReqQryOrder,ret:%d\n", ret);
				m_bQryOrder = true;
			}
			//
		}
		else
		{
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pRspInfo->ErrorMsg);
		}
	}

	void zd_api::OnRspUserLogout(CTShZdUserLogoutField *pUserLogout, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::error("zd_api::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);

		if (pRspInfo->ErrorID == 0)
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "zd_api::OnRspUserLogout Receive Logout Msg Error Id 0");
		else
			loggerv2::error("zd_api::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);


	}
	void log_CThostFtdcInvestorPositionField(CTShZdInvestorPositionField *pInvestorPosition)
	{

	}	
	/*
	///直达投资者持仓
	struct CTShZdInvestorPositionField
	{
	///交易所代码  直达
	TShZdExchangeIDType	ExchangeID;
	///合约代码  直达
	TShZdInstrumentIDType	InstrumentID;
	///资金代码  直达
	TShZdInvestorIDType	InvestorID;
	///持买量  直达
	TShZdVolumeType	HoldBuyVolume;
	///持买开均价  直达
	TShZdMoneyType	HoldBuyOpenPrice;
	///持买均价 直达
	TShZdMoneyType	HoldBuyPrice;
	///持卖量  直达
	TShZdVolumeType	HoldSaleVolume;
	//持卖开均价  直达
	TShZdMoneyType	HoldSaleOpenPrice;
	///持卖均价  直达
	TShZdMoneyType	HoldSalePrice;
	///持买保证金  直达
	TShZdMoneyType	HoldBuyAmount;
	///持卖保证金  直达
	TShZdMoneyType	HoldSaleAmount;
	///开仓量  直达
	TShZdVolumeType	OpenVolume;
	///成交量  直达
	TShZdVolumeType	FilledVolume;
	///成交均价  直达
	TShZdMoneyType	FilledAmount;
	///手续费  直达
	TShZdMoneyType	Commission;
	///持仓盈亏  直达
	TShZdMoneyType	PositionProfit;
	///交易日  直达
	TShZdDateType	TradingDay;
	};
	*/
	void zd_api::OnRspQryInvestorPosition(CTShZdInvestorPositionField *pInvestorPosition, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (bIsLast == true)
		{
			if (m_pConnection->getRequestInstruments() == true)
			{
				sleep_by_milliseconds(2000);
				this->request_instruments();	
				m_pConnection->setRequestInstruments(false);
			}
		}
#if 0		
		if (pInvestorPosition)
		{
			if (m_pConnection->m_debug)
			{
				loggerv2::info("zd_api::OnRspQryInvestorPosition,"
					"ExchangeID:%s,"
					"InstrumentID:%s,"
					"InvestorID:%s,"
					"HoldBuyVolume:%d,"
					"HoldSaleVolume:%d,"
					"bIsLast:%d,",
					pInvestorPosition->ExchangeID,
					pInvestorPosition->InstrumentID,
					pInvestorPosition->InvestorID,
					pInvestorPosition->HoldBuyVolume,
					pInvestorPosition->HoldSaleVolume,
					bIsLast
					);
			}
		}
#endif
#if 1		

		bool islog = false;
		if (pInvestorPosition && strlen(pInvestorPosition->InstrumentID)>0)
		{

			std::string sInstrCode = std::string(pInvestorPosition->InstrumentID) + "@" + m_pConnection->getName();

			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str()/* pInvestorPosition->InstrumentID*/);
			if (i)
			{
				if (i->get_tot_long_position() != pInvestorPosition->HoldBuyVolume)
				{
					loggerv2::info("zd_api::OnRspQryInvestorPosition:total long position change,%s", pInvestorPosition->InstrumentID);
					i->dumpinfo();
					i->set_tot_long_position(pInvestorPosition->HoldBuyVolume);					
					i->dumpinfo();
					islog = true;
				}

				if (i->get_tot_short_position() != pInvestorPosition->HoldSaleVolume)
				{
					loggerv2::info("zd_api::OnRspQryInvestorPosition:total short position change,%s", pInvestorPosition->InstrumentID);
					i->dumpinfo();
					i->set_tot_short_position(pInvestorPosition->HoldSaleVolume);				
					i->dumpinfo();
					islog = true;
				}
				
				if (islog)
					i->set_last_sychro_timepoint(get_lwtp_now());
			}
			else
			{
				//loggerv2::info("zd_api::OnRspQryInvestorPosition cannot find tradeitem %s by second key", std::string(pInvestorPosition->InstrumentID).c_str());
			}
		}
		else
		{
			//loggerv2::warn("zd_api::OnRspQryInvestorPosition could not get tradeitem %s.", pInvestorPosition->InstrumentID);
		}
#endif
	}
	void zd_api::OnRspOrderInsert(CTShZdInputOrderField *pInputOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		m_inputQueue.CopyPush(pInputOrder);
	}


	void zd_api::OnErrRtnOrderInsert(CTShZdInputOrderField* pInputOrder, CTShZdRspInfoField* pRspInfo)
	{
		loggerv2::info("zd_api::OnErrRtnOrderInsert ErrorID:%d(%s),OrderLocalID:%s", pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrder->OrderLocalID);	 
		pInputOrder->ForceCloseReason = 'E';
		pInputOrder->MinVolume        = pRspInfo->ErrorID;
		m_inputQueue.CopyPush(pInputOrder);
	}

	/*
	///直达报单操作（撤单、改单）回报
	struct CTShZdInputOrderActionField
	{
	///资金账号  直达
	TShZdInvestorIDType	InvestorID;
	///报单操作引用  直达
	TShZdOrderActionRefType	OrderActionRef;
	///订单号 直达
	TShZdOrderRefType	OrderRef;
	///请求编号  直达
	TShZdRequestIDType	RequestID;
	///交易所代码  直达
	TShZdExchangeIDType	ExchangeID;
	///系统号  直达
	TShZdOrderSysIDType	OrderSysID;
	///操作标志  0 撤单 3 改单  直达
	TShZdActionFlagType	ActionFlag;
	///价格变化  改单后的价格  直达
	TShZdPriceType	LimitPrice;
	///数量变化  改单后的数量 撤单数量  直达
	TShZdVolumeType	VolumeChange;
	///已成交数量
	TShZdVolumeType    VolumeFilled;
	///报单价格
	TShZdPriceType  OrderPrice;
	///报单数量   撤单直达
	TShZdVolumeType  OrderVolume;
	///用户代码  直达
	TShZdUserIDType	UserID;
	///合约代码  直达
	TShZdInstrumentIDType	InstrumentID;
	///有效期类型  （1=当日有效, 2=永久有效（GTC），3=OPG，4=IOC，5=FOK，6=GTD，7=ATC，8=FAK）
	TShZdTimeConditionType	TimeCondition;
	///买卖方向   1买 2卖  直达
	TShZdDirectionType	Direction;
	///报单价格条件   1限价单 2市价单 3限价止损（stop to limit），4止损（stop to market）
	TShZdOrderPriceTypeType	OrderPriceType;
	///改单触发价格
	TShZdPriceType  ModifyTriggerPrice;
	///操作日期(改单日期、撤单日期)
	TShZdDateType	ActionDate;
	///操作时间(改单时间、撤单时间)
	TShZdTimeType	ActionTime;
	};
	*/
	void zd_api::OnRspOrderAction(CTShZdInputOrderActionField *pInputOrderAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		//loggerv2::info("calling zd_api::OnRspOrderAction . request id=[%d] --> need to implement",nRequestID);

		if (!bIsLast)
		{
			return;
		}
		
		loggerv2::info("zd_api::OnRspOrderAction pInputOrderAction - "
			"OrderActionRef[%d]"
			"OrderRef[%s]"
			"ExchangeID[%s]"
			"OrdSysID[%s]"
			"ActionFlag[%c]"
			"LimitPrice[%f]"
			"VolumeChange[%d]"
			"UserID[%s]"
			"InstrumentID[%s],"
			"ActionDate:%s,"
			"ActionTime:%s",
			pInputOrderAction->OrderActionRef,
			pInputOrderAction->OrderRef,
			pInputOrderAction->ExchangeID,
			pInputOrderAction->OrderSysID,
			pInputOrderAction->ActionFlag,
			pInputOrderAction->LimitPrice,
			pInputOrderAction->VolumeChange,
			pInputOrderAction->UserID,
			pInputOrderAction->InstrumentID,
			pInputOrderAction->ActionDate,
			pInputOrderAction->ActionTime
			);

		loggerv2::info("zd_api::OnRspOrderActionpRspInfo - "
			"ErrorID[%d]"
			"ErrorMsg[%s]",
			pRspInfo->ErrorID,
			pRspInfo->ErrorMsg
			);

		int orderRef = get_ord_ref_from_reqid(pInputOrderAction->OrderRef);
		if (!orderRef)
		{
			loggerv2::error("could not find associated order for nRequestId %d", nRequestID);
			return;
		}

		int* i = new int(orderRef);
		m_inputActionQueue.Push(i);
	}

	void zd_api::OnRspQryTradingAccount(CTShZdTradingAccountField *pTradingAccount, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		/*
		///直达资金账户
		struct CTShZdTradingAccountField
		{
		///用户代码  直达
		TShZdUserIDType	UserID;
		///资金账号  直达
		TShZdAccountIDType	AccountID;
		///昨可用  直达
		TShZdMoneyType	PreMortgage;
		///昨权益 直达
		TShZdMoneyType	PreCredit;
		///昨结存 直达
		TShZdMoneyType	PreDeposit;
		///今权益  直达
		TShZdMoneyType	CurrBalance;
		///今可用 直达
		TShZdMoneyType	CurrUse;
		///今结存 直达
		TShZdMoneyType	CurrDeposit;
		///入金金额   直达
		TShZdMoneyType	Deposit;
		///出金金额   直达
		TShZdMoneyType	Withdraw;
		///冻结的保证金  直达
		TShZdMoneyType	FrozenMargin;
		///当前保证金总额  直达
		TShZdMoneyType	CurrMargin;
		///手续费  直达
		TShZdMoneyType	Commission;
		///平仓盈亏  直达
		TShZdMoneyType	CloseProfit;
		///净盈利（总盈亏） 直达
		TShZdMoneyType	NetProfit;
		///未到期平盈   直达
		TShZdMoneyType	UnCloseProfit;
		///未冻结平盈  直达
		TShZdMoneyType	UnFrozenCloseProfit;
		///交易日
		TShZdDateType	TradingDay;
		///信用额度  直达
		TShZdMoneyType	Credit;
		///配资资金  直达
		TShZdMoneyType	Mortgage;
		///维持保证金  直达
		TShZdMoneyType	KeepMargin;
		///期权利金  直达
		TShZdMoneyType	RoyaltyMargin;
		///初始资金  直达
		TShZdMoneyType	FirstInitMargin;
		///盈利率  直达
		TShZdMoneyType	ProfitRatio;
		///风险率  直达
		TShZdMoneyType	RiskRatio;
		///币种，账号的币种  直达
		TShZdCurrencyNoType CurrencyNo;
		///货币与基币的汇率  直达
		TShZdMoneyType	CurrencyRatio;
		};
		*/
		if (pTradingAccount)
		{
#if 0
			double ratio = 0;

			if (string(pTradingAccount->AccountID) == m_pConnection->get_investor_id())
			{
			
				if (pTradingAccount->CurrBalance != 0)
				{
					ratio = pTradingAccount->CurrMargin / (pTradingAccount->CurrBalance);
				}

			loggerv2::info("zd_api::OnRspQryTradingAccount,"
				"UserID:%s,"
				"AccountID:%s,"
				"RiskRatio:%f"
				"CurrUse:%f,"
				"CurrBalance:%f,"
				"CurrMargin:%f,"
				"Ratio:%f", pTradingAccount->UserID, pTradingAccount->AccountID, pTradingAccount->RiskRatio, pTradingAccount->CurrUse, pTradingAccount->CurrBalance, pTradingAccount->CurrMargin, ratio);

			}					
			m_pConnection->set_RiskDegree(ratio);
#endif
		}
	}

	void zd_api::OnRtnOrder(CTShZdOrderField* pOrder)
	{
		m_orderQueue.CopyPush(pOrder);
	}

	void zd_api::OnRtnTrade(CTShZdTradeField* pTrade)
	{
		m_tradeQueue.CopyPush(pTrade);
	}
	//
	bool zd_api::ReqOrderInsert(CTShZdInputOrderField* pRequest)
	{
		int ret = m_pUserApi->ReqOrderInsert(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}
	/*
	///直达报单操作  撤单 、改单 请求
	struct CTShZdOrderActionField
	{
	///订单编号
	TShZdOrderRefType	OrderRef;
	///系统编号
	TShZdOrderSysIDType	OrderSysID;
	///操作标志
	TShZdActionFlagType	ActionFlag;
	///修改的价格 （改单填写）
	TShZdPriceType	LimitPrice;
	///数量变化(改单填写)
	TShZdVolumeType	VolumeChange;
	///用户代码
	TShZdUserIDType	UserID;
	///报单客户端类型  API的用户只需填写C 或者  P
	TShZdOrderTypeType OrderType;
	};
	*/
	bool zd_api::ReqOrderAction(CTShZdOrderActionField* pRequest)
	{
		int ret = m_pUserApi->ReqOrderAction(pRequest, ++m_nRequestId);	
		if (ret != 0)
		{
			return false;
		}
		return true;
	}

	void zd_api::OnRspQryInstrument(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (m_pConnection != nullptr)
		{
			m_pConnection->OnRspQryInstrument(pInstrument, pRspInfo, nRequestID, bIsLast);
		}
	}

	void zd_api::request_instruments()
	{
		/*
		int ReqQryInstrument(CTShZdQryInstrumentField *pQryInstrument, int nRequestID)
		///直达查询合约
		struct CTShZdQryInstrumentField
		{
		///合约代码。查询单个合约
		TShZdInstrumentIDType	InstrumentID;
		///交易所代码，如果填写值，查询一个交易所的合约
		TShZdExchangeIDType	ExchangeID;
		///合约在交易所的代码
		TShZdExchangeInstIDType	ExchangeInstID;
		///产品代码 ，如果填写值，查询一个产品的合约
		TShZdInstrumentIDType	ProductID;
		///开始时间,如果填写，是这个时间以后新增的
		TShZdTimeType	InsertTimeStart;
		///查询多少条,每次返回的条数
		TShZdVolumeType	Index;
		///查询合约的类别  直达
		TShZdProductClassType ProductType;
		};
		*/

		CTShZdQryInstrumentField request;
		memset(&request, 0, sizeof(request));
#if 0		
		strcpy(request.ExchangeID,"CME");
		strcpy(request.InsertTimeStart, "20170101");
#else
		strcpy(request.ExchangeID, m_pConnection->get_futures().c_str());
		strcpy(request.InsertTimeStart, m_pConnection->getInsertTimeStart().c_str());
#endif

		strcpy(request.ProductID, "");		
		
		request.ProductType = TSHZD_PC_Futures;
		request.Index=500; 
		int ret = m_pUserApi->ReqQryInstrument(&request, ++m_nRequestId);
		loggerv2::info("zd_api::request_instruments ret:%d\n", ret);

		/*int ReqQryExchange(CTShZdQryExchangeField *pQryExchange, int nRequestID)*/
		//CTShZdQryExchangeField request;
		//memset(&request, 0, sizeof(request));
		//int ret = m_pUserApi->ReqQryExchange(&request, ++m_nRequestId);
		//printf_ex("zd_api::ReqQryExchange ret:%d\n", ret);

	}
	void zd_api::OnRspQryExchange(CTShZdExchangeField *pExchange, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pExchange)
		{
			loggerv2::info("zd_api::OnRspQryExchange,%s,%s", pExchange->ExchangeID, pExchange->ExchangeName);
		}
	}
}

