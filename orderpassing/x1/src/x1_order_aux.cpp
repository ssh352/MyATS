#include "x1_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "x1_connection.h"

namespace x1
{
	//x1_order::x1_order(x1_connection* pConnection) : order(pConnection)
	//{
	// m_nSpdId = 0;
	//}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspPriOrderField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		/*
		 TX1FtdcRequestIDType                  RequestID;                    ///< 请求ID
		 TX1FtdcX1OrderIDType                  X1OrderID;                    ///< 柜台委托号
		 TX1FtdcOrderAnswerStatusType          OrderStatus;                  ///< 委托状态
		 TX1FtdcInstrumentIDType               InstrumentID;                 ///< 合约代码
		 TX1FtdcBuySellTypeType                BuySellType;                  ///< 买卖
		 TX1FtdcOpenCloseTypeType              OpenClose;                    ///< 开平标志
		 TX1FtdcPriceType                      InsertPrice;                  ///< 委托价
		 TX1FtdcAmountType                     OrderAmount;                  ///< 委托数量
		 TX1FtdcPriceType                      MatchedPrice;                 ///< 成交价格
		 TX1FtdcAmountType                     MatchedAmount;                ///< 成交数量
		 TX1FtdcAmountType                     CancelAmount;                 ///< 撤单数量
		 TX1FtdcInsertType                     InsertType;                   ///< 自动单类别
		 TX1FtdcSpeculatorType                 Speculator;                   ///< 投保
		 TX1FtdcDateType                       CommTime;                     ///< 委托时间
		 TX1FtdcDateType                       SubmitTime;                   ///< 申报时间
		 TX1FtdcClientIDType                   ClientID;                     ///< 交易编码
		 TX1FtdcExchangeIDType                 ExchangeID;                   ///< 交易所ID
		 TX1FtdcFrontAddrType                  OperStation;                  ///< 委托地址
		 TX1FtdcAccountIDType                  AccountID;                    ///< 客户号
		 TX1FtdcInstrumentTypeType             InstrumentType;               ///< 合约类型
		 TX1FtdcSessionIDType                  SessionId;                    ///< 会话ID(此版本不支持)
		 TX1FtdcReservedType                   ReservedType2;                ///< 预留字段2
		 TX1FtdcOrderSysIDType                 OrderSysID;                   ///< 报单编号
		 TX1FtdcCustomCategoryType             CustomCategory;               ///< 自定义类别
		 TX1FtdcPriceType                      Margin;                       ///< 保证金
		 TX1FtdcPriceType                      Fee;                          ///< 手续费
		 TX1FtdcLocalOrderIDType               LocalOrderID;                 ///< 本地委托号 是下单时该单子的本地委托号，如果同一账号从多个客户端下单，则查询返回的LocalOrderID可能是重复的
		 TX1FtdcPriceType                      ProfitLossPrice;              ///< 止损止盈价
		 TX1FtdcOrderTypeType                  OrderType;                    ///< 报单类别
		 TX1FtdcOrderPropertyType              OrderProperty;                ///< 订单属性
		 */

		//std::string sInstrCode = std::string(pOrder->InstrumentID) + "." + pOrder->ExchangeID + "@" + pConnection->getName();
		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->InstrumentID), std::string(pOrder->ExchangeID));

		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = -1;
		int nTradingType = 0;
		int localID = pOrder->LocalOrderID;

		char *ptr = pOrder->CustomCategory;
		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pOrder->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_localId2Portfolio.find(pOrder->LocalOrderID);
			if (it != pConnection->m_localId2Portfolio.end())
			{
				nAccount = it->second.accId;
				id = it->second.orderId;
				nTradingType = it->second.nTradingType;
				if (nAccount != pConnection->getAccountNum())
				{
					if (id > 0)
						id = id + 1000000 * nAccount;
				}
				o->set_portfolio(it->second.portfolio.c_str());
			}
			else
				id = -localID;
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}
		
		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));

		pConnection->insert_spId2order(pOrder->X1OrderID, o);
		pConnection->insert_localId2order(localID, o);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->OrderAmount);
		o->set_price(pOrder->InsertPrice);

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->OpenCloseType)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;
		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);

		//
		if (pOrder->OrderProperty == X1_FTDC_SP_FOK)
		{
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		}
		else if (pOrder->OrderProperty == X1_FTDC_SP_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//

		switch (pOrder->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

		auto time = get_lwtp_now();
		o->set_last_time(time);

		o->set_rebuild_time(time);

		o->set_status(AtsType::OrderStatus::Ack);

		return o;

	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspPriCancelOrderField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->InstrumentID), std::string(pOrder->ExchangeID));

		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = -1;
		int nTradingType = 0;
		int localID = pOrder->LocalOrderID;

		char *ptr = pOrder->CustomCategory;
		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pOrder->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_x1Id2Portfolio.find(pOrder->X1OrderID);
			if (it != pConnection->m_x1Id2Portfolio.end())
			{
				nAccount = it->second.accId;
				id = it->second.orderId;
				nTradingType = it->second.nTradingType;
				if (nAccount != pConnection->getAccountNum())
				{
					if (id > 0)
						id = id + 1000000 * nAccount;
				}
				o->set_portfolio(it->second.portfolio.c_str());
			}
			else
				id = -localID;
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}




		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));

		pConnection->insert_spId2order(pOrder->X1OrderID, o);
		pConnection->insert_localId2order(localID, o);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->OrderAmount);
		o->set_price(pOrder->InsertPrice);

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->OpenCloseType)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;
		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);


		switch (pOrder->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

		auto time = get_lwtp_now();
		o->set_last_time(time);

		o->set_rebuild_time(time);

		o->set_status(AtsType::OrderStatus::Ack);

		return o;
	
	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspPriMatchInfoField* pTrade)
	{
		order* o = pConnection->get_order_from_pool();

		/*DFITCLocalOrderIDType               localOrderID;                 //本地委托号
		DFITCOrderSysIDType                 OrderSysID;                   //报单编号(交易所报单编号)
		DFITCMatchIDType                    matchID;                      //成交编号
		DFITCInstrumentIDType               instrumentID;                 //合约代码
		DFITCBuySellTypeType                buySellType;                  //买卖
		DFITCOpenCloseTypeType              openCloseType;                //开平标志
		DFITCPriceType                      matchedPrice;                 //成交价格
		DFITCAmountType                     orderAmount;                  //委托数量
		DFITCAmountType                     matchedAmount;                //成交数量
		DFITCDateType                       matchedTime;                  //成交时间
		DFITCPriceType                      insertPrice;                  //报价
		DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
		DFITCMatchType                      matchType;                    //成交类型
		DFITCSpeculatorType                 speculator;                   //投保
		DFITCExchangeIDType                 exchangeID;                   //交易所ID
		DFITCFeeType                        fee;                          //手续费
		DFITCSessionIDType                  sessionID;                    //会话标识
		DFITCInstrumentTypeType             instrumentType;               //合约类型
		DFITCAccountIDType                  accountID;                    //资金账号
		DFITCOrderAnswerStatusType          orderStatus;                  //申报结果
		DFITCPriceType                      margin;                       //开仓为保证金,平仓为解冻保证金
		DFITCPriceType                      frozenCapita;                 //成交解冻委托冻结的资金
		DFITCAdjustmentInfoType             adjustmentInfo;               //组合或对锁的保证金调整信息,格式:[合约代码,买卖标志,投资类别,调整金额;]
		DFITCCustomCategoryType             CustomCategory;               //自定义类别
		DFITCPriceType                      turnover;                     //成交金额
		DFITCOrderTypeType                  orderType;                    //报单类型
		DFITCInsertType                     insertType;                   //自动单类别
		DFITCClientIDType                   clientID;                     //交易编码 */


		//std::string sInstrCode = std::string(pTrade->InstrumentID) + "." + pTrade->ExchangeID + "@" + pConnection->getName();
		std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->InstrumentID), std::string(pTrade->ExchangeID));
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pTrade->LocalOrderID;
		// client field : orderId|tradingType|portfolio
		char *ptr = pTrade->CustomCategory;
		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pTrade->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_x1Id2Portfolio.find(pTrade->X1OrderID);
			if (it != pConnection->m_x1Id2Portfolio.end())
			{
				nAccount = it->second.accId;
				id = it->second.orderId;
				nTradingType = it->second.nTradingType;
				if (nAccount != pConnection->getAccountNum())
				{
					if (id > 0)
						id = id + 1000000 * nAccount;
				}
				o->set_portfolio(it->second.portfolio.c_str());
			}
			else
				id = -localID;
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}
			
		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pTrade->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));

		pConnection->insert_spId2order(pTrade->X1OrderID, o);
		pConnection->insert_localId2order(localID, o);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->OrderAmount);
		o->set_exec_quantity(pTrade->MatchedAmount);
		//o->set_price(pTrade->Price);
		o->set_price(pTrade->InsertPrice);
		o->set_exec_price(pTrade->MatchedPrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pTrade->OpenCloseType)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);


		switch (pTrade->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("x1_order - anchor: unknown order_way[%c]", pTrade->BuySellType);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}

		auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->MatchedTime));
		o->set_last_time(tp);
		
		o->set_last_time(tp);
		o->set_rebuild_time(tp);


		o->set_status(AtsType::OrderStatus::Ack);

		return o;
	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspOrderField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		//DFITCRequestIDType                  lRequestID;                   //请求ID
		//DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
		//DFITCOrderAnswerStatusType          orderStatus;                  //委托状态
		//DFITCInstrumentIDType               instrumentID;                 //合约代码
		//DFITCBuySellTypeType                buySellType;                  //买卖
		//DFITCOpenCloseTypeType              openClose;                    //开平标志
		//DFITCPriceType                      insertPrice;                  //委托价
		//DFITCAmountType                     orderAmount;                  //委托数量
		//DFITCPriceType                      matchedPrice;                 //成交价格
		//DFITCAmountType                     matchedAmount;                //成交数量
		//DFITCAmountType                     cancelAmount;                 //撤单数量
		//DFITCInsertType                     insertType;                   //自动单类别
		//DFITCSpeculatorType                 speculator;                   //投保
		//DFITCDateType                       commTime;                     //委托时间
		//DFITCDateType                       submitTime;                   //申报时间
		//DFITCClientIDType                   clientID;                     //交易编码
		//DFITCExchangeIDType                 exchangeID;                   //交易所ID
		//DFITCFrontAddrType                  operStation;                  //委托地址
		//DFITCAccountIDType                  accountID;                    //客户号
		//DFITCInstrumentTypeType             instrumentType;               //合约类型
		//DFITCSessionIDType                  sessionId;                    //会话ID
		//DFITCReservedType                   reservedType2;                //预留字段2
		//DFITCOrderSysIDType                 OrderSysID;                   //报单编号
		//DFITCCustomCategoryType             CustomCategory;               //自定义类别
		//DFITCPriceType                      margin;                       //保证金
		//DFITCPriceType                      fee;                          //手续费
		//DFITCLocalOrderIDType               localOrderID;                 //本地委托号
		//DFITCPriceType                      profitLossPrice;              //止损止盈价
		//DFITCOrderTypeType                  orderType;                    //报单类别
		//DFITCOrderPropertyType              orderProperty;                //订单属性

		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->InstrumentID), std::string(pOrder->ExchangeID));
		//std::string sInstrCode = std::string(pOrder->InstrumentID) + "." + pOrder->ExchangeID + "@" + pConnection->getName();

		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pOrder->LocalOrderID;
		//重连时交易所发送的ack报文中devDecInfo为空
		//int id = pOrder->localOrderID;
		char *ptr = pOrder->CustomCategory;

		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pOrder->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_x1Id2Portfolio.find(pOrder->X1OrderID);
			if (it != pConnection->m_x1Id2Portfolio.end())
			{
				nAccount = it->second.accId;
				id = it->second.orderId;
				nTradingType = it->second.nTradingType;
				if (nAccount != pConnection->getAccountNum())
				{
					if (id > 0)
						id = id + 1000000 * nAccount;
				}
				o->set_portfolio(it->second.portfolio.c_str());
			}
			else
				id = -localID;
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}

		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));
		pConnection->insert_spId2order(pOrder->X1OrderID, o);
		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->OrderAmount);
		o->set_price(pOrder->InsertPrice);


		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->OpenClose)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;
		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);

		//
		if (pOrder->OrderProperty == X1_FTDC_SP_FOK)
		{
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		}
		else if (pOrder->OrderProperty == X1_FTDC_SP_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//

		switch (pOrder->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

		auto time = get_lwtp_now();
		o->set_last_time(time);

		o->set_rebuild_time(time);

		o->set_status(AtsType::OrderStatus::Ack);

		return o;
	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspMatchField* pTrade)
	{
		order* o = pConnection->get_order_from_pool();

		//DFITCRequestIDType                  lRequestID;                   //请求ID
		//DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
		//DFITCAccountIDType                  accountID;                    //资金账号
		//DFITCExchangeIDType                 exchangeID;                   //交易所ID
		//DFITCInstrumentIDType               instrumentID;                 //合约代码
		//DFITCBuySellTypeType                buySellType;                  //买卖
		//DFITCOpenCloseTypeType              openClose;                    //开平
		//DFITCPriceType                      matchedPrice;                 //成交价格
		//DFITCAmountType                     matchedAmount;                //成交数量
		//DFITCPriceType                      matchedMort;                  //成交金额
		//DFITCSpeculatorType                 speculator;                   //投保类别
		//DFITCDateType                       matchedTime;                  //成交时间
		//DFITCMatchIDType                    matchedID;                    //成交编号
		//DFITCLocalOrderIDType               localOrderID;                 //本地委托号
		//DFITCClientIDType                   clientID;                     //交易编码
		//DFITCMatchType                      matchType;                    //成交类型
		//DFITCInstrumentTypeType             instrumentType;               //合约类型
		//DFITCSessionIDType                  sessionId;                    //会话ID
		//DFITCReservedType                   reservedType2;                //预留字段2
		//DFITCCustomCategoryType             CustomCategory;               //自定义类别
		//DFITCPriceType                      fee;                          //手续费
		//DFITCOrderTypeType                  orderType;                    //报单类型
		//DFITCOrderSysIDType                 OrderSysID;                   //报单编号

		std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->InstrumentID), std::string(pTrade->ExchangeID));
		//std::string sInstrCode = std::string(pTrade->InstrumentID) + "." + pTrade->ExchangeID + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pTrade->LocalOrderID;
		// client field : orderId|tradingType|portfolio
		char *ptr = pTrade->CustomCategory;
		int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ' || ptr[14] == ' ')
		{
			//loggerv2::error("x1_order::anchor - cannot parse UserID field[%-*.*s]. Trade may from another application", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			return NULL;
			//id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pTrade->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}


		// ok
		//x1_order* o = new x1_order(pConnection);
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->MatchedAmount);

		o->set_exec_quantity(pTrade->MatchedAmount);
		//o->set_price(pTrade->Price);
		//o->set_price(pTrade->InsertPrice);
		o->set_exec_price(pTrade->MatchedPrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pTrade->OpenClose)
		{
		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);


		switch (pTrade->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("x1_order - anchor: unknown order_way[%c]", pTrade->BuySellType);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}


		if (ptr[1] == ' ' || ptr[14] == ' ')
		{
			o->set_unknown_order();

		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		x1_order_aux::set_spdId(o, pTrade->X1OrderID);

		auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->MatchedTime));
		o->set_last_time(tp);

		o->set_user_orderid(nUserOrdId);

		auto now = get_lwtp_now();
		o->set_rebuild_time(now);

		/* std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
		 o->set_rebuild_time_point(tpnow);*/
		return o;
	}

	quote*  x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcQuoteRtnField* pQuote)
	{
		return nullptr;
	}

	quote*  x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcQuoteMatchRtnField* pQuote)
	{
		return nullptr;
	}

	quote*  x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcQuoteCanceledRtnField* pQuote)
	{
		return nullptr;

	}

}

