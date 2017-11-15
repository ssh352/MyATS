#include "sl_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "sl_connection.h"
namespace sl
{	
	order* sl_order_aux::anchor(sl_connection* pConnection, EES_OrderAcceptField* pField)
	{
		/*
		/// 下单被柜台系统接受消息
		struct EES_OrderAcceptField
		{
		EES_ClientToken     m_ClientOrderToken;				///< 下单的时候，返回给你的token
		EES_MarketToken     m_MarketOrderToken;				///< 市场里面挂单的token
		EES_OrderState      m_OrderState;					///< 订单状态
		EES_UserID          m_UserID;						///< 订单的 user id
		EES_Nanosecond      m_AcceptTime;					///< 从1970年1月1日0时0分0秒开始的纳秒时间，请使用ConvertFromTimestamp接口转换为可读的时间
		EES_Account         m_Account;						///< 用户代码
		EES_SideType        m_Side;							///< 买卖方向
		EES_ExchangeID      m_Exchange;						///< 交易所
		EES_Symbol          m_Symbol;						///< 合约代码
		EES_SecType         m_SecType;						///< 交易品种
		double              m_Price;						///< 价格
		unsigned int        m_Qty;							///< 数量
		EES_OptExecFlag		m_OptExecFlag;					///< 期权行权标志位
		EES_OrderTif		m_Tif;							///< 用户下单时指定的值
		unsigned int		m_MinQty;						///< 用户下单时指定的值
		EES_CustomFieldType m_CustomField;					///< 用户下单时指定的值
		EES_MarketSessionId m_MarketSessionId;				///< 报单送往交易所的席位代码，有可能和下单时指定的不同。不同的原因有：当前该席位尚未连接好；指定的席位代号非法等；指定0：由REM自行决定
		EES_HedgeFlag		m_HedgeFlag;					///< 投机套利标志
		};
		*/
		//1.
		std::string sInstrCode = std::string(pField->m_Symbol) + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("sl_order::anchor - tradeitem [%-*.*s] not found when EES_OrderAcceptField", sizeof(pField->m_Symbol), sizeof(pField->m_Symbol), pField->m_Symbol);
			return nullptr;
		}
		//2.
		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int id = -1;
#if 0
		id = pQueryOrder->m_CustomField;
		//to do ......find the user info
		pConnection->get_user_info(pQueryOrder->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
#else	
		if (pConnection->get_local_user_id()==pField->m_UserID)//local order
		{
			pConnection->get_user_info(pField->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = pConnection->get_external_order_id(pField->m_ClientOrderToken);
		}
		else
		{
			id = pConnection->get_external_order_id(pField->m_MarketOrderToken);
		}
#endif
		if (id < 1)
		{
			loggerv2::error("sl_order_aux::anchor didn't find the orderId,by the m_ClientOrderToken:%d\n", pField->m_ClientOrderToken);
			id = FAKE_ID_MIN + pField->m_MarketOrderToken;
			//return nullptr;
		}
		//3.
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);
		//
		o->set_quantity(pField->m_Qty);
		o->set_price(pField->m_Price);
		//盛立的特殊之处
		//o->set_exec_quantity(pQueryOrder->m_FilledQty);
		//
		switch (pField->m_Side)
		{
		case EES_SideType_open_long:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_today_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_close_ovn_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		case EES_SideType_close_today_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_open_short:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_ovn_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		default:
			break;
		}
		//
		if (pField->m_Tif == EES_OrderTif_IOC)
		{
			if (pField->m_MinQty > 0)
			{
				o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
			}
			else
			{
				o->set_restriction(AtsType::OrderRestriction::FillAndKill);
			}
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);
		//last time
		tm tmResult;
		unsigned int nanosecond = 0;
		pConnection->m_pUserApi->ConvertFromTimestamp(pField->m_AcceptTime, tmResult, nanosecond);

		time_t tt = mktime(&tmResult);
		std::chrono::seconds sec(tt);
		lwtp tp(sec);

		o->set_last_time(tp);
		//for debug		
		//printf("sl_order_aux::anchor m_Price:%f,m_Quantity:%d,markettime:%s,m_ClientOrderToken:%d,m_MarketOrderId:%s,orderId:%d\n", pQueryOrder->m_Price, pQueryOrder->m_Quantity, to_iso_extended_string(MarketTime).c_str(), pQueryOrder->m_ClientOrderToken, pQueryOrder->m_MarketOrderId,o->get_id());
		//
		o->save_previous_values();
		//rebuild time
		auto time = get_lwtp_now();
		o->set_rebuild_time(time);
		//
		sl_order_aux::set_client_token(o, pField->m_ClientOrderToken);
		sl_order_aux::set_market_token(o, std::to_string(pField->m_MarketOrderToken).c_str());
		//
		loggerv2::info("sl_order::anchor - successfully rebuild order [%d][%x] when receive EES_OrderAcceptField msg", id, o);
		return o;
	}
	order* sl_order_aux::anchor(sl_connection* pConnection, EES_OrderMarketAcceptField* pField)
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
		
		return nullptr;
	}
	order* sl_order_aux::anchor(sl_connection* pConnection, EES_OrderExecutionField* pExec)
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
		string symbol;
		int id = -1;		
		if (pConnection->m_UserId == pExec->m_Userid)//local order
		{
			if (pConnection->get_symbol(pExec->m_ClientOrderToken, symbol) == false)
			{
				loggerv2::warn("sl_order_aux::anchor didn't find the symbol,m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d\n", pExec->m_ClientOrderToken, pExec->m_MarketExecID, pExec->m_Userid);
				return nullptr;
			}
		}
		else
		{
			if (pConnection->get_external_symbol(pExec->m_MarketOrderToken, symbol) == false)
			{
				loggerv2::warn("sl_order_aux::anchor didn't find the symbol,m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d\n", pExec->m_ClientOrderToken, pExec->m_MarketExecID, pExec->m_Userid);
				return nullptr;
			}
		}
		std::string sInstrCode = symbol + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("sl_order_aux::anchor didn't find the tradeItem,symbol:%s,when EES_OrderExecutionField\n", symbol.c_str());
			return nullptr;
		}
		//2.
		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		//int id           = -1;
		EES_SideType type = 0;
#if 0
		pConnection->get_user_info(pQueryOrderExec->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
#else
		if (pConnection->m_UserId == pExec->m_Userid)//local order
		{
			pConnection->get_user_info(pExec->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			type = pConnection->get_ess_side_type(pExec->m_ClientOrderToken);
			id = pConnection->get_order_id(pExec->m_ClientOrderToken);
		}
		else
		{
			type = pConnection->get_ess_external_side_type(pExec->m_MarketOrderToken);
			id = pConnection->get_external_order_id(pExec->m_ClientOrderToken);
		}
#endif
		if (id < 1)
		{
			loggerv2::error("sl_order_aux::anchor didn't find the orderId,by the m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d\n", pExec->m_ClientOrderToken, pExec->m_MarketExecID, pExec->m_Userid);
			id = FAKE_ID_MIN + pExec->m_MarketOrderToken;
			//return nullptr;
		}
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);
		//
		o->set_exec_quantity(pExec->m_Quantity);
		o->set_exec_price(pExec->m_Price);
		//
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);
		//last time
		tm tmResult;
		unsigned int nanosecond = 0;
		pConnection->m_pUserApi->ConvertFromTimestamp(pExec->m_Timestamp, tmResult, nanosecond);

		time_t tt = mktime(&tmResult);
		std::chrono::seconds sec(tt);
		lwtp tp(sec);

		o->set_last_time(tp);
		//for debug
		//printf("++sl_order_aux::anchor m_ExecutionPrice:%f,TradeTime:%s\n", pQueryOrderExec->m_ExecutionPrice, to_iso_extended_string(TradeTime).c_str());
		//
		auto time = get_lwtp_now();
		o->set_rebuild_time(time);
		//
		//EES_SideType type = pConnection->get_ess_side_type(pQueryOrderExec->m_ClientOrderToken);
		switch (type)
		{
		case EES_SideType_open_long:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_today_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_close_ovn_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		case EES_SideType_close_today_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_open_short:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_ovn_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		default:
			break;
		}
		//
		sl_order_aux::set_client_token(o, pExec->m_ClientOrderToken);
		sl_order_aux::set_market_token(o, std::to_string(pExec->m_MarketOrderToken).c_str());
		//
		loggerv2::info("sl_order_aux::anchor - successfully rebuild order [%d][%x] when receive EES_OrderExecutionField msg", id, o);
		return o;
	}
	/*
	1.查询合约信息
	2.查询order id
	3.order status is ack,and update book
	4.
	*/
	order* sl_order_aux::anchor(sl_connection* pConnection, EES_QueryAccountOrder* pQueryOrder)
	{
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

		//1.
		std::string sInstrCode = std::string(pQueryOrder->m_symbol) + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("sl_order::anchor - tradeitem [%-*.*s] not found", sizeof(pQueryOrder->m_symbol), sizeof(pQueryOrder->m_symbol), pQueryOrder->m_symbol);
			return nullptr;
		}
		//2.
		int nAccount     =  0;
		int nUserOrdId   = -1;
		int nInternalRe  = -1;
		int nPortfolio   =  0;
		int nTradingType =  0;
		int id           = -1; 
#if 0
		id = pQueryOrder->m_CustomField;
		//to do ......find the user info
		pConnection->get_user_info(pQueryOrder->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
#else
		if (pConnection->get_local_user_id()==pQueryOrder->m_Userid)//local order
		{
			pConnection->get_user_info(pQueryOrder->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = pConnection->get_order_id(pQueryOrder->m_ClientOrderToken);
		}
		else
		{
			id = pConnection->get_external_order_id(pQueryOrder->m_MarketOrderToken);
		}
#endif
		if (id < 1)
		{
			loggerv2::error("sl_order_aux::anchor didn't find the orderId,by the m_ClientOrderToken:%d,m_MarketOrderId:%s\n", pQueryOrder->m_ClientOrderToken, pQueryOrder->m_MarketOrderId);
			id = FAKE_ID_MIN + pQueryOrder->m_MarketOrderToken;
			//return nullptr;
		}
		//3.
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);
		//
		o->set_quantity(pQueryOrder->m_Quantity);
		o->set_price(pQueryOrder->m_Price);
		//盛立的特殊之处
		//o->set_exec_quantity(pQueryOrder->m_FilledQty);
		//
		switch (pQueryOrder->m_SideType)
		{
		case EES_SideType_open_long:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_today_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_close_ovn_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		case EES_SideType_close_today_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_open_short:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_ovn_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		default:
			break;
		}
		//
		if (pQueryOrder->m_Tif == EES_OrderTif_IOC)
		{
			if (pQueryOrder->m_MinQty > 0)
			{
				o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
			}
			else
			{
				o->set_restriction(AtsType::OrderRestriction::FillAndKill);
			}
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);				
		//last time
		tm tmResult;
		unsigned int nanosecond = 0;
		pConnection->m_pUserApi->ConvertFromTimestamp(pQueryOrder->m_Timestamp, tmResult, nanosecond);

		time_t tt = mktime(&tmResult);
		std::chrono::seconds sec(tt);
		lwtp tp(sec);

		o->set_last_time(tp);
		//for debug		
		//printf("sl_order_aux::anchor m_Price:%f,m_Quantity:%d,markettime:%s,m_ClientOrderToken:%d,m_MarketOrderId:%s,orderId:%d\n", pQueryOrder->m_Price, pQueryOrder->m_Quantity, to_iso_extended_string(MarketTime).c_str(), pQueryOrder->m_ClientOrderToken, pQueryOrder->m_MarketOrderId,o->get_id());
		//
		o->save_previous_values();
		//rebuild time
		auto time = get_lwtp_now();	
		o->set_rebuild_time(time);		
		//
		sl_order_aux::set_client_token(o, pQueryOrder->m_ClientOrderToken);		
		sl_order_aux::set_market_token(o, std::to_string(pQueryOrder->m_MarketOrderToken).c_str());
		//
		loggerv2::info("sl_order::anchor - successfully rebuild order [%d][%x] when receive EES_QueryAccountOrder msg", id, o);
		return o;
	}
	order* sl_order_aux::anchor(sl_connection* pConnection, EES_QueryOrderExecution* pQueryOrderExec)
	{
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
		//printf("----sl_order_aux::anchor m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d,m_ExecutedQuantity:%d\n", pQueryOrderExec->m_ClientOrderToken, pQueryOrderExec->m_MarketExecID, pQueryOrderExec->m_Userid, pQueryOrderExec->m_ExecutedQuantity);
		//1.
		string symbol;
		int id = -1;
		if (pConnection->get_local_user_id()==pQueryOrderExec->m_Userid)
		{
		if (pConnection->get_symbol(pQueryOrderExec->m_ClientOrderToken,symbol) == false)
		{
			loggerv2::warn("sl_order_aux::anchor didn't find the symbol,m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d\n", pQueryOrderExec->m_ClientOrderToken, pQueryOrderExec->m_MarketExecID, pQueryOrderExec->m_Userid);
			return nullptr;
		}
		}
		else
		{
			if (pConnection->get_external_symbol(pQueryOrderExec->m_MarketOrderToken, symbol) == false)
			{
				loggerv2::warn("sl_order_aux::anchor didn't find the symbol,m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d\n", pQueryOrderExec->m_ClientOrderToken, pQueryOrderExec->m_MarketExecID, pQueryOrderExec->m_Userid);
				return nullptr;
			}
		}
		std::string sInstrCode = symbol + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("sl_order_aux::anchor didn't find the tradeItem,symbol:%s\n", symbol.c_str());
			return nullptr;
		}
		//2.
		int nAccount     = 0;
		int nUserOrdId   = -1;
		int nInternalRe  = -1;
		int nPortfolio   = 0;
		int nTradingType = 0;
		//int id           = -1;
		EES_SideType type =0;
#if 0
		pConnection->get_user_info(pQueryOrderExec->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);			
		id = nInternalRe;
#else
		if (pConnection->get_local_user_id() == pQueryOrderExec->m_Userid)//local order
		{
			pConnection->get_user_info(pQueryOrderExec->m_ClientOrderToken, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			type = pConnection->get_ess_side_type(pQueryOrderExec->m_ClientOrderToken);
			id = pConnection->get_order_id(pQueryOrderExec->m_ClientOrderToken);
		}
		else
		{
			type = pConnection->get_ess_external_side_type(pQueryOrderExec->m_MarketOrderToken);
			id = pConnection->get_external_order_id(pQueryOrderExec->m_MarketOrderToken);
		}
#endif
		if (id < 1)
		{
			loggerv2::error("sl_order_aux::anchor didn't find the orderId,by the m_ClientOrderToken:%d,m_MarketExecID:%s,userId:%d\n", pQueryOrderExec->m_ClientOrderToken, pQueryOrderExec->m_MarketExecID, pQueryOrderExec->m_Userid);
			id = FAKE_ID_MIN + pQueryOrderExec->m_MarketOrderToken;
			//return nullptr;
		}
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);
		//
		o->set_exec_quantity(pQueryOrderExec->m_ExecutedQuantity);
		o->set_exec_price(pQueryOrderExec->m_ExecutionPrice);
		//
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);
		//last time
		tm tmResult;
		unsigned int nanosecond = 0;
		pConnection->m_pUserApi->ConvertFromTimestamp(pQueryOrderExec->m_Timestamp, tmResult, nanosecond);

		time_t tt = mktime(&tmResult);
		std::chrono::seconds sec(tt);
		lwtp tp(sec);

		o->set_last_time(tp);
		//for debug
		//printf("++sl_order_aux::anchor m_ExecutionPrice:%f,TradeTime:%s\n", pQueryOrderExec->m_ExecutionPrice, to_iso_extended_string(TradeTime).c_str());
		//
		auto time =get_lwtp_now();
		o->set_rebuild_time(time);		
		//
		//EES_SideType type = pConnection->get_ess_side_type(pQueryOrderExec->m_ClientOrderToken);
		switch (type)
		{
		case EES_SideType_open_long:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_today_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_close_ovn_short:
			o->set_way(AtsType::OrderWay::Buy);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		case EES_SideType_close_today_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		case EES_SideType_open_short:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case EES_SideType_close_ovn_long:
			o->set_way(AtsType::OrderWay::Sell);
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		default:
			break;
		}
		//
		sl_order_aux::set_client_token(o, pQueryOrderExec->m_ClientOrderToken);
		sl_order_aux::set_market_token(o, std::to_string(pQueryOrderExec->m_MarketOrderToken).c_str());		
		//
		loggerv2::info("sl_order_aux::anchor - successfully rebuild order [%d][%x] when receive EES_QueryOrderExecution msg", id, o);
		return o;
	}
}

