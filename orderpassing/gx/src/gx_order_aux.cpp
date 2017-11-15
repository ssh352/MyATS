#include "gx_connection.h"
#include "gx_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"



namespace gx
{
	order* gx_order_aux::anchor(gx_connection* pConnection,const string & clientID,const string & instrumentID, int quantity, double price,const string & date)
	{
		//order_action action = AtsType::OrderAction::Created;
		std::string sInstrCode = instrumentID + "@" + pConnection->getName();
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("gx_order_aux::anchor - tradeitem:%s not found",sInstrCode.c_str());
			return nullptr;
		}

		int id = -1;
		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;

		string & userID = pConnection->get_user_info_ex(clientID);
		pConnection->get_user_info(userID.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
				
		order* o = pConnection->get_order_from_pool();
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		//o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(quantity);			
		o->set_price(price);

		// way & open close
				
		int way = pConnection->get_way(clientID);
		o->set_way((OrderWay::type)way);
	
		o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		o->set_trading_type(nTradingType);
		
		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);			
			
		auto time = get_lwtp_now();;
		o->set_last_time(time);
				
		o->save_previous_values();
					
		return o;
	}
}

