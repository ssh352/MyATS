#include "lts_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "lts_connection.h"


namespace lts
{
	//lts_order::lts_order(lts_connection* pConnection) : order(pConnection)
	//{
	//	m_orderRef = 0;
	//	m_orderLocalId = ""; //mandatory for security
	//	m_traderId = "";     //mandatory for security
	//	//m_frontId =0;
	//	//m_sessionId=0;
	//	m_orderSysId = "";

	//	//date_time now = date_time(time(NULL))/*.set_time(0, 0, 1)*/;
	//	//set_rebuild_time(static_cast<long>(now.get_date()));
	//	auto time = microsec_clock::local_time();
	//	set_rebuild_time(time);

	//	/*std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
	//	set_rebuild_time_point(tpnow);*/

	//}

	order* lts_order_aux::anchor(lts_connection* pConnection, CSecurityFtdcInputOrderField* pOrder)
	{
		//order_action action = AtsType::OrderAction::Created;
		//std::string sInstrCode = std::string(pOrder->InstrumentID) + "." + pOrder->ExchangeID + "@" + pConnection->getName();

		std::string sInstrCode = pConnection->compute_second_key(pOrder->InstrumentID, pOrder->ExchangeID);
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("lts_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
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
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			//loggerv2::error("lts_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return NULL;
			id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
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
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->VolumeTotalOriginal);

		//o->set_price(pOrder->LimitPrice);
		o->set_price(strtod(pOrder->LimitPrice, NULL));

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->CombOffsetFlag[0])
		{
		case SECURITY_FTDC_OF_Open:
			oc = AtsType::OrderOpenClose::Open;
			break;
		case SECURITY_FTDC_OF_Close:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case SECURITY_FTDC_OF_CloseToday:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		default:
			break;
		}
		o->set_open_close(oc);


		switch (pOrder->Direction)
		{
		case SECURITY_FTDC_D_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case SECURITY_FTDC_D_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		case SECURITY_FTDC_D_Covered:
		{
			switch (oc)
			{
			case AtsType::OrderOpenClose::Open:
				o->set_way(AtsType::OrderWay::CoveredSell);
				break;
			case AtsType::OrderOpenClose::Close:
			case AtsType::OrderOpenClose::CloseToday:
				o->set_way(AtsType::OrderWay::CoveredBuy);
				break;
			default:
				o->set_way(AtsType::OrderWay::Undef);
				break;
			}
		}
		break;

		case SECURITY_FTDC_D_Freeze:
		{
			switch (oc)
			{
			case AtsType::OrderOpenClose::Open:
				o->set_way(AtsType::OrderWay::Freeze);
				break;
			case AtsType::OrderOpenClose::Close:
				o->set_way(AtsType::OrderWay::Unfreeze);
				break;
			default:
				o->set_way(AtsType::OrderWay::Undef);
				break;
			}
		}
		break;


		case SECURITY_FTDC_D_ETFPur:

		{
			o->set_way(AtsType::OrderWay::ETFPur);
		}
		break;
		case SECURITY_FTDC_D_ETFRed:
		{
			o->set_way(AtsType::OrderWay::ETFRed);
		}
		break;
		case SECURITY_FTDC_D_OFPur:
		{
			o->set_way(AtsType::OrderWay::OFPur);
		}
		break;

		case SECURITY_FTDC_D_OFRed:
		{
			o->set_way(AtsType::OrderWay::OFRed);
		}
		break;

		case SECURITY_FTDC_D_PledgeBondIn:
		{
			o->set_way(AtsType::OrderWay::PLEDGE_BOND_IN);
		}
		break;
		case SECURITY_FTDC_D_PledgeBondOut:
		{
			o->set_way(AtsType::OrderWay::PLEDGE_BOND_OUT);
		}
		break;



		default:
		{
			loggerv2::error("lts_order - anchor: unknown order_way[%c]", pOrder->Direction);
			o->set_way(AtsType::OrderWay::Undef);
		}
		break;

		}


		//
		if (pOrder->TimeCondition == SECURITY_FTDC_TC_IOC)
		{
			if (pOrder->VolumeCondition == SECURITY_FTDC_VC_CV)
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



		if (ptr[1] == ' ')
		{

			o->set_unknown_order();
		}

		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		lts_order_aux::set_order_ref(o,atoi(pOrder->OrderRef));

		//lts order info

		//date_time ordTime = time(NULL);
		auto time = get_lwtp_now();
		o->set_last_time(time);

		//o->set_account(o->get_portfolio());
		// save original quantity / price
		//o->save_previous_values();

		//date_time now = date_time(time(NULL));
		o->set_rebuild_time(time);

		/*	std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
			o->set_rebuild_time_point(tpnow);*/

		//loggerv2::info("lts_order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}

	order* lts_order_aux::anchor(lts_connection* pConnection, CSecurityFtdcOrderField* pOrder)
	{
		//loggerv2::info("lts_order::anchoring order");
		// tradeitem
		//std::string sInstrCode = std::string(pOrder->InstrumentID) + "." + pOrder->ExchangeID + "@" + pConnection->getName();
		std::string sInstrCode = pConnection->compute_second_key(pOrder->InstrumentID, pOrder->ExchangeID);
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("lts_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
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
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			//loggerv2::error("lts_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return NULL;
			id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
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
		o->set_quantity(pOrder->VolumeTotalOriginal);
		//o->set_exec_quantity(pOrder->VolumeTraded);
		//o->set_book_quantity(pOrder->VolumeTotal);
		o->set_price(strtod(pOrder->LimitPrice, NULL));


		// way
		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->CombOffsetFlag[0])
		{
		case SECURITY_FTDC_OF_Open:
			oc = AtsType::OrderOpenClose::Open;
			break;
		case SECURITY_FTDC_OF_Close:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case SECURITY_FTDC_OF_CloseToday:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		default:
			break;
		}
		o->set_open_close(oc);


		switch (pOrder->Direction)
		{
		case SECURITY_FTDC_D_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case SECURITY_FTDC_D_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		case SECURITY_FTDC_D_Covered:
		{
			switch (oc)
			{
			case AtsType::OrderOpenClose::Open:
				o->set_way(AtsType::OrderWay::CoveredSell);
				break;
			case AtsType::OrderOpenClose::Close:
			case AtsType::OrderOpenClose::CloseToday:
				o->set_way(AtsType::OrderWay::CoveredBuy);
				break;
			default:
				o->set_way(AtsType::OrderWay::Undef);
				break;
			}
		}
		break;

		case SECURITY_FTDC_D_Freeze:
		{
			switch (oc)
			{
			case AtsType::OrderOpenClose::Open:
				o->set_way(AtsType::OrderWay::Freeze);
				break;
			case AtsType::OrderOpenClose::Close:
				o->set_way(AtsType::OrderWay::Unfreeze);
				break;
			default:
				o->set_way(AtsType::OrderWay::Undef);
				break;
			}
		}
		break;

		case SECURITY_FTDC_D_ETFPur:

		{
			o->set_way(AtsType::OrderWay::ETFPur);
		}
		break;
		case SECURITY_FTDC_D_ETFRed:

		{
			o->set_way(AtsType::OrderWay::ETFRed);
		}
		break;

		case SECURITY_FTDC_D_OFPur:
		{
			o->set_way(AtsType::OrderWay::OFPur);
		}
		break;

		case SECURITY_FTDC_D_OFRed:
		{
			o->set_way(AtsType::OrderWay::OFRed);
		}
		break;
		case SECURITY_FTDC_D_PledgeBondIn:
		{
			o->set_way(AtsType::OrderWay::PLEDGE_BOND_IN);
		}
		break;
		case SECURITY_FTDC_D_PledgeBondOut:
		{
			o->set_way(AtsType::OrderWay::PLEDGE_BOND_OUT);
		}
		break;



		default:
			loggerv2::error("lts_order - anchor: unknown order_way[%c]", pOrder->Direction);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}

		//
		if (pOrder->TimeCondition == SECURITY_FTDC_TC_IOC)
		{
			if (pOrder->VolumeCondition == SECURITY_FTDC_VC_CV)
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

		if (ptr[1] == ' ')
		{
			o->set_unknown_order();

		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		lts_order_aux::set_order_ref(o,atoi(pOrder->OrderRef));

		// lastAction
		switch (pOrder->OrderSubmitStatus)
		{
		case SECURITY_FTDC_OSS_InsertSubmitted:
		case SECURITY_FTDC_OSS_InsertRejected:
			o->set_last_action(AtsType::OrderAction::Created);
			break;

		case SECURITY_FTDC_OSS_CancelSubmitted:
		case SECURITY_FTDC_OSS_CancelRejected:
			o->set_last_action(AtsType::OrderAction::Cancelled);
			break;

		case SECURITY_FTDC_OSS_ModifySubmitted:
		case SECURITY_FTDC_OSS_ModifyRejected:
			o->set_last_action(AtsType::OrderAction::Modified);
			break;

		case SECURITY_FTDC_OSS_Accepted:
			break;

		default:
			break;
		}
		//ctp security order
		//o->set_order_local_id(pOrder->OrderLocalID);
		//o->set_trader_id(pOrder->BranchPBU);
		//o->set_order_sys_id(pOrder->OrderSysID);
		//
		set_order_local_id(o, pOrder->OrderLocalID);
		set_trader_id(o, pOrder->BranchPBU);
		set_order_sys_id(o, pOrder->OrderSysID);
		//
		o->set_status(AtsType::OrderStatus::WaitServer);
		// save original quantity / price

		lwtp tp = string_to_lwtp(from_undelimited_string(pOrder->InsertDate), (pOrder->InsertTime));

		o->set_last_time(tp);

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//o->save_previous_values();

		//date_time now = date_time(time(NULL));
		auto time = get_lwtp_now();;
		o->set_rebuild_time(time);
		set_IsETF(o, pOrder->IsETF);
		//o->set_IsETF(pOrder->IsETF);

		/*	std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
			o->set_rebuild_time_point(tpnow);*/

		//loggerv2::info("lts_order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}

	order* lts_order_aux::anchor(lts_connection* pConnection, CSecurityFtdcTradeField* pTrade, bool isETFSotck)
	{
		// tradeitem

		//std::string sInstrCode = std::string(pTrade->InstrumentID) + "." + pTrade->ExchangeID + "@" + pConnection->getName();
		std::string sInstrCode = pConnection->compute_second_key(pTrade->InstrumentID, pTrade->ExchangeID);
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("lts_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
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
		char *ptr = pTrade->UserID;
		int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			//loggerv2::error("lts_order::anchor - cannot parse UserID field[%-*.*s]. Trade may from another application", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			//return NULL;
			id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pTrade->UserID, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}


		// ok
		order* o = pConnection->get_order_from_pool();
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->Volume);
		//o->set_price(pTrade->Price);
		o->set_price(strtod(pTrade->Price, NULL));

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pTrade->OffsetFlag)
		{
		case SECURITY_FTDC_OF_Open:
			oc = AtsType::OrderOpenClose::Open;
			break;
		case SECURITY_FTDC_OF_Close:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case SECURITY_FTDC_OF_CloseToday:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		default:
			break;
		}
		o->set_open_close(oc);


		switch (pTrade->Direction)
		{
		case SECURITY_FTDC_D_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case SECURITY_FTDC_D_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		case SECURITY_FTDC_D_Covered:
		{
			switch (oc)
			{
			case AtsType::OrderOpenClose::Open:
				o->set_way(AtsType::OrderWay::CoveredSell);
				break;
			case AtsType::OrderOpenClose::Close:
			case AtsType::OrderOpenClose::CloseToday:
				o->set_way(AtsType::OrderWay::CoveredBuy);
				break;
			default:
				o->set_way(AtsType::OrderWay::Undef);
				break;
			}
		}
		break;

		case SECURITY_FTDC_D_ETFPur:
		{
			o->set_way(AtsType::OrderWay::ETFPur);
			if (isETFSotck)
				o->set_open_close(AtsType::OrderOpenClose::Close);
		}
		break;
		case SECURITY_FTDC_D_ETFRed:

		{
			o->set_way(AtsType::OrderWay::ETFRed);
			if (isETFSotck)
				o->set_open_close(AtsType::OrderOpenClose::Open);

		}
		break;

		case SECURITY_FTDC_D_OFPur:
		{
			o->set_way(AtsType::OrderWay::OFPur);
		}
		break;

		case SECURITY_FTDC_D_OFRed:
		{
			o->set_way(AtsType::OrderWay::OFRed);
		}
		break;

		case SECURITY_FTDC_D_PledgeBondIn:
		{
			o->set_way(AtsType::OrderWay::PLEDGE_BOND_IN);
		}
		break;
		case SECURITY_FTDC_D_PledgeBondOut:
		{
			o->set_way(AtsType::OrderWay::PLEDGE_BOND_OUT);
		}
		break;


		default:
			loggerv2::error("lts_order - anchor: unknown order_way[%c]", pTrade->Direction);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}


		if (ptr[1] == ' ')
		{
			o->set_unknown_order();
		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		lts_order_aux::set_order_ref(o, atoi(pTrade->OrderRef));
		//ctp security 

		lts_order_aux::set_order_local_id(o, pTrade->OrderLocalID);
		lts_order_aux::set_trader_id(o, pTrade->BranchPBU);
		//o->set_front_id(pTrade-> ->FrontID);
		//o->set_session_id(pTrade-> ->SessionID);
		lts_order_aux::set_order_sys_id(o, pTrade->OrderSysID);


		lwtp tp = string_to_lwtp(from_undelimited_string(pTrade->TradeDate), (pTrade->TradeTime));

		o->set_last_time(tp);

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//date_time now = date_time(time(NULL));
		auto time = get_lwtp_now();;
		o->set_rebuild_time(time);

		/*	std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
			o->set_rebuild_time_point(tpnow);*/
		if (isETFSotck)
		{
			o->set_status(AtsType::OrderStatus::Exec);
			o->set_exec_quantity(0);
		}
		

		set_IsETF(o, isETFSotck);

		//loggerv2::info("lts_order::anchor - successfully rebuild order [%d][%x]", id, o);
		return o;
	}
}

