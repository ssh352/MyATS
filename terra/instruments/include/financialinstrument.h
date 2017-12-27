#ifndef _FINANCIAL_INSTRUMENTS_V2_H_
#define _FINANCIAL_INSTRUMENTS_V2_H_
#include "AtsType_types.h"
#include "instrumentcommon.h"
#include "tickrule.h"
namespace terra
{
	namespace instrument
	{
		class instrumentclass;
		class currency;
		class tickrule;
		class financialinstrument
		{
		public:
			financialinstrument(std::string & code)
				:m_strCode(code)
			{
				this->m_strExchange.empty();
				this->m_strISIN.empty();
				this->m_strRIC.empty();
				this->m_iPointValue = 0;
				this->m_pRefClass = nullptr;
				this->m_pRefCurrency = nullptr;
				this->m_tick_rule = nullptr;
			}
			virtual ~financialinstrument(){}
		protected:
			/*10000515@LTSUDP|10000515.SSE@LTS|10000515.SH@TDF|SH10000515@XS*/
			map_ex<string, string> m_FeedCodes;
			/*10000515.SSE@LTS|10000515.SH@XS*/
			map_ex<string, string> m_TradingCodes;
		public:
			double tick_up(double price, int depth = 1);
			double tick_down(double price, int depth = 1);
			/*
			to_string as key in the map
			*/
			//virtual std::string to_string()
			//{
			//	return this->m_strCode;
			//}
			string& get_code(){ return m_strCode; }
			void set_class(instrumentclass * pClass)
			{
				m_pRefClass = pClass;
			}
			instrumentclass * get_class()
			{
				return m_pRefClass;
			}
			currency * get_currency()
			{
				return m_pRefCurrency;
			}
			void set_currency(currency * pCurrency)
			{
				this->m_pRefCurrency = pCurrency;
			}
			double get_to_reference();
			double get_fees_float_exchange();
			double get_fees_fix_exchange();
			double get_fees_float_broker();
			double get_fees_fix_broker();
			double get_fees_sell_amount();
			bool get_not_close_today();
			tickrule * get_tick_rule();
			void set_feed_codes(std::string & strFeedCodes);
			void set_trading_code(std::string & strTradingCodes);
			map_ex<string, string> & get_feed_codes(){ return m_FeedCodes; }
			map_ex<string, string> & get_trading_codes(){ return m_TradingCodes; }
			AtsType::InstrType::type get_type(){ return m_enType; }
			void set_type(AtsType::InstrType::type type){ m_enType = type; }
			virtual void show();
			int get_point_value();
			void set_tick_rule(tickrule * tick){ m_tick_rule = tick; }
			int get_point_value_self(){ return m_iPointValue; }
		protected:
			std::string        m_strCode;
			instrumentclass  * m_pRefClass;
			currency         * m_pRefCurrency;
			AtsType::InstrType::type          m_enType;
			string             m_strFeedCodes;
			string             m_strTradingCodes;
			tickrule         * m_tick_rule;		
			std::string   m_strExchange;
			std::string   m_strISIN;
			std::string   m_strRIC;
			int m_iPointValue;
		public:
			string& get_exchange(){ return m_strExchange; }
			string& get_isin(){ return m_strISIN; }
			string& get_ric(){ return m_strRIC; }			
			void   set_exchange(string value){ m_strExchange = value; }
			void   set_isin(string value){ m_strISIN = value; }
			void   set_ric(string value){ m_strRIC = value; }
			void   set_point_value(int value){ m_iPointValue = value; }
		};
	}
}
#endif //_FINANCIAL_INSTRUMENTS_V2_H_ 