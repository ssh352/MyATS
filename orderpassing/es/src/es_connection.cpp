#include "es_connection.h"
#include "string_tokenizer.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>
#include "tradeItem_gh.h"
#include "terra_logger.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "iconnection_event_handler.h"
using namespace boost::posix_time;
using namespace terra::common;
namespace es
{
	//
	// es_connection
	//
	es_connection::es_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "es_connection";		
		m_connectionStatus = false;
		m_isAlive = true;
		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		m_bKey_with_exchange = false;
		m_pUserApi = nullptr;
	}
	bool es_connection::init_config(const string &name, const std::string &strConfigFile)
	{
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 4)
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
		printf_ex("es_connection::init_config %s\n", GetTapTradeAPIVersion());
		//
		TAPIINT32 iResult = TAPIERROR_SUCCEED;
		TapAPIApplicationInfo stAppInfo;
		memset(&stAppInfo, 0, sizeof(TapAPIApplicationInfo));
		strncpy(stAppInfo.AuthCode, m_strAuthCode.c_str(),512);
		
		m_strApiLogPath = root.get<string>(name + ".log_path", "");
		printf_ex("es_connection::init_config KeyOperationLogPath:%s\n", m_strApiLogPath.c_str());
		loggerv2::info("es_connection::init_config KeyOperationLogPath:%s\n", m_strApiLogPath.c_str());
		strcpy(stAppInfo.KeyOperationLogPath, m_strApiLogPath.c_str());

		m_pUserApi = CreateTapTradeAPI(&stAppInfo, iResult);
		if (m_pUserApi != nullptr)
		{
			m_pUserApi->SetAPINotify(this);
		}
		else
		{
			printf_ex("es_connection::init_config m_pUserApi is nullptr,auth:%d\n", strlen(stAppInfo.AuthCode));
			loggerv2::error("es_connection::init_config m_pUserApi is Null,auth:%d\n", strlen(stAppInfo.AuthCode));
			return false;
		}
		return true;
	}
	void es_connection::request_investor_full_positions()
	{		
		for (auto &it : m_trade_item_map)
		{
			it.second->set_yst_long_position(0);
			it.second->set_today_long_position(0);
			it.second->set_tot_long_position(0);

			it.second->set_yst_short_position(0);
			it.second->set_today_short_position(0);
			it.second->set_tot_short_position(0);
		}
		TAPIUINT32 sessionId = 0;		
		TapAPIPositionQryReq PositionQryReq;
		this->m_pUserApi->QryPosition(&sessionId, &PositionQryReq);
	}
	//
	void es_connection::req_RiskDegree()
	{
		/*
		//! 资金查询请求
		struct TapAPIFundReq
		{
		TAPISTR_20					AccountNo;						///< 客户资金帐号
		};
		*/
		TapAPIFundReq FundReq;
		TAPIUINT32 sessionId = 0;
		strcpy(FundReq.AccountNo, this->m_sUsername.c_str());
		this->m_pUserApi->QryFund(&sessionId, &FundReq);
		//loggerv2::info("QryFund:%s", FundReq.AccountNo);
	}
	///请求查询资金账户
	void es_connection::request_trading_account()
	{
    	if (this->m_pUserApi != nullptr)
		{
			///查询帐户资金
			TAPIUINT32 sessionId = 0;
			TapAPIAccQryReq tapAPIAccQryReq;
			strcpy(tapAPIAccQryReq.AccountNo, this->m_sUsername.c_str());
			this->m_pUserApi->QryAccount(&sessionId, &tapAPIAccQryReq);
		}		
	}
	void es_connection::init_connection()
	{
		this->init_api();
		//init_process(io_service_type::trader, 10);
#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
	}

#ifdef Linux
	bool es_connection::write_fd()
	{
		uint64_t buf = 1;
		int wlen = 0;
		while(1)
		{
			wlen = write(efd, &buf, sizeof(buf));
			if (wlen >= 0)
				return true;
			else
			{
				if (errno == EAGAIN || errno == EINTR)
				{
					continue;
				}
				else
				{
					loggerv2::error("write efd fail");
					break;
				}
			}
		}
		return false;
	}


	void  es_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&es_connection::process, this));
		m_orderQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_userInfoQueue.set_fd(efd);
		
		m_outboundQueue.set_fd(efd);
		m_outquoteboundQueue.set_fd(efd);
	}
#endif

	void es_connection::release()
	{
		ctpbase_connection::release();
		this->release_api();
	}
	void es_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("es_connection::connect connecting to");

			on_status_changed(AtsType::ConnectionStatus::WaitConnect);

			this->connect_api(); 
		}
	}
	void es_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{		
			if (this->disconnect_api() == true)
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "es_connection - disconnect failed");
			}
	    }
    }
	void es_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_outquoteboundQueue.Pops_Handle_Keep(10);
#ifdef Linux
		bool vaild = m_outboundQueue.read_available() || m_outquoteboundQueue.read_available();
		if (vaild)
		{
			write_fd();
		}
#endif
		this->Process_api();
	}
	/*
	//! 客户下单请求结构
	struct TapAPINewOrder
	{
	TAPISTR_20					AccountNo;						///< 客户资金帐号
	TAPISTR_10					ExchangeNo;						///< 交易所编号
	TAPICommodityType			CommodityType;					///< 品种类型
	TAPISTR_10					CommodityNo;					///< 品种编码类型
	TAPISTR_10					ContractNo;						///< 合约1
	TAPISTR_10					StrikePrice;					///< 执行价格1
	TAPICallOrPutFlagType		CallOrPutFlag;					///< 看张看跌1
	TAPISTR_10					ContractNo2;					///< 合约2
	TAPISTR_10					StrikePrice2;					///< 执行价格2
	TAPICallOrPutFlagType		CallOrPutFlag2;					///< 看张看跌2
	TAPIOrderTypeType			OrderType;						///< 委托类型
	TAPIOrderSourceType			OrderSource;					///< 委托来源
	TAPITimeInForceType			TimeInForce;					///< 委托有效类型
	TAPIDATETIME				ExpireTime;						///< 有效日期(GTD情况下使用)
	TAPIYNFLAG					IsRiskOrder;					///< 是否风险报单
	TAPISideType				OrderSide;						///< 买入卖出
	TAPIPositionEffectType		PositionEffect;					///< 开平标志1
	TAPIPositionEffectType		PositionEffect2;				///< 开平标志2
	TAPISTR_50					InquiryNo;						///< 询价号
	TAPIHedgeFlagType			HedgeFlag;						///< 投机保值
	TAPIREAL64					OrderPrice;						///< 委托价格1
	TAPIREAL64					OrderPrice2;					///< 委托价格2，做市商应价使用
	TAPIREAL64					StopPrice;						///< 触发价格
	TAPIUINT32					OrderQty;						///< 委托数量
	TAPIUINT32					OrderMinQty;					///< 最小成交量
	TAPIUINT32					MinClipSize;					///< 冰山单最小随机量
	TAPIUINT32					MaxClipSize;					///< 冰山单最大随机量
	TAPIINT32					RefInt;							///< 整型参考值
	TAPISTR_50					RefString;						///< 字符串参考值
	TAPITacticsTypeType			TacticsType;					///< 策略单类型
	TAPITriggerConditionType	TriggerCondition;				///< 触发条件
	TAPITriggerPriceTypeType	TriggerPriceType;				///< 触发价格类型
	TAPIYNFLAG					AddOneIsValid;					///< 是否T+1有效
	TAPIUINT32					OrderQty2;						///< 委托数量2
	TAPIHedgeFlagType			HedgeFlag2;						///< 投机保值2
	TAPIMarketLevelType			MarketLevel;					///< 市价撮合深度
	TAPIYNFLAG					OrderDeleteByDisConnFlag;		///< 心跳检测失败时，服务器自动撤单标识
	TAPISTR_10					UpperChannelNo;					///< 上手通道号
	};

	*/
	int es_connection::market_create_order_async(order* o, char* pszReason)
	{		
		char userId[32];
		memset(userId, 0, sizeof(userId));
		compute_userId(o, userId,sizeof(userId));

		TapAPINewOrder *request = es_create_pool.get_mem();
		memset(request, 0, sizeof(TapAPINewOrder));
		strcpy(request->AccountNo, this->m_sUsername.c_str());
		request->CallOrPutFlag  = TAPI_CALLPUT_FLAG_NONE;
		request->CallOrPutFlag2 = TAPI_CALLPUT_FLAG_NONE;

		if (o->get_instrument()->getInstrument()->get_exchange().find("ZCE") != string::npos)
		{
			strcpy(request->ExchangeNo, "ZCE");
		}
		else
		{
			strcpy(request->ExchangeNo, o->get_instrument()->getInstrument()->get_exchange().c_str());
		}
		switch (o->get_instrument()->getInstrument()->get_type())
		{
		case  AtsType::InstrType::type::Future:
		case  AtsType::InstrType::type::Futurespread:
			request->CommodityType = TAPI_COMMODITY_TYPE_FUTURES;
			break;
		case  AtsType::InstrType::type::Call:
			request->CommodityType = TAPI_COMMODITY_TYPE_OPTION;
			request->CallOrPutFlag = TAPI_CALLPUT_FLAG_CALL;
			strcpy(request->StrikePrice, get_Strike(o->get_instrument()->get_trading_code()).c_str());
			break;
		case  AtsType::InstrType::type::Put:
			request->CommodityType = TAPI_COMMODITY_TYPE_OPTION;
			request->CallOrPutFlag = TAPI_CALLPUT_FLAG_PUT;
			strcpy(request->StrikePrice, get_Strike(o->get_instrument()->get_trading_code()).c_str());
			break;
		default:
			break;
		}		
		strcpy(request->CommodityNo,get_CommodityNo(o->get_instrument()->get_trading_code()).c_str());
		strcpy(request->ContractNo,get_ContractNo(o->get_instrument()->get_trading_code()).c_str());
		/*
		委托来源
		*/
		request->OrderSource    = TAPI_ORDER_SOURCE_ESUNNY_API;
		/*
		委托有效类型,to do ...
		*/
		request->TimeInForce    = TAPI_ORDER_TIMEINFORCE_GFD;
		/*
		买卖类型
		*/
		request->OrderSide      = TAPI_SIDE_NONE;
		/*
		开平类型,to do ...
		*/
		request->PositionEffect = TAPI_PositionEffect_NONE;
		request->PositionEffect2= TAPI_PositionEffect_NONE;
		/*
		投机保值类型
		*/
		request->HedgeFlag      = TAPI_HEDGEFLAG_T;
		request->OrderPrice     = o->get_price();
		request->OrderQty       = o->get_quantity();		
		/*
		策略单类型
		*/
		request->TacticsType      = TAPI_TACTICS_TYPE_NONE;
		/*
		触发条件类型
		*/
		request->TriggerCondition = TAPI_TRIGGER_CONDITION_NONE;
		/*
		触发价格类型
		*/
		request->TriggerPriceType = TAPI_TRIGGER_PRICE_NONE;
		/*
		市价撮合深度
		*/
		request->MarketLevel      = TAPI_MARKET_LEVEL_0;

		request->IsRiskOrder      = APIYNFLAG_NO;

		request->OrderDeleteByDisConnFlag = APIYNFLAG_NO;

		request->AddOneIsValid            = APIYNFLAG_NO;
				
		request->HedgeFlag2               = TAPI_HEDGEFLAG_NONE;

		request->OrderQty2                = 0;

		if (o->get_open_close() == OrderOpenClose::Undef)
		{
			o->set_open_close(compute_open_close(o, m_bCloseToday));
		}

		if (o->get_way() == AtsType::OrderWay::Buy)
		{
			request->OrderSide = TAPI_SIDE_BUY;
		}
		else if (o->get_way() == AtsType::OrderWay::Sell)
		{
			request->OrderSide = TAPI_SIDE_SELL;
		}
		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
		{			
			request->PositionEffect = TAPI_PositionEffect_OPEN;
			break;
		}
		case AtsType::OrderOpenClose::Close:
		{
			request->PositionEffect = TAPI_PositionEffect_COVER;
			break;
		}
		case AtsType::OrderOpenClose::CloseToday:
		{
			request->PositionEffect = TAPI_PositionEffect_COVER_TODAY;
			break;
		}
		default:
			break;
		}
		if (o->get_restriction() == AtsType::OrderRestriction::None)
		{			
			request->TimeInForce = TAPI_ORDER_TIMEINFORCE_GFD;
		}
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
		{			
			request->TimeInForce = TAPI_ORDER_TIMEINFORCE_FOK;
		}
		else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
		{
			request->TimeInForce = TAPI_ORDER_TIMEINFORCE_FAK;
		}
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			es_create_pool.free_mem(request);
			return 0;
		}
		/*
		委托类型,to do ...
		*/
		if (o->get_way() != AtsType::OrderWay::Exercise)
		{
			request->OrderType = TAPI_ORDER_TYPE_LIMIT;
		}
		else
		{
			//期权行权
			request->OrderType  = TAPI_ORDER_TYPE_OPT_EXEC;
			request->OrderPrice = 0;
			request->OrderSide  = TAPI_SIDE_NONE;
			if (request->PositionEffect == TAPI_PositionEffect_OPEN)
			{
				request->PositionEffect = TAPI_PositionEffect_COVER;
				o->set_open_close(AtsType::OrderOpenClose::Close);
				//o->set_price_mode(OrderPriceMode::Undef);
			}
		}
		strcpy(request->RefString, userId);		
		//to do ...		
		if (!this->ReqOrderInsert(request))
		{			
			snprintf(pszReason, REASON_MAXLENGTH, "es api reject!\n");
			es_create_pool.free_mem(request);
			return 0;
		}		
		es_create_pool.free_mem(request);
		return 1;
	}
	/*
	//! 客户撤单请求结构
	struct TapAPIOrderCancelReq
	{
	TAPIINT32					RefInt;							///< 整型参考值
	TAPISTR_50					RefString;						///< 字符串参考值
	TAPICHAR					ServerFlag;						///< 服务器标识
	TAPISTR_20					OrderNo;						///< 委托编码
	};
	*/
	int es_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());				
		TapAPIOrderCancelReq *request = es_cancel_pool.get_mem();
		memset(request, 0, sizeof(TapAPIOrderCancelReq));
		//to do ...		
		strcpy(request->OrderNo,es_order_aux::get_order_no(o).c_str());
		//printf_ex("es_connection::market_cancel_order_async request->RefInt:%d,request->OrderNo:%s\n", request->RefInt, request->OrderNo);
		//end
		if (!this->ReqOrderAction(request))
		{
			es_cancel_pool.free_mem(request);
			return 0;
		}
		es_cancel_pool.free_mem(request);
		return 1;		
	}
	int es_connection::market_create_quote_async(quote* q, char* pszReason)
	{
		char userId[32];
		memset(userId, 0, sizeof(userId));
		compute_userId(q, userId, sizeof(userId));

		TapAPINewOrder *request = es_create_pool.get_mem();
		memset(request, 0, sizeof(TapAPINewOrder));
		strcpy(request->AccountNo, this->m_sUsername.c_str());
		request->CallOrPutFlag  = TAPI_CALLPUT_FLAG_NONE;
		request->CallOrPutFlag2 = TAPI_CALLPUT_FLAG_NONE;
		
		strcpy(request->InquiryNo, q->get_FQR_ID().c_str());

		if (q->get_instrument()->getInstrument()->get_exchange().find("ZCE") != string::npos)
		{
			strcpy(request->ExchangeNo, "ZCE");
		}
		else
		{
			strcpy(request->ExchangeNo, q->get_instrument()->getInstrument()->get_exchange().c_str());
		}
		switch (q->get_instrument()->getInstrument()->get_type())
		{
		case  AtsType::InstrType::type::Future:
		case  AtsType::InstrType::type::Futurespread:
			request->CommodityType = TAPI_COMMODITY_TYPE_FUTURES;
			break;
		case  AtsType::InstrType::type::Call:
			request->CommodityType = TAPI_COMMODITY_TYPE_OPTION;
			request->CallOrPutFlag = TAPI_CALLPUT_FLAG_CALL;
			strcpy(request->StrikePrice, get_Strike(q->get_instrument()->get_trading_code()).c_str());
			break;
		case  AtsType::InstrType::type::Put:
			request->CommodityType = TAPI_COMMODITY_TYPE_OPTION;
			request->CallOrPutFlag = TAPI_CALLPUT_FLAG_PUT;
			strcpy(request->StrikePrice, get_Strike(q->get_instrument()->get_trading_code()).c_str());
			break;
		default:
			break;
		}
		strcpy(request->CommodityNo, get_CommodityNo(q->get_instrument()->get_trading_code()).c_str());
		strcpy(request->ContractNo, get_ContractNo(q->get_instrument()->get_trading_code()).c_str());	
		/*
		委托类型,to do ...
		*/
		request->OrderType   = TAPI_ORDER_TYPE_RSPQUOT;
		/*
		委托来源
		*/
		request->OrderSource = TAPI_ORDER_SOURCE_ESUNNY_API;
		/*
		委托有效类型,to do ...
		*/
		request->TimeInForce = TAPI_ORDER_TIMEINFORCE_GFD;
		/*
		买卖类型
		*/
		request->OrderSide   = TAPI_SIDE_NONE;
		/*
		开平类型,to do ...
		*/
		request->PositionEffect  = TAPI_PositionEffect_OPEN;
		request->PositionEffect2 = TAPI_PositionEffect_OPEN;
		//
		if (q->get_bid_order()->get_open_close() == OrderOpenClose::Undef)
		{
			q->get_bid_order()->set_open_close(compute_open_close(q->get_bid_order(), m_bCloseToday));
		}

		switch (q->get_bid_order()->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:			
			request->PositionEffect = TAPI_PositionEffect_OPEN;
			break;
		case AtsType::OrderOpenClose::Close:			
			request->PositionEffect = TAPI_PositionEffect_COVER;
			break;
		case AtsType::OrderOpenClose::CloseToday:			
			request->PositionEffect = TAPI_PositionEffect_COVER_TODAY;
			break;
		default:
			break;
		}

		if (q->get_ask_order()->get_open_close() == OrderOpenClose::Undef)
		{
			q->get_ask_order()->set_open_close(compute_open_close(q->get_ask_order(), m_bCloseToday));
		}

		switch (q->get_ask_order()->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			request->PositionEffect2 = TAPI_PositionEffect_OPEN;
			break;
		case AtsType::OrderOpenClose::Close:
			request->PositionEffect2 = TAPI_PositionEffect_COVER;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->PositionEffect2 = TAPI_PositionEffect_COVER_TODAY;
			break;
		default:
			break;
		}
		//
		/*
		投机保值类型
		*/
		request->HedgeFlag   = TAPI_HEDGEFLAG_T;
		request->OrderPrice  = q->get_bid_order()->get_price();
		request->OrderQty    = q->get_bid_order()->get_quantity();

		request->OrderPrice2 = q->get_ask_order()->get_price();
		request->OrderQty2   = q->get_ask_order()->get_quantity();

		/*
		策略单类型
		*/
		request->TacticsType     = TAPI_TACTICS_TYPE_NONE;
		/*
		触发条件类型
		*/
		request->TriggerCondition = TAPI_TRIGGER_CONDITION_NONE;
		/*
		触发价格类型
		*/
		request->TriggerPriceType = TAPI_TRIGGER_PRICE_NONE;
		/*
		市价撮合深度
		*/
		request->MarketLevel = TAPI_MARKET_LEVEL_0;

		request->IsRiskOrder = APIYNFLAG_NO;

		request->OrderDeleteByDisConnFlag = APIYNFLAG_NO;

		request->AddOneIsValid = APIYNFLAG_NO;

		request->HedgeFlag2    = TAPI_HEDGEFLAG_NONE;			
		//request->RefInt = o->get_id();
		strcpy(request->RefString, userId);
		//to do ...		
		if (!this->ReqOrderInsert(request))
		{
			snprintf(pszReason, REASON_MAXLENGTH, "es api reject!\n");
			es_create_pool.free_mem(request);
			return 0;
		}
		es_create_pool.free_mem(request);
		return 1;
	}
	int es_connection::market_cancel_quote_async(quote* q, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_quote_async : %d", q->get_id());
		TapAPIOrderCancelReq *request = es_cancel_pool.get_mem();
		memset(request, 0, sizeof(TapAPIOrderCancelReq));
		//to do ...		
		strcpy(request->OrderNo,es_order_aux::get_order_no(q).c_str());
		//printf_ex("es_connection::market_cancel_quote_async id:%d,request->OrderNo:%s\n", q->get_id(), request->OrderNo);
		//end
		if (!this->ReqOrderAction(request))
		{
			es_cancel_pool.free_mem(request);
			return 0;
		}
		es_cancel_pool.free_mem(request);
		return 1;		
	}
	void es_connection::init_user_info(char * user_info_file)
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
			info->OrderNo = tokenizer[0];
			info->UserID  = tokenizer[1];			
			m_user_info_map.emplace(info->OrderNo, info);		
			/*to do ... create the order id map*/
#if 0
			int orderid = get_order_id(info->OrderNo);
			m_order_id_map.emplace(orderid,info->OrderNo);
#endif
		}
		stream.close();
	}
	void es_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s,%s\n",info->OrderNo.c_str(),info->UserID.c_str());
		stream << buffer;
		stream.close();
	}
	void es_connection::init_api()
	{
		m_orderQueue.setHandler(boost::bind(&es_connection::OnRtnOrderAsync, this, _1));
		m_tradeQueue.setHandler(boost::bind(&es_connection::OnRtnTradeAsync, this, _1));
		m_userInfoQueue.setHandler(boost::bind(&es_connection::OnUserInfoAsync, this, _1));
	}
	void es_connection::release_api()
	{
		if (m_pUserApi != nullptr)
		{
			m_pUserApi->Disconnect();
			FreeTapTradeAPI(m_pUserApi);
			m_pUserApi = nullptr;
		}
	}
	//
	// RTThread
	//
	void es_connection::Process_api()
	{
		m_orderQueue.Pops_Handle(0);
		m_tradeQueue.Pops_Handle(0);
		m_userInfoQueue.Pops_Handle(0);
#ifdef Linux
		bool vaild = m_orderQueue.read_available() || m_tradeQueue.read_available() || m_userInfoQueue.read_available();
		if (vaild)
		{
			write_fd();
		}
#endif
	}
	bool es_connection::connect_api()
	{
		//loggerv2::info("calling es_connection::connect");
		// For first connection, we are disconnected so we need to connect API first (RequestLogin will be done on API UP).
		// For later connections (disconnect / reconnect), API is already up so we just need to relogin.
		//
		if (m_connectionStatus == false)
		{
			if (m_pUserApi != nullptr)
			{
				TAPIINT32 iErr = TAPIERROR_SUCCEED;
				iErr = m_pUserApi->SetHostAddress(this->m_sHostname.c_str(), atoi(this->m_sService.c_str()));
				if (TAPIERROR_SUCCEED != iErr) {
					printf_ex("es_connection::init connect_api Error:%d\n", iErr);
					return false;
				}
				request_login();
				loggerv2::info("es_connection::connect api intialized");
			}		
		}
		else
		{
			request_login();
		}	
		return true;
	}
	bool es_connection::disconnect_api()
	{
		int ret = false;
		if (this->m_pUserApi)
		{
			ret = m_pUserApi->Disconnect();
			printf_ex("es_connection::disconnect_api ret:%d\n",ret);
		}
		return ret;
	}
	void es_connection::request_login()
	{
		loggerv2::info("es_connection::request_login");
		if (m_pUserApi != nullptr)
		{
			//to do ... 登录服务器
			TapAPITradeLoginAuth stLoginAuth;
			memset(&stLoginAuth, 0, sizeof(stLoginAuth));
			strcpy(stLoginAuth.UserNo, this->m_sUsername.c_str());
			strcpy(stLoginAuth.Password, this->m_sPassword.c_str());
			stLoginAuth.ISModifyPassword = APIYNFLAG_NO;
			stLoginAuth.ISDDA = APIYNFLAG_NO;
			/*
			发起登录请求。API将先连接服务，建立链路，发起登录认证。
			在使用函数函数前用户需要完成服务器的设置SetHostAddress()，并且创建TapAPIQuoteLoginAuth类型的用户信息， 并且需要设置好回调接口。
			连接建立后的用户验证回馈信息通过回调OnLogin()返回给用户。
			登录成功后API会自动进行API的初始化，API向服务器请求基础数据，查询完以后会通过回调OnAPIReady() 指示用户API初始化完成，可以进行后续的操作了。
			*/
			TAPIINT32 iErr = TAPIERROR_SUCCEED;
			iErr = m_pUserApi->Login(&stLoginAuth);
			if (TAPIERROR_SUCCEED != iErr) {
				cout << "Login Error:" << iErr << endl;
				return;
			}
		}
	}	
	//通过传入不同的参数order,来实现不同的功能:询价应价行权弃权
	bool es_connection::ReqOrderInsert(TapAPINewOrder* pRequest)
	{		
		//to do ... 
		TAPIUINT32 sessionId = 0;
		int ret = m_pUserApi->InsertOrder(&sessionId, (const TapAPINewOrder*)pRequest);
		//printf_ex("es_connection::ReqOrderInsert sessionId:%d,ret:%d\n",sessionId,ret);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}
	bool es_connection::ReqOrderAction(TapAPIOrderCancelReq* pRequest)
	{		
		TAPIUINT32 sessionId = 0;
		int ret = m_pUserApi->CancelOrder(&sessionId,pRequest);
		//printf_ex("es_connection::ReqOrderAction sessionId:%d,ret:%d\n", sessionId, ret);
		if (ret != 0)		
		{			
			return false;
		}
		return true;
	}
	void es_connection::request_instruments()
	{		
		TAPIUINT32 sessionId = 0;
		TapAPICommodity tapAPICommodity;
		if (m_pUserApi != nullptr)
		{
			memset(&tapAPICommodity, 0, sizeof(TapAPICommodity));
			//strcpy(tapAPICommodity.ExchangeNo, "ZCE");
			//tapAPICommodity.CommodityType = TAPI_COMMODITY_TYPE_OPTION;
			int ret = m_pUserApi->QryContract(&sessionId, &tapAPICommodity);
			printf_ex("es_connection::request_instruments QryContract ret:%d\n",ret);
			loggerv2::error("es_connection::request_instruments QryContract ret:%d\n", ret);
		}
	}
	void es_connection::request_commodity()
	{
		TAPIUINT32 sessionId = 0;		
		if (m_pUserApi != nullptr)
		{		
			int ret = m_pUserApi->QryCommodity(&sessionId);
			printf_ex("es_connection::request_instruments QryCommodity ret:%d\n", ret);
			loggerv2::error("es_connection::request_instruments QryCommodity ret:%d\n", ret);
		}
	}
	//end add on 20160929
	void TAP_CDECL es_connection::OnConnect()
	{
		loggerv2::info("es_connection::OnConnect\n");
		m_connectionStatus = true;
		//this->request_login();
	}
	void TAP_CDECL es_connection::OnRspLogin(TAPIINT32 errorCode, const TapAPITradeLoginRspInfo *loginRspInfo)
	{
		loggerv2::error("es_connection::OnRspLogin ret:%d\n",errorCode);
		if (errorCode == 0)
		{

		}
		else
		{
			this->on_status_changed(AtsType::ConnectionStatus::Disconnected,"");
		}
	}
	void TAP_CDECL es_connection::OnAPIReady()
	{
		this->on_status_changed(AtsType::ConnectionStatus::Connected, "");
		loggerv2::error("es_connection::OnAPIReady\n");
		//to do ...
		if (this->m_pUserApi != nullptr && m_bPosition == false)
		{
			TAPIUINT32 sessionId = 0;
			TapAPIOrderQryReq OrderQryReq;
			OrderQryReq.OrderQryType = TAPI_ORDER_QRY_TYPE_ALL;
			this->m_pUserApi->QryOrder(&sessionId, &OrderQryReq);
			//
		    //reconnect			
			TapAPIPositionQryReq PositionQryReq;
			this->m_pUserApi->QryPosition(&sessionId, &PositionQryReq);
			//m_bPosition = true;			
			//
			if (this->getRequestInstruments() == true)
			{
				this->request_instruments();
			}
		}
	}
	void TAP_CDECL es_connection::OnDisconnect(TAPIINT32 reasonCode)
	{
		this->on_status_changed(AtsType::ConnectionStatus::Disconnected, "");
		loggerv2::error("es_connection::OnDisconnect reasonCode:%d\n", reasonCode);
		m_connectionStatus = false;
	}
	void TAP_CDECL es_connection::OnRspChangePassword(TAPIUINT32 sessionID, TAPIINT32 errorCode)
	{
		printf_ex("es_connection::OnRspChangePassword\n");
	}
	void TAP_CDECL es_connection::OnRspSetReservedInfo(TAPIUINT32 sessionID, TAPIINT32 errorCode, const TAPISTR_50 info)
	{
		printf_ex("es_connection::OnRspSetReservedInfo\n");
	}
	/*
	8.获取用户资金账号
	*/
	void TAP_CDECL es_connection::OnRspQryAccount(TAPIUINT32 sessionID, TAPIUINT32 errorCode, TAPIYNFLAG isLast, const TapAPIAccountInfo *info)
	{
		printf_ex("es_connection::OnRspQryAccount\n");
		//to do ...

	}
	/*
	9.获取用户资金账号的详细信息
	*/
	void TAP_CDECL es_connection::OnRspQryFund(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIFundData *info)
	{		
		if (info)
		{
			double ratio = 0;
			
			if (info->AccountMaintenanceMargin + info->Available != 0)
			{
				ratio = info->AccountMaintenanceMargin / (info->AccountMaintenanceMargin + info->Available);
			}
			loggerv2::info("es_connection::OnRspQryFund,"
				"AccountNo:%s,"
				"AccountMaintenanceMargin:%f,"
				"Available:%f,"
				"ratio:%f", info->AccountNo, info->AccountMaintenanceMargin, info->Available,ratio);

			this->set_RiskDegree(ratio);
		}
	}
	/*
	用户资金变化通知

	用户的委托成交后会引起资金数据的变化，因此需要向用户实时反馈

	*/
	void TAP_CDECL es_connection::OnRtnFund(const TapAPIFundData *info)
	{
		/*
		//! 资金账号资金信息
		struct TapAPIFundData
		{
			TAPISTR_20					AccountNo;						///< 客户资金账号
			TAPISTR_20					ParentAccountNo;				///< 上级资金账号
			TAPISTR_10					CurrencyGroupNo;				///< 币种组号
			TAPISTR_10					CurrencyNo;						///< 币种号(为空表示币种组基币资金)
			TAPIREAL64					TradeRate;						///< 交易汇率
			TAPIFutureAlgType			FutureAlg;                      ///< 期货算法
			TAPIOptionAlgType			OptionAlg;                      ///< 期权算法
			TAPIREAL64					PreBalance;						///< 上日结存
			TAPIREAL64					PreUnExpProfit;					///< 上日未到期平盈
			TAPIREAL64					PreLMEPositionProfit;			///< 上日LME持仓平盈
			TAPIREAL64					PreEquity;						///< 上日权益
			TAPIREAL64					PreAvailable1;					///< 上日可用
			TAPIREAL64					PreMarketEquity;				///< 上日市值权益
			TAPIREAL64					CashInValue;					///< 入金
			TAPIREAL64					CashOutValue;					///< 出金
			TAPIREAL64					CashAdjustValue;				///< 资金调整
			TAPIREAL64					CashPledged;					///< 质押资金
			TAPIREAL64					FrozenFee;						///< 冻结手续费
			TAPIREAL64					FrozenDeposit;					///< 冻结保证金
			TAPIREAL64					AccountFee;						///< 客户手续费包含交割手续费
			TAPIREAL64					ExchangeFee;					///< 汇兑手续费
			TAPIREAL64					AccountDeliveryFee;				///< 客户交割手续费
			TAPIREAL64					PremiumIncome;					///< 权利金收取
			TAPIREAL64					PremiumPay;						///< 权利金支付
			TAPIREAL64					CloseProfit;					///< 平仓盈亏
			TAPIREAL64					DeliveryProfit;					///< 交割盈亏
			TAPIREAL64					UnExpProfit;					///< 未到期平盈
			TAPIREAL64					ExpProfit;						///< 到期平仓盈亏
			TAPIREAL64					PositionProfit;					///< 不含LME持仓盈亏 
			TAPIREAL64					LmePositionProfit;				///< LME持仓盈亏
			TAPIREAL64					OptionMarketValue;				///< 期权市值
			TAPIREAL64					AccountIntialMargin;			///< 客户初始保证金
			TAPIREAL64					AccountMaintenanceMargin;		///< 客户维持保证金
			TAPIREAL64					UpperInitalMargin;				///< 上手初始保证金
			TAPIREAL64					UpperMaintenanceMargin;			///< 上手维持保证金
			TAPIREAL64					Discount;						///< LME贴现
			TAPIREAL64					Balance;						///< 当日结存
			TAPIREAL64					Equity;							///< 当日权益
			TAPIREAL64					Available;						///< 当日可用
			TAPIREAL64					CanDraw;						///< 可提取
			TAPIREAL64					MarketEquity;					///< 账户市值
			TAPIREAL64					OriginalCashInOut;				///< 币种原始出入金
			TAPIREAL64					FloatingPL;						///< 逐笔浮盈
			TAPIREAL64					FrozenRiskFundValue;			///< 风险冻结资金
			TAPIREAL64					ClosePL;						///< 逐笔平盈
			TAPIREAL64					NoCurrencyPledgeValue;          ///< 非货币质押
			TAPIREAL64					PrePledgeValue;                 ///< 期初质押
			TAPIREAL64					PledgeIn;                       ///< 质入
			TAPIREAL64					PledgeOut;                      ///< 质出
			TAPIREAL64					PledgeValue;                    ///< 质押余额
			TAPIREAL64					BorrowValue;                    ///< 借用金额
			TAPIREAL64					SpecialAccountFrozenMargin;     ///< 特殊产品冻结保证金
			TAPIREAL64					SpecialAccountMargin;           ///< 特殊产品保证金   
			TAPIREAL64					SpecialAccountFrozenFee;        ///< 特殊产品冻结手续费
			TAPIREAL64					SpecialAccountFee;              ///< 特殊产品手续费
			TAPIREAL64					SpecialFloatProfit;             ///< 特殊产品浮盈
			TAPIREAL64					SpecialCloseProfit;             ///< 特殊产品平盈
			TAPIREAL64					SpecialFloatPL;                 ///< 特殊产品逐笔浮盈
			TAPIREAL64					SpecialClosePL;                 ///< 特殊产品逐笔平盈
		};
		*/
		//if (info)
		//{
		//	loggerv2::info("es_connection::OnRtnFund,"
		//		"AccountNo:%s,"
		//		"AccountMaintenanceMargin:%f,"
		//		"Available:%f,", info->AccountNo, info->AccountMaintenanceMargin, info->Available);
		//}
	}
	void TAP_CDECL es_connection::OnRspQryExchange(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIExchangeInfo *info)
	{
		//printf_ex("es_connection::OnRspQryExchange\n");
	}
	void TAP_CDECL es_connection::OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPICommodityInfo *info)
	{
		if (info->CommodityType != TAPI_COMMODITY_TYPE_OPTION)
		{
			if (isLast == APIYNFLAG_YES && this->get_is_last() == false)
			{
				this->set_is_last(true);
			}
			return;
		}
		//printf_ex("es_connection::OnRspQryCommodity,%s,%d\n", info->CommodityNo, info->ContractSize);
		std::string sInstClass = "O_" + string(info->CommodityNo);
		std::string sSearch = "select * from Options where InstrumentClass= '" + sInstClass + "'";
		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());
		std::string sCmd = "";
		sCmd = "UPDATE Options SET ";
		sCmd += "PointValue ='" + std::to_string((int)info->ContractSize) + "'";
		sCmd += " where InstrumentClass = '" + sInstClass + "';";
		if (pTree->size() > 0)
		{
			int rc = m_database->executeNonQuery(sCmd.c_str());
			if (rc == 0)
			{
				loggerv2::info("es_connection::OnRspQryCommodity:failed to update the database,error is %d", rc);
			}
			else
			{
				loggerv2::info("es_connection::OnRspQryCommodity:ok to update the database,%s", sCmd.c_str());
			}
		}
		m_database->close_databse();
		if (isLast == APIYNFLAG_YES && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
	}
	void es_connection::OnRspQryInstrument_Future(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPITradeContractInfo *info)
	{
		string symbol;
		switch (info->CommodityType)
		{
		case TAPI_COMMODITY_TYPE_FUTURES:
			symbol = string(info->CommodityNo) + string(info->ContractNo1);
			break;
		case TAPI_COMMODITY_TYPE_OPTION:
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%s%s%c%s", info->CommodityNo, info->ContractNo1, info->CallOrPutFlag1, info->StrikePrice1);
			symbol = buffer;
			break;
		default:
			break;
		}
		std::string sInstr = symbol;
		
		std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";
		char *zErrMsg = 0;
		std::string sUnderlying = info->CommodityNo; //

		std::string sInstClass = "F_" + string(info->CommodityNo);		
		std::string sMat = info->ContractExpDate;
		std::string sCmd = "";
		std::string sExcge = info->ExchangeNo;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Futures VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + sInstr + "@" + get_type() + "|" + sInstr + "@CTP" + "',";
			sCmd += "'" + sInstr + "@" + get_type() + "|" + sInstr + "@CTP" + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("es_connection::OnRspQryInstrument_Future:failed to insert into database, ret is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("es_connection::OnRspQryInstrument_Future cmd:%s\n", sCmd.c_str());
				//printf_ex("es_connection::OnRspQryInstrument_Future cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = sInstr + "@" + get_type();
			sCmd = "UPDATE Futures SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "|" + sInstr + "@CTP" + "',";
			sCmd += "ConnectionCodes='" + sConnectionCodes + "|" + sInstr + "@CTP" + "',";
			sCmd += "Underlying='" + sUnderlying + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("es_connection::OnRspQryInstrument_Future:failed to update the database,error is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("es_connection::OnRspQryInstrument_Future update to the cmd:%s\n", sCmd.c_str());
				//printf_ex("es_connection::OnRspQryInstrument_Future update to the cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		m_database->close_databse();
		if (isLast == APIYNFLAG_YES && this->get_is_last() == false)
		{
			//this->set_is_last(true);
			this->request_commodity();
		}
	}
	void TAP_CDECL es_connection::OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPITradeContractInfo *info)
	{		
		if (info)
		{
			//printf_ex("es_connection::OnRspQryContract %s,%s,%s,%c\n", info->ExchangeNo, info->CommodityNo, info->ContractNo1, info->CommodityType);
			loggerv2::info("es_connection::OnRspQryContract %s,%s,%s,%c\n", info->ExchangeNo, info->CommodityNo, info->ContractNo1, info->CommodityType);
			string symbol;
			switch (info->CommodityType)
			{
			case TAPI_COMMODITY_TYPE_FUTURES:
				symbol = string(info->CommodityNo) + string(info->ContractNo1);
				break;
			case TAPI_COMMODITY_TYPE_OPTION:
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%s%s%c%s", info->CommodityNo, info->ContractNo1, info->CallOrPutFlag1, info->StrikePrice1);
				symbol = buffer;
				break;
			default:
				break;
			}
			std::string sInstr = symbol;				
			if ((info->CommodityType != TAPI_COMMODITY_TYPE_FUTURES && info->CommodityType != TAPI_COMMODITY_TYPE_OPTION) || strlen(info->CommodityNo) < 1)
			{
				if (isLast == APIYNFLAG_YES && this->get_is_last() == false)
				{
					//this->set_is_last(true);
					this->request_commodity();
				}
				return;
			}
			if (info->CommodityType == TAPI_COMMODITY_TYPE_FUTURES)
				return OnRspQryInstrument_Future(sessionID, errorCode, isLast, info);
			std::string sSearch = "select * from Options where Code= '" + sInstr + "'";			
			//char *zErrMsg = 0;
			std::string sUnderlying = string(info->CommodityNo) + string(info->ContractNo1); //
			std::string sCP = "N";  //"CallPut"
			switch (info->CallOrPutFlag1)
			{
			case TAPI_CALLPUT_FLAG_CALL:
				sCP = "C";
				break;
			case TAPI_CALLPUT_FLAG_PUT:
				sCP = "P";
				break;
			default:				
				break;
			}
			std::string sInstClass = "O_" + string(info->CommodityNo);			
			std::string sMat = info->ContractExpDate; 
			std::string sCmd = "";
			std::string sExcge = info->ExchangeNo;
			m_database->open_database();
			std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());
			if (pTree->size() == 0) //tradeitem doesn't exist
			{
				sCmd = "INSERT INTO Options VALUES (";
				sCmd += "'" + sInstr + "',";
				sCmd += "'" + sExcge + "',";
				sCmd += "'" + sInstr + "',";
				sCmd += "' ',";
				sCmd += "'" + sInstr + "@" + get_type() + "|" + sInstr + "@CTP" + "',";
				sCmd += "'" + sInstr + "@" + get_type() + "|" + sInstr + "@CTP" + "',";
				sCmd += "'" + sUnderlying + "',";
				sCmd += "'" + sMat + "',";
				sCmd += "'" + string(info->StrikePrice1) + "',";
				sCmd += "'" + string("0")+"',";
				sCmd += "'" + sCP + "',";
				sCmd += "'" + sInstClass + "')";

				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					loggerv2::info("failed to insert into database, ret is %d",rc);
					//sqlite3_free(zErrMsg);
				}
			}
			else //exists
			{
				std::string sConnectionCodes = sInstr + "@" + get_type();
				sCmd = "UPDATE Options SET ";
				sCmd += "Code = '" + sInstr + "',";
				sCmd += "Exchange = '" + sExcge + "',";
				sCmd += "ISIN = '" + sInstr + "',";
				sCmd += "Maturity = '" + sMat + "',";
				sCmd += "Strike = '" + string(info->StrikePrice1) + "',";
				sCmd += "PointValue ='" + string("0")+"',";
				sCmd += "FeedCodes='" + sConnectionCodes + "|" + sInstr + "@CTP" + "',";
				sCmd += "ConnectionCodes='" + sConnectionCodes + "|" + sInstr + "@CTP" + "'";
				sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					loggerv2::info("failed to update the database,error is %d",rc);
					//sqlite3_free(zErrMsg);
				}
				else
				{
					loggerv2::info("ok to update the database,%s",sCmd.c_str());
				}					
			}
			m_database->close_databse();
			if (isLast == APIYNFLAG_YES && this->get_is_last() == false)
			{
				//this->set_is_last(true);
				this->request_commodity();
			}
		}
	}
	void TAP_CDECL es_connection::OnRtnContract(const TapAPITradeContractInfo *info)
	{
		printf_ex("es_connection::OnRtnContract\n");
	}
	/*
	返回新委托。新下的或者其他地方下的推送过来的。

	服务器接收到客户下的委托内容后就会保存起来等待触发，同时向用户回馈一个 新委托信息说明服务器正确处理了用户的请求，返回的信息中包含了全部的委托信息， 同时有一个用来标示此委托的委托号。

	*/
	void TAP_CDECL es_connection::OnRtnOrder(const TapAPIOrderInfoNotice *info)
	{
		if (info->OrderInfo == nullptr)
			return;		
		//to do ...
		this->create_user_info(info->OrderInfo);
		//
		m_orderQueue.CopyPush(info->OrderInfo);
	}
	/*
	返回对报单的主动操作结果

	如下单，撤单等操作的结果。

	注解
	该接口目前没有用到，所有操作结果通过OnRtnOrder返回
	*/
	void TAP_CDECL es_connection::OnRspOrderAction(TAPIUINT32 sessionID, TAPIUINT32 errorCode, const TapAPIOrderActionRsp *info)
	{
		printf_ex("es_connection::OnRspOrderAction not implement!\n");
	}	
	void TAP_CDECL es_connection::OnRspQryOrder(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIOrderInfo *info)
	{
		//printf_ex("es_connection::OnRspQryOrder\n");
		TapAPIOrderInfoNotice notice;
		notice.OrderInfo = (TapAPIOrderInfo *)info;
		this->OnRtnOrder(&notice);
	}
	/*
	返回查询的委托变化流程信息
	*/
	void TAP_CDECL es_connection::OnRspQryOrderProcess(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIOrderInfo *info)
	{
		printf_ex("es_connection::OnRspQryOrderProcess\n");
	}
	/*
	返回查询的成交信息
	*/
	void TAP_CDECL es_connection::OnRspQryFill(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIFillInfo *info)
	{
		// printf_ex("es_connection::OnRspQryFill\n");
		this->OnRtnFill(info);
	}
	/*
	推送来的成交信息

	用户的委托成交后将向用户推送成交信息。

	*/
	void TAP_CDECL es_connection::OnRtnFill(const TapAPIFillInfo *info)
	{
		//printf_ex("es_connection::OnRtnFill\n");
		m_tradeQueue.CopyPush((TapAPIFillInfo *)info);
	}
	void TAP_CDECL es_connection::OnRspQryPosition(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIPositionInfo *info)
	{		
		if (info != nullptr)
		{				
			string symbol;
			switch (info->CommodityType)
			{
			case TAPI_COMMODITY_TYPE_FUTURES:
				symbol = string(info->CommodityNo) + string(info->ContractNo);
				break;
			case TAPI_COMMODITY_TYPE_OPTION:
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%s%s%c%s", info->CommodityNo, info->ContractNo, info->CallOrPutFlag, info->StrikePrice);
				symbol = buffer;
				break;
			default:
				break;
			}
			//loggerv2::info("es_connection::OnRspQryPosition %s,qty:%d,positionNo:%s,OpenCloseMode:%d,isHistory:%c,side:%c\n", symbol.c_str(), info->PositionQty, info->PositionNo, info->OpenCloseMode, info->IsHistory,info->MatchSide);
			std::string sInstrCode = symbol + "@" + this->getName();			
			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
			if (i == nullptr)
				return;
			//
			if (m_trade_item_map.find(sInstrCode) == m_trade_item_map.end())
			{				
				i = new tradeitem(nullptr, sInstrCode, nullptr, sInstrCode, sInstrCode, AtsType::InstrType::Undef, sInstrCode, false);
				m_trade_item_map.emplace(sInstrCode,i);
			}
			else
			{
				i = m_trade_item_map[sInstrCode];
			}
			//
			i->set_last_sychro_timepoint(get_lwtp_now());
			switch (info->MatchSide)
			{
			case TAPI_SIDE_BUY://long
				{
					if (info->IsHistory == APIYNFLAG_YES)//昨仓
					{
						i->set_yst_long_position(i->get_yst_long_position()+info->PositionQty);
					}
					else//今仓
					{
						i->set_today_long_position(i->get_today_long_position()+info->PositionQty);
					}
					i->set_tot_long_position(i->get_today_long_position() + i->get_yst_long_position());
					//printf_ex("es_connection::OnRspQryPosition %s,i->get_today_long_position:%d,i->get_yst_long_position:%d\n", symbol.c_str(), i->get_today_long_position(), i->get_yst_long_position());
					break;
				}
			case TAPI_SIDE_SELL://short
				{
					if (info->IsHistory == APIYNFLAG_YES)//昨仓
					{
						i->set_yst_short_position(i->get_yst_short_position()+info->PositionQty);
					}
					else//今仓
					{
						i->set_today_short_position(i->get_today_short_position()+info->PositionQty);
					}
					i->set_tot_short_position(i->get_today_short_position() + i->get_yst_short_position());
					//printf_ex("es_connection::OnRspQryPosition %s,i->get_today_short_position:%d,i->get_yst_short_position:%d\n", symbol.c_str(), i->get_today_short_position(), i->get_yst_short_position());
					break;
				}
			default:
				break;
			}
		}
		if (isLast == APIYNFLAG_YES)
		{
			loggerv2::info("es_connection::OnRspQryPosition isLast:%c\n", isLast);
			for (auto &it : m_trade_item_map)
			{				
				tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(it.first.c_str());
				if (i != nullptr)
				{
					i->set_last_sychro_timepoint(it.second->get_last_sychro_timepoint());

					if (i->get_yst_long_position() != it.second->get_yst_long_position())
					{
						loggerv2::info("es_connection::OnRspQryPosition,yst_long_position change,%s",i->getCode().c_str());
						i->dumpinfo();
						i->set_yst_long_position(it.second->get_yst_long_position());
						i->dumpinfo();
					}
					if (i->get_today_long_position() != it.second->get_today_long_position())
					{
						loggerv2::info("es_connection::OnRspQryPosition,today_long_position change,%s",i->getCode().c_str());
						i->dumpinfo();
						i->set_today_long_position(it.second->get_today_long_position());
						i->dumpinfo();
					}
					if (i->get_tot_long_position() != it.second->get_tot_long_position())
					{
						loggerv2::info("es_connection::OnRspQryPosition,tot_long_position change,%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_tot_long_position(it.second->get_tot_long_position());
						i->dumpinfo();
					}
					if (i->get_yst_short_position() != it.second->get_yst_short_position())
					{
						loggerv2::info("es_connection::OnRspQryPosition,yst_short_position change,%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_yst_short_position(it.second->get_yst_short_position());
						i->dumpinfo();
					}
					if (i->get_today_short_position() != it.second->get_today_short_position())
					{
						loggerv2::info("es_connection::OnRspQryPosition,today_short_position change,%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_today_short_position(it.second->get_today_short_position());
						i->dumpinfo();
					}
					if (i->get_tot_short_position() != it.second->get_tot_short_position())
					{
						loggerv2::info("es_connection::OnRspQryPosition,tot_short_position change,%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_tot_short_position(it.second->get_tot_short_position());
						i->dumpinfo();
					}
				}
			}
			//for (auto &it : tradeitem_gh::get_instance().container().get_map())
			//{
				//it.second->dumpinfo();
			//}
			if (m_bPosition == false)
			{
				TAPIUINT32 sessionId = 0;
				TapAPIFillQryReq FillQryReq;
				this->m_pUserApi->QryFill(&sessionId, &FillQryReq);
				m_bPosition = true;
			}
		}
	}
	/*
	持仓变化推送通知
	*/
	void TAP_CDECL es_connection::OnRtnPosition(const TapAPIPositionInfo *info)
	{
		printf_ex("es_connection::OnRtnPosition\n");
	}
	void TAP_CDECL es_connection::OnRspQryClose(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPICloseInfo *info)
	{
		printf_ex("es_connection::OnRspQryClose\n");
	}
	/*
	平仓数据变化推送 
	*/
	void TAP_CDECL es_connection::OnRtnClose(const TapAPICloseInfo *info)
	{
		printf_ex("es_connection::OnRtnClose\n");
	}
	/*
	持仓盈亏通知 
	*/
	void TAP_CDECL es_connection::OnRtnPositionProfit(const TapAPIPositionProfitNotice *info)
	{
		//printf_ex("es_connection::OnRtnPositionProfit\n");
	}
	/*
	14.查深度行情
	*/
	void TAP_CDECL es_connection::OnRspQryDeepQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIDeepQuoteQryRsp *info)
	{
		printf_ex("es_connection::OnRspQryDeepQuote\n");
	}
	void TAP_CDECL es_connection::OnRspQryExchangeStateInfo(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIExchangeStateInfo * info)
	{
		//printf_ex("es_connection::OnRspQryExchangeStateInfo\n");
	}
	void TAP_CDECL es_connection::OnRtnExchangeStateInfo(const TapAPIExchangeStateInfoNotice * info)
	{
		//printf_ex("es_connection::OnRtnExchangeStateInfo\n");
	}
	/*
	询价通知
	*/
	void TAP_CDECL es_connection::OnRtnReqQuoteNotice(const TapAPIReqQuoteNotice *info) //V9.0.2.0 20150520
	{
		if (info == nullptr)
			return;
		//printf_ex("es_connection::OnRtnReqQuoteNotice,%s%s%c%s,%s,%c\n",info->CommodityNo,info->ContractNo,info->CallOrPutFlag,info->StrikePrice,info->ExchangeNo,info->CommodityType);
		loggerv2::info("es_connection::OnRtnReqQuoteNotice,%s%s%c%s,%s,%c\n", info->CommodityNo, info->ContractNo, info->CallOrPutFlag, info->StrikePrice, info->ExchangeNo, info->CommodityType);
		string symbol;
		switch (info->CommodityType)
		{
		case TAPI_COMMODITY_TYPE_FUTURES:
			symbol = string(info->CommodityNo) + string(info->ContractNo);
			break;
		case TAPI_COMMODITY_TYPE_OPTION:
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%s%s%c%s", info->CommodityNo, info->ContractNo, info->CallOrPutFlag, info->StrikePrice);
			symbol = buffer;
			break;
		default:
			break;
		}
		for (iconnection_event_handler* pHandler : m_connectionEventHandlers)
		{
			std::string sInstrCode = symbol + "@" + this->getName();
			tradeitem* instr       = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
			if (instr)
			{
				pHandler->set_quote_sys_id_cb(instr, info->InquiryNo);
			}
		}
	}
	/*
	15.查上手信息
	*/
	void TAP_CDECL es_connection::OnRspUpperChannelInfo(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIUpperChannelInfo * info)
	{
		printf_ex("es_connection::OnRspUpperChannelInfo\n");
	}
	/*
	16.查客户最终费率
	*/
	void TAP_CDECL es_connection::OnRspAccountRentInfo(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIAccountRentInfo * info)
	{
		printf_ex("es_connection::OnRspAccountRentInfo\n");
	}
	string es_connection::get_CommodityNo(string code)
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
		//printf_ex("get_CommodityNo code:%s,CommodityNo:%s\n", code.c_str(), str.c_str());
		return str;
	}
	string es_connection::get_ContractNo(string code)
	{
		char buffer[32];
		memset(buffer, 0, sizeof(buffer));
		strcat(buffer, code.c_str());
		string str;
		bool bFound = false;
		for (int i = 0; i < strlen(buffer); i++)
		{
			if (bFound == false)
			{
				if (isdigit(buffer[i]) != 0)//digit
				{
					bFound = true;
					str.push_back(buffer[i]);
				}
			}
			else
			{
				if (isdigit(buffer[i]) != 0)//digit
				{
					str.push_back(buffer[i]);
				}
				else
					break;
		}
		}
		//printf_ex("get_ContractNo code:%s,ContractNo:%s\n", code.c_str(), str.c_str());
		return str;
	}
	string es_connection::get_Strike(string code)
	{
		string str = code.substr(code.length() - 4, code.length() - 1);
		//printf_ex("get_Strike code:%s,strike:%s\n", code.c_str(), str.c_str());
		return str;
	}
	/*
	//! 委托完整信息
	struct TapAPIOrderInfo
	{
	TAPISTR_20					AccountNo;						///< 客户资金帐号
	TAPISTR_10					ExchangeNo;						///< 交易所编号
	TAPICommodityType			CommodityType;					///< 品种类型
	TAPISTR_10					CommodityNo;					///< 品种编码类型
	TAPISTR_10					ContractNo;						///< 合约1
	TAPISTR_10					StrikePrice;					///< 执行价格1
	TAPICallOrPutFlagType		CallOrPutFlag;					///< 看张看跌1
	TAPISTR_10					ContractNo2;					///< 合约2
	TAPISTR_10					StrikePrice2;					///< 执行价格2
	TAPICallOrPutFlagType		CallOrPutFlag2;					///< 看张看跌2

	TAPIOrderTypeType			OrderType;						///< 委托类型
	TAPIOrderSourceType			OrderSource;					///< 委托来源
	TAPITimeInForceType			TimeInForce;					///< 委托有效类型
	TAPIDATETIME				ExpireTime;						///< 有效日期(GTD情况下使用)

	TAPIYNFLAG					IsRiskOrder;					///< 是否风险报单
	TAPISideType				OrderSide;						///< 买入卖出
	TAPIPositionEffectType		PositionEffect;					///< 开平标志1
	TAPIPositionEffectType		PositionEffect2;				///< 开平标志2
	TAPISTR_50					InquiryNo;						///< 询价号
	TAPIHedgeFlagType			HedgeFlag;						///< 投机保值
	TAPIREAL64					OrderPrice;						///< 委托价格1
	TAPIREAL64					OrderPrice2;					///< 委托价格2，做市商应价使用
	TAPIREAL64					StopPrice;						///< 触发价格
	TAPIUINT32					OrderQty;						///< 委托数量
	TAPIUINT32					OrderMinQty;					///< 最小成交量
	TAPIUINT32					OrderCanceledQty;				///< 撤单数量

	TAPIUINT32					MinClipSize;					///< 冰山单最小随机量
	TAPIUINT32					MaxClipSize;					///< 冰山单最大随机量
	TAPISTR_50					LicenseNo;						///< 软件授权号

	TAPIINT32					RefInt;							///< 整型参考值
	TAPISTR_50					RefString;						///< 字符串参考值
	TAPISTR_20					ParentAccountNo;				///< 上级资金账号

	TAPICHAR					ServerFlag;						///< 服务器标识
	TAPISTR_20					OrderNo;						///< 委托编码
	TAPISTR_50                  ClientOrderNo;					///< 客户端本地委托编号
	TAPISTR_20					OrderLocalNo;					///< 本地号
	TAPISTR_50					OrderSystemNo;					///< 系统号
	TAPISTR_50					OrderExchangeSystemNo;			///< 交易所系统号
	TAPISTR_20					OrderParentNo;					///< 父单号
	TAPISTR_50					OrderParentSystemNo;			///< 父单系统号
	TAPISTR_10					TradeNo;						///< 交易编码

	TAPISTR_10					UpperNo;						///< 上手号
	TAPISTR_10					UpperChannelNo;					///< 上手通道号
	TAPISTR_20					UpperSettleNo;					///< 会员号和清算号
	TAPISTR_20					UpperUserNo;					///< 上手登录号
	TAPISTR_20					OrderInsertUserNo;				///< 下单人
	TAPIDATETIME				OrderInsertTime;				///< 下单时间
	TAPISTR_20					OrderCommandUserNo;				///< 录单操作人
	TAPISTR_20					OrderUpdateUserNo;				///< 委托更新人
	TAPIDATETIME				OrderUpdateTime;				///< 委托更新时间
	TAPIOrderStateType			OrderState;						///< 委托状态
	TAPIREAL64					OrderMatchPrice;				///< 成交价1
	TAPIREAL64					OrderMatchPrice2;				///< 成交价2
	TAPIUINT32					OrderMatchQty;					///< 成交量1
	TAPIUINT32					OrderMatchQty2;					///< 成交量2

	TAPIUINT32					ErrorCode;						///< 最后一次操作错误信息码
	TAPISTR_50					ErrorText;						///< 错误信息

	TAPIYNFLAG					IsBackInput;					///< 是否为录入委托单
	TAPIYNFLAG					IsDeleted;						///< 委托成交删除标
	TAPIYNFLAG					IsAddOne;						///< 是否为T+1单

	TAPIUINT32					OrderStreamID;					///< 委托流水号
	TAPIUINT32					UpperStreamID;					///< 上手流号

	TAPIREAL64					ContractSize;					///< 每手乘数，计算参数
	TAPIREAL64					ContractSize2;					///< 每手乘数，计算参数
	TAPISTR_10					CommodityCurrencyGroup;			///< 品种币种组
	TAPISTR_10					CommodityCurrency;				///< 品种币种
	TAPICalculateModeType		FeeMode;						///< 手续费计算方式
	TAPIREAL64					FeeParam;						///< 手续费参数值 冻结手续费均按照开仓手续费计算
	TAPISTR_10					FeeCurrencyGroup;				///< 客户手续费币种组
	TAPISTR_10					FeeCurrency;					///< 客户手续费币种
	TAPICalculateModeType		FeeMode2;						///< 手续费计算方式
	TAPIREAL64					FeeParam2;						///< 手续费参数值 冻结手续费均按照开仓手续费计算
	TAPISTR_10					FeeCurrencyGroup2;				///< 客户手续费币种组
	TAPISTR_10					FeeCurrency2;					///< 客户手续费币种
	TAPICalculateModeType		MarginMode;						///< 保证金计算方式
	TAPIREAL64					MarginParam;					///< 保证金参数值
	TAPICalculateModeType		MarginMode2;					///< 保证金计算方式
	TAPIREAL64					MarginParam2;					///< 保证金参数值
	TAPIREAL64					PreSettlePrice;					///< 昨结算价  比例方式的市价单和组合订单使用
	TAPIREAL64					PreSettlePrice2;				///< 昨结算价  比例方式的市价单和组合订单使用
	TAPIUINT32					OpenVol;						///< 预开仓数量 委托数量中的开仓部分
	TAPIUINT32					CoverVol;						///< 预平仓数量 委托数量中的平仓部分
	TAPIUINT32					OpenVol2;						///< 预开仓数量 委托数量中的开仓部分
	TAPIUINT32					CoverVol2;						///< 预平仓数量 委托数量中的平仓部分
	TAPIREAL64					FeeValue;						///< 冻结手续费
	TAPIREAL64					MarginValue;					///< 冻结保证金 合并计算的此项为0
	TAPITacticsTypeType			TacticsType;					///< 策略单类型
	TAPITriggerConditionType	TriggerCondition;				///< 触发条件
	TAPITriggerPriceTypeType	TriggerPriceType;				///< 触发价格类型
	TAPIYNFLAG					AddOneIsValid;					///< 是否T+1有效
	TAPIUINT32					OrderQty2;						///< 委托数量2
	TAPIHedgeFlagType			HedgeFlag2;						///< 投机保值2
	TAPIMarketLevelType			MarketLevel;					///< 市价撮合深度
	TAPIYNFLAG					OrderDeleteByDisConnFlag;		///< 心跳检测失败时，服务器自动撤单标识
	};
	*/
	void es_connection::OnRtnOrderAsync(TapAPIOrderInfo * pField)
	{
		if (pField == nullptr)
			return;
		//
		if (m_debug)
		{
			loggerv2::info("es_connection::OnRtnOrderAsync "
				"AccountNo:%s,"
				"ExchangeNo:%s,"
				"CommodityType:%c,"
				"CommodityNo:%s,"
				"ContractNo:%s,"
				"trikePrice:%s,"
				"CallOrPutFlag:%c,"
				"ContractNo2:%s,"
				"StrikePrice2:%s,"
				"CallOrPutFlag2:%c,"

				"OrderType:%c,"
				"OrderSource:%c,"
				"TimeInForce:%c,"
				"ExpireTime:%s,"

				"IsRiskOrder:%c,"
				"OrderSide:%c,"
				"PositionEffect:%c,"
				"PositionEffect2:%c,"
				"InquiryNo:%s,"
				"HedgeFlag:%c,"
				"OrderPrice:%f,"
				"OrderPrice2:%f,"
				"StopPrice:%f,"
				"OrderQty:%d,"
				"OrderMinQty:%d,"
				"OrderCanceledQty:%d,"

				"MinClipSize:%d,"
				"MaxClipSize:%d,"
				"LicenseNo:%s,"

				"RefInt:%d,"
				"RefString:%s,"
				"ParentAccountNo:%s,"

				"ServerFlag:%c,"
				"OrderNo:%s,"
				"ClientOrderNo:%s,"
				"OrderLocalNo:%s,"
				"OrderSystemNo:%s,"
				"OrderExchangeSystemNo:%s,"
				"OrderParentNo:%s,"
				"OrderParentSystemNo:%s,"
				"TradeNo:%s,"

				"UpperNo:%s,"
				"UpperChannelNo:%s,"
				"UpperSettleNo:%s,"
				"UpperUserNo:%s,"
				"OrderInsertUserNo:%s,"
				"OrderInsertTime:%s,"
				"OrderCommandUserNo:%s,"
				"OrderUpdateUserNo:%s,"
				"OrderUpdateTime:%s,"
				"OrderState:%c,"
				"OrderMatchPrice:%f,"
				"OrderMatchPrice2:%f,"
				"OrderMatchQty:%d,"
				"OrderMatchQty2:%d,"

				"ErrorCode:%d,"
				"ErrorText:%s,"

				"IsBackInput:%c,"
				"IsDeleted:%c,"
				"IsAddOne:%c,"

				"OrderStreamID:%d,"
				"UpperStreamID:%d,",


				pField->AccountNo,
				pField->ExchangeNo,
				pField->CommodityType,
				pField->CommodityNo,
				pField->ContractNo,
				pField->StrikePrice,
				pField->CallOrPutFlag,
				pField->ContractNo2,
				pField->StrikePrice2,
				pField->CallOrPutFlag2,
				pField->OrderType,
				pField->OrderSource,
				pField->TimeInForce,
				pField->ExpireTime,
				pField->IsRiskOrder,
				pField->OrderSide,
				pField->PositionEffect,
				pField->PositionEffect2,
				pField->InquiryNo,
				pField->HedgeFlag,
				pField->OrderPrice,
				pField->OrderPrice2,
				pField->StopPrice,
				pField->OrderQty,
				pField->OrderMinQty,
				pField->OrderCanceledQty,
				pField->MinClipSize,
				pField->MaxClipSize,
				pField->LicenseNo,
				pField->RefInt,
				pField->RefString,
				pField->ParentAccountNo,
				pField->ServerFlag,
				pField->OrderNo,
				pField->ClientOrderNo,
				pField->OrderLocalNo,
				pField->OrderSystemNo,
				pField->OrderExchangeSystemNo,
				pField->OrderParentNo,
				pField->OrderParentSystemNo,
				pField->TradeNo,
				pField->UpperNo,
				pField->UpperChannelNo,
				pField->UpperSettleNo,
				pField->UpperUserNo,
				pField->OrderInsertUserNo,
				pField->OrderInsertTime,
				pField->OrderCommandUserNo,
				pField->OrderUpdateUserNo,
				pField->OrderUpdateTime,
				pField->OrderState,
				pField->OrderMatchPrice,
				pField->OrderMatchPrice2,
				pField->OrderMatchQty,
				pField->OrderMatchQty2,
				pField->ErrorCode,
				pField->ErrorText,
				pField->IsBackInput,
				pField->IsDeleted,
				pField->IsAddOne,
				pField->OrderStreamID,
				pField->UpperStreamID);
		}
		//
		if (strlen(pField->CommodityNo) < 1)
			return;		

		switch (pField->CommodityType)
		{
		case TAPI_COMMODITY_TYPE_FUTURES:
			{
			//printf_ex("es_connection::OnRtnOrderAsync %s%s,%c,order_type:%c,order_no:%s,side:%c,order state:%c\n", pField->CommodityNo, pField->ContractNo, pField->CommodityType, pField->OrderType, pField->OrderNo, pField->OrderSide, pField->OrderState);
			break;
			}
		case TAPI_COMMODITY_TYPE_OPTION:
			{
			//printf_ex("es_connection::OnRtnOrderAsync %s%s%c%s,%c,order_type:%c,order_no:%s,side:%c,price:%f,order state:%c\n", pField->CommodityNo, pField->ContractNo, pField->CallOrPutFlag, pField->StrikePrice, pField->CommodityType, pField->OrderType, pField->OrderNo, pField->OrderSide, pField->OrderPrice, pField->OrderState);
			//loggerv2::error("es_connection::OnRtnOrderAsync %s%s%c%s,%c,order_type:%c,order_no:%s,side:%c,price:%f,order state:%c\n", pField->CommodityNo, pField->ContractNo, pField->CallOrPutFlag, pField->StrikePrice, pField->CommodityType, pField->OrderType, pField->OrderNo, pField->OrderSide, pField->OrderPrice, pField->OrderState);
			if (pField->OrderSide == TAPI_SIDE_ALL)
				{
				return OnRtnOrderAsync_Quote(pField);
				}
			break;
			}
		default:
			break;
		}

		int ret;
		
		//int account, bidId, askId, portfolioId, ntradingType;
		int account = 0;
		int bidId   = 0;
		int askId   = 0;
		int portfolioId  = 0;
		int ntradingType = 0;		
		ctpbase_connection::get_user_info(pField->RefString, account, bidId, askId, portfolioId, ntradingType);
		OrderWay::type way = pField->OrderSide == TAPI_SIDE_BUY ? OrderWay::Buy : OrderWay::Sell;
		int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;
		if (orderId == 0)
		{
			orderId = FAKE_ID_MIN + atoi(pField->OrderLocalNo);
			portfolioId = -1;
		}
		order* o    = get_order_from_map(orderId, ret);
		bool  bFind = false;
		//
		if (o==nullptr && m_order_local_no_id_map.find(pField->OrderLocalNo) != m_order_local_no_id_map.end())
		{
			bFind   = true;
			int tmpId = m_order_local_no_id_map[pField->OrderLocalNo];
			o = get_order_from_map(tmpId, ret);
			if (o != nullptr)
			{
				loggerv2::info("es_connection::OnRtnOrderAsync find the order,id:%d,%s",tmpId,pField->OrderLocalNo);
				o->set_id(orderId);
				o->set_quantity(pField->OrderQty);
				o->set_price(pField->OrderPrice);
				if (portfolioId == -1)
				{
					o->set_unknown_order();
				}
				else
				{
					o->set_portfolio(this->getPortfolioName(portfolioId).c_str());
					o->set_trading_type(ntradingType);
					o->set_user_orderid(bidId);
				}
				//								
				if (ret == 0)
				{		
					tbb::concurrent_hash_map<int, order*>::accessor ra;				
					m_activeOrders.erase(tmpId);
					//
					m_activeOrders.insert(ra, orderId);
					ra->second = o;
					//	
				}
				else
				{					
					m_deadOrders.emplace(orderId,o);					
				}
				//
			}
			else
			{
				loggerv2::error("es_connection::OnRtnOrderAsync cannot find the order,id:%d,%s",tmpId, pField->OrderLocalNo);
			}
		}
		//
		switch (ret)
		{
		case 0:			
			break;
		case 1:			
			loggerv2::warn("es_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			//anchor
			o = es_order_aux::anchor(this, pField);
			if (o == nullptr)
			{
				loggerv2::error("es_connection::OnRtnOrderAsync cannot anchor order:%s",pField->OrderNo);
				return;
			}
			add_pending_order(o);
			break;
		default:
		{
			printf_ex("es_connection::OnRtnOrderAsync didn't find the order,ret:%d\n", ret);
			break;
		}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("es_connection::OnRtnOrderAsync - order is nullptr");
			return;
		}
		//
		es_order_aux::set_order_no(o,pField->OrderNo);
		//
		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("es_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}

		//auto tp = string_to_lwtp(day_clock::local_day(),pField->OrderUpdateTime);
		auto utime = this->get_time(string(pField->OrderUpdateTime));
		auto tp = ptime_to_lwtp(utime);

		o->set_last_time(tp);
		//to do ...
		if (bidId > 0 && o->get_binding_quote() == nullptr)
		{
			int ret1;
			quote* q = get_quote_from_map(bidId, ret1);
			switch (ret1)
			{
			case 0:
				if (way == OrderWay::Buy)
				{
					q->set_bid_order(o);
				}
				if (way == OrderWay::Sell)
				{
					q->set_ask_order(o);
				}
				//o = reinterpret_cast<cffex_order*>(ord);
				break;
			case 1:
				if (way == OrderWay::Buy)
				{
					q->set_bid_order(o);
				}
				if (way == OrderWay::Sell)
				{
					q->set_ask_order(o);
				}
				//o = reinterpret_cast<cffex_order*>(ord);
				loggerv2::info("es_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
				break;

			case 2:
			{
				q = get_quote_from_pool();
				if (way == OrderWay::Buy)
				{
					q->set_bid_order(o);
				}
				if (way == OrderWay::Sell)
				{
					q->set_ask_order(o);
				}
				q->set_instrument(o->get_instrument());
				q->set_id(bidId);
				//to do ...			
				auto ltime = o->get_last_time();
				q->set_last_time(ltime);

				q->set_portfolio(getPortfolioName(portfolioId).c_str());
				q->set_trading_type(ntradingType);
				add_pending_quote(q);
				break;
			}
			default:
				break;
			}
		}
		//
		switch (pField->OrderState)
		{
		case TAPI_ORDER_STATE_FINISHED:
		case TAPI_ORDER_STATE_PARTFINISHED:
		case TAPI_ORDER_STATE_ACCEPT://to wait market/*已受理 */
			//o->set_status(AtsType::OrderStatus::WaitMarket);
			//connection::update_pending_order(o);
			//break;
		case TAPI_ORDER_STATE_QUEUED:/*已排队*/		
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{
				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}
			else if ((bFind == true || ret ==1 ) && pField->OrderState == TAPI_ORDER_STATE_FINISHED)
			{
				update_instr_on_ack_from_market_cb(o,o->get_exec_quantity());
			}
			break;
		case TAPI_ORDER_STATE_FAIL:			
			update_instr_on_nack_from_market_cb(o);			
			on_nack_from_market_cb(o, pField->ErrorText);			
			break;
		case TAPI_ORDER_STATE_CANCELED:/*完全撤单 */
			update_instr_on_cancel_from_market_cb(o);			
			on_cancel_from_market_cb(o);					
			break;		
		case TAPI_ORDER_STATE_APPLY:
			update_instr_on_ack_from_market_cb(o);
			on_ack_from_market_cb(o);			
			break;
		default:
			//printf_ex("es_connection::OnRtnOrderAsync didn't do with the order state:%c\n", pField->OrderState);
			loggerv2::info("es_connection::OnRtnOrderAsync didn't do with the order state:%c\n", pField->OrderState);
			break;
		}	
		//
#if 0
		tbb::concurrent_hash_map<string, exec_vector*>::accessor ra;
		if (m_local_no_map.find(ra, pField->OrderLocalNo))
		{
			exec_vector * ex_vector = ra->second;

			for (auto &it : *ex_vector)
			{
				exec * e = it;				
				//update order state/book  
			 	int execQty = e->getQuantity();
				double execPrice = e->getPrice();
				int cumulatedQty = o->get_exec_quantity() + execQty;
				double averagePrc = (o->get_exec_quantity() * o->get_exec_price() + execQty * execPrice) / (cumulatedQty);
				int totQty = o->get_quantity();
				int bookQty = totQty - cumulatedQty;
				if (bookQty < 0)
				{
					bookQty = 0;					
					o->set_status(OrderStatus::Exec);
				}				
				o->set_book_quantity(bookQty);
				o->set_exec_quantity(cumulatedQty);
				o->set_exec_price(averagePrc);
				//
				e->setOrderId(o->get_id());
				string portfolio = o->get_portfolio();
				e->setPortfolioName(portfolio);
				e->setWay(o->get_way());
				e->setTradingType(o->get_trading_type());	
				e->setOpenClose(o->get_open_close());		
				e->set_account_num(o->get_account_num());
				e->set_hedge(o->get_hedge());				
				//
				loggerv2::info("OnRtnOrderAsync id:%d,%s,call on_exec_from_market_cb\n", o->get_id(), pField->OrderLocalNo);
				on_exec_from_market_cb(o, e);
				bool onlyUpdatePending = false;

				ptime  tradeTime = time_from_string(e->get_time());
				int hour = tradeTime.time_of_day().hours();
				lwtp tp = ptime_to_lwtp(tradeTime);

				if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
					onlyUpdatePending = true;
				if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
					onlyUpdatePending = true;
				loggerv2::info("OnRtnOrderAsync id:%d,%s,call update_instr_on_exec_from_market_cb\n", o->get_id(), pField->OrderLocalNo);
				update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
			}		
			o->on_update_order();
			update_pending_order(o);			
			//printf_ex("OnRtnOrderAsync id:%d,%s\n",o->get_id(),pField->OrderLocalNo);		
			loggerv2::info("OnRtnOrderAsync id:%d,%s\n", o->get_id(), pField->OrderLocalNo);
			//			
			delete ex_vector;
			ra.release();
			m_local_no_map.erase(pField->OrderLocalNo);			
			//
		}
		//
#endif
	}
	void es_connection::OnRtnOrderAsync_Quote(TapAPIOrderInfo * pField)
	{
		if (pField==nullptr)
			return;
		//int account, bidId, askId, portfolioId, ntradingType;
		int account = 0;
		int bidId   = 0;
		int askId   = 0;
		int portfolioId = 0;
		int ntradingType = 0;
		ctpbase_connection::get_user_info(pField->RefString, account, bidId, askId, portfolioId, ntradingType);
		OrderWay::type way = (pField->OrderSide == TAPI_SIDE_BUY || pField->OrderSide == TAPI_SIDE_ALL) ? OrderWay::Buy : OrderWay::Sell;
		int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;
		if (orderId == 0)
		{
			orderId = FAKE_ID_MIN + atoi(pField->OrderLocalNo);
		}
		int ret;
		quote *q = get_quote_from_map(orderId, ret);
		switch (ret)
		{
		case 0:			
			break;
		case 1:			
			loggerv2::info("es_connection::OnRtnOrderAsync_Quote - message received on dead quote[%d]...", orderId);
			break;
		case 2:
			q = es_order_aux::anchor_quote(this, pField);
			if (q == nullptr)
			{
				loggerv2::error("es_connection::OnRtnOrderAsync_Quote cannot anchor quote");
				return;
			}
			add_pending_quote(q);
			break;
		default:
			break;
		}

		if (q == nullptr) // should not happen
		{
			loggerv2::error("es_connection::OnRtnOrderAsync_Quote - quote recovered nullptr");
			return;
		}

		if (strlen(pField->InquiryNo) > 0)
		{
			q->set_FQR_ID(pField->InquiryNo);
		}

		es_order_aux::set_order_no(q, pField->OrderNo);

		//auto tp = string_to_lwtp(day_clock::local_day(),pField->OrderUpdateTime);
		auto utime = this->get_time(string(pField->OrderUpdateTime));
		auto tp = ptime_to_lwtp(utime);

		q->set_last_time(tp);

		switch (pField->OrderState)
		{
		case TAPI_ORDER_STATE_ACCEPT://to wait market/*已受理 */
			q->set_status(AtsType::OrderStatus::WaitMarket);
			connection::update_pending_quote(q);
			break;
		case TAPI_ORDER_STATE_QUEUED:/*已排队*/
			update_instr_on_ack_from_market_cb(q->get_bid_order());
			update_instr_on_ack_from_market_cb(q->get_ask_order());
			on_ack_quote_from_market_cb(q);			
			break;		
		case TAPI_ORDER_STATE_FAIL:
			on_nack_quote_from_market_cb(q, pField->ErrorText);
			break;
		case TAPI_ORDER_STATE_CANCELED:/*完全撤单 */
			update_instr_on_cancel_from_market_cb(q->get_bid_order());
			update_instr_on_cancel_from_market_cb(q->get_ask_order());
			on_cancel_quote_from_market_cb(q);			
			break;
		default:
			//loggerv2::info("es_connection::OnRtnOrderAsync_Quote didn't do with the order state:%c\n", pField->OrderState);
			break;
		}
	}				
	/*
	//! 成交信息
	struct TapAPIFillInfo
	{
		TAPISTR_20					AccountNo;						///< 客户资金帐号
		TAPISTR_20					ParentAccountNo;				///< 上级资金账号
		TAPISTR_10					ExchangeNo;						///< 交易所编号
		TAPICommodityType			CommodityType;					///< 品种类型
		TAPISTR_10					CommodityNo;					///< 品种编码类型
		TAPISTR_10					ContractNo;						///< 合约1
		TAPISTR_10					StrikePrice;					///< 执行价格
		TAPICallOrPutFlagType		CallOrPutFlag;					///< 看张看跌

		TAPISTR_10					ExchangeNoRef;					///< 交易所编号
		TAPICommodityType			CommodityTypeRef;				///< 品种类型
		TAPISTR_10					CommodityNoRef;					///< 品种编码类型
		TAPISTR_10					ContractNoRef;					///< 合约
		TAPISTR_10					StrikePriceRef;					///< 执行价格
		TAPICallOrPutFlagType		CallOrPutFlagRef;				///< 看张看跌
		TAPISTR_10					OrderExchangeNo;				///< 交易所编号
		TAPICommodityType			OrderCommodityType;				///< 品种类型
		TAPISTR_10					OrderCommodityNo;				///< 品种编码类型

		TAPIOrderTypeType			OrderType;						///< 委托类型
		TAPIMatchSourceType			MatchSource;					///< 委托来源
		TAPITimeInForceType			TimeInForce;					///< 委托有效类型
		TAPIDATETIME				ExpireTime;						///< 有效日期(GTD情况下使用)

		TAPIYNFLAG					IsRiskOrder;					///< 是否风险报单
		TAPISideType				MatchSide;						///< 买入卖出
		TAPIPositionEffectType		PositionEffect;					///< 开平标志1
		TAPIPositionEffectType		PositionEffectRef;				///< 开平标志2
		TAPIHedgeFlagType			HedgeFlag;						///< 投机保值
		TAPICHAR					ServerFlag;						///< 服务器标识

		TAPISTR_20					OrderNo;						///< 委托编码
		TAPISTR_20					OrderLocalNo;					///< 委托本地号
		TAPISTR_20					MatchNo;						///< 本地成交号
		TAPISTR_70					ExchangeMatchNo;				///< 交易所成交号
		TAPIDATETIME				MatchDateTime;					///< 成交时间
		TAPIDATETIME				UpperMatchDateTime;				///< 上手成交时间

		TAPISTR_10					UpperNo;						///< 上手号
		TAPISTR_10					UpperChannelNo;					///< 上手通道号
		TAPISTR_20					UpperSettleNo;					///< 会员号和清算代码
		TAPISTR_20					UpperUserNo;					///< 上手登录号
		TAPISTR_10					TradeNo;						///< 交易编码
		TAPIREAL64					MatchPrice;						///< 成交价
		TAPIUINT32					MatchQty;						///< 成交量

		TAPIYNFLAG					IsBackInput;					///< 是否为录入委托单
		TAPIYNFLAG					IsDeleted;						///< 委托成交删除标
		TAPIYNFLAG					IsAddOne;						///< 是否为T+1单
		TAPIUINT32					MatchStreamID;					///< 委托流水号
		TAPIUINT32					UpperStreamID;					///< 上手流号
		TAPIOpenCloseModeType		OpenCloseMode;                  ///< 开平方式
		TAPIREAL64					ContractSize;					///< 每手乘数，计算参数
		TAPISTR_10					CommodityCurrencyGroup;			///< 品种币种组
		TAPISTR_10					CommodityCurrency;				///< 品种币种
		TAPICalculateModeType		FeeMode;						///< 手续费计算方式
		TAPIREAL64					FeeParam;						///< 手续费参数值 冻结手续费均按照开仓手续费计算
		TAPISTR_10					FeeCurrencyGroup;				///< 客户手续费币种组
		TAPISTR_10					FeeCurrency;					///< 客户手续费币种
		TAPIREAL64					PreSettlePrice;					///< 昨结算价
		TAPIREAL64					FeeValue;						///< 手续费
		TAPIYNFLAG					IsManualFee;					///< 人工客户手续费标记
		TAPIREAL64					Turnover;						///< 成交金额
		TAPIREAL64					PremiumIncome;					///< 权利金收取
		TAPIREAL64					PremiumPay;						///< 权利金支付
		TAPISTR_20					OppoMatchNo;					///< 对手本地成交号（开仓和平仓对应编号）
		TAPIREAL64					CloseProfit;					///< 平仓盈亏
		TAPIREAL64					UnExpProfit;					///< 未到期平盈
		TAPIREAL64					UpperMatchPrice;				///< 上手成交价
		TAPIREAL64					QuotePrice;						///< 当前行情价
		TAPIREAL64					ClosePL;                        ///< 逐笔平盈
	};
	*/
	void es_connection::OnRtnTradeAsync(TapAPIFillInfo  * pTrade)
	{		
		if (pTrade == nullptr)
			return;
		//
		if (m_debug)
		{
			loggerv2::info("es_connection::OnRtnTradeAsync "
				"AccountNo:%s,"
				"ParentAccountNo:%s,"
				"ExchangeNo:%s,"
				"CommodityType:%c,"
				"CommodityNo:%s,"
				"ContractNo:%s,"
				"trikePrice:%s,"
				"CallOrPutFlag:%c,"

				"OrderType:%c,"
				"MatchSource:%c,"
				"TimeInForce:%c,"
				"ExpireTime:%s,"

				"MatchSide:%c,"
				"PositionEffect:%c,"
				
				"OrderNo:%s,"
				"OrderLocalNo:%s,"
				"MatchNo:%s,"
				"ExchangeMatchNo:%s,"
				"MatchDateTime:%s,"
				"UpperMatchDateTime:%s,"
				
				"TradeNo:%s,"
				"MatchPrice:%f,"
				"MatchQty:%d,"
				,

				pTrade->AccountNo,
				pTrade->ParentAccountNo,
				pTrade->ExchangeNo,
				pTrade->CommodityType,
				pTrade->CommodityNo,
				pTrade->ContractNo,
				pTrade->StrikePrice,
				pTrade->CallOrPutFlag,							
				
				pTrade->OrderType,
				pTrade->MatchSource,
				pTrade->TimeInForce,
				pTrade->ExpireTime,

				pTrade->MatchSide,
				pTrade->PositionEffect,

				pTrade->OrderNo,
				pTrade->OrderLocalNo,
				pTrade->MatchNo,
				pTrade->ExchangeMatchNo,
				pTrade->MatchDateTime,
				pTrade->UpperMatchDateTime,

				pTrade->TradeNo,
				pTrade->MatchPrice,
				pTrade->MatchQty);
		}
		//
		if (pTrade &&strlen(pTrade->CommodityNo) < 1)
			return;
		int ret=-1;
		OrderWay::type way = pTrade->MatchSide == TAPI_SIDE_BUY ? OrderWay::Buy : OrderWay::Sell;
		//int account, bidId, askId, portfolioId, ntradingType;
		int account = 0;
		int bidId   = 0;
		int askId   = 0;
		int portfolioId  = 0;
		int ntradingType = 0;
		//to do ...get the usr id by the orderNo		
		this->get_user_info_ex(pTrade->OrderNo, account, bidId, askId, portfolioId, ntradingType);
		int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;
		if (orderId == 0)
		{
			orderId = FAKE_ID_MIN + atoi(pTrade->OrderLocalNo);
		}
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:			
			break;
		case 1:			
			loggerv2::warn("es_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			//anchor 
			o = es_order_aux::anchor(this, pTrade);
			if (o == nullptr)
			{
				loggerv2::error("es_connection::OnRtnTradeAsync cannot anchor order:%s",pTrade->OrderNo);
				return;
			}
			//if (o->get_id() < FAKE_ID_MIN)
			{
				add_pending_order(o);
			}
			break;
		default:
		{
			printf_ex("es_connection::OnRtnTradeAsync ret:%d\n", ret);
			break;
		}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("es_connection::OnRtnTradeAsync - order recovered nullptr");
			return;
		}
		//
#if 0
		//if (ret != 2 && (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer))
		//{
		//	loggerv2::error("es_connection::OnRtnTradeAsync id:%d,%s,ret:%d\n", o->get_id(), pTrade->OrderLocalNo, ret);
		//	m_order_local_no_id_map.emplace(pTrade->OrderLocalNo, o->get_id());
		//}
#endif
		//
		// 2 - treat message
		int execQty            = pTrade->MatchQty;
		double execPrc         = pTrade->MatchPrice;
		const char* pszExecRef = pTrade->ExchangeMatchNo;
		char* pszTime    = pTrade->MatchDateTime;

		lwtp tp = string_to_lwtp(pszTime);
		exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);
		//if (ret != 2)
		{
			on_exec_from_market_cb(o, e);
		}
		//
		if (ret == 2)
		{
			//printf_ex("es_connection::OnRtnTradeAsync id:%d,%s\n", o->get_id(), pTrade->OrderLocalNo);
			loggerv2::error("es_connection::OnRtnTradeAsync id:%d,%s,ret:%d\n", o->get_id(), pTrade->OrderLocalNo,ret);
#if 0
			tbb::concurrent_hash_map<string, exec_vector*>::accessor ra;
			if (m_local_no_map.find(ra,pTrade->OrderLocalNo)==false)
			{
				exec_vector * ex_vector = new exec_vector();
				ex_vector->push_back(e);				
				m_local_no_map.insert(ra, pTrade->OrderLocalNo);
				ra->second = ex_vector;
			}
			else			
			{
				exec_vector * ex_vector = ra->second;
				ex_vector->push_back(e);
			}
#else
			m_order_local_no_id_map.emplace(pTrade->OrderLocalNo, o->get_id());
#endif
		}
		//
		bool onlyUpdatePending = false;		

		int hour = get_hour_from_lwtp(tp);
		tp = tp + std::chrono::seconds(2);//允许2s的误差


		if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			onlyUpdatePending = true;
		if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;		
		//if (ret != 2)
		{
			update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
		}
	}
	void es_connection::create_user_info(TapAPIOrderInfo * pField)
	{
		if (pField == nullptr)
			return;		
		if (m_user_info_map.find(pField->OrderNo) == m_user_info_map.end())
		{
			user_info * info = new user_info();
			info->OrderNo    = pField->OrderNo;
			info->UserID     = pField->RefString;
						
			m_user_info_map.emplace(info->OrderNo, info);

			//to do ... append the file every day
			m_userInfoQueue.CopyPush(info);			

			/*to do ... create the order id map*/
#if 0
			int orderid = pField->RefInt;
			if (m_order_id_map.find(orderid) == m_order_id_map.end())
			{
				m_order_id_map.emplace(orderid, info->OrderNo);
			}
#endif
		}
		else
		{
			//printf_ex("warn:sl_connection::create_user_info already include the OrderNo:%s\n", pField->OrderNo);
		}
	}	
	void es_connection::get_user_info_ex(string orderNo, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType)
	{
		if (m_user_info_map.find(orderNo) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[orderNo];
			ctpbase_connection::get_user_info(info->UserID.c_str(), nAccount, userOrderId, internalRef, nPortfolio, nTradeType);
		}
		else
		{
			loggerv2::warn("es_connection::get_user_info didn't find the orderNo:%s\n", orderNo.c_str());
		}
	}
	void es_connection::OnUserInfoAsync(user_info* pInfo)
	{
		this->append(pInfo);
	}
	/*
	170309094633.736535 in the TapAPIOrderInfo.OrderUpdateTime
	*/
	ptime es_connection::get_time(const string & time_buffer)
	{
		string updateTime = time_buffer;
		string end;		
		int index = updateTime.find(".");
		if (index != std::string::npos)
		{
			end = updateTime.substr(index + 1);

			updateTime = updateTime.substr(0, index);
		}
		else
		{
			return microsec_clock::local_time();			
		}
		updateTime.insert(6, "T");
		updateTime.insert(0, "20");		
		ptime MarketTime(from_iso_string(updateTime));		
		if (end.length() > 0)
		{
			MarketTime = MarketTime + microseconds(atoi(end.c_str()));
		}		
		return MarketTime;
	}
}

