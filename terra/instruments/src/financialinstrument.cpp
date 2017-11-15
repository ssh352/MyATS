#include <vector>
#include "financialinstrument.h"
#include <boost/algorithm/string.hpp>
#include "tickrule.h"
#include "optionclass.h"
#include "terra_logger.h"
namespace terra
{
	namespace instrument
	{
		void financialinstrument::set_feed_codes(std::string & strFeedCodes)
		{
			m_strFeedCodes = strFeedCodes;
			this->m_FeedCodes.clear();
			if (strFeedCodes.length() <= 0)
			{
				loggerv2::error("financialinstrument::set_feed_codes FeedCodes is empty!");
				return;
			}
			std::vector<std::string> strs;
			boost::split(strs, strFeedCodes, boost::is_any_of("|"));
			for (std::string& fullcode : strs)
			{
				std::vector<std::string> strs1;
				boost::split(strs1, fullcode, boost::is_any_of("@"));
				this->m_FeedCodes[strs1[1]] = strs1[0];
			}
		}
		void financialinstrument::set_trading_code(std::string & strTradingCodes)
		{
			m_strTradingCodes = strTradingCodes;
			this->m_TradingCodes.clear();
			if (strTradingCodes.length() <= 0)
			{
				loggerv2::error("financialinstrument::set_trading_code TradingCodes for %s is empty!", m_strCode.c_str());
				return;
			}
			std::vector<std::string> strs;
			boost::split(strs, strTradingCodes, boost::is_any_of("|"));
			for (std::string& fullcode : strs)
			{
				std::vector<std::string> strs1;
				boost::split(strs1, fullcode, boost::is_any_of("@"));
				this->m_TradingCodes[strs1[1]] = strs1[0];
			}
		}
		tickrule * financialinstrument::get_tick_rule()
		{
			return m_tick_rule;
		}
		double financialinstrument::get_to_reference()
		{
			if (this->m_pRefCurrency != nullptr)
				return m_pRefCurrency->get_to_reference();
			return 0.0;
		}
		double financialinstrument::get_fees_float_exchange()
		{
			if (this->m_pRefClass != nullptr)
			{
				return m_pRefClass->get_fees_float_exchange();
			}
			return 0.0;
		}
		double financialinstrument::get_fees_fix_exchange()
		{
			if (this->m_pRefClass != nullptr)
			{
				return m_pRefClass->get_fees_fix_exchange();
			}
			return 0.0;
		}
		double financialinstrument::get_fees_float_broker()
		{
			if (this->m_pRefClass != nullptr)
			{
				return m_pRefClass->get_fees_float_broker();
			}
			return 0.0;
		}
		double financialinstrument::get_fees_fix_broker()
		{
			if (this->m_pRefClass != nullptr)
			{
				return m_pRefClass->get_fees_fix_broker();
			}
			return 0.0;
		}
		double financialinstrument::get_fees_sell_amount()
		{
			if (this->m_pRefClass != nullptr)
			{
				return m_pRefClass->get_fees_sell_amount();
			}
			return 0.0;
		}

		bool financialinstrument::get_not_close_today()
		{
			if (this->m_pRefClass != nullptr)
			{
				return m_pRefClass->get_not_close_today();
			}
			return 0.0;
		}

		void financialinstrument::show()
		{
			loggerv2::info("financialinstrument::show enter");
			loggerv2::info("financialinstrument::show Code:%s", this->m_strCode.c_str());
			loggerv2::info("financialinstrument::show Exchange:%s", this->m_strExchange.c_str());
			loggerv2::info("financialinstrument::show ISIN:%s", this->m_strISIN.c_str());
			loggerv2::info("financialinstrument::show RIC:%s", this->m_strRIC.c_str());
			loggerv2::info("financialinstrument::show FeedCodes:%s", this->m_strFeedCodes.c_str());
			loggerv2::info("financialinstrument::show TradingCodes:%s", this->m_strTradingCodes.c_str());
			loggerv2::info("financialinstrument::show type:%d", m_enType);
			loggerv2::info("financialinstrument::show pointValue:%d", m_iPointValue);
			if (this->m_pRefClass != nullptr)
			{
				loggerv2::info("financialinstrument::show className:%s", m_pRefClass->get_name().c_str());
			}
			if (this->m_pRefCurrency != nullptr)
			{
				loggerv2::info("financialinstrument::show currency:%s", m_pRefCurrency->get_name().c_str());
			}
			loggerv2::info("financialinstrument::show end");
		}
		double financialinstrument::tick_up(double price, int depth)
		{
			double tempPrice = price;
			if (m_tick_rule == nullptr)
				return price;
			for (int i = 0; i < depth; i++)
			{				
				tempPrice = m_tick_rule->tick_up(tempPrice);
			}
			return tempPrice;
		}
		double financialinstrument::tick_down(double price, int depth)
		{
			double tempPrice = price;
			if (m_tick_rule == nullptr)
				return price;
			for (int i = 0; i < depth; i++)
			{				
				tempPrice = m_tick_rule->tick_down(tempPrice);
			}
			return tempPrice < 0 ? 0 : tempPrice;
		}

		int   financialinstrument::get_point_value()
		{
			if (m_pRefClass != nullptr)
				return m_pRefClass->get_point_value();
			return -1;
		}
	}
}