#ifndef _FEES_STRUCTURE_H_
#define _FEES_STRUCTURE_H_
#pragma once
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class feesstructure
		{
		public:
			feesstructure();
			~feesstructure();
		protected:
			double m_dFeesFloatExchange;
			double m_dFeesFloatBroker;
			double m_dFeesFixExchange;
			double m_dFeesFixBroker;
			double m_dFeesSellAmount;
			bool   m_bNotCloseToday;
		public:
			double get_fees_float_exchange(){ return m_dFeesFloatExchange; }
			double get_fees_float_broker(){ return m_dFeesFloatBroker; }
			double get_fees_fix_exchange(){ return m_dFeesFixExchange; }
			double get_fees_fix_broker(){ return m_dFeesFixBroker; }
			double get_fees_sell_amount(){ return m_dFeesSellAmount; }
			void   set_fees_float_exchange(double value){ m_dFeesFloatExchange = value; }
			void   set_fees_float_broker(double value){ m_dFeesFloatBroker = value; }
			void   set_fees_fix_exchange(double value){ m_dFeesFixExchange = value; }
			void   set_fees_fix_broker(double value){ m_dFeesFixBroker = value; }
			void   set_fees_sell_amount(double value){ m_dFeesSellAmount = value; }
			void   save(boost::property_tree::ptree & pt);
			void   load(boost::property_tree::ptree & pt);
			bool   get_not_close_today(){ return m_bNotCloseToday; }
			void   set_not_close_today(bool value){ m_bNotCloseToday = value; }
		};
	}
}
#endif //_FEES_STRUCTURE_H_


