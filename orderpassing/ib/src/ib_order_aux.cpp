#include "ib_connection.h"
#include "ib_order_aux.h"
//#include "log.h"
#include "terra_logger.h"
//#include "tradeitem_gh.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"

namespace ib
{
   //ib_order::ib_order(ib_connection* pConnection) 
	  // : order(pConnection)
   //{
   //   //m_orderRef = 0;
	  ////m_orderLocalId = ""; //mandatory for security
	  ////m_traderId ="";     //mandatory for security
	  ////m_frontId =0;
	  ////m_sessionId=0;
	  ////m_orderSysId="";
   //}

   order* ib_order_aux::anchor(ib_connection* pConnection, const Contract& contract, const Order& Ord, const OrderState& OrdStatus)
   {
#if 0	   
	   std::string sInstrCode = contract.localSymbol + "@" + pConnection->getName();	  
#else
	   std::string sInstrCode;
	   if (contract.secType == "CASH")
	   {
		   sInstrCode = contract.symbol + "@" + pConnection->getName();
	   }
	   else
	   {
		   sInstrCode = contract.localSymbol + "@" + pConnection->getName();
	   }
#endif
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_key(sInstrCode.c_str());
	   if (instr == NULL)
	   {
		   //logger::info("ib_order anchor - instrument(2nd key) %s doesn't exists", contract.localSymbol.c_str());
		   return NULL;
	   }
	   //
	   int nAccount = 0;
	   int nUserOrdId = 0;
	   int nInternalRe = 0;
	   int nPortfolio = 0;
	   int nTradingType = 0;
	   int id = 0;
	   //to do ......find the user info
	   pConnection->get_user_info(Ord.orderRef.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
	   //

	   order* o = pConnection->get_order_from_pool();
	   o->set_instrument(instr);

	   o->set_id(Ord.orderId);
	   o->set_last_action(AtsType::OrderAction::Created);

	   if (OrdStatus.status=="Submitted")
		   o->set_status(AtsType::OrderStatus::Ack);
	   else if (OrdStatus.status == "Cancelled")
		   o->set_status(AtsType::OrderStatus::Cancel);
	   else if (OrdStatus.status == "Filled")
		   o->set_status(AtsType::OrderStatus::Exec);
	   else 
		   o->set_status(AtsType::OrderStatus::WaitMarket);

	   if (Ord.orderType=="LMT")
		   o->set_price_mode(AtsType::OrderPriceMode::Limit);

	   o->set_quantity(Ord.totalQuantity);
	   o->set_price(Ord.lmtPrice);
	  
	   if (Ord.action == "BUY")
		   o->set_way(AtsType::OrderWay::Buy);
	   else if (Ord.action == "SELL")
		   o->set_way(AtsType::OrderWay::Sell);
	   else
		   o->set_way(AtsType::OrderWay::Undef);

	   //o->set_unknown_order();
	   //o->set_account(Ord.account.c_str());
	   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
	   o->set_trading_type(nTradingType);
	   o->set_user_orderid(nUserOrdId);
	   
	   auto time = get_lwtp_now();;
	   o->set_rebuild_time(time);
	   o->set_last_time(time);

	   loggerv2::info("ib_order::anchor - rebuild order id %d ", Ord.orderId);

	   return o;
   
   }
   order* ib_order_aux::anchor(ib_connection* pConnection, const Contract& contract, const Execution& execution)
   {
	   std::string sInstrCode;
	   if (contract.secType == "CASH")
	   {
		   sInstrCode = contract.symbol + "@" + pConnection->getName();
	   }
	   else
	   {
		   sInstrCode = contract.localSymbol + "@" + pConnection->getName();
	   }
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_key(sInstrCode.c_str());
	   if (instr == nullptr)
	   {
		   //logger::info("ib_order anchor - instrument(2nd key) %s doesn't exists", contract.localSymbol.c_str());
		   return nullptr;
	   }

	   int nAccount = 0;
	   int nUserOrdId = 0;
	   int nInternalRe = 0;
	   int nPortfolio = 0;
	   int nTradingType = 0;
	   int id = 0;
	   //to do ......find the user info
	   pConnection->get_user_info(execution.orderRef.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);

	   int ordId = execution.orderId;
	   order* o = pConnection->get_order_from_pool();
	   o->set_instrument(instr);
	   o->set_id(ordId);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);

	   o->set_quantity((int)execution.shares);
	   o->set_price(execution.avgPrice);	   
	   if (execution.side == "BOT")
		   o->set_way(AtsType::OrderWay::Buy);
	   else if (execution.side == "SLD")
		   o->set_way(AtsType::OrderWay::Sell);
	   	   
	   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
	   o->set_trading_type(nTradingType);	   
	   o->set_user_orderid(nUserOrdId);

	   auto time = get_lwtp_now();;
	   o->set_rebuild_time(time);

	   //o->set_last_time(time);
	   string ts = execution.time.c_str();
	   ptime t(time_from_string(ts));
	   lwtp tp = ptime_to_lwtp(t);
	   o->set_last_time(tp);

	   loggerv2::info("ib_order::anchor - rebuild order id %d when receive Execution", ordId);
	   return o;
   }

}

