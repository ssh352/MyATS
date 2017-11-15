#include "sl_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "es_connection.h"
namespace es
{	
	order* es_order_aux::anchor(es_connection* pConnection, TapAPIOrderInfo * pField)
	{
		if (pConnection == nullptr || pField == nullptr)
			return nullptr;		
		//1.
		string symbol;
		switch (pField->CommodityType)
		{
		case TAPI_COMMODITY_TYPE_FUTURES:
			symbol = string(pField->CommodityNo) + string(pField->ContractNo);
			break;
		case TAPI_COMMODITY_TYPE_OPTION:
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%s%s%c%s", pField->CommodityNo, pField->ContractNo, pField->CallOrPutFlag, pField->StrikePrice);
			symbol = buffer;
			break;
		default:
			break;
		}

		std::string sInstrCode = symbol + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("es_order::anchor - tradeitem;%s not found",symbol.c_str());
			return nullptr;
		}
		//2.
		int nAccount    = 0;
		int nUserOrdId  = 0;
		int nInternalRe = 0;
		int nPortfolio  = 0;
		int nTradingType= 0;
		int id = 0;		
		//to do ......find the user info
		pConnection->get_user_info(pField->RefString, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);		
		OrderWay::type way = pField->OrderSide == TAPI_SIDE_BUY ? OrderWay::Buy : OrderWay::Sell;
		id = (way == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;		
		if (id == 0)
		{
			loggerv2::error("es_order_aux::anchor didn't find the orderId,by the OrderNo:%s,%s\n", pField->OrderNo, symbol.c_str());
			id = FAKE_ID_MIN + atoi(pField->OrderLocalNo);
			//return nullptr;
		}
		//3.
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);
		//
		o->set_quantity(pField->OrderQty);
		o->set_price(pField->OrderPrice);		
		switch (pField->OrderSide)
		{
		case TAPI_SIDE_BUY:
			o->set_way(AtsType::OrderWay::Buy);			
			break;
		case TAPI_SIDE_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;		
		default:
			break;
		}
		//to do ...
		if (pField->OrderType == TAPI_ORDER_TYPE_OPT_EXEC)
		{
			o->set_way(AtsType::OrderWay::Exercise);
		}
		switch (pField->PositionEffect)
		{
		case TAPI_PositionEffect_OPEN:
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case TAPI_PositionEffect_COVER:
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		case TAPI_PositionEffect_COVER_TODAY:
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		default:
			break;
		}
		//
		//
		if (pField->TimeInForce == TAPI_ORDER_TIMEINFORCE_FOK)
		{	
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		}
		if (pField->TimeInForce == TAPI_ORDER_TIMEINFORCE_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//
		//
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);
		//last time							
		//printf_ex("es_order_aux::anchor pField->OrderUpdateTime:%s\n", pField->OrderUpdateTime);

		auto utime = pConnection->get_time(string(pField->OrderUpdateTime));
		//auto tp = string_to_lwtp(day_clock::local_day(), pField->OrderUpdateTime);
		auto tp = ptime_to_lwtp(utime);

		o->set_last_time(tp);
		//for debug		
		//printf_ex("es_order_aux::anchor %s\n", to_simple_string(o->get_last_time().time_of_day()).c_str());		
		//
		o->save_previous_values();
		//rebuild time
		auto time = get_lwtp_now();
		o->set_rebuild_time(time);
		
		loggerv2::info("es_order_aux::anchor - successfully rebuild order [%d][%x] when receive TapAPIOrderInfo msg,state:%c,%s", id, o, pField->OrderState,symbol.c_str());
		//
		if (pField->OrderState == TAPI_ORDER_STATE_CANCELED || pField->OrderState == TAPI_ORDER_STATE_FAIL)
		{
			o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			pConnection->update_instr_on_ack_from_market_cb(o);
			pConnection->on_ack_from_market_cb(o);
		}
		//
		return o;
	}
	quote* es_order_aux::anchor_quote(es_connection* pConnection, TapAPIOrderInfo * pField)
	{
		//1.
		string symbol;
		switch (pField->CommodityType)
		{
		case TAPI_COMMODITY_TYPE_FUTURES:
			symbol = string(pField->CommodityNo) + string(pField->ContractNo);
			break;
		case TAPI_COMMODITY_TYPE_OPTION:
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%s%s%c%s", pField->CommodityNo, pField->ContractNo, pField->CallOrPutFlag, pField->StrikePrice);
			symbol = buffer;
			break;
		default:
			break;
		}
		std::string sInstrCode = symbol + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("es_order::anchor_quote - tradeitem;%s not found", symbol.c_str());
			return nullptr;
		}		
		//2.
		int nAccount     = 0;		
		int bidId        = 0;
		int askId        = 0;
		int nPortfolio   = 0;
		int nTradingType = 0;
		int id = 0;
		//to do ......find the user info
		pConnection->get_user_info(pField->RefString, nAccount, bidId, askId, nPortfolio, nTradingType);
		OrderWay::type way = (pField->OrderSide == TAPI_SIDE_BUY || pField->OrderSide == TAPI_SIDE_ALL) ? OrderWay::Buy : OrderWay::Sell;
		id = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;
		if (id == 0)
		{
			loggerv2::error("es_order_aux::anchor didn't find the orderId,by the OrderNo:%s\n", pField->OrderNo);
			id = FAKE_ID_MIN + atoi(pField->OrderLocalNo);
			//return nullptr;
		}		
		//3.
		quote* q = pConnection->get_quote_from_pool();
		es_order_aux::set_order_no(q, pField->OrderNo);
		q->set_id(bidId);


		if (strlen(pField->InquiryNo)>0)
			q->set_FQR_ID(pField->InquiryNo);

		q->set_instrument(instr);
		int ret;

		auto utime = pConnection->get_time(string(pField->OrderUpdateTime));
		//auto tp = string_to_lwtp(day_clock::local_day(), pField->OrderUpdateTime);
		lwtp tp = ptime_to_lwtp(utime);

		order* bid_order = pConnection->get_order_from_map(bidId, ret);
		if (ret == 2)
		{
			bid_order = pConnection->create_order(instr, OrderWay::Buy, pField->OrderQty, pField->OrderPrice);			
			bid_order->set_last_time(tp);
			//to do ...
			es_order_aux::set_order_no(bid_order, pField->OrderNo);
			bid_order->set_id(bidId);
			pConnection->add_pending_order(bid_order);
		}
		order* ask_order = pConnection->get_order_from_map(askId, ret);
		if (ret == 2)
		{
			ask_order = pConnection->create_order(instr, OrderWay::Sell, pField->OrderQty2, pField->OrderPrice2);		
			ask_order->set_last_time(tp);
			//to do ...
			es_order_aux::set_order_no(ask_order, pField->OrderNo);
			ask_order->set_id(askId);
			pConnection->add_pending_order(ask_order);
		}
		q->set_bid_order(bid_order);
		q->set_ask_order(ask_order);
		q->get_bid_order()->set_id(bidId);
		q->get_ask_order()->set_id(askId);		
		q->set_last_action(AtsType::OrderAction::Created);
		q->set_status(AtsType::OrderStatus::WaitMarket);

		q->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		q->set_trading_type(nTradingType);		

		q->set_last_time(tp);
		loggerv2::info("cffex_order::anchor - successfully rebuild quote [%d][%x]", bidId, q);

		return q;
	}
	order* es_order_aux::anchor(es_connection* pConnection, TapAPIFillInfo  * pTrade)
	{
		if (pConnection == nullptr || pTrade == nullptr)
			return nullptr;
		//1.
		string symbol;

		switch (pTrade->CommodityType)
		{
		case TAPI_COMMODITY_TYPE_FUTURES:
			symbol = string(pTrade->CommodityNo) + string(pTrade->ContractNo);
			break;
		case TAPI_COMMODITY_TYPE_OPTION:
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%s%s%c%s", pTrade->CommodityNo, pTrade->ContractNo, pTrade->CallOrPutFlag, pTrade->StrikePrice);
			symbol = buffer;
			break;
		default:
			break;
		}

		std::string sInstrCode = symbol + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("es_order::anchor - tradeitem;%s not found", symbol.c_str());
			return nullptr;
		}
		//2.
		int nAccount     = 0;
		int nUserOrdId   = 0;
		int nInternalRe  = 0;
		int nPortfolio   = 0;
		int nTradingType = 0;
		int id = 0;
		pConnection->get_user_info_ex(pTrade->OrderNo, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);		
		OrderWay::type way = pTrade->MatchSide == TAPI_SIDE_BUY ? OrderWay::Buy : OrderWay::Sell;
		id = (way == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
		if (id == 0)
		{
			loggerv2::error("es_order_aux::anchor didn't find the orderId,by the OrderNo:%s\n", pTrade->OrderNo);
			id = FAKE_ID_MIN + atoi(pTrade->OrderLocalNo);
			//return nullptr;
		}		
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);
		//
		o->set_quantity(pTrade->MatchQty);
		o->set_price(0);
		//
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//auto mtime = time_from_string(pTrade->MatchDateTime);
		auto utime = pConnection->get_time(string(pTrade->MatchDateTime));
		//auto tp = string_to_lwtp(day_clock::local_day(), pTrade->MatchDateTime);
		auto tp = ptime_to_lwtp(utime);

		o->set_last_time(tp);
		//for debug
		//printf("++sl_order_aux::anchor m_ExecutionPrice:%f,TradeTime:%s\n", pQueryOrderExec->m_ExecutionPrice, to_iso_extended_string(TradeTime).c_str());
		//
		auto time = get_lwtp_now();;
		o->set_rebuild_time(time);		
		//
		switch (pTrade->MatchSide)
		{
		case TAPI_SIDE_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case TAPI_SIDE_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		default:
			break;
		}
		//to do ...
		if (pTrade->OrderType == TAPI_ORDER_TYPE_OPT_EXEC)
		{
			o->set_way(AtsType::OrderWay::Exercise);
		}
		switch (pTrade->PositionEffect)
		{
		case TAPI_PositionEffect_OPEN:
			o->set_open_close(AtsType::OrderOpenClose::Open);
			break;
		case TAPI_PositionEffect_COVER:
			o->set_open_close(AtsType::OrderOpenClose::Close);
			break;
		case TAPI_PositionEffect_COVER_TODAY:
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
			break;
		default:
			break;
		}
		//
		if (pTrade->TimeInForce == TAPI_ORDER_TIMEINFORCE_FOK)
		{
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		}
		if (pTrade->TimeInForce == TAPI_ORDER_TIMEINFORCE_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//
		loggerv2::info("es_order_aux::anchor - successfully rebuild order [%d][%x] when TapAPIFillInfo", id, o);
		return o;
	}
}

