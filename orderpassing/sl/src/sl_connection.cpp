#include "sl_connection.h"
#include "string_tokenizer.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>
#include "tradeItem_gh.h"
#include "terra_logger.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ini_parser.hpp>
//using namespace boost::posix_time;
using namespace terra::common;
namespace sl
{
	//
	// sl_connection
	//
	sl_connection::sl_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "sl_connection";		
		m_connectionStatus = false;
		m_isAlive = true;
		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		m_bKey_with_exchange = false;
		//
		m_retry_count = 0;
		//
	}
	bool sl_connection::init_config(const string &name, const std::string &strConfigFile)
	{
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 4)
			m_bTsession = true;
		else
			m_bTsession = false;
		//
		boost::property_tree::ptree root;
		boost::property_tree::ini_parser::read_ini(strConfigFile, root);
		m_strQryServerIp   = root.get<string>(name + ".qry_hostname", "");
		m_strQryServerPort = root.get<string>(name + ".qry_service", "");
		//
		return true;
	}
	void sl_connection::request_investor_full_positions()
	{
#if 0
		//CThostFtdcQryInvestorPositionField pRequest;
		//memset(&pRequest, 0, sizeof(pRequest));
		//strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		//strcpy(pRequest.InvestorID, get_login_id().c_str());
		//this->ReqQryInvestorPosition(&pRequest);
#else
		if (this->m_pUserApi != nullptr)
		{
			this->m_pUserApi->QueryAccountPosition(this->get_account().c_str(), ++m_nRequestId);
		}
#endif
		if (m_debug)
			loggerv2::info("sl_connection:: calling request_investor_full_positions");
		return;
	}
	void sl_connection::req_RiskDegree()
	{
		request_trading_account();
	}
	///请求查询资金账户
	void sl_connection::request_trading_account()
	{
		if (m_debug)
			loggerv2::info("sl_connection:: calling request_trading_account");
    	if (this->m_pUserApi != nullptr)
		{
			///查询帐户资金
			this->m_pUserApi->QueryAccountBP(get_account().c_str(), ++m_nRequestId);
		}
		return;
	}
	void sl_connection::init_connection()
	{
		this->init_api();
		//init_process(io_service_type::trader, 10);
		//std::thread th(boost::bind(&sl_connection::set_kernel_timer_thread, this));
		//m_thread.swap(th);
#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
	}

#ifdef Linux
	void  sl_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&sl_connection::process, this));
		m_inputQueue.set_fd(efd);
		m_orderQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_inputActionQueue.set_fd(efd);
		m_rejectQueue.set_fd(efd);
		m_marketRejectQueue.set_fd(efd);
		m_userInfoQueue.set_fd(efd);
		m_outboundQueue.set_fd(efd);
	}
#endif

	void sl_connection::release()
	{
		//is_alive(false);
		//m_thread.join();
		ctpbase_connection::release();
		this->release_api();
	}
	void sl_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("sl_connection::connect connecting to fs...");

			on_status_changed(AtsType::ConnectionStatus::WaitConnect);

			this->connect_api(); 
		}
	}
	void sl_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
#if 0
			this->disconnect_api();
#else
			if (this->disconnect_api() == true)
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "sl_connection - disconnect ok");
			}
#endif
	    }
    }
	void sl_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		this->Process_api();
	}
	int sl_connection::market_create_order_async(order* o, char* pszReason)
	{		
		/*
		/// 下单消息
		struct EES_EnterOrderField
		{
		EES_Account         m_Account;						///< 用户代码
		EES_SideType        m_Side;							///< 买卖方向
		EES_ExchangeID      m_Exchange;						///< 交易所
		EES_Symbol          m_Symbol;						///< 合约代码
		EES_SecType         m_SecType;						///< 交易品种
		double              m_Price;						///< 价格
		unsigned int        m_Qty;							///< 数量
		EES_ForceCloseType  m_ForceCloseReason;				///< 强平原因
		EES_ClientToken		m_ClientOrderToken;				///< 整型，必须保证，这次比上次的值大，并不一定需要保证连续
		EES_OrderTif		m_Tif;							///< 当需要下FAK/FOK报单时，需要设置为EES_OrderTif_IOC
		unsigned int		m_MinQty;						///< 当需要下FAK/FOK报单时，该值=0：映射交易所的FAK-任意数量；
		///< 当需要下FAK/FOK报单时，该值>0且<m_Qty：映射交易所的FAK-最小数量，且最小数量即该值
		///< 当需要下FAK/FOK报单时，该值=m_Qty：映射交易所的FOK；
		///< 常规日内报单，请设为0.无论哪种情况，该值如果>m_Qty将被REM系统拒绝

		EES_CustomFieldType m_CustomField;					///< 用户自定义字段，8个字节。用户在下单时指定的值，将会在OnOrderAccept，OnQueryTradeOrder事件中返回
		typedef unsigned long long int EES_CustomFieldType;		///< 用户可存放自定义8位数字值
		EES_MarketSessionId m_MarketSessionId;				///< 交易所席位代码，从OnResponseQueryMarketSessionId获取合法值，如果填入0或者其他非法值，REM系统将自行决定送单的席位
		EES_HedgeFlag		m_HedgeFlag;					///< 投机套利标志
		EES_EnterOrderField()
		{
		m_Tif = EES_OrderTif_Day;
		m_MinQty = 0;
		m_MarketSessionId = 0;
		m_HedgeFlag = EES_HedgeFlag_Speculation;
		}

		};
		*/
		char userId[64];
		memset(userId, 0, sizeof(userId));
		compute_userId(o, userId,sizeof(userId));
		EES_EnterOrderField *request = sl_create_pool.get_mem();
		memset(request, 0, sizeof(EES_EnterOrderField));				
		strcpy(request->m_Account, this->get_account().c_str());
		strcpy(request->m_Symbol, o->get_instrument()->get_trading_code());
		request->m_Exchange  = EES_ExchangeID_shfe;//to do ...
		request->m_SecType   = EES_SecType_fut;
		request->m_Price     = o->get_price();
		request->m_Qty       = o->get_quantity();		
		request->m_Tif       = EES_OrderTif_Day;
		request->m_MinQty    = 0;
		request->m_Side      = EES_SideType_open_long;//to do ...		
		request->m_HedgeFlag = EES_HedgeFlag_Speculation;		

		/*
		typedef unsigned char EES_SideType;						///< 买卖方向
		#define EES_SideType_open_long                  1		///< =买单（开今）
		#define EES_SideType_close_today_long           2		///< =卖单（平今）
		#define EES_SideType_close_today_short          3		///< =买单（平今）
		#define EES_SideType_open_short                 4		///< =卖单（开今）
		#define EES_SideType_close_ovn_short            5		///< =买单（平昨）
		#define EES_SideType_close_ovn_long             6		///< =卖单（平昨）
		#define EES_SideType_force_close_ovn_short      7		///< =买单 （强平昨）
		#define EES_SideType_force_close_ovn_long       8		///< =卖单 （强平昨）
		#define EES_SideType_force_close_today_short    9		///< =买单 （强平今）
		#define EES_SideType_force_close_today_long     10		///< =卖单 （强平今）
		*/
		if (o->get_open_close() == OrderOpenClose::Undef)
		{
			o->set_open_close(compute_open_close(o, m_bCloseToday));
		}
		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
		{
			if (o->get_way() == AtsType::OrderWay::Buy)
				request->m_Side = EES_SideType_open_long;
			else if (o->get_way() == AtsType::OrderWay::Sell)
				request->m_Side = EES_SideType_open_short;
			break;
		}
		case AtsType::OrderOpenClose::Close:
		{
			if (o->get_way() == AtsType::OrderWay::Buy)
				request->m_Side = EES_SideType_close_ovn_short;
			else if (o->get_way() == AtsType::OrderWay::Sell)
				request->m_Side = EES_SideType_close_ovn_long;
			break;
		}
		case AtsType::OrderOpenClose::CloseToday:
		{
			if (o->get_way() == AtsType::OrderWay::Buy)
				request->m_Side = EES_SideType_close_today_short;
			else if (o->get_way() == AtsType::OrderWay::Sell)
				request->m_Side = EES_SideType_close_today_long;
			break;
		}
		default:
			break;
		}
		/*
		typedef unsigned int EES_OrderTif;						///< 成交条件
		#define EES_OrderTif_IOC						0		///< 当需要下FAK/FOK订单时，需要将TIF设置为0
		#define EES_OrderTif_Day						99998	///< 日内报单
		*/
		if (o->get_restriction() == AtsType::OrderRestriction::None)
		{
			request->m_Tif = EES_OrderTif_Day;
		}
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
		{			
			request->m_Tif    = EES_OrderTif_IOC;			
			request->m_MinQty = request->m_Qty;
		}
		else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
		{
			request->m_Tif    = EES_OrderTif_IOC;
			request->m_MinQty = 0;
		}
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			sl_create_pool.free_mem(request);
			return 0;
		}
		//to do ...
		request->m_CustomField = o->get_id();
		if (!this->ReqOrderInsert(request))
		{
			this->create_user_info(o,request->m_ClientOrderToken, userId, request->m_Symbol,request->m_Side);
			m_ees_local_client_token_map.emplace(request->m_ClientOrderToken, o->get_id());
			snprintf(pszReason, REASON_MAXLENGTH, "sl api reject!\n");
			sl_create_pool.free_mem(request);
			return 0;
		}
		this->create_user_info(o, request->m_ClientOrderToken, userId, request->m_Symbol,request->m_Side);
		m_ees_local_client_token_map.emplace(request->m_ClientOrderToken, o->get_id());
		sl_create_pool.free_mem(request);
		return 1;
	}
	int sl_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());		
		/*
		/// 下单撤单指令
		struct EES_CancelOrder
		{
		EES_MarketToken m_MarketOrderToken;					///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int    m_Quantity;							///< 这是该单子被取消后所希望剩下的数量，如为0，改单子为全部取消。在中国目前必须填0，其他值当0处理。
		EES_Account     m_Account;							///< 帐户ID号
		};
		*/
		EES_CancelOrder *request = sl_cancel_pool.get_mem();
		memset(request, 0, sizeof(EES_CancelOrder));		
		strcpy(request->m_Account, this->get_account().c_str());
		request->m_Quantity = 0;//在中国目前必须填0，其他值当0处理。
		//to do ...
		request->m_MarketOrderToken = std::atoll(sl_order_aux::get_market_token(o).c_str());//same to orderSysId
		//end
		if (!this->ReqOrderAction(request))
		{
			sl_cancel_pool.free_mem(request);
			return 0;
		}
		sl_cancel_pool.free_mem(request);
		return 1;		
	}
	//
	// 1.OrderStatus:WaitServer to WaitMarket
	// 2.fill the m_MarketOrderToken
	// 只需要更新订单状态
	void sl_connection::OnRspOrderInsertAsync(EES_OrderAcceptField* pField)
	{
		/*
		/// 下单被柜台系统接受消息
		struct EES_OrderAcceptField
		{
		EES_ClientToken     m_ClientOrderToken;				///< 下单的时候，返回给你的token
		EES_MarketToken     m_MarketOrderToken;				///< 市场里面挂单的token
		EES_OrderState      m_OrderState;					///< 订单状态
		//EES_OrderState		m_OrderState;					///< 单子状态，绝大多时候是1，但是也有可能是2.    1=order live（单子活着）    2=order dead（单子死了）
		EES_UserID          m_UserID;						///< 订单的 user id
		EES_Nanosecond      m_AcceptTime;					///< 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_Account         m_Account;						///< 用户代码
		EES_SideType        m_Side;							///< 买卖方向
		EES_ExchangeID      m_Exchange;						///< 交易所
		EES_Symbol          m_Symbol;						///< 合约代码
		EES_SecType         m_SecType;						///< 交易品种
		double              m_Price;						///< 价格
		unsigned int        m_Qty;							///< 数量
		EES_ForceCloseType  m_ForceCloseReason;				///< 强平原因
		EES_OrderTif		m_Tif;							///< 用户下单时指定的值
		unsigned int		m_MinQty;						///< 用户下单时指定的值
		EES_CustomFieldType m_CustomField;					///< 用户下单时指定的值
		//typedef unsigned long long int EES_CustomFieldType;		///< 用户可存放自定义8位数字值
		EES_MarketSessionId m_MarketSessionId;				///< 报单送往交易所的席位代码，有可能和下单时指定的不同。不同的原因有：当前该席位尚未连接好；指定的席位代号非法等；指定0：由REM自行决定
		EES_HedgeFlag		m_HedgeFlag;					///< 投机套利标志
		};
		*/		
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnRspOrderInsertAsync "
				            "m_ClientOrderToken:%d,"
							"m_MarketOrderToken:%I64d,"
							"m_OrderState:%d,"
							"m_UserID:%d,"							
							"m_Side:%d,"
							"m_Exchange:%d,"
							"m_Price:%f,"
							"m_Qty:%d,"
							"m_CustomField:%I64d,"
							"m_Account:%s,",
							pField->m_ClientOrderToken,
							pField->m_MarketOrderToken,
							pField->m_OrderState,
							pField->m_UserID,							
							pField->m_Side,
							pField->m_Exchange,
							pField->m_Price,
							pField->m_Qty,
							pField->m_CustomField,
							pField->m_Account
							);
		}
		//to do ...
		int orderId = pField->m_CustomField;	
#if 0
		m_ees_local_market_token_map.emplace(pField->m_MarketOrderToken, orderId);
		m_ees_local_client_token_map.emplace(pField->m_ClientOrderToken, orderId);
#endif
		//end to do
		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnRspOrderInsertAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = sl_order_aux::anchor(this, pField);
			if (o)
			{
				add_pending_order(o);
			}
			break;
		default:
			{
			loggerv2::info("sl_connection::OnRspOrderInsertAsync ret:%d\n", ret);
			break;
			}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sl_connection::OnRspOrderInsertAsync - order recovered nullptr");
			return;
		}
		//
		if (m_UserId == pField->m_UserID)//local order
		{
			m_ees_local_market_token_map.emplace(pField->m_MarketOrderToken, o->get_id());
			m_ees_local_client_token_map.emplace(pField->m_ClientOrderToken, o->get_id());
		}
		else//external order
		{
			user_info_ex * user_info = nullptr;
			if (m_external_user_info_map.find(pField->m_MarketOrderToken) != m_external_user_info_map.end())
			{
				user_info = m_external_user_info_map[pField->m_MarketOrderToken];
			}
			else
			{
				user_info = new user_info_ex();
				user_info->MarketToken = pField->m_MarketOrderToken;
				m_external_user_info_map[pField->m_MarketOrderToken] = user_info;
			}
			user_info->Symbol   = pField->m_Symbol;
			user_info->SideType = pField->m_Side;
			user_info->OrderId  = o->get_id();
		}
		//
		if (o->get_quantity() != pField->m_Qty)
		{
			if (m_debug)
				loggerv2::debug("sl_connection::OnRspOrderInsertAsync resetting order quantity to %d", pField->m_Qty);
			o->set_quantity(pField->m_Qty);
		}
		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("sl_connection::OnRspOrderInsertAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
				o->set_price(pField->m_Price);
			}
		}
		// 2=order dead（单子死了）
		if (pField->m_OrderState == 2)
		{
			char szErrorMsg[32 + 1];
			memset(szErrorMsg, 0, sizeof(szErrorMsg));
			snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d(%s)", pField->m_OrderState,"order dead");
			on_nack_from_market_cb(o, szErrorMsg);		
		}		
		else
		{
			sl_order_aux::set_market_token(o, std::to_string(pField->m_MarketOrderToken).c_str());					
#if 0
			o->set_status(AtsType::OrderStatus::WaitMarket);
			//比较特殊		
			sl_order_aux::set_pre_book_qty(o, pField->m_Qty);
			connection::update_pending_order(o);
#else
			//
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{
				//update_instr_on_ack_from_market_cb(o);
				//on_ack_from_market_cb(o);
			}
			//
#endif
		}
	}
	//撤单,需要
	//1.更新order的book
	//2.更新合约的可平仓量
	void sl_connection::OnRspOrderActionAsync(EES_OrderCxled* pCancel)
	{
		if (m_debug)
			loggerv2::info("sl_connection::OnRspOrderActionAsync");		
		/*
		/// 订单撤销完成
		struct EES_OrderCxled
		{
		EES_UserID        m_Userid;							///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond    m_Timestamp;						///< 撤单时间，从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ClientToken   m_ClientOrderToken;				///< 原来单子的token
		EES_MarketToken   m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int      m_Decrement;						///< 这次信息所取消的单子量
		EES_CxlReasonCode m_Reason;							///< 原因，见下表
		typedef unsigned char EES_CxlReasonCode;				///< 撤单成功的原因
		#define EES_CxlReasonCode_by_account			1		///< =用户撤单
		#define EES_CxlReasonCode_timeout				2		///< =系统timeout, 单子到期被交易所系统取消
		#define EES_CxlReasonCode_supervisory			3		///< =Supervisory, 被盛立系统管理者取消
		#define EES_CxlReasonCode_by_market				4		///< =被市场拒绝
		#define EES_CxlReasonCode_another				255		///< =其他
		};
		*/
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnRspOrderActionAsync "
				"m_Userid:%d,"
				"m_local_Userid:%d,"
				"m_ClientOrderToken:%d,"
				"m_MarketOrderToken:%I64d,"
				"m_Decrement:%d,"
				"m_Reason:%d,",
				pCancel->m_Userid,
				this->get_local_user_id(),
				pCancel->m_ClientOrderToken,
				pCancel->m_MarketOrderToken,
				pCancel->m_Decrement,
				pCancel->m_Reason
				);

		}
		int orderId = -1;
		if (m_ees_local_market_token_map.find(pCancel->m_MarketOrderToken) != m_ees_local_market_token_map.end())
		{
			orderId = m_ees_local_market_token_map[pCancel->m_MarketOrderToken];
		}
		else
		{
			if (m_external_user_info_map.find(pCancel->m_MarketOrderToken) != m_external_user_info_map.end())
			{
				orderId = m_external_user_info_map[pCancel->m_MarketOrderToken]->OrderId;
		}
		else
		{
			loggerv2::warn("sl_connection::OnRspOrderActionAsync didn't find the order id,by the m_MarketOrderToken:%I64d\n", pCancel->m_MarketOrderToken);			
		}
		}
		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnRspOrderActionAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
		default:
			break;
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sl_connection::OnRspOrderActionAsync - order recovered nullptr");
			return;
		}		
		string strError = "default";
		switch (pCancel->m_Reason)
		{
		case EES_CxlReasonCode_by_account:
			strError = "cancel by account";
			break;
		case EES_CxlReasonCode_timeout:
			strError = "cancel by timeout";
			break;
		case EES_CxlReasonCode_supervisory:
			strError = "cancel by supervisory";
			break;
		case EES_CxlReasonCode_by_market:
			strError = "cancel by market";
			break;
		case EES_CxlReasonCode_another:
			strError = "cancel by another";
			break;
		default:
			break;
		}
		o->set_lastreason(strError.c_str());
		update_instr_on_cancel_from_market_cb(o, pCancel->m_Decrement);
		on_cancel_from_market_cb(o);		
	}	
	//WaitMarket changed to Ack
	//订单已经被市场接受，需要：
	//1.order的book
	//2.合约的可平仓量
	void sl_connection::OnRtnOrderAsync(EES_OrderMarketAcceptField * pField)
	{
		/*
		/// 下单被市场接受消息
		struct EES_OrderMarketAcceptField
		{
		EES_Account       m_Account;          ///< 用户代码
		EES_MarketToken   m_MarketOrderToken; ///< 盛立系统产生的单子号，和盛立交流时可用该号。
		EES_MarketOrderId m_MarketOrderId;    ///< 市场订单号
		EES_Nanosecond    m_MarketTime;       ///< 市场时间信息
		};
		*/	
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnRtnOrderAsync "
				"m_Account:%s,"
				"m_MarketOrderToken:%I64d,"
				"m_MarketOrderId:%s"
				,
				pField->m_Account,
				pField->m_MarketOrderToken,
				pField->m_MarketOrderId
				);
		}
		tm tmResult;
		unsigned int nanosecond =0;
		this->m_pUserApi->ConvertFromTimestamp(pField->m_MarketTime, tmResult, nanosecond);
		//ptime MarketTime = ptime_from_tm(tmResult);
		//string str = to_iso_extended_string(MarketTime);
		//to do ...
		int orderId = -1;
		if (m_ees_local_market_token_map.find(pField->m_MarketOrderToken) != m_ees_local_market_token_map.end())
		{
			orderId = m_ees_local_market_token_map[pField->m_MarketOrderToken];
		}
		else
		{
			if (m_external_user_info_map.find(pField->m_MarketOrderToken) != m_external_user_info_map.end())
			{
				orderId = m_external_user_info_map[pField->m_MarketOrderToken]->OrderId;
		}
		else
		{
			loggerv2::warn("sl_connection::OnRtnOrderAsync didn't find the order id,by the m_MarketOrderToken:%I64d\n", pField->m_MarketOrderToken);			
		}
		}
		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = sl_order_aux::anchor(this, pField);
			if (o)
			{
				add_pending_order(o);
			}
			break;
		default:
			{	
				loggerv2::info("sl_connection::OnRtnOrderAsync ret:%d\n", ret);
				break;
			}
		}			
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sl_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}
#if 0
		o->set_book_quantity(sl_order_aux::get_pre_book_qty(o));
		on_ack_from_market_cb(o);
		update_instr_on_ack_from_market_cb(o);				
#else
		if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
		{
			update_instr_on_ack_from_market_cb(o);
			on_ack_from_market_cb(o);
		}
#endif
	}

	void sl_connection::OnRtnTradeAsync(EES_OrderExecutionField* pExec)
	{
		/*
		/// 订单成交消息体
		struct EES_OrderExecutionField
		{
		EES_UserID        m_Userid;							///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond    m_Timestamp;						///< 成交时间，从1970年1月1日0时0分0秒开始的纳秒时间
		EES_ClientToken   m_ClientOrderToken;				///< 原来单子的你的token
		EES_MarketToken   m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int      m_Quantity;						///< 单子成交量
		double            m_Price;							///< 单子成交价
		EES_MarketToken   m_ExecutionID;					///< 单子成交号(TAG 1017)
		EES_MarketExecId  m_MarketExecID;					///< 交易所成交号
		};
		*/
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnRtnTradeAsync "
				"m_Userid:%d,"
				"m_local_Userid:%d,"
				"m_ClientOrderToken:%d,"
				"m_MarketOrderToken:%I64d,"
				"m_Quantity:%d,"
				"m_Price:%f,"
				"m_ExecutionID:%d,"
				"m_MarketExecID:%s,"
#ifndef Linux
				"m_Timestamp:%I64d,",
#else
				"m_Timestamp:%lld,",
#endif
				pExec->m_Userid,
				this->get_local_user_id(),
				pExec->m_ClientOrderToken,
				pExec->m_MarketOrderToken,
				pExec->m_Quantity,
				pExec->m_Price,
				pExec->m_ExecutionID,
				pExec->m_MarketExecID,
				pExec->m_Timestamp
				);
		}
#if 0
		int orderId = get_order_id(pTrade->UserID);
#else
		int orderId = 0/*get_order_id(pTrade->OrderRef)*/;
		//if (m_ees_local_market_token_map.find(pExec->m_MarketOrderToken) != m_ees_local_market_token_map.end())
		//{
		//	orderId = m_ees_local_market_token_map[pExec->m_MarketOrderToken];		
		//}
		//else
		//{
		//	loggerv2::warn("sl_connection::OnRtnTradeAsync didn't find the order id,by the m_MarketOrderToken:%I64d,m_MarketExecID:%s\n", pExec->m_MarketOrderToken, pExec->m_MarketExecID);
		//}
		
		if ((m_UserId==pExec->m_Userid) && m_ees_local_client_token_map.find(pExec->m_ClientOrderToken) != m_ees_local_client_token_map.end())
		{
			orderId = m_ees_local_client_token_map[pExec->m_ClientOrderToken];
			//
			m_ees_local_market_token_map.emplace(pExec->m_MarketOrderToken, orderId);
			//
		}
		else
		{
			if (m_external_user_info_map.find(pExec->m_MarketOrderToken) != m_external_user_info_map.end())
			{
				orderId = m_external_user_info_map[pExec->m_MarketOrderToken]->OrderId;
			}
			else
			{
			loggerv2::warn("sl_connection::OnRtnTradeAsync didn't find the order id,by the m_ClientOrderToken:%d,m_MarketExecID:%s\n", pExec->m_ClientOrderToken, pExec->m_MarketExecID);
		}
		}
#endif
		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = sl_order_aux::anchor(this, pExec);
			if (o)
			{
				add_pending_order(o);
			}
			break;
		default:
			{
			loggerv2::info("sl_connection::OnRtnTradeAsync ret:%d\n", ret);
			break;
			}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sl_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}
		//printf("sl_connection::OnRtnTradeAsync orderId:%d,pTrade->OrderRef:%s,ret:%d,trandeId:%s\n",orderId,pTrade->OrderRef,ret,pTrade->TradeID);
		// 2 - treat message
		int execQty = pExec->m_Quantity;
		double execPrc = pExec->m_Price;
		const char* pszExecRef = pExec->m_MarketExecID;
		const char* pszTime = nullptr;				
		tm tmResult;
		unsigned int nanosecond =0;
		this->m_pUserApi->ConvertFromTimestamp(pExec->m_Timestamp,tmResult,nanosecond);

		time_t tt = mktime(&tmResult);
		std::chrono::seconds sec(tt);
		lwtp tp(sec);
		int hour = get_hour_from_lwtp(tp);
		std::string traderingTime = lwtp_to_simple_time_string(tp);
		tp = tp + std::chrono::seconds(2);
		
		
		exec* e = new exec(o, pszExecRef, execQty, execPrc, traderingTime.data());
		on_exec_from_market_cb(o, e);		
		bool onlyUpdatePending = false;

		if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			onlyUpdatePending = true;
		if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;
		if (onlyUpdatePending)
		{
			loggerv2::info("sl_connection::OnRtnTradeAsync tradeTime %s,tmResult.tm_hour:%d,tmResult.tm_min:%d,nanosecond:%d,onlyUpdatePending:%d", traderingTime.data(), tmResult.tm_hour, tmResult.tm_min, nanosecond, onlyUpdatePending);
		}
		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
	}
	void sl_connection::OnRspRejectActionAsync(EES_OrderRejectField* pReject)
	{
		/*
		/// 下单被柜台系统拒绝
		struct EES_OrderRejectField
		{
		EES_UserID				m_Userid;			///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond			m_Timestamp;		///< 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ClientToken			m_ClientOrderToken;	///< 原来单子的token
		EES_RejectedMan			m_RejectedMan;		///< 被谁拒绝，盛立系统还是下面连的交易所 1=盛立
		EES_ReasonCode			m_ReasonCode;		///< 单子被拒绝的理由。这张表将来会增加。请见下表。
		EES_GrammerResult		m_GrammerResult;	///< 语法检查的结果数组，每个字符映射一种检查错误原因，见文件末尾的附录
		EES_RiskResult			m_RiskResult;		///< 风控检查的结果数组，每个字符映射一种检查错误原因，见文件末尾的附录
		EES_GrammerResultText	m_GrammerText;		///< 语法检查的结果文字描述
		EES_RiskResultText		m_RiskText;			///< 风控检查的结果文字描述
		};
		*/
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnRspRejectActionAsync "
				"m_Userid:%d,"
				"m_local_Userid:%d,"
				"m_ClientOrderToken:%d,"
				"m_ReasonCode:%d,"
				"m_GrammerText:%s,",
				pReject->m_Userid,
				this->get_local_user_id(),
				pReject->m_ClientOrderToken,
				pReject->m_ReasonCode,
				pReject->m_GrammerText
				);
		}
		int orderId = -1;
		if (m_ees_local_client_token_map.find(pReject->m_ClientOrderToken) != m_ees_local_client_token_map.end())
		{
			orderId = m_ees_local_client_token_map[pReject->m_ClientOrderToken];
		}
		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;
		default:
			{
			loggerv2::info("sl_connection::OnRspRejectActionAsync ret:%d\n", ret);
			break;
			}
		}
		if (o != nullptr)
		{
			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer,"%d[%s][%s]",pReject->m_ReasonCode, pReject->m_GrammerText, pReject->m_RiskText);
			on_nack_from_market_cb(o, buffer);
		}
	}
	void sl_connection::OnRspMarketRejectActionAsync(EES_OrderMarketRejectField* pReject)
	{
		/*
		/// 下单被市场拒绝
		struct EES_OrderMarketRejectField
		{
			EES_Account     m_Account;           ///< 用户代码
			EES_MarketToken m_MarketOrderToken;	 ///< 盛立系统产生的单子号，和盛立交流时可用该号。
			EES_Nanosecond  m_MarketTimestamp;   ///< 市场时间信息, 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
			EES_ReasonText  m_ReasonText;      
		};
		*/
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnRspMarketRejectActionAsync "
				"m_Account:%s"
				"m_MarketOrderToken:%I64d"
				"m_ReasonText:%s",
				pReject->m_Account,
				pReject->m_MarketOrderToken,
				pReject->m_ReasonText				
				);
		}
		int orderId = -1;
		if (m_ees_local_market_token_map.find(pReject->m_MarketOrderToken) != m_ees_local_market_token_map.end())
		{
			orderId = m_ees_local_market_token_map[pReject->m_MarketOrderToken];
		}
		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;
		default:
		{
			loggerv2::info("sl_connection::OnRspRejectActionAsync ret:%d\n", ret);
			break;
		}
		}
		if (o != nullptr)
		{
			update_instr_on_nack_from_market_cb(o);
			on_nack_from_market_cb(o, pReject->m_ReasonText);
		}
	}
	void sl_connection::OnUserInfoAsync(user_info* pInfo)
	{
		this->append(pInfo);
	}
	//	
	int sl_connection::get_external_order_id(EES_MarketToken marketToken)
	{
		if (m_external_user_info_map.find(marketToken) != m_external_user_info_map.end())
		{
			return  m_external_user_info_map[marketToken]->OrderId;
		}
		return -1;
	}

	bool sl_connection::get_external_symbol(EES_MarketToken marketToken, string & symbol)
	{
		if (m_external_user_info_map.find(marketToken) != m_external_user_info_map.end())
		{
			symbol = m_external_user_info_map[marketToken]->Symbol;
			return true;
		}
		return false;
	}
	EES_SideType sl_connection::get_ess_external_side_type(EES_MarketToken marketToken)
	{
		if (m_external_user_info_map.find(marketToken) != m_external_user_info_map.end())
		{
			return  m_external_user_info_map[marketToken]->SideType;
		}
		return 0;
	}
	int sl_connection::get_order_id(EES_ClientToken clientTocken)
	{		
		if (m_ees_local_client_token_map.find(clientTocken) != m_ees_local_client_token_map.end())
		{
			return m_ees_local_client_token_map[clientTocken];
		}		
		return -1;
	}
	bool sl_connection::get_symbol(EES_ClientToken clientToken, string & symbol)
	{
		if (m_local_user_info_map.find(clientToken) != m_local_user_info_map.end())
		{
			symbol = m_local_user_info_map[clientToken]->Symbol;
			return true;
		}
		return false;
	}
	EES_SideType sl_connection::get_ess_side_type(EES_ClientToken clientToken)
	{
		if (m_local_user_info_map.find(clientToken) != m_local_user_info_map.end())
		{
			return  m_local_user_info_map[clientToken]->SideType;			
		}
		return 0;
	}
	/*根据order.internalRef简历internalRef<->orderRef的映射关系*/
	void sl_connection::create_user_info(order * o, EES_ClientToken clientToken, string userID, string symbol,EES_SideType type)
	{
		if (o == nullptr)
			return;		
		sl_order_aux::set_client_token(o,clientToken);		
		if (m_local_user_info_map.find(clientToken) == m_local_user_info_map.end())
		{
			user_info * info = new user_info();
			info->ClientToken = clientToken;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			ctpbase_connection::compute_userId(o, buffer, sizeof(buffer));
			info->UserID = buffer;

			info->Symbol = symbol;

			info->SideType = type;

			//
			info->OrderId = o->get_id();
			//
			m_local_user_info_map.emplace(clientToken, info);
			//to do ... append the file every day
#if 0
			this->append(info);
#else
			m_userInfoQueue.CopyPush(info);
#endif
			//printf("sl_connection::create_user_info orderRef:%d,size:%d\n", orderRef, m_local_user_info_map.size());
		}
		else
		{
			loggerv2::info("warn:sl_connection::create_user_info already include the userid:%s,clientToken:%d\n", userID.c_str(), clientToken);
		}
	}
	void sl_connection::get_user_info(EES_ClientToken clientToken, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType)
	{	
		if (m_local_user_info_map.find(clientToken) != m_local_user_info_map.end())
		{
			user_info * info = m_local_user_info_map[clientToken];

			ctpbase_connection::get_user_info(info->UserID.c_str(), nAccount, userOrderId, internalRef, nPortfolio, nTradeType);

		}
		else
		{
			loggerv2::warn("sl_connection::get_user_info didn't find the clientToken:%d\n", clientToken);
		}		
	}
	void sl_connection::init_user_info(char * user_info_file)
	{
		if (user_info_file == nullptr)
			return;
		boost::filesystem::path p;
		p.clear();
		p.append(user_info_file);
		p.append("user_info.csv");
		m_user_info_file_name = p.string();
		printf("sl_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
			class user_info
			{
			public:
				EES_ClientToken ClientToken;
				string UserID;				
				//int    InternalRef;
				//int    AccountNum;
				//int    UserOrderID;		
				//int    TradeType;
				//int    Portfolio;						
				string Symbol;
				};
			*/
			user_info * info = new user_info();
			info->ClientToken = atoi(tokenizer[0]);
			info->UserID = tokenizer[1];
			info->Symbol = tokenizer[2];
			info->SideType = atoi(tokenizer[3]);
			//
			info->OrderId = atoi(tokenizer[4]);
			//
			m_local_user_info_map.emplace(info->ClientToken, info);
			//to do ...
#if 0
			if (m_ees_local_client_token_map.find(info->ClientToken) == m_ees_local_client_token_map.end())
			{				
				int nAccount     = 0;
				int nUserOrdId   = -1;
				int nInternalRe  = -1;
				int nPortfolio   = 0;
				int nTradingType = 0;
				ctpbase_connection::get_user_info(info->UserID.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
				m_ees_local_client_token_map.emplace(info->ClientToken, nInternalRe);
			}
#else
			m_ees_local_client_token_map.emplace(info->ClientToken, info->OrderId);
#endif
		}
		stream.close();
	}

	void sl_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d,%s,%s,%d,%d\n",info->ClientToken,info->UserID.c_str(),info->Symbol.c_str(),info->SideType,info->OrderId);
		stream << buffer;
		stream.close();
	}
	void sl_connection::init_api()
	{
		m_inputQueue.setHandler(boost::bind(&sl_connection::OnRspOrderInsertAsync, this, _1));
		m_orderQueue.setHandler(boost::bind(&sl_connection::OnRtnOrderAsync, this, _1));
		m_tradeQueue.setHandler(boost::bind(&sl_connection::OnRtnTradeAsync, this, _1));
		m_inputActionQueue.setHandler(boost::bind(&sl_connection::OnRspOrderActionAsync, this, _1));
		m_pUserApi = CreateEESTraderApi();
		m_rejectQueue.setHandler(boost::bind(&sl_connection::OnRspRejectActionAsync, this, _1));
		m_marketRejectQueue.setHandler(boost::bind(&sl_connection::OnRspMarketRejectActionAsync, this, _1));
		m_userInfoQueue.setHandler(boost::bind(&sl_connection::OnUserInfoAsync, this, _1));
	}
	void sl_connection::release_api()
	{
		if (m_pUserApi != nullptr)
		{
			m_pUserApi->DisConnServer();
			DestroyEESTraderApi(m_pUserApi);
			m_pUserApi = nullptr;
		}
	}
	//
	// RTThread
	//
	void sl_connection::Process_api()
	{
		m_inputQueue.Pops_Handle(10);
		m_orderQueue.Pops_Handle(10);
		m_tradeQueue.Pops_Handle(10);
		m_inputActionQueue.Pops_Handle(10);
		m_rejectQueue.Pops_Handle(10);
		m_marketRejectQueue.Pops_Handle(10);
		m_userInfoQueue.Pops_Handle(10);
#ifdef Linux
		bool vaild = m_inputQueue.read_available() || m_orderQueue.read_available() || m_tradeQueue.read_available()
			|| m_inputActionQueue.read_available() || m_rejectQueue.read_available() || m_marketRejectQueue.read_available()||m_userInfoQueue.read_available();
		if (vaild)
		{
			uint64_t buf = 1;
			int wlen = 0;
			while (1)
			{
				wlen = write(efd, &buf, sizeof(buf));
				if (wlen >= 0)
					break;
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
		}
#endif
	}
	bool sl_connection::connect_api()
	{
		//loggerv2::info("calling sl_connection::connect");
		// For first connection, we are disconnected so we need to connect API first (RequestLogin will be done on API UP).
		// For later connections (disconnect / reconnect), API is already up so we just need to relogin.
		//
		if (m_connectionStatus == false)
		{
			if (m_pUserApi != nullptr)
			{
				RESULT ret = 0;
				if (m_strQryServerIp.size() < 1)
				{
					ret = m_pUserApi->ConnServer(this->m_sHostname.c_str(), atoi(this->m_sService.c_str()), this, m_strQryServerIp.c_str(), atoi(this->m_strQryServerPort.c_str()));
					loggerv2::info("sl_connection::connect api intialized,%s:%s,ret:%d", m_sHostname.c_str(), this->m_sService.c_str(), ret);
					printf_ex("sl_connection::connect api intialized,%s:%s,ret:%d\n", m_sHostname.c_str(), this->m_sService.c_str(), ret);
				}
				else
				{
					ret = m_pUserApi->ConnServer(this->m_sHostname.c_str(), atoi(this->m_sService.c_str()), this, m_strQryServerIp.c_str(), atoi(this->m_strQryServerPort.c_str()));
					loggerv2::info("sl_connection::connect api intialized,%s:%s,ret:%d,qry %s:%s", m_sHostname.c_str(), this->m_sService.c_str(), ret, m_strQryServerIp.c_str(), m_strQryServerPort.c_str());
					printf_ex("sl_connection::connect api intialized,%s:%s,ret:%d,qry %s:%s\n", m_sHostname.c_str(), this->m_sService.c_str(), ret, m_strQryServerIp.c_str(), m_strQryServerPort.c_str());
				}
			}		
		}
		else
		{
			request_login();
		}	
		return true;
	}
	bool sl_connection::disconnect_api()
	{	
		m_connectionStatus = false;
		m_connectionStatus = false;
		if (this->m_pUserApi)
		{
			RESULT ret = m_pUserApi->DisConnServer();
			printf("sl_connection::disconnect_api ret:%d\n",ret);
			loggerv2::info("sl_connection::disconnect_api ret:%d\n", ret);
			return (NO_ERROR==ret);
		}
		return false;
	}
	void sl_connection::request_login()
	{
		loggerv2::info("sl_connection::request_login");
		if (m_pUserApi != nullptr)
		{
			/*
			virtual RESULT UserLogon(const char* user_id, const char* user_pwd, const char* prodInfo, const char* macAddr ) = 0 ;
			*/
			m_pUserApi->UserLogon(this->get_login_id().c_str(), this->m_sPassword.c_str(),"AAA","BBB");
		}
	}
	//
	// prder sending
	//
	bool sl_connection::ReqOrderInsert(EES_EnterOrderField* pRequest)
	{
		//EES_ClientToken order_token = 0;
		//m_tradeApi->GetMaxToken(&order_token);
		//to do ... may be use the m_nCurrentOrderRef = pLogon->m_MaxToken;
		pRequest->m_ClientOrderToken = ++m_nCurrentOrderRef;
		loggerv2::info("sl_connection::ReqOrderInsert pRequest->m_ClientOrderToken:%d,orderId:%d\n", pRequest->m_ClientOrderToken, pRequest->m_CustomField);
		int ret = m_pUserApi->EnterOrder(pRequest);

		if (ret != 0)
		{
			return false;
		}
		return true;
	}
	bool sl_connection::ReqOrderAction(EES_CancelOrder* pRequest)
	{
		/*
		/// 下单撤单指令
		struct EES_CancelOrder
		{
		EES_MarketToken m_MarketOrderToken;					///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int    m_Quantity;							///< 这是该单子被取消后所希望剩下的数量，如为0，改单子为全部取消。在中国目前必须填0，其他值当0处理。
		EES_Account     m_Account;							///< 帐户ID号
		};
		*/
		int ret = m_pUserApi->CancelOrder(pRequest);
		if (ret != 0)
		{
			loggerv2::info("error:sl_connection::ReqOrderAction ret:%d\n", ret);
			return false;
		}
		return true;
	}
	std::string sl_connection::getMaturity(std::string& sMat)
	{
		std::string newMat;
		newMat = sMat.substr(0, 4);
		newMat += "-";
		newMat += sMat.substr(4, 2);
		newMat += "-";
		newMat += sMat.substr(6, 2);
		return newMat.c_str();
	}
	void sl_connection::request_instruments()
	{
#if 0
		fstech::CThostFtdcQryInstrumentField request;
		memset(&request, 0, sizeof(request));
		m_pUserApi->ReqQryInstrument(&request, ++m_nRequestId);
#else
		RESULT ret = m_pUserApi->QuerySymbolList();
		printf_ex("sl_connection::request_instruments ret:%d\n",ret);
#endif
	}
	//end add on 20160929

	/// 连接消息的回调

	///	\brief	服务器连接事件
	///	\param  errNo                   连接成功能与否的消息
	///	\param  pErrStr                 错误信息
	///	\return void  

	void sl_connection::OnConnection(ERR_NO errNo, const char* pErrStr)
	{
		loggerv2::info("sl_connection::OnConnection is UP");

		m_connectionStatus = true;

		if (this->getStatus() == AtsType::ConnectionStatus::WaitConnect)
		{
			request_login();
		}
		else
		{
			loggerv2::info("sl_connection::OnConnection not asking for reconnect...");
		}
	}

	/// 连接断开消息的回调

	/// \brief	服务器主动断开，会收到这个消息
	/// \param  ERR_NO errNo         连接成功能与否的消息
	/// \param  const char* pErrStr  错误信息
	/// \return void  

	void sl_connection::OnDisConnection(ERR_NO errNo, const char* pErrStr)
	{
		loggerv2::error("ssl_connection::OnDisConnection is DOWN,err:%d(%s)",errNo,pErrStr);

		m_connectionStatus = false;

		this->on_status_changed(AtsType::ConnectionStatus::Disconnected, pErrStr);
		//
		m_retry_count++;

		if (m_retry_count < MAX_RETRY_COUNT)
		{
		//自动重连
			this->connect();
		}
		//
	}

	/// 登录消息的回调

	/// \param  pLogon                  登录成功或是失败的结构
	/// \return void 

	void sl_connection::OnUserLogon(EES_LogonResponse* pLogon)
	{
		if (pLogon)
		{
			string err = "default";
			switch (pLogon->m_Result)
			{
			case 0:
				//
				m_UserId = pLogon->m_UserId;
				//
				m_nCurrentOrderRef = pLogon->m_MaxToken;
				printf_ex("sl_connection::OnUserLogon m_nCurrentOrderRef:%d,m_UserId:%d\n", m_nCurrentOrderRef, m_UserId);
				loggerv2::info("sl_connection::OnUserLogon m_nCurrentOrderRef:%d,m_UserId:%d\n", m_nCurrentOrderRef, m_UserId);
				this->on_status_changed(AtsType::ConnectionStatus::Connected,"");
				//
				m_retry_count = 0;
				//to do ...查询订单信息
				if (this->m_bRequestPosition)
				{
					this->request_investor_full_positions();
				}
				else if (this->getRequestInstruments()==true)
				{
					this->request_instruments();
				}
				if (m_pUserApi && m_bPosition == false)
				{
					m_pUserApi->QueryAccountOrder(this->get_account().c_str());
					//m_pUserApi->QueryAccountOrderExecution(this->get_account().c_str());		
					//m_bPosition = true;
				}
				return;
				break;
			case 1:
				err = "用户名/密码错误";
				break;
			case 2:
				err = "用户存在配置问题，如账户列表为空等";
				break;
			case 3:
				err = "不允许重复登录";
				break;
			case 5:
				err = "缺失客户端产品信息、MAC地址等必要信息";
				break;
			default:
				break;
			}
			this->on_status_changed(AtsType::ConnectionStatus::Disconnected, err.c_str());
		}		
	}

	/// 修改密码响应回调

	/// \param  nResult                  服务器响应的成功与否返回码
	/// \return void 

	void sl_connection::OnRspChangePassword(EES_ChangePasswordResult nResult)
	{

	}

	/// 查询用户下面帐户的返回事件

	/// \param  pAccountInfo	        帐户的信息
	/// \param  bFinish	                如果没有传输完成，这个值是 false ，如果完成了，那个这个值为 true 
	/// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pAccountInfo值无效。
	/// \return void 

	void sl_connection::OnQueryUserAccount(EES_AccountInfo * pAccoutnInfo, bool bFinish)
	{
		/*
		/// 帐户信息基本信息
		struct EES_AccountInfo
		{
		EES_Account			m_Account;						///< 帐户ID
		EES_Previlege		m_Previlege;					///< 操作权限，目前硬件暂不支持，也就是说都是完全操作权限 99：完全操作  1：只读 2：只平仓
		double				m_InitialBp;					///< 初始权益
		double				m_AvailableBp;					///< 总可用资金
		double				m_Margin;						///< 所有仓位占用的保证金
		double				m_FrozenMargin;					///< 所有挂单冻结的保证金
		double				m_CommissionFee;				///< 已扣除的手续费总金额
		double				m_FrozenCommission;				///< 挂单冻结的总手续费金额
		};
		*/
		if (pAccoutnInfo)
		{
			loggerv2::info("sl_connection::OnQueryUserAccount,"
				"m_Account:%s,"
				"m_AvailableBp:%f,"
				"m_Margin:%f", pAccoutnInfo->m_Account, pAccoutnInfo->m_AvailableBp, pAccoutnInfo->m_Margin);
			
		}
	}

	/// 查询帐户下面仓位信息的返回事件

	/// \param  pAccount	                帐户ID 	
	/// \param  pAccoutnPosition	        帐户的仓位信息					   
	/// \param  nReqId		                发送请求消息时候的ID号。
	/// \param  bFinish	                    如果没有传输完成，这个值是false，如果完成了，那个这个值为 true 
	/// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pAccountInfo值无效。
	/// \return void 

	void sl_connection::OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish)
	{
		if (bFinish == true)
		{
			if (this->getRequestInstruments() == true)
			{
				sleep_by_milliseconds(2000);
				this->request_instruments();
				this->setRequestInstruments(false);
			}
		}
		/*
		/// 帐户的仓位信息
		struct EES_AccountPosition
		{
		EES_Account			m_actId;						///< Value  Notes
		EES_Symbol			m_Symbol;						///< 合约名称/股票代码
		EES_PosiDirection	m_PosiDirection;				///< 多空方向 1：多头 5：空头
		unsigned int		m_InitOvnQty;					///< 隔夜仓初始数量，这个值不会变化，除非通过HelpDesk手工修改
		unsigned int		m_OvnQty;						///< 当前隔夜仓数量，可以为0
		unsigned int		m_FrozenOvnQty;					///< 冻结的昨仓数量
		unsigned int		m_TodayQty;						///< 当前今仓数量，可能为0
		unsigned int		m_FrozenTodayQty;				///< 冻结的今仓数量
		double				m_OvnMargin;					///< 隔夜仓占用保证金
		double				m_TodayMargin;					///< 今仓占用的保证金
		EES_HedgeFlag		m_HedgeFlag;					///< 仓位对应的投机套利标志
		};
		*/		
		if (pAccoutnPosition && strlen(pAccoutnPosition->m_Symbol)>0)
		{
			//
#if 0
			loggerv2::info("Calling sl_connection::OnQueryAccountPosition "
				"m_actId[%s],"
				"m_Symbol[%s],"
				"m_PosiDirection[%d],"
				"m_InitOvnQty[%d],"
				"m_OvnQty[%d],"
				"m_FrozenOvnQty[%d],"
				"m_TodayQty[%d],"
				"m_FrozenTodayQty[%d],"
				"m_OvnMargin[%f],"
				"m_TodayMargin[%f],"
				"m_HedgeFlag[%c]"
				"bFinish:%d",
				pAccoutnPosition->m_actId,
				pAccoutnPosition->m_Symbol,
				pAccoutnPosition->m_PosiDirection,
				pAccoutnPosition->m_InitOvnQty,   ///今总持仓量
				pAccoutnPosition->m_OvnQty, ///昨持仓量
				pAccoutnPosition->m_FrozenOvnQty,///开仓冻结持仓
				pAccoutnPosition->m_TodayQty,
				pAccoutnPosition->m_FrozenTodayQty, ///平仓冻结持仓
				pAccoutnPosition->m_OvnMargin,  ///冻结的保证金
				pAccoutnPosition->m_TodayMargin,
				pAccoutnPosition->m_HedgeFlag,
				bFinish
				);
#endif
		
			//
			std::string sInstrCode = std::string(pAccoutnPosition->m_Symbol) + "@" + this->getName();
			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
			if (i)
			{
				//std::string str = i->getMarket();			
				if (pAccoutnPosition->m_PosiDirection == 1)//long///持仓多空方向
				{
					//总的保证金，包括今仓和昨仓的保证金
					//i->set_long_used_margin(pAccoutnPosition->m_OvnMargin + pAccoutnPosition->m_TodayMargin);					
					//今仓
#if 0
					i->set_today_long_position(pAccoutnPosition->m_TodayQty);				
#else
					if (i->get_today_long_position() != pAccoutnPosition->m_TodayQty)
					{
						loggerv2::info("sl_connection::OnQueryAccountPosition:today_long_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_today_long_position(pAccoutnPosition->m_TodayQty);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();
						//islog = true;
					}
#endif
					//i->set_pending_short_close_today_qty(pAccoutnPosition->m_FrozenTodayQty);
					//隔夜仓,表示昨仓
#if 0
					i->set_yst_long_position(pAccoutnPosition->m_OvnQty);
#else
					if (i->get_yst_long_position() != pAccoutnPosition->m_OvnQty)
					{
						loggerv2::info("sl_connection::OnQueryAccountPosition:yst_long_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_yst_long_position(pAccoutnPosition->m_OvnQty);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();						
					}
#endif
					//?
					//i->set_pending_short_close_qty(pAccoutnPosition->m_FrozenOvnQty + pAccoutnPosition->m_FrozenTodayQty);
				}
				else if (pAccoutnPosition->m_PosiDirection == 5)//short///持仓多空方向
				{
					//i->set_short_used_margin(pAccoutnPosition->m_OvnMargin + pAccoutnPosition->m_TodayMargin);	
					//今仓
#if 0
					i->set_today_short_position(pAccoutnPosition->m_TodayQty);
#else
					if (i->get_today_short_position() != pAccoutnPosition->m_TodayQty)
					{
						loggerv2::info("sl_connection::OnQueryAccountPosition:today_short_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_today_short_position(pAccoutnPosition->m_TodayQty);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();
						//islog = true;
					}
#endif
					//i->set_pending_long_close_today_qty(pAccoutnPosition->m_FrozenTodayQty);

					//
#if 0			
					i->set_yst_short_position(pAccoutnPosition->m_OvnQty);			
#else
					if (i->get_yst_short_position() != pAccoutnPosition->m_OvnQty)
					{
						loggerv2::info("sl_connection::OnQueryAccountPosition:yst_short_position change,Code=%s", i->getCode().c_str());
						i->dumpinfo();
						i->set_yst_short_position(pAccoutnPosition->m_OvnQty);
						i->set_last_sychro_timepoint(get_lwtp_now());
						i->dumpinfo();		
					}
#endif
					//i->set_pending_long_close_qty(pAccoutnPosition->m_FrozenOvnQty + pAccoutnPosition->m_FrozenTodayQty);

				}		
#if 0
				i->set_tot_long_position(i->get_today_long_position() + i->get_yst_long_position());
				i->set_tot_short_position(i->get_today_short_position() + i->get_yst_short_position());			
				i->set_last_sychro_timepoint(get_lwtp_now());			
#endif
				if (i->get_tot_long_position() != i->get_today_long_position() + i->get_yst_long_position())
				{
					loggerv2::info("sl_connection::OnQueryAccountPosition:tot_long_position change,Code=%s", i->getCode().c_str());
					i->dumpinfo();
					i->set_tot_long_position(i->get_today_long_position() + i->get_yst_long_position());
					i->set_last_sychro_timepoint(get_lwtp_now());
					i->dumpinfo();
					//islog = true;
				}
				if (i->get_tot_short_position() != i->get_today_short_position() + i->get_yst_short_position())
				{
					loggerv2::info("sl_connection::OnQueryAccountPosition:tot_short_position change,Code=%s", i->getCode().c_str());
					i->dumpinfo();
					i->set_tot_short_position(i->get_today_short_position() + i->get_yst_short_position());
					i->set_last_sychro_timepoint(get_lwtp_now());
					i->dumpinfo();
					//islog = true;
				}
			}
			else
			{
				//loggerv2::error("sl_connection::OnQueryAccountPosition cannot find tradeitem %s by second key", std::string(pAccoutnPosition->m_Symbol).c_str());
			}
		}
		if (bFinish == true)
		{
			//loggerv2::info("sl_connection::OnRspQryInvestorPosition bFinish == true");
#if 0
			for (auto &it : tradeitem_gh::get_instance().container().get_map())
			{
				it.second->dumpinfo();
			}
#endif
		}
	}

	/// 查询帐户下面资金信息的返回事件

	/// \param  pAccount	                帐户ID 	
	/// \param  pAccoutnPosition	        帐户的仓位信息					   
	/// \param  nReqId		                发送请求消息时候的ID号
	/// \return void 

	void sl_connection::OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccoutnPosition, int nReqId)
	{
		/*
		/// 帐户的仓位信息
		struct EES_AccountBP
		{
		EES_Account			m_account;						///< Value  Notes
		double				m_InitialBp;					///< 初始权益
		double				m_AvailableBp;					///< 总可用资金
		double				m_Margin;						///< 所有仓位占用的保证金
		double				m_FrozenMargin;					///< 所有挂单冻结的保证金
		double				m_CommissionFee;				///< 已扣除的手续费总金额
		double				m_FrozenCommission;				///< 挂单冻结的总手续费金额
		double				m_OvnInitMargin;				///< 初始昨仓保证金
		double				m_TotalLiquidPL;				///< 总平仓盈亏
		double				m_TotalMarketPL;				///< 总持仓盈亏
		};
		*/		
		if (pAccoutnPosition)
		{
			double ratio = 0;
			
			if (pAccoutnPosition->m_Margin + pAccoutnPosition->m_AvailableBp != 0)
			{
				ratio = pAccoutnPosition->m_Margin / (pAccoutnPosition->m_Margin + pAccoutnPosition->m_AvailableBp);
			}

			loggerv2::info("sl_connection::OnQueryAccountBP,"
				"m_Account:%s,"
				"m_AvailableBp:%f,"
				"m_Margin:%f,"
				"ratio:%f", pAccoutnPosition->m_account, pAccoutnPosition->m_AvailableBp, pAccoutnPosition->m_Margin,ratio);

			this->set_RiskDegree(ratio);
		}
	}

	/// 查询合约列表的返回事件

	/// \param  pSymbol	                    合约信息   
	/// \param  bFinish	                    如果没有传输完成，这个值是 false，如果完成了，那个这个值为 true   
	/// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pSymbol 值无效。
	/// \return void 

	void sl_connection::OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish)
	{
		/*
		/// 合约列表
		struct EES_SymbolField
		{
		EES_SecType			m_SecType;						///< 3=Future，目前仅支持期货
		EES_Symbol			m_symbol;						///< 合约名称/股票代码
		EES_SymbolName		m_symbolName;					///< 合约名称
		EES_ExchangeID		m_ExchangeID;					///< 102=中金所   103=上期所    104=大商所    105=郑商所
		EES_ProductID		m_ProdID;						///< 产品代码
		unsigned int		m_DeliveryYear;					///< 交割年
		unsigned int		m_DeliveryMonth;				///< 交割月
		unsigned int		m_MaxMarketOrderVolume;			///< 市价单最大下单量
		unsigned int		m_MinMarketOrderVolume;			///< 市价单最小下单量
		unsigned int		m_MaxLimitOrderVolume;			///< 限价单最大下单量
		unsigned int		m_MinLimitOrderVolume;			///< 限价单最小下单量
		unsigned int		m_VolumeMultiple;				///< 合约乘数
		double				m_PriceTick;					///< 最小变动价位
		unsigned int		m_CreateDate;					///< 创建日期
		unsigned int		m_OpenDate;						///< 上市日期
		unsigned int		m_ExpireDate;					///< 到期日
		unsigned int		m_StartDelivDate;				///< 开始交割日
		unsigned int		m_EndDelivDate;					///< 结束交割日
		unsigned int		m_InstLifePhase;				///< 合约生命周期状态   0=未上市    1=上市    2=停牌    3=到齐
		unsigned int		m_IsTrading;					///< 当前是否交易   0=未交易    1=交易
		};
		*/
		if (pSymbol && bFinish==false)
		{
			loggerv2::info("sl_connection::OnQuerySymbol "
				"m_SecType:%d,"
				"m_symbol:%s,"
				"m_symbolName:%s,"
				"m_ExchangeID:%d,"
				"m_ProdID：%s,"
				"m_VolumeMultiple:%d,"
				"m_PriceTick:%f,"
				"m_ExpireDate:%d,"
				"m_InstLifePhase:%d,"
				"m_IsTrading:%d,"
				"bFinish:%d",
				pSymbol->m_SecType,
				pSymbol->m_symbol,
				pSymbol->m_symbolName,
				pSymbol->m_ExchangeID,
				pSymbol->m_ProdID,
				pSymbol->m_VolumeMultiple,
				pSymbol->m_PriceTick,
				pSymbol->m_ExpireDate,
				pSymbol->m_InstLifePhase,
				pSymbol->m_IsTrading, 
				bFinish
				);
			//
			m_database->open_database();
			string sInstr           = pSymbol->m_symbol;
			std::string sUnderlying = pSymbol->m_ProdID; //
			std::string strExecDate = std::to_string(pSymbol->m_ExpireDate);
			std::string sMat = getMaturity(strExecDate); 
			std::string sCmd = "";
			std::string sExcge = std::to_string(pSymbol->m_ExchangeID);
			if (pSymbol->m_SecType == EES_SecType_fut)
			{
				transform(sUnderlying.begin(), sUnderlying.end(), sUnderlying.begin(), static_cast<int(*)(int)>(std::toupper));

				std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";

				std::string sInstClass = "F_" + sUnderlying;
				
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
						loggerv2::info("sl_connection::OnQuerySymbol:failed to insert into database, ret is %d,cmd:%s", rc, sCmd.c_str());
					}
					else
					{
						loggerv2::info("sl_connection::OnQuerySymbol cmd:%s\n", sCmd.c_str());
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
						loggerv2::info("sl_connection::OnQuerySymbol:failed to update the database,error is %d,cmd:%s", rc, sCmd.c_str());
					}
					else
					{
						loggerv2::info("sl_connection::OnQuerySymbol update to the cmd:%s\n", sCmd.c_str());
					}
				}
			}
			m_database->close_databse();
			//
		}
		if (bFinish == true)
		{
			printf_ex("sl_connection::OnQuerySymbol bFinish == true\n");
			loggerv2::info("sl_connection::OnQuerySymbol bFinish == true\n");
			if (get_is_last() == false)
			{
				set_is_last(true);
			}
		}
	}

	/// 查询帐户交易保证金的返回事件

	/// \param  pAccount                    帐户ID 
	/// \param  pSymbolMargin               帐户的保证金信息 
	/// \param  bFinish	                    如果没有传输完成，这个值是 false，如果完成，那个这个值为 true 
	/// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pSymbolMargin 值无效。
	/// \return void 

	void sl_connection::OnQueryAccountTradeMargin(const char* pAccount, EES_AccountMargin* pSymbolMargin, bool bFinish)
	{
		/*
		/// 查询帐户的保证金率
		struct EES_AccountMargin
		{
		EES_SecType			m_SecType;						///< 3=Future，目前仅支持期货
		EES_Symbol			m_symbol;						///< 合约名称/股票代码
		EES_ExchangeID		m_ExchangeID;					///< 102=中金所   103=上期所    104=大商所    105=郑商所
		EES_ProductID		m_ProdID;						///< 4  Alpha 产品代码
		double				m_LongMarginRatio;				///< 多仓保证金率
		double				m_ShortMarginRatio;				///< 空仓保证金率，目前用不上
		};
		*/
		loggerv2::info("sl_connection::OnQueryAccountTradeMargin m_symbol:%s,m_LongMarginRatio:%f,m_ShortMarginRatio:%f\n", pSymbolMargin->m_symbol, pSymbolMargin->m_LongMarginRatio, pSymbolMargin->m_ShortMarginRatio);
	}

	/// 查询帐户交易费用的返回事件

	/// \param  pAccount                    帐户ID 
	/// \param  pSymbolFee	                帐户的费率信息	 
	/// \param  bFinish	                    如果没有传输完成，这个值是 false，如果完成了，那个这个值为 true    
	/// \remark 如果碰到 bFinish == true ，那么是传输结束，并且 pSymbolFee 值无效。
	/// \return void 

	void sl_connection::OnQueryAccountTradeFee(const char* pAccount, EES_AccountFee* pSymbolFee, bool bFinish)
	{
		/*
		/// 帐户合约费率查询
		struct EES_AccountFee
		{
		EES_SecType			m_SecType;						///<  3=Future，目前仅支持期货
		EES_Symbol			m_symbol;						///<  合约名称/股票代码
		EES_ExchangeID		m_ExchangeID;					///<  102=中金所    103=上期所    104=大商所    105=郑商所
		EES_ProductID		m_ProdID;						///<  产品代码
		double				m_OpenRatioByMoney;				///<  开仓手续费率，按照金额
		double				m_OpenRatioByVolume;			///<  开仓手续费率，按照手数
		double				m_CloseYesterdayRatioByMoney;	///<  平昨手续费率，按照金额
		double				m_CloseYesterdayRatioByVolume;	///<  平昨手续费率，按照手数
		double				m_CloseTodayRatioByMoney;		///<  平今手续费率，按照金额
		double				m_CloseTodayRatioByVolume;		///<  平今手续费率，按照手数
		EES_PosiDirection	m_PositionDir;					///<  1: 多头；2: 空头
		};
		*/
		loggerv2::info("sl_connection::OnQueryAccountTradeFee m_OpenRatioByVolume:%f,m_CloseYesterdayRatioByVolume:%f,m_CloseTodayRatioByVolume:%f,m_PositionDir:%d,m_symbol:%s\n", pSymbolFee->m_OpenRatioByVolume, pSymbolFee->m_CloseYesterdayRatioByVolume, pSymbolFee->m_CloseTodayRatioByVolume, pSymbolFee->m_PositionDir, pSymbolFee->m_symbol);
	}

	/// 下单被柜台系统接受的事件

	/// \brief 表示这个订单已经被柜台系统正式的接受
	/// \param  pAccept	                    订单被接受以后的消息体
	/// \return void 

	void sl_connection::OnOrderAccept(EES_OrderAcceptField* pAccept)
	{
#if 0
		pOrder->IsAutoSuspend = pRspInfo->ErrorID;
		m_inputQueue.CopyPush(pOrder);
#else
		/*
		/// 下单被柜台系统接受消息
		struct EES_OrderAcceptField
		{
		EES_ClientToken     m_ClientOrderToken;				///< 下单的时候，返回给你的token
		EES_MarketToken     m_MarketOrderToken;				///< 市场里面挂单的token
		EES_OrderState      m_OrderState;					///< 订单状态
		//EES_OrderState		m_OrderState;					///< 单子状态，绝大多时候是1，但是也有可能是2.    1=order live（单子活着）    2=order dead（单子死了）
		EES_UserID          m_UserID;						///< 订单的 user id
		EES_Nanosecond      m_AcceptTime;					///< 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_Account         m_Account;						///< 用户代码
		EES_SideType        m_Side;							///< 买卖方向
		EES_ExchangeID      m_Exchange;						///< 交易所
		EES_Symbol          m_Symbol;						///< 合约代码
		EES_SecType         m_SecType;						///< 交易品种
		double              m_Price;						///< 价格
		unsigned int        m_Qty;							///< 数量
		EES_ForceCloseType  m_ForceCloseReason;				///< 强平原因
		EES_OrderTif		m_Tif;							///< 用户下单时指定的值
		unsigned int		m_MinQty;						///< 用户下单时指定的值
		EES_CustomFieldType m_CustomField;					///< 用户下单时指定的值
		EES_MarketSessionId m_MarketSessionId;				///< 报单送往交易所的席位代码，有可能和下单时指定的不同。不同的原因有：当前该席位尚未连接好；指定的席位代号非法等；指定0：由REM自行决定
		EES_HedgeFlag		m_HedgeFlag;					///< 投机套利标志
		};
		*/		
		m_inputQueue.CopyPush(pAccept);
#endif
	}


	/// 下单被市场接受的事件

	/// \brief 表示这个订单已经被交易所正式的接受
	/// \param  pAccept	                    订单被接受以后的消息体，里面包含了市场订单ID
	/// \return void 
	void sl_connection::OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept)
	{
		/*
		/// 下单被市场接受消息
		struct EES_OrderMarketAcceptField
		{
		EES_Account       m_Account;          ///< 用户代码
		EES_MarketToken   m_MarketOrderToken; ///< 盛立系统产生的单子号，和盛立交流时可用该号。
		EES_MarketOrderId m_MarketOrderId;    ///< 市场订单号
		EES_Nanosecond    m_MarketTime;       ///< 市场时间信息
		};
		*/
		m_orderQueue.CopyPush(pAccept);
	}


	///	下单被柜台系统拒绝的事件

	/// \brief	订单被柜台系统拒绝，可以查看语法检查或是风控检查。 
	/// \param  pReject	                    订单被接受以后的消息体
	/// \return void 

	void sl_connection::OnOrderReject(EES_OrderRejectField* pReject)
	{
		/*
		/// 下单被柜台系统拒绝
		struct EES_OrderRejectField
		{
		EES_UserID				m_Userid;			///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond			m_Timestamp;		///< 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ClientToken			m_ClientOrderToken;	///< 原来单子的token
		EES_RejectedMan			m_RejectedMan;		///< 被谁拒绝，盛立系统还是下面连的交易所 1=盛立
		EES_ReasonCode			m_ReasonCode;		///< 单子被拒绝的理由。这张表将来会增加。请见下表。
		EES_GrammerResult		m_GrammerResult;	///< 语法检查的结果数组，每个字符映射一种检查错误原因，见文件末尾的附录
		EES_RiskResult			m_RiskResult;		///< 风控检查的结果数组，每个字符映射一种检查错误原因，见文件末尾的附录
		EES_GrammerResultText	m_GrammerText;		///< 语法检查的结果文字描述
		EES_RiskResultText		m_RiskText;			///< 风控检查的结果文字描述
		};
		*/
		loggerv2::info("skip:sl_connection::OnOrderReject m_ClientOrderToken:%d,m_RiskText:%s\n", pReject->m_ClientOrderToken, pReject->m_RiskText);
		m_rejectQueue.CopyPush(pReject);
	}


	///	下单被市场拒绝的事件

	/// \brief	订单被市场拒绝，可以查看语法检查或是风控检查。 
	/// \param  pReject	                    订单被接受以后的消息体，里面包含了市场订单ID
	/// \return void 

	void sl_connection::OnOrderMarketReject(EES_OrderMarketRejectField* pReject)
	{
		/*
		/// 下单被市场拒绝
		struct EES_OrderMarketRejectField
		{
		EES_Account     m_Account;           ///< 用户代码
		EES_MarketToken m_MarketOrderToken;	 ///< 盛立系统产生的单子号，和盛立交流时可用该号。
		EES_Nanosecond  m_MarketTimestamp;   ///< 市场时间信息, 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ReasonText  m_ReasonText;
		};
		*/
		loggerv2::info("skip:sl_connection::OnOrderMarketReject m_MarketOrderToken:%I64d,m_ReasonText:%s\n", pReject->m_MarketOrderToken, pReject->m_ReasonText);
		m_marketRejectQueue.CopyPush(pReject);
	}


	///	订单成交的消息事件

	/// \brief	成交里面包括了订单市场ID，建议用这个ID查询对应的订单
	/// \param  pExec	                   订单被接受以后的消息体，里面包含了市场订单ID
	/// \return void 

	void sl_connection::OnOrderExecution(EES_OrderExecutionField* pExec)
	{
		/*
		/// 订单成交消息体
		struct EES_OrderExecutionField
		{
		EES_UserID        m_Userid;							///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond    m_Timestamp;						///< 成交时间，从1970年1月1日0时0分0秒开始的纳秒时间
		EES_ClientToken   m_ClientOrderToken;				///< 原来单子的你的token
		EES_MarketToken   m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int      m_Quantity;						///< 单子成交量
		double            m_Price;							///< 单子成交价
		EES_MarketToken   m_ExecutionID;					///< 单子成交号(TAG 1017)
		EES_MarketExecId  m_MarketExecID;					///< 交易所成交号
		};
		*/
		m_tradeQueue.CopyPush(pExec);
	}

	///	订单成功撤销事件

	/// \brief	成交里面包括了订单市场ID，建议用这个ID查询对应的订单
	/// \param  pCxled		               订单被接受以后的消息体，里面包含了市场订单ID
	/// \return void 

	void sl_connection::OnOrderCxled(EES_OrderCxled* pCxled)
	{
		/*
		/// 订单撤销完成
		struct EES_OrderCxled
		{
		EES_UserID        m_Userid;							///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond    m_Timestamp;						///< 撤单时间，从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ClientToken   m_ClientOrderToken;				///< 原来单子的token
		EES_MarketToken   m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int      m_Decrement;						///< 这次信息所取消的单子量
		EES_CxlReasonCode m_Reason;							///< 原因，见下表
		};
		*/
		//loggerv2::info("sl_connection::OnOrderCxled m_MarketOrderToken:%I64d,m_Reason:%d\n", pCxled->m_MarketOrderToken, pCxled->m_Reason);
		//printf("sl_connection::OnOrderCxled m_Reason:%d\n",pCxled->m_Reason);
		m_inputActionQueue.CopyPush(pCxled);
	}

	///	撤单被拒绝的消息事件

	/// \brief	一般会在发送撤单以后，收到这个消息，表示撤单被拒绝
	/// \param  pReject	                   撤单被拒绝消息体
	/// \return void 

	void sl_connection::OnCxlOrderReject(EES_CxlOrderRej* pReject)
	{
		/*
		/// 撤单被拒绝的消息体
		struct EES_CxlOrderRej
		{
		EES_Account			m_account;						///< 客户帐号.
		EES_MarketToken		m_MarketOrderToken;				///< 盛立内部用的orderID
		unsigned int		m_ReasonCode;					///< 错误码，每个字符映射一种检查错误原因，见文件末尾的附录，这是用二进制位表示的一个原因组合
		EES_ReasonText		m_ReasonText;					///< 错误字符串，未使用
		};
		*/
		loggerv2::info("sl_connection::OnCxlOrderReject m_MarketOrderToken:%I64d,m_ReasonText:%s,m_ReasonCode:%d\n", pReject->m_MarketOrderToken, pReject->m_ReasonText, pReject->m_ReasonCode);
	}

	///	查询订单的返回事件

	/// \brief	查询订单信息时候的回调，这里面也可能包含不是当前用户下的订单
	/// \param  pAccount                 帐户ID 
	/// \param  pQueryOrder	             查询订单的结构
	/// \param  bFinish	                 如果没有传输完成，这个值是 false，如果完成了，那个这个值为 true    
	/// \remark 如果碰到 bFinish == true，那么是传输结束，并且 pQueryOrder值无效。
	/// \return void 

	void sl_connection::OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish)
	{
		if (bFinish == true)
		{
			m_pUserApi->QueryAccountOrderExecution(this->get_account().c_str());		
			m_bPosition = true;
		}
		/*
		/// 查询订单的结构
		struct EES_QueryAccountOrder
		{
		EES_UserID			m_Userid;						///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond		m_Timestamp;					///< 订单创建时间，从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ClientToken		m_ClientOrderToken;				///< 原来单子的token
		EES_SideType		m_SideType;						///< 1 = 买单（开今） 2 = 卖单（平今）  3= 买单（平今） 4 = 卖单（开今）  5= 买单（平昨） 6= 卖单（平昨） 7=买单（强平昨）  8=卖单（强平昨）  9=买单（强平今）  10=买单（强平今）
		unsigned int		m_Quantity;						///< 数量（股票为股数，期货为手数）
		EES_SecType			m_InstrumentType;				///< 1＝Equity 股票 2＝Options 期权 3＝Futures 期货
		EES_Symbol			m_symbol;						///< 股票代码，期货代码或者期权代码，以中国交易所标准 (目前6位就可以)
		double				m_Price;						///< 价格
		EES_Account			m_account;						///< 61 16  Alpha 客户帐号.  这个是传到交易所的客户帐号。验证后，必须是容许的值，也可能是这个连接的缺省值。
		EES_ExchangeID		m_ExchengeID;					///< 100＝上交所  101=深交所  102=中金所  103=上期所  104=大商所  105=郑商所  255= done-away  See appendix
		EES_ForceCloseType	m_ForceCloseReason;				///< 强平原因： - 0=非强平  - 1=资金不足  - 2=客户超仓  - 3=会员超仓  - 4=持仓非整数倍  - 5=违规  - 6=其他
		EES_MarketToken		m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		EES_OrderStatus		m_OrderStatus;					///< 请参考EES_OrderStatus的定义
		typedef unsigned char EES_OrderStatus;					///< 按照二进制与存放多个订单状态
		#define EES_OrderStatus_shengli_accept			0x80	///< bit7=1：EES系统已接受
		#define EES_OrderStatus_mkt_accept				0x40	///< bit6=1：市场已接受或者手工干预订单
		#define EES_OrderStatus_executed				0x20	///< bit5=1：已成交或部分成交
		#define EES_OrderStatus_cancelled				0x10 	///< bit4=1：已撤销, 可以是部分成交后撤销
		#define EES_OrderStatus_cxl_requested			0x08	///< bit3=1：发过客户撤单请求
		#define EES_OrderStatus_reserved1				0x04	///< bit2：保留, 目前无用
		#define EES_OrderStatus_reserved2				0x02	///< bit1：保留, 目前无用
		#define EES_OrderStatus_closed					0x01	///< bit0=1：已关闭, (拒绝/全部成交/已撤销)
		EES_Nanosecond		m_CloseTime;					///< 订单关闭事件，从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		int					m_FilledQty;					///< 0  4 Int4  成交数量
		EES_OrderTif		m_Tif;							///< 用户下单时指定的值
		unsigned int		m_MinQty;						///< 用户下单时指定的值
		EES_CustomFieldType m_CustomField;					///< 用户下单时指定的值
		EES_MarketOrderId	m_MarketOrderId;				///< 交易所单号
		EES_HedgeFlag		m_HedgeFlag;					///< 投机套利标志
		};
		*/
		if (strlen(pQueryOrder->m_symbol)<1)
		{
			return;
		}
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnQueryTradeOrder "
				"m_Userid:%d,"
				"m_local_Userid:%d,"
				"m_MarketOrderToken:%I64d,"
				"m_ClientOrderToken:%d,"
				"m_SideType:%d,"
				"m_Quantity:%d,"
				"m_InstrumentType:%d,"
				"m_symbol:%s,"
				"m_Price:%f,"
				"m_account:%s,"
				"m_OrderStatus:%d,"
				"m_CustomField:%I64d,"
				"m_MarketOrderId:%s,"
				"bFinish:%d",
				pQueryOrder->m_Userid,
				this->get_local_user_id(),
				pQueryOrder->m_MarketOrderToken,
				pQueryOrder->m_ClientOrderToken,
				pQueryOrder->m_SideType,
				pQueryOrder->m_Quantity,
				pQueryOrder->m_InstrumentType,
				pQueryOrder->m_symbol,
				pQueryOrder->m_Price,
				pQueryOrder->m_account,
				pQueryOrder->m_OrderStatus,
				pQueryOrder->m_CustomField,
				pQueryOrder->m_MarketOrderId,
				bFinish
				);
		}
		//printf("-----sl_connection::OnQueryTradeOrder pAccount:%s,symbol:%s,bFinish:%d,m_OrderStatus:%d,m_ClientOrderToken:%d,m_Price:%f,m_MarketOrderId:%s\n", pAccount, pQueryOrder->m_symbol, bFinish, pQueryOrder->m_OrderStatus, pQueryOrder->m_ClientOrderToken, pQueryOrder->m_Price, pQueryOrder->m_MarketOrderId);
#if 0		
		order *o = sl_order_aux::anchor(this, pQueryOrder);
		if (o == nullptr)
		{
			loggerv2::error("sl_connection::OnQueryTradeOrder cannot anchor order");
			return;
		}
		add_pending_order(o);	
#else
		int ret     = -1;
		int orderId = -1;
		if (m_UserId == pQueryOrder->m_Userid)
		{
		orderId  = this->get_order_id(pQueryOrder->m_ClientOrderToken);
	    }
		else
		{
			orderId = this->get_external_order_id(pQueryOrder->m_MarketOrderToken);
			//if (m_ees_local_market_token_map.find(pQueryOrder->m_MarketOrderToken) != m_ees_local_market_token_map.end())
			//{
			//	orderId = m_ees_local_market_token_map[pQueryOrder->m_MarketOrderToken];
			//}
		}
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnQueryTradeOrder - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = sl_order_aux::anchor(this, pQueryOrder);
			if (o)
			{
				add_pending_order(o);
			}
			break;
		default:
			break;
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sl_connection::OnQueryTradeOrder - order recovered nullptr");
			return;
		}
#endif
		//for debug
#if 0
		if (o->get_id() == 6)
		{
			printf("only debug orderid:%d\n",o->get_id());
		}
#endif
		//
		if (m_UserId == pQueryOrder->m_Userid)
		{			
			m_ees_local_market_token_map.emplace(pQueryOrder->m_MarketOrderToken, o->get_id());
			m_ees_local_client_token_map.emplace(pQueryOrder->m_ClientOrderToken, o->get_id());
		}
		else//external
		{
			user_info_ex * user_info = nullptr;
			if (m_external_user_info_map.find(pQueryOrder->m_MarketOrderToken) != m_external_user_info_map.end())
			{
				user_info = m_external_user_info_map[pQueryOrder->m_MarketOrderToken];
			}	
			else
			{
				user_info = new user_info_ex();
				user_info->MarketToken = pQueryOrder->m_MarketOrderToken;
				m_external_user_info_map[pQueryOrder->m_MarketOrderToken] = user_info;
			}			
			user_info->Symbol      = pQueryOrder->m_symbol;
			user_info->SideType    = pQueryOrder->m_SideType;
			user_info->OrderId     = o->get_id();
		}
		//
		#if 0
		///< bit0=1：已关闭, (拒绝/全部成交/已撤销)
		if ((pQueryOrder->m_OrderStatus & EES_OrderStatus_closed) == EES_OrderStatus_closed)
		{
            //#define EES_OrderStatus_cancelled				0x10 	///< bit4=1：已撤销, 可以是部分成交后撤销			
			if ((pQueryOrder->m_OrderStatus & EES_OrderStatus_cancelled) == EES_OrderStatus_cancelled)
			{
				//撤销成功
				o->set_last_action(AtsType::OrderAction::Cancelled);

				//o->get_open_close(),o->get_way()
				update_instr_on_ack_from_market_cb(o, pQueryOrder->m_Quantity - pQueryOrder->m_FilledQty);
				
				//o->get_quantity(),o->get_exec_quantity() or o->get_last_action()
				on_ack_from_market_cb(o);										
			}
            //#define EES_OrderStatus_executed				0x20	///< bit5=1：已成交或部分成交			
			if ((pQueryOrder->m_OrderStatus & EES_OrderStatus_executed) == EES_OrderStatus_executed)
			{
				//全部成交
				//1.需要更新order的book	
				//o->get_quantity(),o->get_exec_quantity() or o->get_last_action()				
				on_ack_from_market_cb(o);
				//2.需要更新合约的可平仓量
				//o->get_open_close(),o->get_way()				
				this->update_instr_on_ack_from_market_cb(o);			
			}	
            ///#define EES_OrderStatus_shengli_accept			0x80	///< bit7=1：EES系统已接受
			if ((pQueryOrder->m_OrderStatus & EES_OrderStatus_shengli_accept) == EES_OrderStatus_shengli_accept)
			{
				//柜台接受waitserver->waitmarket
				o->set_status(AtsType::OrderStatus::WaitMarket);
				//比较特殊		
				sl_order_aux::set_pre_book_qty(o, pQueryOrder->m_Quantity);
				connection::update_pending_order(o);
			}
			//#define EES_OrderStatus_mkt_accept				0x40	///< bit6=1：市场已接受或者手工干预订单
			if ((pQueryOrder->m_OrderStatus & EES_OrderStatus_mkt_accept) == EES_OrderStatus_mkt_accept)
			{
				//市场接受waitmarket->ack
				o->set_book_quantity(sl_order_aux::get_pre_book_qty(o));
				on_ack_from_market_cb(o);
				this->update_instr_on_ack_from_market_cb(o);
			}
		}
		else
		{

		}		
		#else 
		//1.柜台拒绝129,10000001
		if (pQueryOrder->m_OrderStatus == 129)
		{
			//printf("1.sl_connection::OnQueryTradeOrder m_OrderStatus:%d,m_ClientOrderToken:%d,orderId:%d,m_MarketOrderId:%s,m_ForceCloseReason:%d\n", pQueryOrder->m_OrderStatus, pQueryOrder->m_ClientOrderToken, o->get_id(), pQueryOrder->m_MarketOrderId, pQueryOrder->m_ForceCloseReason);
			//o->set_status(AtsType::OrderStatus::Reject);
			//update_instr_on_nack_from_market_cb(o);
			on_nack_from_market_cb(o,"129");			
		}
		//2.柜台接受
		//3.市场拒绝
		//4.市场接受
		//4.1部分成交
		//4.2全部成交225,11100001
		else if (pQueryOrder->m_OrderStatus == 225)
		{
			this->update_instr_on_ack_from_market_cb(o, pQueryOrder->m_Quantity);
			on_ack_from_market_cb(o);					
		}
		else if (pQueryOrder->m_OrderStatus == 192/* || pQueryOrder->m_OrderStatus == 128*/)
		{
			this->update_instr_on_ack_from_market_cb(o, pQueryOrder->m_Quantity);
			on_ack_from_market_cb(o);
		}
		//4.3撤销
		//4.3.1全部撤销209,11010001
		else if (pQueryOrder->m_OrderStatus == 209)
		{
			//update_instr_on_cancel_from_market_cb(o);
			on_cancel_from_market_cb(o);		
		}
		//4.3.2部分成交后撤销249,11111001
		else if (pQueryOrder->m_OrderStatus == 249)
		{
			//this->update_instr_on_cancel_from_market_cb(o);
			on_cancel_from_market_cb(o);			
		}
		//4.3.3发送过撤单请求
		else if (pQueryOrder->m_OrderStatus == 217)
		{
			//this->update_instr_on_cancel_from_market_cb(o);
			on_cancel_from_market_cb(o);			
		}
		else
		{
			loggerv2::error("sl_connection::OnQueryTradeOrder didn't do with the order status:%d\n", pQueryOrder->m_OrderStatus);
		}
		#endif
	}

	///	查询订单的返回事件

	/// \brief	查询订单信息时候的回调，这里面也可能包含不是当前用户下的订单成交
	/// \param  pAccount                        帐户ID 
	/// \param  pQueryOrderExec	                查询订单成交的结构
	/// \param  bFinish	                        如果没有传输完成，这个值是false，如果完成了，那个这个值为 true    
	/// \remark 如果碰到 bFinish == true，那么是传输结束，并且pQueryOrderExec值无效。
	/// \return void 

	void sl_connection::OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish)
	{
		if (bFinish == true)
			return;
		/*
		/// 查询订单成交的结构
		struct EES_QueryOrderExecution
		{
		EES_UserID			m_Userid;						///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond		m_Timestamp;					///< 成交时间，从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_ClientToken		m_ClientOrderToken;				///< 原来单子的你的token
		EES_MarketToken		m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int		m_ExecutedQuantity;				///< 单子成交量
		double				m_ExecutionPrice;				///< 单子成交价
		EES_MarketToken		m_ExecutionID;					///< 单子成交号(TAG 1017)
		EES_MarketExecId	m_MarketExecID;					///< 交易所成交号
		};
		*/
		//printf("sl_connection::OnQueryTradeOrderExec pAccount:%s,m_MarketExecID:%s,m_ClientOrderToken:%d,m_MarketOrderToken:%I64d\n", pAccount, pQueryOrderExec->m_MarketExecID, pQueryOrderExec->m_ClientOrderToken, pQueryOrderExec->m_MarketOrderToken);
		if (m_debug)
		{
			loggerv2::info("sl_connection::OnQueryTradeOrderExec "
				"m_Userid:%d,"
				"m_local_Userid:%d,"
				"m_ClientOrderToken:%d,"
				"m_MarketOrderToken:%I64d,"
				"m_ExecutedQuantity:%d,"
				"m_ExecutionPrice:%f,"
				"m_ExecutionID:%I64d,"
				"m_MarketExecID:%s,"
#ifndef Linux
				"m_Timestamp:%I64d,"
#else
				"m_Timestamp:%lld,"
#endif
				"bFinish:%d",				
				pQueryOrderExec->m_Userid,
				this->get_local_user_id(),
				pQueryOrderExec->m_ClientOrderToken,
				pQueryOrderExec->m_MarketOrderToken,
				pQueryOrderExec->m_ExecutedQuantity,
				pQueryOrderExec->m_ExecutionPrice,
				pQueryOrderExec->m_ExecutionID,
				pQueryOrderExec->m_MarketExecID, 
				pQueryOrderExec->m_Timestamp,
				bFinish
				);
		}		
		int ret     = -1;
		int orderId = -1;
		if ( m_UserId == pQueryOrderExec->m_Userid)
		{
		orderId = this->get_order_id(pQueryOrderExec->m_ClientOrderToken);
	    }
		else
		{
			orderId = this->get_external_order_id(pQueryOrderExec->m_MarketOrderToken);			
		}
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("sl_connection::OnQueryTradeOrderExec - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = sl_order_aux::anchor(this, pQueryOrderExec);
			if (o)
			{
				add_pending_order(o);
			}
			break;
		default:
			break;
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sl_connection::OnQueryTradeOrderExec - order recovered nullptr");
			return;
		}		
		//
		//
		if (m_UserId == pQueryOrderExec->m_Userid)
		{
			m_ees_local_market_token_map.emplace(pQueryOrderExec->m_MarketOrderToken, o->get_id());
			m_ees_local_client_token_map.emplace(pQueryOrderExec->m_ClientOrderToken, o->get_id());
		}
		else//external
		{
			user_info_ex * user_info = nullptr;
			if (m_external_user_info_map.find(pQueryOrderExec->m_MarketOrderToken) != m_external_user_info_map.end())
			{
				user_info = m_external_user_info_map[pQueryOrderExec->m_MarketOrderToken];
			}
			else
			{
				user_info = new user_info_ex();
				user_info->MarketToken = pQueryOrderExec->m_MarketOrderToken;
				m_external_user_info_map[pQueryOrderExec->m_MarketOrderToken] = user_info;
			}		
			user_info->OrderId = o->get_id();
		}
		//
		int execQty            = pQueryOrderExec->m_ExecutedQuantity;
		double execPrc         = pQueryOrderExec->m_ExecutionPrice;
		const char* pszExecRef = pQueryOrderExec->m_MarketExecID;
		const char* pszTime = nullptr;
		tm tmResult;
		unsigned int nanosecond = 0;
		m_pUserApi->ConvertFromTimestamp(pQueryOrderExec->m_Timestamp, tmResult, nanosecond);

		time_t tt = mktime(&tmResult);
		std::chrono::seconds sec(tt);
		lwtp tp(sec);
		int hour = get_hour_from_lwtp(tp);
		std::string traderingTime = lwtp_to_simple_time_string(tp);
		tp = tp + std::chrono::seconds(2);

		exec* e = new exec(o, pszExecRef, execQty, execPrc, traderingTime.data());
		on_exec_from_market_cb(o, e);		
		bool onlyUpdatePending = false;

		if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			onlyUpdatePending = true;
		if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;
		if (onlyUpdatePending)
		{			
			loggerv2::info("sl_connection::OnQueryTradeOrderExec tradeTime %s,tmResult.tm_hour:%d,tmResult.tm_min：%d,nanosecond:%d,onlyUpdatePending:%d", traderingTime.data(), tmResult.tm_hour, tmResult.tm_min, nanosecond, onlyUpdatePending);
		}
		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
	}

	///	接收外部订单的消息

	/// \brief	一般会在系统订单出错，进行人工调整的时候用到。
	/// \param  pPostOrder	                    查询订单成交的结构
	/// \return void 

	void sl_connection::OnPostOrder(EES_PostOrder* pPostOrder)
	{
		loggerv2::info("sl_connection::OnPostOrder\n");
	}

	///	接收外部订单成交的消息

	/// \brief	一般会在系统订单出错，进行人工调整的时候用到。
	/// \param  pPostOrderExecution	             查询订单成交的结构
	/// \return void 

	void sl_connection::OnPostOrderExecution(EES_PostOrderExecution* pPostOrderExecution)
	{
		/*
		/// 被动成交
		struct EES_PostOrderExecution
		{
		EES_UserID			m_Userid;						///< 原来单子的用户，对应着LoginID。
		EES_Nanosecond		m_Timestamp;					///< 被动成交时间，从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_MarketToken		m_MarketOrderToken;				///< 盛立系统产生的单子号，和盛立交流时可用该号。
		unsigned int		m_ExecutedQuantity;				///< 单子成交量
		double				m_ExecutionPrice;				///< 单子成交价
		EES_MarketToken		m_ExecutionNumber;				///< 单子成交号
		};
		*/
		loggerv2::info("sl_connection::OnPostOrderExecution\n");
	}

	///	查询交易所可用连接的响应

	/// \brief	每个当前系统支持的汇报一次，当bFinish= true时，表示所有交易所的响应都已到达，但本条消息本身不包含有用的信息。
	/// \param  pPostOrderExecution	             查询订单成交的结构
	/// \return void 
	void sl_connection::OnQueryMarketSession(EES_ExchangeMarketSession* pMarketSession, bool bFinish)
	{
		loggerv2::info("sl_connection::OnQueryMarketSession\n");
	}

	///	交易所连接状态变化报告，

	/// \brief	当交易所连接发生连接/断开时报告此状态
	/// \param  MarketSessionId: 交易所连接代码
	/// \param  ConnectionGood: true表示交易所连接正常，false表示交易所连接断开了。
	/// \return void 
	void sl_connection::OnMarketSessionStatReport(EES_MarketSessionId MarketSessionId, bool ConnectionGood)
	{
		loggerv2::info("sl_connection::OnMarketSessionStatReport\n");
	}

	///	合约状态变化报告

	/// \brief	当合约状态发生变化时报告
	/// \param  pSymbolStatus: 参见EES_SymbolStatus合约状态结构体定义
	/// \return void 
	void sl_connection::OnSymbolStatusReport(EES_SymbolStatus* pSymbolStatus)
	{
		//printf("sl_connection::OnSymbolStatusReport\n");
	}


	///	合约状态查询响应

	/// \brief  响应合约状态查询请求
	/// \param  pSymbolStatus: 参见EES_SymbolStatus合约状态结构体定义
	/// \param	bFinish: 当为true时，表示查询所有结果返回。此时pSymbolStatus为空指针NULL
	/// \return void 
	void sl_connection::OnQuerySymbolStatus(EES_SymbolStatus* pSymbolStatus, bool bFinish)
	{
		/*
		struct EES_SymbolStatus
		{
		EES_ExchangeID	m_ExchangeID;		///< 102=中金所    103=上期所    104=大商所    105=郑商所
		EES_Symbol		m_Symbol;			///< 合约代码
		unsigned char	m_InstrumentStatus;	///< 交易状态： '0':开盘前; '1':非交易; '2':连续交易; '3':集合竞价报单; '4'集合竞价价格平衡; '5':集合竞价撮合; '6': 收盘;
		unsigned int	m_TradingSegmentSN;	///< 交易阶段编号
		char			m_EnterTime[9];		///< 进入本状态时间
		unsigned char	m_EnterReason;		///< 进入本状态原因: '1': 自动切换; '2': 手动切换; '3': 熔断; '4': 熔断手动;
		};
		*/
		loggerv2::info("sl_connection::OnQuerySymbolStatus m_Symbol:%s\n", pSymbolStatus->m_Symbol);
	}	

}

