#include "zd_connection.h"
#include "ShZdFutureUserApiStruct.h"
#include <string>
#include "string_tokenizer.h"
#include <boost/property_tree/ini_parser.hpp>
using namespace terra::common;
namespace zd
{

	zd_connection::zd_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "zd_connection_ex";
		m_pZdApi = new zd_api(this);
		m_bKey_with_exchange = false;
	}
	zd_connection::~zd_connection()
	{
		delete m_pZdApi;
	}	
	bool zd_connection::init_config(const string &name, const std::string &strConfigFile)
	{
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 8)
			m_bTsession = true;
		else
			m_bTsession = false;
		boost::filesystem::path p(strConfigFile);
		if (!boost::filesystem::exists(p))
		{
			printf_ex("es_connection::load config_file:%s not exist!\n", strConfigFile.c_str());
			return false;
		}
		boost::property_tree::ptree root;
		boost::property_tree::ini_parser::read_ini(strConfigFile, root);
		m_strAuthCode = root.get<string>(name + ".auth_code", "");
		//
		m_strFutures = root.get<string>(name + ".Futures", "");
		m_strInsertTimeStart = root.get<string>(name + ".InsertTimeStart", "");
		//
		return true;
	}
	void zd_connection::request_instruments()
	{
		m_pZdApi->request_instruments();
	}
	void zd_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{
#if 0
		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pZdApi->ReqQryInvestorPosition(&pRequest);
		if (m_debug)
			loggerv2::info("zd_connection::request_investor_position requesting investor position for tradeitem %s", i->getCode().c_str());
#endif
	}
	void zd_connection::request_investor_full_positions()
	{
#if 0
		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		m_pZdApi->ReqQryInvestorPosition(&pRequest);
		if (m_debug)
			loggerv2::info("zd_connection:: calling OnRspQryInvestorPosition ");
#else
		/*
		int ReqQryInvestorPosition(CTShZdQryInvestorPositionField *pQryInvestorPosition, int nRequestID)
		CTShZdQryInvestorPositionField pQryInvestorPosition;
		memset(&pQryInvestorPosition,0,sizeof(CTShZdQryInvestorPositionField));
		memcpy(pQryInvestorPosition.UserID,"MN000301",13);
		memcpy(pQryInvestorPosition.InvestorID,"MN00000903",18);
		apiTrade->ReqQryInvestorPosition(&pQryInvestorPosition,12);
		*/	
		m_pZdApi->ReqQryInvestorPosition();
#endif
	}
	//
	void zd_connection::req_RiskDegree()
	{
		request_trading_account();
	}
	//
	void zd_connection::request_trading_account()
	{
//#if 1
		//if (m_debug)
			//loggerv2::info("zd_connection:: calling ReqQryTradingAccount ");
		/*
		///直达查询资金账户
		struct CTShZdQryTradingAccountField
		{
		///经纪公司代码
		TShZdBrokerIDType	BrokerID;
		///投资者代码  资金账号
		TShZdInvestorIDType	InvestorID;
		///用户代码
		TShZdUserIDType	UserID;
		};
		*/
		CTShZdQryTradingAccountField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, get_investor_id().c_str());
		strcpy(pRequest.UserID, get_user_id().c_str());
		m_pZdApi->ReqQryTradingAccount(&pRequest);
//#endif
	}
	void zd_connection::init_connection()
	{
		loggerv2::info("zd_connection::init_connection create trader api..");
		m_pZdApi->init();

#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif		
		m_bKey_with_exchange = false;
	}
#ifdef Linux
	void  zd_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&zd_connection::process, this));

		m_pZdApi->get_input_queue() ->set_fd(efd);
		//m_pZdApi->get_input_quote_queue()->set_fd(efd);

		m_pZdApi->get_order_queue()->set_fd(efd);
		//m_pZdApi->get_order_quote_queue()->set_fd(efd);

		m_pZdApi->get_trade_queue()->set_fd(efd);
		m_pZdApi->get_input_action_queue()->set_fd(efd);
		//m_pZdApi->get_input_action_quote_queue()->set_fd(efd);

		m_outboundQueue.set_fd(efd);
		//m_outquoteboundQueue.set_fd(efd);
	}
#endif
	void zd_connection::release()
	{				
		ctpbase_connection::release();
		m_pZdApi->release();
	}
	void zd_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("zd_connection::connect connecting to zd...");
			on_status_changed(AtsType::ConnectionStatus::WaitConnect);
			m_pZdApi->connect();
		}
	}
	void zd_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			if (m_pZdApi->disconnect() == false)
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "zd_connection_ex - ReqUserLogout failed");
			}
		}
	}
	void zd_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_outquoteboundQueue.Pops_Handle_Keep(10);
		m_pZdApi->Process();
	}
	/*
	CTShZdInputOrderField pOrder;
	memset(&pOrder,0,sizeof(CTShZdInputOrderField));

	memcpy(pOrder.OrderLocalID,"20161128141700",14);
	memcpy(pOrder.ExchangeID,"CME",11);//交易所
	memcpy(pOrder.InvestorID,"MN00000903",13);//资金帐号

	memcpy(pOrder.InstrumentID,"CL1703",31);
	memcpy(pOrder.UserID,"MN000301",16);
	pOrder.Direction='1';
	pOrder.VolumeTotalOriginal = 3;
	pOrder.LimitPrice = atof("45");
	pOrder.OrderPriceType='2';
	pOrder.OrderType ='P';
	apiTrade->ReqOrderInsert(&pOrder,0);
	*/
	int zd_connection::market_create_order_async(order* o, char* pszReason)
	{		
		if (o->get_way() != AtsType::OrderWay::Exercise)
		{
			CTShZdInputOrderField *request = zd_create_pool.get_mem();
			memset(request, 0, sizeof(CTShZdInputOrderField));

			strcpy(request->InvestorID, this->get_investor_id().c_str());
			
			strcpy(request->UserID, this->get_user_id().c_str());

			strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());

			strcpy(request->ExchangeID, o->get_exchange_id().c_str());

			//
			request->OrderType = TSHZD_ORDT_Api;
			//

			if (o->get_way() == AtsType::OrderWay::Buy)
				request->Direction = TSHZD_D_Buy;
			else if (o->get_way() == AtsType::OrderWay::Sell)
				request->Direction = TSHZD_D_Sell;

			request->LimitPrice = o->get_price();
			request->VolumeTotalOriginal = o->get_quantity();
			/*
			/////////////////////////////////////////////////////////////////////////
			///有效期类型类型
			/////////////////////////////////////////////////////////////////////////
			///当日有效  直达
			#define TSHZD_TC_DAY '1'
			///永久有效（GTC）  直达
			#define TSHZD_TC_GTC '2'
			///OPG
			#define TSHZD_TC_OPG '3'
			///IOC
			#define TSHZD_TC_IOC '4'
			///FOK
			#define TSHZD_TC_FOK '5'
			///集合竞价有效
			#define TSHZD_TC_GFA '6'

			typedef char TShZdTimeConditionType;
			*/
			/*
			/////////////////////////////////////////////////////////////////////////
			///成交量类型类型
			/////////////////////////////////////////////////////////////////////////
			///任何数量
			#define TSHZD_VC_AV '1'
			///最小数量
			#define TSHZD_VC_MV '2'
			///全部数量
			#define TSHZD_VC_CV '3'

			typedef char TShZdVolumeConditionType;
			*/
			if (o->get_restriction() == AtsType::OrderRestriction::None)
				request->TimeCondition = TSHZD_TC_DAY;
			else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)
				request->TimeCondition = TSHZD_TC_IOC;
			else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			{
				request->TimeCondition   = TSHZD_TC_IOC;
				request->VolumeCondition = TSHZD_VC_AV;
			}
			else
			{
				snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
				zd_create_pool.free_mem(request);
				return 0;
			}
			/*
			/////////////////////////////////////////////////////////////////////////
			///开平标志类型
			/////////////////////////////////////////////////////////////////////////
			///开仓
			#define TSHZD_OF_Open '0'
			///平仓
			#define TSHZD_OF_Close '1'
			///强平
			#define TSHZD_OF_ForceClose '2'
			///平今
			#define TSHZD_OF_CloseToday '3'
			///平昨
			#define TSHZD_OF_CloseYesterday '4'
			///强减
			#define TSHZD_OF_ForceOff '5'
			///本地强平
			#define TSHZD_OF_LocalForceClose '6'

			typedef char TShZdOffsetFlagType;
			*/
			TShZdOffsetFlagType oc = TSHZD_OF_Open;
			if (o->get_open_close() == OrderOpenClose::Undef)
			{
				o->set_open_close(compute_open_close(o, m_bCloseToday));
			}
			switch (o->get_open_close())
			{
			case AtsType::OrderOpenClose::Open:
				break;

			case AtsType::OrderOpenClose::Close:
				oc = TSHZD_OF_Close;
				break;
			case AtsType::OrderOpenClose::CloseToday:
				oc = TSHZD_OF_CloseToday;
				break;

			default:

				break;
			}
			request->CombOffsetFlag[0] = oc;
			if (m_debug)
				loggerv2::info("zd_connection::market_create_order CombOffsetFlag is %c", oc);			

			/*
			/////////////////////////////////////////////////////////////////////////
			///报单价格条件类型
			/////////////////////////////////////////////////////////////////////////
			///限价单
			#define TSHZD_OPT_LimitPrice '1'
			///市价单
			#define TSHZD_OPT_AnyPrice '2'
			///限价止损
			#define TSHZD_OPT_BestPrice '3'
			///止损
			#define TSHZD_OPT_LastPrice '4'

			typedef char TShZdOrderPriceTypeType;
			*/
			switch (o->get_price_mode())
			{
				case AtsType::OrderPriceMode::Limit:
				{
					request->OrderPriceType = TSHZD_OPT_LimitPrice;
					//strcpy(request->LimitPrice, std::to_string(o->get_price()).c_str());
					request->LimitPrice = o->get_price();
				}
				break;
				case AtsType::OrderPriceMode::Market:
				{
					request->OrderPriceType = TSHZD_OPT_AnyPrice;
				}

				break;
				default:
				{
					snprintf(pszReason, REASON_MAXLENGTH, "undefined price mode.\n");
					o->set_status(AtsType::OrderStatus::Reject);
					zd_create_pool.free_mem(request);
					return 0;
				}
			}
			/*
			/////////////////////////////////////////////////////////////////////////
			///投机套保标志类型
			/////////////////////////////////////////////////////////////////////////
			///投机
			#define TSHZD_HF_Speculation '1'
			///套利
			#define TSHZD_HF_Arbitrage '2'
			///套保
			#define TSHZD_HF_Hedge '3'

			typedef char TShZdHedgeFlagType;
			*/
			request->CombHedgeFlag[0] = TSHZD_HF_Speculation;
			//request->TimeCondition = THOST_FTDC_TC_GFD;
			request->VolumeCondition = TSHZD_VC_AV;
			if (o->get_restriction() == AtsType::OrderRestriction::None)
			{
				request->TimeCondition = TSHZD_TC_DAY; // or GFS ???
				strcpy(request->GTDDate, "");
			}
			else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
			{
				request->TimeCondition   = TSHZD_TC_IOC;
				request->VolumeCondition = TSHZD_VC_CV;
			}
			else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			{
				request->TimeCondition  = TSHZD_TC_IOC;
				request->VolumeCondition = TSHZD_VC_AV;
			}
			else
			{
				snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
				zd_create_pool.free_mem(request);
				return 0;
			}
			//request->VolumeCondition = THOST_FTDC_VC_AV;
			request->ContingentCondition = TSHZD_CC_Immediately;
			request->ForceCloseReason    = TSHZD_FCC_NotForceClose;

			request->MinVolume = 1;
			//request->IsAutoSuspend = 0;
			//request->UserForceClose = 0;
//#if 1			
			sprintf(request->OrderLocalID, "%d", o->get_id());		
			char user_id[256];
			memset(user_id, 0, sizeof(user_id));
			compute_userId(o, user_id, sizeof(user_id));
			this->create_user_info(user_id, request->OrderLocalID);
//#endif
			if (!m_pZdApi->ReqOrderInsert(request))
			{				
				snprintf(pszReason, REASON_MAXLENGTH, "zd api reject!\n");
				zd_create_pool.free_mem(request);
				loggerv2::error("zd_connection::market_create_order_async fail to ReqOrderInsert,id:%d",o->get_id());
				return 0;
			}
			zd_create_pool.free_mem(request);
		}
		return 1;
	}	
	/*
	//撤单
	CTShZdOrderActionField pCancel;
	memset(&pCancel,0,sizeof(CTShZdInputOrderActionField));

	printf("依次输入系统号，订单号(空格分开)\n");
	scanf("%s%s",pCancel.OrderRef,pCancel.OrderSysID);
	pCancel.ActionFlag = '0';//0是撤单
	pCancel.OrderType ='P';
	apiTrade->ReqOrderAction(&pCancel,0);
	*/
	int zd_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());			
		if (o->get_way() != AtsType::OrderWay::Exercise)
		{
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
			CTShZdOrderActionField *request = zd_cancel_pool.get_mem();
			memset(request, 0, sizeof(CTShZdOrderActionField));

			strcpy(request->UserID, this->get_user_id().c_str());

			strcpy(request->OrderSysID, zd_order_aux::get_order_sys_id(o).c_str());
			sprintf(request->OrderRef,"%d", zd_order_aux::get_ord_ref(o));

			//
			if (zd_order_aux::get_ord_ref(o) < 1)
			{
				zd_cancel_pool.free_mem(request);
				loggerv2::info("zd_connection::market_cancel_order_async should not be called for the orderRef:%d is invalid", zd_order_aux::get_ord_ref(o));
				return -1;
			}
			//

			request->ActionFlag = TSHZD_AF_Delete;

			if (!m_pZdApi->ReqOrderAction(request))
			{
				zd_cancel_pool.free_mem(request);
				loggerv2::error("zd_connection::market_cancel_order_async fail to call the ReqOrderAction,id:%d",o->get_id());
				return 0;
			}
			m_pZdApi->getOrdInputActiondRefMap().insert(std::pair<string, int>(request->OrderRef, o->get_id()));
			loggerv2::info("zd_connection::market_cancel_order_async update the OrdInputActiondRefMap by the %s:%d",request->OrderRef, o->get_id());
			//
			zd_cancel_pool.free_mem(request);
			return 1;
		}
	}
	//
	//  zd callbacks
	//
	/*
	///直达输入报单
	struct CTShZdInputOrderField
	{
	///交易所代码  直达
	TShZdExchangeIDType	ExchangeID;
	///直达的资金账号  直达
	TShZdInvestorIDType	InvestorID;
	///合约代码 直达
	TShZdInstrumentIDType	InstrumentID;
	///系统编号  直达
	TShZdOrderSysIDType	OrderSysID;
	///本地报单编号  直达
	TShZdOrderLocalIDType	OrderLocalID;
	///用户代码  直达
	TShZdUserIDType	UserID;
	///报单价格条件   1限价单 2市价单 3限价止损（stop to limit），4止损（stop to market） 直达
	TShZdOrderPriceTypeType	OrderPriceType;
	///买卖方向   1买 2卖  直达
	TShZdDirectionType	Direction;
	///组合开平标志
	TShZdCombOffsetFlagType	CombOffsetFlag;
	///组合投机套保标志
	TShZdCombHedgeFlagType	CombHedgeFlag;
	///价格  直达
	TShZdPriceType	LimitPrice;
	///数量  直达
	TShZdVolumeType	VolumeTotalOriginal;
	///有效期类型  1=当日有效, 2=永久有效（GTC） 直达
	TShZdTimeConditionType	TimeCondition;
	///强平编号  直达
	TShZdDateType	GTDDate;
	///成交量类型  1=regular 2=FOK 3=IOC
	TShZdVolumeConditionType	VolumeCondition;
	///最小成交量  必须小于等于委托量；有效期=4时，ShowVolume>=1小于委托量时是FOK，等于委托量时是FAK  直达
	TShZdVolumeType	MinVolume;
	///触发条件
	TShZdContingentConditionType	ContingentCondition;
	///止损价  触发价  直达
	TShZdPriceType	StopPrice;
	///强平原因
	TShZdForceCloseReasonType	ForceCloseReason;
	/// 如果是冰山单，ShowVolume的值1到orderNumber，不是冰山单，ShowVolume的值为0  直达
	TShZdVolumeType	ShowVolume;
	///报单客户端类型  API的用户只需填写C 或者  P
	TShZdOrderTypeType OrderType;
	};
	*/
	void zd_connection::OnRspOrderInsertAsync(CTShZdInputOrderField* pOrder)
	{
		//
		// used only for rejects.
		//		
		// 0 - log
		if (m_debug)
		{
			//loggerv2::info("zd_connection::OnRspOrderInsert - orderRef[%s] userId[%s] RequestID[%d] errorId[%d]", pOrder->OrderRef, pOrder->UserID, pOrder->RequestID, errorId);
			loggerv2::info("zd_connection::OnRspOrderInsertAsync "
				"InstrumentID:%s,"
				"OrderSysID:%s,"
				"OrderLocalID:%s,"
				"InvestorID:%s,"
				"UserID:%s,"
				"OrderPriceType:%c,"
				"Direction:%c,"
				"LimitPrice:%f,"
				"VolumeTotalOriginal:%d,"
				"ForceCloseReason:%c,",
				pOrder->InstrumentID, 
				pOrder->OrderSysID, 
				pOrder->OrderLocalID, 
				pOrder->InvestorID, 
				pOrder->UserID, 
				pOrder->OrderPriceType, 
				pOrder->Direction, 
				pOrder->LimitPrice,
				pOrder->VolumeTotalOriginal,pOrder->ForceCloseReason);
		}
		// 1 - retrieve order
		OrderWay::type way = pOrder->Direction == TSHZD_D_Buy ? OrderWay::Buy : OrderWay::Sell;
		int orderId = get_order_id(get_user_id_ex(pOrder->OrderLocalID).c_str(), way);		
		if (orderId < 1)
		{
			orderId = FAKE_ID_MIN + atoi(pOrder->OrderLocalID);
		}
		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("zd_connection::OnRspOrderInsert - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = zd_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("zd_connection::OnRspOrderInsertAsync cannot anchor order");
				return;
			}
			add_pending_order(o);
			break;
		default:
			break;
		}		
		if (o == NULL) // should not happen
		{
			loggerv2::error("zd_connection::OnRspOrderInsertAsync - order recovered NULL");
			return;
		}		
		// 2 - treat message
		if (pOrder->ForceCloseReason == 'E')
		{
			char szErrorMsg[32 + 1];
			snprintf(szErrorMsg, sizeof(szErrorMsg), "error(%d)",pOrder->MinVolume);

			on_nack_from_market_cb(o, szErrorMsg);
			//bug fix. should not call update_instr_on_nack_from_market_cb, because we haven't ack the order before!
			//
			//update_instr_on_nack_from_market_cb(o);
		}
		else
		{
			//loggerv2::error("zd_connection::OnRspOrderInsertAsync - order[%d] errorId[0] ???", orderId);
		}
	}		
	void zd_connection::OnRspOrderActionAsync(int* nOrdId)
	{
		if (m_debug)
			loggerv2::info("zd_connection::OnRspOrderActionAsync,order[%d]", *nOrdId);
		int ret;
		order *o = get_order_from_map(*nOrdId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("zd_connection::OnRspOrderActionAsync - message received on dead order[%d]...", *nOrdId);
			break;
		case 2:
		default:
			break;
		}
		if (o == NULL) // should not happen
		{
			loggerv2::error("zd_connection::OnRspOrderActionAsync - order recovered NULL");
			return;
		}
		//
		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("zd_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}
		//
		update_instr_on_cancel_from_market_cb(o);
		on_cancel_from_market_cb(o);		
	}

/*
///直达报单
struct CTShZdOrderField
{	
///经纪公司代码  直达
TShZdBrokerIDType	BrokerID;
///资金账号 直达
TShZdInvestorIDType	InvestorID;
///合约代码  直达
TShZdInstrumentIDType	InstrumentID;
///订单号  直达
TShZdOrderRefType	OrderRef;
///用户代码   直达
TShZdUserIDType	UserID;
///报单价格类型   1限价单 2市价单 3限价止损（stop to limit），4止损（stop to market） 直达
TShZdOrderPriceTypeType	OrderPriceType;
///有效期类型  （1=当日有效, 2=永久有效（GTC），3=OPG，4=IOC，5=FOK，6=GTD，7=ATC，8=FAK） 直达
TShZdTimeConditionType	TimeCondition;
///买卖方向  直达
TShZdDirectionType	Direction;
///组合开平标志  
TShZdCombOffsetFlagType	CombOffsetFlag;
///组合投机套保标志
TShZdCombHedgeFlagType	CombHedgeFlag;
///价格  直达
TShZdPriceType	LimitPrice;
///数量   直达
TShZdVolumeType	VolumeTotalOriginal;	
///最小成交量  直达
TShZdVolumeType	MinVolume;	
///止损价、触发价  直达
TShZdPriceType	StopPrice;	
///请求编号  直达
TShZdRequestIDType	RequestID;
///本地编号  直达
TShZdOrderLocalIDType	OrderLocalID;
///交易所代码   直达
TShZdExchangeIDType	ExchangeID;	
///合约在交易所的代码  直达
TShZdExchangeInstIDType	ExchangeInstID;	
///订单状态 1、已请求 2、已排队 3、部分成交 4、完全成交 5、已撤余单 6、已撤单 7、指令失败  直达
TShZdOrderSubmitStatusType	OrderSubmitStatus;
/// 如果是冰山单，ShowVolume的值1到orderNumber，不是冰山单，ShowVolume的值为0  直达
TShZdVolumeType	ShowVolume;	
///交易日  直达
TShZdDateType	TradingDay;	
///系统号  直达
TShZdOrderSysIDType	OrderSysID;	
///报单类型  下单人类别 C客户下单  D是dealor下单 R 是强平（风控）F条件单 O第3方软件报单  直达
TShZdOrderTypeType	OrderType;
///今成交数量  直达
TShZdVolumeType	VolumeTraded;
///成交价格  直达
TShZdPriceType  PriceTraded;	
///报单日期  直达
TShZdDateType	InsertDate;
///委托时间  直达
TShZdTimeType	InsertTime;	
///撤单日期 直达
TShZdDateType  CancelDate;
///撤销时间    直达
TShZdTimeType	CancelTime;	
///用户强评标志  直达
TShZdBoolType	UserForceClose;	
///撤单号  直达
TShZdOrderSysIDType	RelativeOrderSysID;
};
*/
void zd_connection::OnRtnOrderAsync(CTShZdOrderField* pOrder)
	{
		if (m_debug)
		{
			loggerv2::info("zd_connection::OnRtnOrderAsync,"
				"BrokerID:%s,"
				"InvestorID:%s,"
				"InstrumentID:%s,"
				"OrderRef:%s,"
				"UserID:%s,"
				"Direction:%c,"
				"LimitPrice:%f,"
				"VolumeTotalOriginal:%d,"
				"OrderLocalID:%s,"
				"ExchangeID:%s,"
				"OrderSubmitStatus:%c,"
				"OrderSysID:%s,"
				"VolumeTraded:%d,"
				"PriceTraded:%f,"
				"InsertDate:%s,"
				"InsertTime:%s,"
				"CancelDate:%s,"
				"CancelTime:%s,"
				"TradingDay:%s,", 
				pOrder->BrokerID, 
				pOrder->InvestorID, 
				pOrder->InstrumentID, 
				pOrder->OrderRef, 
				pOrder->UserID, 
				pOrder->Direction,
				pOrder->LimitPrice,
				pOrder->VolumeTotalOriginal,
				pOrder->OrderLocalID,
				pOrder->ExchangeID,
				pOrder->OrderSubmitStatus,
				pOrder->OrderSysID,
				pOrder->VolumeTraded, 
				pOrder->PriceTraded, pOrder->InsertDate, pOrder->InsertTime, pOrder->CancelDate, pOrder->CancelTime, pOrder->TradingDay);
		}
		// 1 - retrieve order
		OrderWay::type way = pOrder->Direction == TSHZD_D_Buy ? OrderWay::Buy : OrderWay::Sell;
			
		int account, bidId, askId, portfolioId, ntradingType;
		
		get_user_info(get_user_id_ex(pOrder->OrderLocalID).c_str(), account, bidId, askId, portfolioId, ntradingType);

		int orderId = (way==OrderWay::Buy && bidId > 0) ? bidId : askId;
			
		if (orderId < 1)
		{			
			orderId = FAKE_ID_MIN + atoi(pOrder->OrderLocalID);
		}

		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("zd_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = zd_order_aux::anchor(this, pOrder);
			if (o == nullptr)
			{
				loggerv2::error("zd_connection::OnRtnOrderAsync cannot anchor order");
				return;
			}
			add_pending_order(o);
			break;
		default:
			break;
		}		
		if (o == nullptr) // should not happen
		{
			loggerv2::error("zd_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}
		//
		zd_order_aux::set_order_sys_id(o, pOrder->OrderSysID);
		zd_order_aux::set_ord_ref(o, atoi(pOrder->OrderRef));
		//
		if (o->get_quantity() != pOrder->VolumeTotalOriginal)
		{
			if (m_debug)
				loggerv2::debug("zd_connection::OnRtnOrderAsync resetting order quantity to %d", pOrder->VolumeTotalOriginal);
			o->set_quantity(pOrder->VolumeTotalOriginal);
		}

		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("zd_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}
		/*
		/////////////////////////////////////////////////////////////////////////
		///报单提交状态类型
		///订单状态（1：已请求；2：已排队；3：部分成交；4：完全成交；5：已撤余单；6：已撤单；7：指令失败）
		/////////////////////////////////////////////////////////////////////////
		///已请求
		#define TSHZD_OSS_InsertSubmitted '1'
		///已排队
		#define TSHZD_OSS_Accepted '2'
		///部分成交
		#define TSHZD_OSS_PartTraded '3'
		///完全成交
		#define TSHZD_OSS_AllTraded '4'
		///已撤余单
		#define TSHZD_OSS_CancelSub '5'
		///已撤单
		#define TSHZD_OSS_CancelAll '6'
		///指令失败
		#define TSHZD_OSS_Rejected '7'

		typedef char TShZdOrderSubmitStatusType;
		*/	
		switch (pOrder->OrderSubmitStatus)
		{
		case TSHZD_OSS_InsertSubmitted:
		case TSHZD_OSS_Accepted://ok
		case TSHZD_OSS_PartTraded:
		case TSHZD_OSS_AllTraded:
			{
				string ts = pOrder->InsertDate;
#if 0
				ts.insert(10, " ");
#else
				char buffer[64];
				memset(buffer, 0, sizeof(buffer));
				if (strlen(pOrder->InsertDate) > 0)
				{
					sprintf(buffer, "%s %s", pOrder->InsertDate, pOrder->InsertTime);
				}
				ts = buffer;
#endif
				auto tp = string_to_lwtp(ts.c_str());
				o->set_last_time(tp);
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{

					update_instr_on_ack_from_market_cb(o);
					on_ack_from_market_cb(o);
				}
			}
			break;
		case TSHZD_OSS_CancelAll:
		case TSHZD_OSS_CancelSub:
			{
				on_cancel_from_market_cb(o);
			}
			break;		
		case TSHZD_OSS_Rejected:
			update_instr_on_nack_from_market_cb(o);
			on_nack_from_market_cb(o, "");
			break;
		default:
			//loggerv2::info("zd_connection::OnRtnOrderAsync didn't do with the status:%d",pOrder->OrderSubmitStatus);
			string ts = pOrder->InsertDate;
#if 0
			ts.insert(10, " ");
#else
			char buffer[64];
			memset(buffer, 0, sizeof(buffer));
			if (strlen(pOrder->InsertDate) > 0)
			{
				sprintf(buffer, "%s %s", pOrder->InsertDate, pOrder->InsertTime);
			}
			ts = buffer;
#endif
			auto tp = string_to_lwtp(ts.c_str());
			o->set_last_time(tp);
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{

				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}
			break;
		}
	}

	/*
	///直达成交
	struct CTShZdTradeField
	{
	///资金账号  直达
	TShZdInvestorIDType	InvestorID;
	///合约代码  直达
	TShZdInstrumentIDType	InstrumentID;
	///订单编号  直达
	TShZdOrderRefType	OrderRef;
	///用户代码  直达
	TShZdUserIDType	UserID;
	///交易所代码  直达
	TShZdExchangeIDType	ExchangeID;
	///成交编号   直达
	TShZdTradeIDType	TradeID;
	///买卖方向  直达
	TShZdDirectionType	Direction;
	///系统编号  直达
	TShZdOrderSysIDType	OrderSysID;
	///开平标志  直达
	TShZdOffsetFlagType	OffsetFlag;
	///投机套保标志
	TShZdHedgeFlagType	HedgeFlag;
	///价格  直达
	TShZdPriceType	Price;
	///数量  直达
	TShZdVolumeType	Volume;
	///成交时期  直达
	TShZdDateType	TradeDate;
	///成交时间   直达
	TShZdTimeType	TradeTime;
	///成交类型
	TShZdTradeTypeType	TradeType;
	///本地报单编号   直达
	TShZdOrderLocalIDType	OrderLocalID;
	///调期后的交割日期
	TShZdDateType	ChangeTradingDay;
	///成交手续费
	TShZdPriceType	PriceFree;
	};
	*/
void zd_connection::OnRtnTradeAsync(CTShZdTradeField* pTrade)
{
	// 0 - log
	//loggerv2::info();
	if (m_debug)
		loggerv2::info("zd_connection::OnRtnTradeAsync,"
		"InvestorID[%*.*s] "
		"InstrumentID[%*.*s] "
		"OrderRef[%*.*s] "
		"UserID[%*.*s] "

		"OffsetFlag[%c] "
		"HedgeFlag[%c] "
		"Price[%f] "
		"Volume[%d] "
		"TradeTime[%*.*s] "
		"TradeType[%c] "
		"OrderLocalID:%s,"
		"OrderSysID:%s,"
		"Direction:%c,"
		"TradeID:%s,"
		"TradeDate:%s",

		sizeof(pTrade->InvestorID), sizeof(pTrade->InvestorID), pTrade->InvestorID,
		sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID,
		sizeof(pTrade->OrderRef), sizeof(pTrade->OrderRef), pTrade->OrderRef,
		sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID,

		pTrade->OffsetFlag,
		pTrade->HedgeFlag,
		pTrade->Price,

		pTrade->Volume,
		sizeof(pTrade->TradeTime), sizeof(pTrade->TradeTime), pTrade->TradeTime,
		pTrade->TradeType,
		pTrade->OrderLocalID, pTrade->OrderSysID, pTrade->Direction, pTrade->TradeID,pTrade->TradeDate
		);

	OrderWay::type way = pTrade->Direction == TSHZD_D_Buy ? OrderWay::Buy : OrderWay::Sell;
	bool onlyUpdatePending = false;

	int account, bidId, askId, portfolioId, ntradingType;
	get_user_info(get_user_id_ex(pTrade->OrderLocalID).c_str(), account, bidId, askId, portfolioId, ntradingType);

	int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;	
	if (orderId < 1)
	{
		orderId = FAKE_ID_MIN + atoi(pTrade->OrderLocalID);
	}
	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("zd_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
		break;
	case 2:
		o = zd_order_aux::anchor(this, pTrade);
		if (o == NULL)
		{
			loggerv2::error("zd_connection::OnRtnTradeAsync cannot anchor order");
			return;
		}
		add_pending_order(o);
		break;
	default:
		break;
	}
	if (o == NULL) // should not happen
	{
		loggerv2::error("zd_connection::OnRtnTradeAsync - order recovered NULL");
		return;
	}
	// 2 - treat message
	int execQty = pTrade->Volume;
	double execPrc = pTrade->Price;
	const char* pszExecRef = pTrade->TradeID;
	const char* pszTime = pTrade->TradeTime;
	exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);
	on_exec_from_market_cb(o, e);
	if (account == m_account_num)//这个回报对应的account是对的，按orderID来判断这个回报是否为历史回报
	{
		if (orderId > m_pZdApi->m_begin_Id)//当前id大于beginID，这个回报不是历史回包
			onlyUpdatePending = false;
		else
			onlyUpdatePending = true;
	}
	else	
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
		ptime t(time_from_string(ts));
		lwtp tp = ptime_to_lwtp(t);

		//int hour = get_hour_from_lwtp(tp);
		tp = tp + std::chrono::seconds(2);//允许2s的误差

		//if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
		//	onlyUpdatePending = true;
		//if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
		//	onlyUpdatePending = true;

		if (o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;

		if (onlyUpdatePending)
		{
			loggerv2::info("zd_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time. tradeTime %s", pTrade->TradeTime);
		}		
	}
	update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
}

std::string zd_connection::getMaturity(std::string& sMat)
{
	std::string newMat;
	newMat = sMat.substr(0, 4);
	newMat += "-";
	newMat += sMat.substr(4, 2);
	newMat += "-";
	newMat += sMat.substr(6, 2);
	return newMat.c_str();
}

string get_instrument_class(string code)
	{
		char buffer[32];
		memset(buffer, 0, sizeof(buffer));
		strcat(buffer, code.c_str());
		string str;
		for (int i = 0; i < strlen(buffer); i++)
		{
			if (isdigit(buffer[i]) == 0)
			{
				str.push_back(buffer[i]);
			}
			else
				break;
		}		
		transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(std::toupper));
		return str;
	}
/*
OnRspQryInstrument,InstrumentID:GC1801,ExchangeID:CME,InstrumentName:纽期金1801,ExchangeInstID:1801,ProductID:GC,ProductName:纽期金,ProductClass:F,bIsLast:0,
*/
void zd_connection::OnRspQryInstrument_Future(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{		
//#if 1
		std::string sInstr = std::string(pInstrument->InstrumentID);
		boost::trim(sInstr);			
		std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";			
		char *zErrMsg = 0;
		std::string sUnderlying = pInstrument->ProductID; //
		if (sUnderlying == "")
		{
			printf_ex("zd_connection::OnRspQryInstrument_Future sInstr：%s,sUnderlyiny:%s,instrument_class:%s\n", sInstr.c_str(), sUnderlying.c_str(), get_instrument_class(sInstr).c_str());
			sUnderlying = get_instrument_class(sInstr);
		}
		std::string sInstClass = "F_" + sUnderlying;
		std::string strExecDate = pInstrument->ExpireDate;
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = pInstrument->ExchangeID;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Futures VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future:failed to insert into database, ret is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future cmd:%s\n", sCmd.c_str());
				printf_ex("zd_connection::OnRspQryInstrument_Future cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@" + get_type();
			sCmd = "UPDATE Futures SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "',";			
			sCmd += "ConnectionCodes='" + sConnectionCodes + "',";			
			sCmd += "Underlying='" + sUnderlying + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future:failed to update the database,error is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future update to the cmd:%s\n", sCmd.c_str());
				printf_ex("zd_connection::OnRspQryInstrument_Future update to the cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		m_database->close_databse();	
		if (bIsLast && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
//#endif
	}
/*
///直达合约
struct CTShZdInstrumentField
{
///合约代码  直达
TShZdInstrumentIDType	InstrumentID;
///交易所代码  直达
TShZdExchangeIDType	ExchangeID;
///合约名称  直达
TShZdInstrumentNameType	InstrumentName;
///合约在交易所的代码  直达
TShZdExchangeInstIDType	ExchangeInstID;
///交易所名称  直达
TShZdExchangeNameType ExchangeName;
///产品代码  直达
TShZdInstrumentIDType	ProductID;
///产品名称  直达
TShZdInstrumentNameType	ProductName;
///产品类型  F期货 O期权  直达
TShZdProductClassType	ProductClass;
///合约货币代码  直达
TShZdCurrencyNoType  CurrencyNo;
///货币名称  直达
TShZdCurrencyNameType  CurrencyName;	
///行情小数为数 直达
TShZdVolumeType	MarketDot;
///行情进阶单位 10进制 32进制  64进制等 直达
TShZdVolumeType	MarketUnit;
///调期小时点位数  直达
TShZdVolumeType	ChangeMarketDot;
///合约数量乘数  点值（一个最小跳点的价值） 直达
TShZdPriceType	VolumeMultiple;
///调期最小变动单位  直达
TShZdPriceType	ChangeMultiple;
///最小变动价位  直达
TShZdPriceType	PriceTick;	
///交割月日  直达
TShZdDateType	StartDelivDate;
///最后更新日  直达
TShZdDateType	LastUpdateDate;
///首次通知日 直达
TShZdDateType	ExpireDate;
///最后交易日  直达
TShZdDateType	EndTradeDate;	
///当前是否交易
TShZdBoolType	IsTrading;
///期权类型
TShZdOptionTypeType	OptionType;
///期权年月  直达
TShZdDateType	OptionDate;
///保证金率  直达
TShZdRatioType	MarginRatio;
///固定保证金  直达
TShZdRatioType	MarginValue;
///手续费率  直达
TShZdRatioType	FreeRatio;
///固定手续费  直达
TShZdRatioType	FreeValue;
///现货商品昨结算价  直达
TShZdPriceType  SpotYesSetPrice;
///现货商品点值  直达
TShZdPriceType  SpotMultiple;
///现货商品最小变动单位  直达
TShZdPriceType	SpotTick;
///期权临界价格  直达
TShZdPriceType	OptionTickPrice;
///期权临界价格以下最小跳点  直达
TShZdPriceType	OptionTick;
///期权执行价  直达
TShZdPriceType	OptionPrice;
///期权对应期货的商品代码 直达
TShZdInstrumentIDType OptionCommodityNo;
///期权对应期货的合约代码 直达
TShZdInstrumentIDType OptionContractNo;
};
*/
void zd_connection::OnRspQryInstrument(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInstrument == nullptr)
	{
		loggerv2::error("zd_connection::OnRspQryInstrument error,nullptr");
	}
	else
	{
		if (m_debug)
		{
			loggerv2::info("zd_connection::OnRspQryInstrument,"
				"InstrumentID:%s,"
				"ExchangeID:%s,"
				"InstrumentName:%s,"
				"ExchangeInstID:%s,"
				"ProductID:%s,"
				"ProductName:%s,"
				"ProductClass:%c,"
				"ExpireDate:%s,"
				"EndTradeDate:%s,"
				"bIsLast:%d,",
				pInstrument->InstrumentID,
				pInstrument->ExchangeID,
				pInstrument->InstrumentName,
				pInstrument->ExchangeInstID,
				pInstrument->ProductID,
				pInstrument->ProductName,
				pInstrument->ProductClass,
				pInstrument->ExpireDate,
				pInstrument->EndTradeDate,
				bIsLast
				);
		}	
		/*
		/////////////////////////////////////////////////////////////////////////
		///产品类型类型
		/////////////////////////////////////////////////////////////////////////
		///期货
		#define TSHZD_PC_Futures 'F'
		///期权
		#define TSHZD_PC_Options 'O'
		///组合
		#define TSHZD_PC_Combination 'C'
		///即期
		#define TSHZD_PC_Spot 'S'
		///期转现
		#define TSHZD_PC_EFP 'T'

		typedef char TShZdProductClassType;
		*/
		if ((pInstrument->ProductClass != TSHZD_PC_Options && pInstrument->ProductClass != TSHZD_PC_Futures) || strlen(pInstrument->ExpireDate) < 1 /*|| strlen(pInstrument->ExchangeInstID) < 10*/)
		{
			if (bIsLast && this->get_is_last() == false)
			{
				this->set_is_last(true);
			}
			return;
		}
			
		if (pInstrument->ProductClass == TSHZD_PC_Futures)
			return OnRspQryInstrument_Future(pInstrument,pRspInfo,nRequestID,bIsLast);
#if 0
		std::string sInstr = std::string(pInstrument->ExchangeInstID);
		boost::trim(sInstr);
		printf_ex("zd_connection::OnRspQryInstrument sInstr：%s,bIsLast:%d,pInstrument->ProductClass:%d\n", sInstr.c_str(), bIsLast, pInstrument->ProductClass);

		std::string sSearch = "select * from Options where Code= '" + sInstr + "'";
		//const char* data = "Callback function called";
		char *zErrMsg = 0;

		std::string sUnderlying = pInstrument->UnderlyingInstrID; //
		std::string sCP = "C";  //"CallPut"
		switch (pInstrument->OptionsType)
		{
		case THOST_FTDC_CP_CallOptions:
			sCP = "C";
			break;
		case THOST_FTDC_CP_PutOptions:
		default:
			sCP = "P";
			break;
		}

		std::string sInstClass = "O_" + get_instrument_class(sInstr);
		std::string strExecDate = pInstrument->ExpireDate;
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = pInstrument->ExchangeID;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Options VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + std::to_string(pInstrument->StrikePrice) + "',";
			sCmd += "'" + std::to_string(pInstrument->VolumeMultiple) + "',";
			sCmd += "'" + sCP + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				//loggerv2::info("failed to insert into database, ret is %d",rc);
				//sqlite3_free(zErrMsg);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@" + get_type();
			sCmd = "UPDATE Options SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "Strike = '" + std::to_string(pInstrument->StrikePrice) + "',";
			sCmd += "PointValue ='" + std::to_string(pInstrument->VolumeMultiple) + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "',";
			sCmd += "ConnectionCodes='" + sConnectionCodes + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				//loggerv2::info("failed to update the database,error is %d",rc);
				//sqlite3_free(zErrMsg);
			}
		}
		m_database->close_databse();
		if (bIsLast && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
#endif
	}
}
void zd_connection::init_user_info(char * user_info_file)
{
	if (user_info_file == nullptr)
		return;
	boost::filesystem::path p;
	p.clear();
	p.append(user_info_file);
	p.append("user_info.csv");
	m_user_info_file_name = p.string();
	printf_ex("es_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
		user_info * info = new user_info();
		info->OrderLocalID = tokenizer[0];
		info->UserID       = tokenizer[1];
		m_user_info_map.emplace(info->OrderLocalID, info);
	}
	stream.close();
}
void zd_connection::append(user_info * info)
{
	if (info == nullptr)
		return;
	boost::filesystem::ofstream stream;
	stream.open(m_user_info_file_name.c_str(), ios::app);
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s,%s\n", info->OrderLocalID.c_str(), info->UserID.c_str());
	stream << buffer;
	stream.close();
}
void zd_connection::OnUserInfoAsync(user_info* pInfo)
{
	this->append(pInfo);
}
void zd_connection::create_user_info(string UserID,string orderLocalId)
{
	if (m_user_info_map.find(orderLocalId) == m_user_info_map.end())
	{
		user_info * info = new user_info();
		info->OrderLocalID = orderLocalId;
		info->UserID       = UserID;

		m_user_info_map.emplace(info->OrderLocalID, info);

		//to do ... append the file every day
		m_userInfoQueue.CopyPush(info);			
	}
	else
	{
		//printf_ex("warn:sl_connection::create_user_info already include the OrderNo:%s\n", pField->OrderNo);
	}
}
string zd_connection::get_user_id_ex(string orderLocalId)
{
	if (m_user_info_map.find(orderLocalId) != m_user_info_map.end())
	{
		user_info * info = m_user_info_map[orderLocalId];
		return info->UserID;
	}
	return "";
}

}

