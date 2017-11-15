#include "fs_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "fs_connection.h"
using namespace fstech;
namespace fs
{
	//fs_order::fs_order(fs_connection* pConnection) : order(pConnection)
	//{
	//	m_ordRef = 0;
	//	//m_exchangeId = "";
	//}

	order* fs_order_aux::anchor(fs_connection* pConnection, CThostFtdcInputOrderField* pOrder)
	{
		//order_action action = AtsType::OrderAction::Created;

		// tradeitem
		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("fs_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return NULL;
		}


		// client field : orderId|tradingType|portfolio

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;

		//pConnection->get_user_info(pOrder->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);


		//string_tokenizer<1024> tokenizer;
		//tokenizer.break_line(pOrder->UserID, '-');
		char *ptr = pOrder->UserID;
		int id = -1;
#if 0
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			loggerv2::error("fs_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return NULL;
			id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
		}

		else
		{
			pConnection->get_user_info(pOrder->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}
#else
		pConnection->get_user_info(pOrder->OrderRef, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
#endif


		// ok
		order* o = pConnection->get_order_from_pool();
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->VolumeTotalOriginal);
		o->set_price(pOrder->LimitPrice);


		// way
		if (pOrder->Direction == THOST_FTDC_D_Buy)
			o->set_way(AtsType::OrderWay::Buy);
		else if (pOrder->Direction == THOST_FTDC_D_Sell)
			o->set_way(AtsType::OrderWay::Sell);
		else
		{
			loggerv2::error("fs_order - anchor: unknown order_way[%c]", pOrder->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}


		if (pOrder->CombOffsetFlag[0] == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pOrder->CombOffsetFlag[0] == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);



		//if (ptr[1] == ' ')
		//{
		//	o->set_unknown_order();
		//}
		//else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);
		// huatai id
		fs_order_aux::set_ord_ref(o,atoi(pOrder->OrderRef));

		//set time, insert current time in long
		//o->set_last_time();

		//date_time ordTime = time(NULL);
		auto time = get_lwtp_now();
		o->set_last_time(time);
		//o->set_account(o->get_portfolio());
		// save original quantity / price
		//o->save_previous_values();


		loggerv2::info("fs_order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}

	order* fs_order_aux::anchor(fs_connection* pConnection, CThostFtdcOrderField* pOrder)
	{
		// tradeitem

		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("fs_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;



		// client field : orderId|tradingType|portfolio
		//string_tokenizer<1024> tokenizer;
		//tokenizer.break_line(pOrder->UserID, '-');
		char *ptr = pOrder->UserID;
		int id = -1;
#if 0
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			loggerv2::error("fs_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return NULL;
			id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pOrder->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}
#else
		pConnection->get_user_info(pOrder->OrderRef, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
#endif



		// ok
		order* o = pConnection->get_order_from_pool();

		o->set_id(id);
		o->set_instrument(instr);
		//o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->VolumeTotalOriginal);
		o->set_price(pOrder->LimitPrice);


		// way
		if (pOrder->Direction == THOST_FTDC_D_Buy)
			o->set_way(AtsType::OrderWay::Buy);
		else if (pOrder->Direction == THOST_FTDC_D_Sell)
			o->set_way(AtsType::OrderWay::Sell);
		else
		{
			loggerv2::error("fs_order - anchor: unknown order_way[%c]", pOrder->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}

		if (pOrder->CombOffsetFlag[0] == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pOrder->CombOffsetFlag[0] == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);




		//if (ptr[1] == ' ')
		//{
		//	o->set_unknown_order();
		//}
		//else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		// huatai id
		fs_order_aux::set_ord_ref(o,atoi(pOrder->OrderRef));


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

		// set time,
		/* Use
		  TThostFtdcDateType	InsertDate;
		  TThostFtdcTimeType	InsertTime;
		  */

		//o->set_last_time();

		/*date_time ordTime;
		ordTime.set_date(pOrder->InsertDate, date_time::date_format::FN2);
		ordTime.set_time(pOrder->InsertTime);
*/
		//ptime ordTime;
		lwtp tp;
#if 0
		ptime ordTime(from_undelimited_string(pOrder->InsertDate), duration_from_string(pOrder->InsertTime));
#else
		if (strlen(pOrder->InsertDate) > 0)
		{
			//ordTime=ptime(from_undelimited_string(pOrder->InsertDate), duration_from_string(pOrder->InsertTime));
			tp = string_to_lwtp(from_undelimited_string(pOrder->InsertDate), pOrder->InsertTime);
		}
		else
		{
			if (strlen(pOrder->GTDDate) > 0)
			{
				//ordTime=ptime(from_undelimited_string(pOrder->GTDDate), duration_from_string(pOrder->InsertTime));
				tp = string_to_lwtp(from_undelimited_string(pOrder->GTDDate), pOrder->InsertTime);
			}
		}
#endif

		o->set_last_time(tp);

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//o->set_account(o->get_portfolio());
		// save original quantity / price
		//o->save_previous_values();


		loggerv2::error("fs_order::anchor - successfully rebuild order [%d][%x][%d]", id, o,o->get_last_action());
		return o;
	}

	order* fs_order_aux::anchor(fs_connection* pConnection, CThostFtdcTradeField* pTrade)
	{
		std::string sInstrCode = std::string(pTrade->InstrumentID) + "@" + pConnection->getName();
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("fs_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;

		//pConnection->get_user_info(pTrade->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);


		// client field : orderId|tradingType|portfolio
		//string_tokenizer<1024> tokenizer;
		//tokenizer.break_line(pOrder->UserID, '-');
		char *ptr = pTrade->UserID;
		int id = -1;
		//if (tokenizer.size() !=2 )
#if 0
		if (ptr[1] == ' ')
		{
			loggerv2::error("fs_order::anchor - cannot parse UserID field[%-*.*s]. Trade may from another application.", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			//return NULL;
			id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pTrade->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}
#else
		pConnection->get_user_info(pTrade->OrderRef, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
#endif


		// ok
		order* o = pConnection->get_order_from_pool();
		//o->set_id(id);
		//o->m_kpi.set_order_id(id);
		o->set_id(id);
		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->Volume);
		o->set_price(pTrade->Price);


		// way
		if (pTrade->Direction == THOST_FTDC_D_Buy)
			o->set_way(AtsType::OrderWay::Buy);
		else if (pTrade->Direction == THOST_FTDC_D_Sell)
			o->set_way(AtsType::OrderWay::Sell);
		else
		{
			loggerv2::error("fs_order - anchor: unknown order_way[%c]", pTrade->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}

		if (pTrade->OffsetFlag == '0')
			o->set_open_close(AtsType::OrderOpenClose::Open);
		else if (pTrade->OffsetFlag == '3')
			o->set_open_close(AtsType::OrderOpenClose::CloseToday);
		else
			o->set_open_close(AtsType::OrderOpenClose::Close);


		//if (ptr[1] == ' ')
		//{
		//	o->set_unknown_order();
		//}
		//else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}



		// huatai id
		fs_order_aux::set_ord_ref(o,atoi(pTrade->OrderRef));

		/*
		  TThostFtdcDateType	TradeDate;
		  TThostFtdcTimeType	TradeTime;
		  */

		/*date_time ordTime;
		ordTime.set_date(pTrade->TradeDate, date_time::date_format::FN2);
		ordTime.set_time(pTrade->TradeTime);*/
		//ptime ordTime;
		lwtp tp;
#if 0
		ptime ordTime(from_undelimited_string(pTrade->TradeDate), duration_from_string(pTrade->TradeTime));
#else		
		if (strlen(pTrade->TradeDate) > 0)
		{
			//ordTime=ptime(from_undelimited_string(pTrade->TradeDate), duration_from_string(pTrade->TradeTime));
			tp = string_to_lwtp(from_undelimited_string(pTrade->TradeDate), pTrade->TradeTime);
		}
		else
		{
			if (strlen(pTrade->TradingDay)>0)
			{
				//ordTime = ptime(from_undelimited_string(pTrade->TradingDay), duration_from_string(pTrade->TradeTime));
				tp = string_to_lwtp(from_undelimited_string(pTrade->TradingDay), pTrade->TradeTime);
			}
		}		

		o->set_last_time(tp);

		//o->set_account(o->get_portfolio());

		//o->set_account(pConnection->getAccountName(nAccount));
		o->set_user_orderid(nUserOrdId);

		loggerv2::info("fs_order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}
#endif
}

