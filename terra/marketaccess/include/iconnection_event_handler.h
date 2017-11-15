#ifndef __CONNECTION_EVENT_HANDLER2_H__
#define __CONNECTION_EVENT_HANDLER2_H__
#include <string>

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class connection;
			class tradeitem;
			class portfolio;

			class iconnection_event_handler
			{
			public:
				//virtual void new_tick_rule_cb(market* m, tick_rule* t) = 0;
				virtual void new_instrument_cb(void* con, tradeitem* i) {};
				virtual void send_statistics(connection* con) {};
				virtual void send_instr_pos(tradeitem* i){};
				virtual void send_portfolio(std::string sPortfolio, std::string conname, tradeitem* i) {};
				//
				virtual void set_quote_sys_id_cb(tradeitem* i, const string & id){};
			};
		}
	}
}
#endif // __CONNECTION_EVENT_HANDLER_H__

