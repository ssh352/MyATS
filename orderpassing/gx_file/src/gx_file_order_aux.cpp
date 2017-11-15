#include "gx_file_connection.h"
#include "gx_file_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"

namespace gx_file
{
	/*
	OrderDate,OrderTime,FilledDate,FilledTime,OrderID,Stock,buy/sell,OrderType,OrderPrice,OrderNumber,EnterNumber,NodealNumber,MatchPrice,OrderStatue,Market,Account
	2017-07-28,10:06:49,2017-07-28,10:06:49,3,002304.SZ,buy,limit,88.04000000000001,100,100,0,88.04000000000001,filled,SZ,410001174092
	2017-07-28,10:19:23,2017-07-28,10:19:23,4,000011.SZ,buy,limit,19.06,100,100,0,19.06,filled,SZ,410001174092
	2017-07-28,10:19:59,2017-07-28,10:19:59,5,000011.SZ,buy,limit,18.87,100,100,0,18.87,filled,SZ,410001174092
	*/
	order* gx_file_order_aux::anchor_order(gx_file_connection* pConnection, string line)
	{
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		tokenizer.break_line(line.c_str(), szSeparators);	

		string OrderDate = tokenizer[0];
		string OrderTime = tokenizer[1];

		string FilledDate = tokenizer[2];
		string FilledTime = tokenizer[3];

		string LocalID = tokenizer[4];//local id
		string Stock = tokenizer[5];//000001.SZ

		string buy_sell = tokenizer[6];
		string OrderType = tokenizer[7];

		string OrderPrice = tokenizer[8];
		string OrderNumber = tokenizer[9];

		string EnterNumber = tokenizer[10];
		string NodealNumber = tokenizer[11];

		string MatchPrice = tokenizer[12];
		string OrderStatus = tokenizer[13];

		string Market = tokenizer[14];

		std::string sInstrCode = Stock.substr(0,6) + "@" + pConnection->getName();
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("gx_file_order_aux::anchor_order - tradeitem:%s not found", sInstrCode.c_str());
			return nullptr;
		}

		int nAccount     = 0;
		int nUserOrdId   = -1;
		int nInternalRe  = -1;
		int nPortfolio   = 0;
		int nTradingType = 0;
		pConnection->get_user_info(pConnection->get_user_id(atoi(LocalID.c_str())).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			
		OrderWay::type way = buy_sell == "buy" ? OrderWay::Buy : OrderWay::Sell;
		int id = (way == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
		if (id < 1)
		{
			id = FAKE_ID_MIN + atoi(LocalID.c_str());
			//return nullptr;
		}
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);
		
		o->set_quantity(atoi(OrderNumber.c_str()));
		o->set_price(atof(OrderPrice.c_str()));

		if (buy_sell == "buy")
		{
			o->set_way(AtsType::OrderWay::Buy);
		}
		else
		{
			o->set_way(AtsType::OrderWay::Sell);
		}

		if (OrderType=="limit")
		{
			o->set_price_mode(OrderPriceMode::Limit);
		}
		else
		{
			o->set_price_mode(OrderPriceMode::Market);
		}

		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		char pszTime[256];
		memset(pszTime, 0, sizeof(pszTime));
		sprintf(pszTime, "%s %s", OrderDate.c_str(), OrderTime.c_str());
		lwtp tp = string_to_lwtp(pszTime);
		o->set_last_time(tp);

		o->save_previous_values();
		//rebuild time
		auto time = get_lwtp_now();
		o->set_rebuild_time(time);

		gx_file_order_aux::set_order_local_id(o, atoi(LocalID.c_str()));
		loggerv2::info("gx_file_order_aux::anchor_order - successfully rebuild order [%d][%x]", id, o);
		return o;
	}
	/*
	资金账号0,    买/卖1,委托期限2,委托数量3,委托日期4,委托时间5,成交数量6,成交日期7,成交时间8,成交价格9,委托编号10,                   委托状态11,委托状态明细12,成交价格13,股票代码14,委托类型15,委托ID16
	410001174092,buy,  gfd,     100,     2017-07-27,11:16:21,100,     2017-07-27,11:16:21,36.450,  0_41-0001-1740-92_2-0170-727_-3001-5593,filled, filled,      36.45,   002032.sz,limit,  3
	*/
	order* gx_file_order_aux::anchor_trade(gx_file_connection* pConnection, string line)
	{
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		tokenizer.break_line(line.c_str(), szSeparators);
		
		string buy_sell  = tokenizer[1];
		string Stock     = tokenizer[14];//000001.SZ
		string OrderType = tokenizer[15];
		string LocalID   = tokenizer[16];//local id

		std::string sInstrCode = Stock.substr(0, 6) + "@" + pConnection->getName();
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("gx_file_order_aux::anchor_order - tradeitem:%s not found", sInstrCode.c_str());
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		pConnection->get_user_info(pConnection->get_user_id(atoi(LocalID.c_str())).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		OrderWay::type way = buy_sell == "buy" ? OrderWay::Buy : OrderWay::Sell;
		int id = (way == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;		
		if (id < 1)
		{
			id = FAKE_ID_MIN + atoi(LocalID.c_str());
			//return nullptr;
		}
		order* o = pConnection->get_order_from_pool();
		o->set_instrument(instr);
		o->set_id(id);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitServer);

		//o->set_quantity(atoi(OrderNumber.c_str()));
		//o->set_price(atof(OrderPrice.c_str()));

		o->set_exec_quantity(atoi(tokenizer[6]));
		o->set_exec_price(atoi(tokenizer[7]));

		if (buy_sell == "buy")
		{
			o->set_way(AtsType::OrderWay::Buy);
		}
		else
		{
			o->set_way(AtsType::OrderWay::Sell);
		}

		if (OrderType == "LIMIT")
		{
			o->set_price_mode(OrderPriceMode::Limit);
		}
		else
		{
			o->set_price_mode(OrderPriceMode::Market);
		}

		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		char pszTime[256];
		memset(pszTime, 0, sizeof(pszTime));
		sprintf(pszTime, "%s %s", tokenizer[4], tokenizer[5]);
		lwtp tp = string_to_lwtp(pszTime);
		o->set_last_time(tp);

		o->save_previous_values();
		//rebuild time
		auto time = get_lwtp_now();
		o->set_rebuild_time(time);

		gx_file_order_aux::set_order_local_id(o, atoi(LocalID.c_str()));
		loggerv2::info("gx_file_order_aux::anchor_trade - successfully rebuild order [%d][%x]", id, o);
		return o;
	}
}

