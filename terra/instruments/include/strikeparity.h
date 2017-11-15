#ifndef _STRIKE_PARITY_H_
#define _STRIKE_PARITY_H_
#pragma once
namespace terra
{
	namespace instrument
	{
		class maturity;
		class option;
		class strikeparity
		{
		public:
			strikeparity(maturity * pMaturity, double strike);
			~strikeparity();
		public:
			void add(option * pOption);
			void show();
			option * get_call(){ return m_pRefCallOption; }
			option * get_put(){ return m_pRefPutOption; }
		protected:
			maturity * m_pRefMaturity;
			option   * m_pRefCallOption;
			option   * m_pRefPutOption;
			double      m_dStrike;
		public:			
			double get_strike(){ return m_dStrike; }
			void   set_strike(double value){ m_dStrike = value; }
		};
	}
}
#endif //_STRIKE_PARITY_H_


