#ifndef _OPTION_H_
#define _OPTION_H_
#pragma once
#include "abstractderivative.h"
#include "optionclass.h"
namespace terra
{
	namespace instrument
	{
		class strikeparity;
		class option : public abstractderivative
		{
		public:
			option(std::string & code);
			~option();
		public:
			void set(strikeparity * pstrikeparity){ m_strike_parity = pstrikeparity; }
			//virtual void on_maturity_time_changed();
		public:
			double get_strike(){ return m_dStrike; }
			void   set_strike(double value){ m_dStrike = value; }
			optionclass * get_option_class(){ return (optionclass*)get_class(); }
		protected:
			strikeparity * m_strike_parity;
			double          m_dStrike;
		};
	}
}
#endif //_OPTION_H_

