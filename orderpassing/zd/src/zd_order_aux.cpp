#include "zd_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "zd_connection.h"
#include "boost/algorithm/string.hpp"
namespace zd
{
	order* zd_order_aux::anchor(zd_connection* pConnection, CTShZdInputOrderField* pOrder)
	{
		//order_action action = AtsType::OrderAction::Created;

		// tradeitem
		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("zd_order_aux::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return NULL;
		}
		
		// client field : orderId|tradingType|portfolio
		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		       
		int id = -1;		
		order* o = pConnection->get_order_from_pool();

		// way
		if (pOrder->Direction == TSHZD_D_Buy)
			o->set_way(AtsType::OrderWay::Buy);
		else if (pOrder->Direction == TSHZD_D_Sell)
			o->set_way(AtsType::OrderWay::Sell);
		else
		{
			loggerv2::error("zd_order_aux- anchor: unknown order_way[%c]", pOrder->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}
		
		pConnection->get_user_info(pConnection->get_user_id_ex(pOrder->OrderLocalID).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = (o->get_way() == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;

		if (id < 1)
		{			
			id = FAKE_ID_MIN + atoi(pOrder->OrderLocalID);
		}
						
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->VolumeTotalOriginal);
		o->set_price(pOrder->LimitPrice);
			

		if (pOrder->CombOffsetFlag[0] == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pOrder->CombOffsetFlag[0] == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);
		
		//
		if (pOrder->TimeCondition == TSHZD_TC_IOC)
		{
			if (pOrder->VolumeCondition == TSHZD_VC_CV)
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

		o->set_user_orderid(nUserOrdId);

		zd_order_aux::set_order_sys_id(o, pOrder->OrderSysID);

		//set time, insert current time in long

		auto time = get_lwtp_now();
		o->set_last_time(time);

		loggerv2::info("zd_order_aux::anchor - successfully rebuild order [%d][%x],when CTShZdInputOrderField", id, o);
		return o;
	}

	order* zd_order_aux::anchor(zd_connection* pConnection, CTShZdOrderField* pOrder)
	{
		// tradeitem

		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("zd_order_aux::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return NULL;
		}
		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;	
	
		int id = -1;		
		order* o = pConnection->get_order_from_pool();

		// way
		if (pOrder->Direction == TSHZD_D_Buy)
			o->set_way(AtsType::OrderWay::Buy);
		else if (pOrder->Direction == TSHZD_D_Sell)
			o->set_way(AtsType::OrderWay::Sell);
		else
		{
			loggerv2::error("order - anchor: unknown order_way[%c]", pOrder->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}

		pConnection->get_user_info(pConnection->get_user_id_ex(pOrder->OrderLocalID).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = (o->get_way() == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;

		if (id<1)
		{			
			id = FAKE_ID_MIN + atoi(pOrder->OrderLocalID);
		}
				
		o->set_id(id);
		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->VolumeTotalOriginal);
		o->set_price(pOrder->LimitPrice);
				
		if (pOrder->CombOffsetFlag[0] == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pOrder->CombOffsetFlag[0] == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);


		//
#if 0
		if (pOrder->TimeCondition == TSHZD_TC_IOC)
		{
			if (pOrder->VolumeCondition == TSHZD_VC_CV)
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
#endif
		//
		
		
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		

		// huatai id
		zd_order_aux::set_ord_ref(o,atoi(pOrder->OrderRef));
		zd_order_aux::set_order_sys_id(o,pOrder->OrderSysID);
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
#if 0
		switch (pOrder->OrderSubmitStatus)
		{
		case TSHZD_OSS_InsertSubmitted:
			o->set_last_action(AtsType::OrderAction::Created);
			break;
		case TSHZD_OSS_CancelAll:
		case TSHZD_OSS_CancelSub:
			o->set_last_action(AtsType::OrderAction::Cancelled);
			break;					
		case TSHZD_OSS_Accepted:
			break;
		default:
			break;
		}

		o->set_status(AtsType::OrderStatus::WaitServer);
#endif

		/*
		InsertDate:2017-06-2910:15:13,InsertTime:10:15:13
		std::string ts("2002-01-20 23:59:59.000");
		ptime t(time_from_string(ts))
		*/
		string ts = strlen(pOrder->InsertDate) != 0 ? pOrder->InsertDate : pOrder->CancelDate;
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
		o->set_last_time(tp);

		o->set_user_orderid(nUserOrdId);

		loggerv2::info("order::anchor - successfully rebuild order [%d][%x],when CTShZdOrderField", id, o);
		return o;
	}

	order* zd_order_aux::anchor(zd_connection* pConnection, CTShZdTradeField* pTrade)
	{
		std::string sInstrCode = std::string(pTrade->InstrumentID) + "@" + pConnection->getName();
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;		
		int id = -1;
		
		// ok
		order* o = pConnection->get_order_from_pool();
		if (pTrade->Direction == TSHZD_D_Buy)
			o->set_way(AtsType::OrderWay::Buy);
		else if (pTrade->Direction == TSHZD_D_Sell)
			o->set_way(AtsType::OrderWay::Sell);
		else
		{
			loggerv2::error("order - anchor: unknown order_way[%c]", pTrade->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}

		pConnection->get_user_info(pConnection->get_user_id_ex(pTrade->OrderLocalID).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = (o->get_way() == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;

		if (id < 1)
		{
			id = FAKE_ID_MIN + atoi(pTrade->OrderLocalID);
		}		
		
		o->set_id(id);
		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		
		o->set_quantity(pTrade->Volume);
		o->set_price(pTrade->Price);
		
	
		if (pTrade->OffsetFlag == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pTrade->OffsetFlag == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);
		
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);

		zd_order_aux::set_ord_ref(o,atoi(pTrade->OrderRef));
		zd_order_aux::set_order_sys_id(o, pTrade->OrderSysID);
				

		/*
		InsertDate:2017-06-2910:15:13,InsertTime:10:15:13
		std::string ts("2002-01-20 23:59:59.000");
		ptime t(time_from_string(ts))
		*/
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
		o->set_last_time(tp);

		o->set_user_orderid(nUserOrdId);

		loggerv2::info("order::anchor - successfully rebuild order [%d][%x],when CTShZdTradeField", id, o);
		return o;
	}
#if 0
	order* zd_order_aux::anchor(zd_connection* pConnection, CThostFtdcExecOrderField* pOrder)
	{
		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("cffex_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;




		char *ptr = pOrder->UserID;
		int id = -1;

		if (ptr[1] == ' ')
		{
			loggerv2::error("cffex_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return NULL;
			id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->ExecOrderRef);
		}
		else
		{
			pConnection->get_user_info(pOrder->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}



		// ok
		order* o = pConnection->get_order_from_pool();

		o->set_id(id);
		o->set_instrument(instr);
		//o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->Volume);
		o->set_price(0);


		// way
		o->set_way(AtsType::OrderWay::Exercise);

		if (pOrder->OffsetFlag == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pOrder->OffsetFlag == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);




		if (ptr[1] == ' ')
		{
			o->set_unknown_order();
		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		// huatai id
		zd_order_aux::set_ord_ref(o, atoi(pOrder->ExecOrderRef));


		// lastAction
		switch (pOrder->OrderSubmitStatus)
		{
		case THOST_FTDC_OSS_InsertSubmitted:
		case THOST_FTDC_OSS_InsertRejected:
			o->set_last_action(AtsType::OrderAction::Created);
			break;

		case THOST_FTDC_OSS_CancelSubmitted:
		case THOST_FTDC_OSS_CancelRejected:
			o->set_last_action(AtsType::OrderAction::Cancelled);
			break;

		case THOST_FTDC_OSS_ModifySubmitted:
		case THOST_FTDC_OSS_ModifyRejected:
			o->set_last_action(AtsType::OrderAction::Modified);
			break;

		case THOST_FTDC_OSS_Accepted:
			break;

		default:
			break;
		}

		o->set_status(AtsType::OrderStatus::WaitServer);

		//ptime ordTime(from_undelimited_string(pOrder->InsertDate), duration_from_string(pOrder->InsertTime));
		auto tp = string_to_lwtp(from_undelimited_string(pOrder->InsertDate), pOrder->InsertTime);

		o->set_last_time(tp);

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//o->set_account(o->get_portfolio());
		// save original quantity / price
		//o->save_previous_values();


		loggerv2::info("order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}

	order* zd_order_aux::anchor(zd_connection* pConnection, CThostFtdcInputExecOrderField* pOrder)
	{
		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("cffex_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;




		char *ptr = pOrder->UserID;
		int id = -1;

		if (ptr[1] == ' ')
		{
			loggerv2::error("cffex_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return NULL;
			id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->ExecOrderRef);
		}
		else
		{
			pConnection->get_user_info(pOrder->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}



		// ok
		order* o = pConnection->get_order_from_pool();

		o->set_id(id);
		o->set_instrument(instr);
		//o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->Volume);
		o->set_price(0);


		// way
		o->set_way(AtsType::OrderWay::Exercise);

		if (pOrder->OffsetFlag == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pOrder->OffsetFlag == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);




		if (ptr[1] == ' ')
		{
			o->set_unknown_order();
		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		// huatai id
		zd_order_aux::set_ord_ref(o, atoi(pOrder->ExecOrderRef));


		// lastAction
		
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);



		auto time = get_lwtp_now();
		o->set_last_time(time);

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//o->set_account(o->get_portfolio());
		// save original quantity / price
		//o->save_previous_values();


		loggerv2::info("order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}

	quote* zd_order_aux::anchor(zd_connection* pConnection, CThostFtdcInputQuoteField* pQuote)
	{
		std::string sInstrCode = std::string(pQuote->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("cffex_order::anchor - tradeitem [%-*.*s] not found", sizeof(pQuote->InstrumentID), sizeof(pQuote->InstrumentID), pQuote->InstrumentID);
			return NULL;
		}


		// client field : orderId|tradingType|portfolio

		int nAccount = 0;
		int bidId = 0;
		int askId = 0;
		int nPortfolio = 0;
		int nTradingType = 0;

		;
		char *ptr = pQuote->UserID;
		//int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			loggerv2::error("cffex_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pQuote->UserID), sizeof(pQuote->UserID), pQuote->UserID);
			//return NULL;
			bidId = atoi(pQuote->BrokerID) * 100000 + atoi(pQuote->QuoteRef);
		}

		else
		{
			pConnection->get_user_info(pQuote->UserID, nAccount, bidId, askId, nPortfolio, nTradingType);
			//id = nInternalRe;
		}

		quote* q = pConnection->get_quote_from_pool();
		//q = pConnection->create_quote(instr, pQuote->BidVolume, pQuote->BidPrice, pQuote->AskVolume, pQuote->AskPrice, pQuote->ForQuoteSysID);
		q->set_id(bidId);
		int asc = (int)(pQuote->ForQuoteSysID[0]);
		if (asc>31)
			q->set_FQR_ID(pQuote->ForQuoteSysID);
		q->set_instrument(instr);
		int ret;
		order* bid_order = pConnection->get_order_from_map(bidId, ret);
		if (ret == 2)
		{
			bid_order = pConnection->create_order(instr, OrderWay::Buy, pQuote->BidVolume, pQuote->BidPrice);
			auto time = get_lwtp_now();
			bid_order->set_last_time(time);
			//to do ...
			bid_order->set_id(bidId);
			pConnection->add_pending_order(bid_order);
		}

		order* ask_order = pConnection->get_order_from_map(askId, ret);
		if (ret == 2)
		{
			ask_order = pConnection->create_order(instr, OrderWay::Sell, pQuote->AskVolume, pQuote->AskPrice);
			auto time = get_lwtp_now();
			ask_order->set_last_time(time);
			//to do ...
			ask_order->set_id(askId);
			pConnection->add_pending_order(ask_order);
		}
		q->set_bid_order(bid_order);
		q->set_ask_order(ask_order);

		q->get_bid_order()->set_id(bidId);
		q->get_ask_order()->set_id(askId);

		q->set_last_action(AtsType::OrderAction::Created);
		q->set_status(AtsType::OrderStatus::WaitMarket);
		



		if (pQuote->AskOffsetFlag == '0')
			q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pQuote->AskOffsetFlag == '3')
			q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::Close);


		if (pQuote->BidOffsetFlag == '0')
			q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pQuote->BidOffsetFlag == '3')
			q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::Close);



		if (ptr[1] == ' ')
		{
			q->set_unknown_quote();

		}
		else
		{
			q->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			q->set_trading_type(nTradingType);
		}

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
//		o->set_user_orderid(nUserOrdId);

		zd_order_aux::set_ord_ref(q->get_bid_order(), atoi(pQuote->BidOrderRef));
		zd_order_aux::set_ord_ref(q->get_ask_order(), atoi(pQuote->AskOrderRef));


		q->custome_ints[0] = atoi(pQuote->QuoteRef);


		//set time, insert current time in long

		auto time = get_lwtp_now();
		q->set_last_time(time);

		////o->save_previous_values();


		loggerv2::info("cffex_order::anchor - successfully rebuild quote [%d][%x]", bidId, q);
		return q;
	}

	quote* zd_order_aux::anchor(zd_connection* pConnection, CThostFtdcQuoteField* pQuote)
	{
		std::string sInstrCode = std::string(pQuote->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("cffex_order::anchor - tradeitem [%-*.*s] not found", sizeof(pQuote->InstrumentID), sizeof(pQuote->InstrumentID), pQuote->InstrumentID);
			return NULL;
		}


		// client field : orderId|tradingType|portfolio

		int nAccount = 0;
		int bidId = 0;
		int askId = 0;
		int nPortfolio = 0;
		int nTradingType = 0;

		;
		char *ptr = pQuote->UserID;
		//int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			loggerv2::error("cffex_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pQuote->UserID), sizeof(pQuote->UserID), pQuote->UserID);
			//return NULL;
			bidId = atoi(pQuote->BrokerID) * 100000 + atoi(pQuote->QuoteRef);
		}

		else
		{
			pConnection->get_user_info(pQuote->UserID, nAccount, bidId, askId, nPortfolio, nTradingType);
			//id = nInternalRe;
		}



		quote* q = pConnection->get_quote_from_pool();
		//q = pConnection->create_quote(instr, pQuote->BidVolume, pQuote->BidPrice, pQuote->AskVolume, pQuote->AskPrice, pQuote->ForQuoteSysID);
		q->set_id(bidId);
		if ((int)(pQuote->ForQuoteSysID[0])>31)
			q->set_FQR_ID(pQuote->ForQuoteSysID);
		q->set_instrument(instr);
		int ret;
		order* bid_order = pConnection->get_order_from_map(bidId, ret);
		if (ret == 2)
		{
			bid_order = pConnection->create_order(instr, OrderWay::Buy, pQuote->BidVolume, pQuote->BidPrice);

			//ptime ordTime(from_undelimited_string(pQuote->InsertDate), duration_from_string(pQuote->InsertTime));
			auto tp = string_to_lwtp(from_undelimited_string(pQuote->InsertDate), pQuote->InsertTime);

			bid_order->set_last_time(tp);
			//to do ...
			bid_order->set_id(bidId);
			pConnection->add_pending_order(bid_order);
		}

		order* ask_order = pConnection->get_order_from_map(askId, ret);
		if (ret == 2)
		{
			ask_order = pConnection->create_order(instr, OrderWay::Sell, pQuote->AskVolume, pQuote->AskPrice);

			//ptime ordTime(from_undelimited_string(pQuote->InsertDate), duration_from_string(pQuote->InsertTime));
			auto tp = string_to_lwtp(from_undelimited_string(pQuote->InsertDate), pQuote->InsertTime);

			ask_order->set_last_time(tp);
			//to do ...
			ask_order->set_id(askId);
			pConnection->add_pending_order(ask_order);
		}
		q->set_bid_order(bid_order);
		q->set_ask_order(ask_order);

		q->get_bid_order()->set_id(bidId);
		q->get_ask_order()->set_id(askId);



		q->set_last_action(AtsType::OrderAction::Created);
		q->set_status(AtsType::OrderStatus::WaitMarket);




		if (pQuote->AskOffsetFlag == '0')
			q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pQuote->AskOffsetFlag == '3')
			q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::Close);


		if (pQuote->BidOffsetFlag == '0')
			q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pQuote->BidOffsetFlag == '3')
			q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::Close);



		if (ptr[1] == ' ')
		{
			q->set_unknown_quote();

		}
		else
		{
			q->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			q->set_trading_type(nTradingType);
		}

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		//		o->set_user_orderid(nUserOrdId);

		zd_order_aux::set_ord_ref(q->get_bid_order(), atoi(pQuote->BidOrderRef));
		zd_order_aux::set_ord_ref(q->get_ask_order(), atoi(pQuote->AskOrderRef));

		zd_order_aux::set_order_sys_id(q->get_bid_order(), pQuote->BidOrderSysID);
		zd_order_aux::set_order_sys_id(q->get_ask_order(), pQuote->AskOrderSysID);

		q->custome_ints[0] = atoi(pQuote->QuoteRef);


		//set time, insert current time in long
#if 0
		auto time = microsec_clock::local_time();
		q->set_last_time(time);
#else
		string times =(pQuote->InsertTime);
		boost::trim(times);
		if (times != "")
		{

			//ptime ordTime(from_undelimited_string(pQuote->InsertDate), duration_from_string(pQuote->InsertTime));
			auto tp = string_to_lwtp(from_undelimited_string(pQuote->InsertDate), pQuote->InsertTime);

			q->set_last_time(tp);
		}
		else
		{
			auto time = get_lwtp_now();
			q->set_last_time(time);
		}
		
#endif

		////o->save_previous_values();


		loggerv2::info("cffex_order::anchor - successfully rebuild quote [%d][%x]", bidId, q);
		return q;

	}
#endif
}

