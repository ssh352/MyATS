#ifndef __TRADE_ITEM2_H__
#define __TRADE_ITEM2_H__
#include "tradeItem_order_book.h"
#include <list>
#include <thread>
#include "financialinstrument.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			enum CLOSE_PRIORITY
			{
				FIX,
				YST_FIRST,
				TODAY_FIRST
			};
			class connection;
			class tradeitem
			{
			public:
				tradeitem(terra::instrument::financialinstrument *instr, std::string &ConnectionName, connection *con, std::string &Code, std::string &tradingCode, AtsType::InstrType::type m_instrType, std::string mktname, bool m_bKey_with_exchange);
				virtual ~tradeitem() {}
				virtual std::string& getCode(){ return m_strCode; }
				terra::instrument::financialinstrument * getInstrument(){ return m_Instrument; }

				connection *getConnection(){ return m_Connection; }

				virtual tradeItem_order_book* get_order_book();
				virtual AtsType::InstrType::type get_instr_type() { return m_instrType; }

				virtual std::string get_ISIN();
				int get_sellable_position();

				const char* get_key() { return m_key.c_str(); }
				void set_key(const char* key) { m_key = key; }

				const char* get_second_key() { return m_secondKey.c_str(); }
				void set_second_key(const char* key) { m_secondKey = key; }
				double get_point_value();

				std::string get_connection_name() { return m_strConnectionName; }
				const char* get_trading_code() { return m_tradingCode.c_str(); }
				void set_trading_code(std::string value) { m_tradingCode = value; }

				void dumpinfo();
				void to_string();
				std::string &getName(){ return m_strName; }

				void set_last_sychro_timepoint(lwtp tv) { m_lastSynchroTimePoint = tv; }
				lwtp& get_last_sychro_timepoint() { return m_lastSynchroTimePoint; }

				void set_underlying(tradeitem* underlying) { m_underlying = underlying; }
				tradeitem* get_underlying() { return m_underlying; }

				int get_underlying_frozen_long_position() { return m_underlying->get_frozen_long_position(); }
				void set_underlying_frozen_long_position(int val) { m_underlying->set_frozen_long_position(val); }

				const char* get_underlying_key() { return m_underlying->get_key(); }
				const char* get_underlying_name() { return m_underlying->getName().c_str(); }

				std::string &getMarket(){ return m_strMarket; }
				void setMarket(std::string value){ m_strMarket = value; }

				int get_etf_unitisize() { return etf_unitisize; }

				void set_cancle_num(unsigned int value){ cancle_num = value; }
				void add_cancle_num(){ ++cancle_num ; }
				unsigned int get_cancel_num(){ return cancle_num; }

				void set_cancel_forbid(bool value){ m_cancel_forbid = value; }
				bool get_cancel_forbid(){ return m_cancel_forbid; }

				CLOSE_PRIORITY get_close_order(){ return m_close_order; }

			private:
				void compute_keys();
				void compute_full_name();

				//std::list<tradeitem *> allTradeItem;
				std::string m_strCode;//产品代码
				std::string m_strName;

				terra::instrument::financialinstrument *m_Instrument;
				connection *m_Connection;
				tradeItem_order_book m_orderbook;
				AtsType::InstrType::type m_instrType;
				std::string m_key;
				std::string m_secondKey;

				std::string m_strConnectionName;
				std::string m_tradingCode;
				std::string m_strMarket;

				lwtp m_lastSynchroTimePoint;
				tradeitem* m_underlying;

				CLOSE_PRIORITY m_close_order;

			public:
				int get_today_long_position() { return today_long_position; }
				void set_today_long_position(int val) { today_long_position = val; }

				int get_today_short_position() { return today_short_position; }
				void set_today_short_position(int val) { today_short_position = val; }

				double get_long_used_margin() { return long_used_margin; }
				void set_long_used_margin(double val) { long_used_margin = val; }

				double get_short_used_margin() { return short_used_margin; }
				void set_short_used_margin(double val) { short_used_margin = val; }

				int get_pending_long_close_qty() { return pending_long_close_qty; }
				void set_pending_long_close_qty(int val) { pending_long_close_qty = val; }
				void add_pending_long_close_qty(int val) 
				{
					if (val > 0)
					{
						pending_long_close_qty += val;
						if (should_pending_long_close_qty > 0)
							should_pending_long_close_qty -= val;
					}
				}

				int get_pending_short_close_qty() { return pending_short_close_qty; }
				void set_pending_short_close_qty(int val) { pending_short_close_qty = val; }
				void add_pending_short_close_qty(int val)
				{
					if (val > 0)
					{
						pending_short_close_qty += val;
						if (should_pending_short_close_qty > 0)
							should_pending_short_close_qty -= val;
					}
				}

				int get_pending_long_close_today_qty() { return pending_long_close_today_qty; }
				void set_pending_long_close_today_qty(int val) { pending_long_close_today_qty = val; }
				void add_pending_long_close_today_qty(int val)
				{
					if (val > 0)
					{
						pending_long_close_today_qty += val;
						if (should_pending_long_close_qty > 0)
							should_pending_long_close_qty -= val;
					}
				}

				int get_pending_short_close_today_qty() { return pending_short_close_today_qty; }
				void set_pending_short_close_today_qty(int val) { pending_short_close_today_qty = val; }
				void add_pending_short_close_today_qty(int val)
				{
					if (val > 0)
					{
						pending_short_close_today_qty += val;
						if (should_pending_short_close_qty > 0)
							should_pending_short_close_qty -= val;
					}
				}

				int get_should_pending_long_close_qty() { return should_pending_long_close_qty; }
				void set_should_pending_long_close_qty(int val) { should_pending_long_close_qty = val; }
				void add_should_pending_long_close_qty(int val) { should_pending_long_close_qty += val; }

				int get_should_pending_short_close_qty() { return should_pending_short_close_qty; }
				void set_should_pending_short_close_qty(int val) { should_pending_short_close_qty = val; }
				void add_should_pending_short_close_qty(int val) { should_pending_short_close_qty += val; }

				int get_tot_long_position() { return tot_long_position; }
				void set_tot_long_position(int val) { tot_long_position = val; }

				int get_tot_short_position() { return tot_short_position; }
				void set_tot_short_position(int val) { tot_short_position = val; }

				int get_frozen_long_position() { return frozen_long_position; }
				void set_frozen_long_position(int val) { frozen_long_position = val; }

				int get_covered_sell_open_position() { return covered_sell_opened_position; }
				void set_covered_sell_open_position(int val) { covered_sell_opened_position = val; }

				int get_pending_covered_sell_close_qty() { return pending_covered_sell_close_qty; }
				void set_pending_covered_sell_close_qty(int val) { pending_covered_sell_close_qty = val; }

				int get_pending_covered_sell_open_qty() { return pending_covered_sell_open_qty; }
				void set_pending_covered_sell_open_qty(int val) { pending_covered_sell_open_qty = val; }

				int get_yst_long_position() { return yst_long_position; }
				void set_yst_long_position(int val) { yst_long_position = val; }

				int get_yst_short_position() { return yst_short_position; }
				void set_yst_short_position(int val) { yst_short_position = val; }

				int get_closed_position() { return closed_position; }
				void set_closed_position(int val) { closed_position = val; }

				int get_open_position() { return open_position; }
				void set_open_position(int val) { open_position = val; }

				int get_today_purred_qty() { return today_purred_qty; }
				void set_today_purred_qty(int qty) { today_purred_qty = qty; }

				int get_yst_comb_long() { return yst_comb_long; }
				void set_yst_comb_long(int val) { yst_comb_long = val; }

				int get_yst_comb_short() { return yst_comb_short; }
				void set_yst_comb_short(int val) { yst_comb_short = val; }

				//int get_today_comb_long() { return today_comb_long; }
				//void set_today_comb_long(int val) { today_comb_long = val; }

				//int get_today_comb_short() { return today_comb_short; }
				//void set_today_comb_short(int val) { today_comb_short = val; }


			private:
				int tot_long_position = 0;
				int tot_short_position = 0;

				int today_long_position = 0;
				int today_short_position = 0;

				double long_used_margin = 0;
				double short_used_margin = 0;

				//used for computing sellable position for stocks
				int yst_long_position = 0;
				int yst_short_position = 0;
				int closed_position = 0;
				int open_position = 0;


				//for combine position
				int yst_comb_long = 0;
				int yst_comb_short = 0;
				//int today_comb_long = 0;
				//int today_comb_short = 0;

				/*for underlying only
				when qty was frozen, it cannot be sold
				*/
				int frozen_long_position = 0;

				/*for call option only.
				qty that was opened (executed) as covered sell.
				when doing a buy back, try to close this qty first
				*/
				int covered_sell_opened_position = 0;

				int pending_long_close_qty = 0;   //pending long order to close the short position
				int pending_short_close_qty = 0;  //pending short order to close the open position

				int pending_long_close_today_qty = 0;
				int pending_short_close_today_qty = 0;

				int should_pending_long_close_qty = 0; //刚下单还没有ack时的pending
				int should_pending_short_close_qty = 0; 

				/*for call option only
				qty that was on ack status when to close covered sell position
				*/
				int pending_covered_sell_close_qty = 0; //pending long order to close covered short sell open position on options.
				int pending_covered_sell_open_qty = 0; //pending short order to open covered short sell position on options.

				//for ETF purchase and redemption
				//int today_pur_qty;  //pur for incoming qty , for both etf and stocks
				int today_purred_qty = 0;  //red for outgoing qty , for both etf and stocks
				int etf_unitisize = 0;  //etf unit size factor 

				unsigned int cancle_num = 0;
				bool m_cancel_forbid = false;
				char m_log_str[512];
			};
			
		}
	}
}


#endif

