#ifndef _INSTRUMENT_CLASS_H_
#define _INSTRUMENT_CLASS_H_
#pragma once
#include <string>
#include "currency.h"
#include "feesstructure.h"
namespace terra
{
	namespace instrument
	{
		class tickrule;
		class instrumentclass
		{
		public:
			instrumentclass(std::string & className, int pointValue, currency * pCurrency);
			virtual ~instrumentclass();
		public:
			//std::string & to_string(){ return this->m_strClassName; }
			std::string & get_name(){ return this->m_strClassName;}
			int get_point_value(){ return this->m_PointValue; }
			tickrule * get_tick_rule(){ return this->m_ptickrule; }
			void set_tick_rule(tickrule * ptickrule)
			{
				this->m_ptickrule = ptickrule;
			}
			double get_fees_float_exchange()
			{
				if (m_pFeesStructure != nullptr)
				{
					return m_pFeesStructure->get_fees_float_exchange();
				}
				return 0.0;
			}
			void set_fees_float_exchange(double value)
			{
				if ( m_pFeesStructure != nullptr)
					m_pFeesStructure->set_fees_float_exchange(value);
			}
			double get_fees_fix_exchange()
			{
				if (m_pFeesStructure != nullptr)
				{
					return m_pFeesStructure->get_fees_fix_exchange();
				}
				return 0.0;
			}
			void set_fees_fix_exchange(double value)
			{
				if (m_pFeesStructure != nullptr)
					m_pFeesStructure->set_fees_fix_exchange(value);
			}
			double get_fees_float_broker()
			{
				if (m_pFeesStructure != nullptr)
				{
					return m_pFeesStructure->get_fees_float_broker();
				}
				return 0.0;
			}
			void set_fees_float_broker(double value)
			{
				if (m_pFeesStructure != nullptr)
					m_pFeesStructure->set_fees_float_broker(value);
			}
			double get_fees_fix_broker()
			{
				if (m_pFeesStructure != nullptr)
				{
					return m_pFeesStructure->get_fees_fix_broker();
				}
				return 0.0;
			}
			void set_fees_fix_broker(double value)
			{
				if (m_pFeesStructure != nullptr)
					m_pFeesStructure->set_fees_fix_broker(value);
			}
			double get_fees_sell_amount()
			{
				if (m_pFeesStructure != nullptr)
				{
					return m_pFeesStructure->get_fees_sell_amount();
				}
				return 0.0;
			}
			void set_fees_sell_amount(double value)
			{
				if (m_pFeesStructure != nullptr)
					m_pFeesStructure->set_fees_sell_amount(value);
			}
			void set_not_close_today(bool value)
			{
				if (m_pFeesStructure != nullptr)
					m_pFeesStructure->set_not_close_today(value);
			}
			bool get_not_close_today()
			{
				if (m_pFeesStructure != nullptr)
					return m_pFeesStructure->get_not_close_today();
				return false;
			}
			string get_currency_name();
			currency* get_currency();
			virtual void save(std::string &filename);
			virtual void load(std::string &filename);
			virtual void show();
		protected:
			virtual void save(boost::property_tree::ptree & pt);
		protected:
			std::string       m_strClassName;
			int               m_PointValue;
			currency      *   m_pRefCurrency;
			feesstructure *   m_pFeesStructure;
			tickrule      *   m_ptickrule;
			bool              m_bImplicitPreopen;
			bool              m_bImplicitPreopenTh;
		};
	}
}
#endif //_INSTRUMENT_CLASS_H_

