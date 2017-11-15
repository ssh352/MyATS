#ifndef _FINANCIAL_INSTRUMENT_H_
#define _FINANCIAL_INSTRUMENT_H_
#pragma once
#include "instrumentclass.h"
#include "underlying.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class maturity;
		class abstractderivative;
		class derivclassbase : public instrumentclass
		{
		public:
			derivclassbase(underlying * pUnderlying, std::string & className, int pointValue, currency * pCurrency);
			virtual ~derivclassbase();
		public:
			underlying * getUnderLying()
			{
				return this->m_pUnderlying;
			}
			virtual void save(std::string &filename);
			virtual void load(std::string &filename);
			void set_maturity_time(time_duration dt);
			time_duration get_maturity_time()
			{
				return  m_MaturityTime;
			}
			virtual void show();
			map_ex<string, maturity*> & get_maturity_map(){ return m_maturity_map; }
			map_ex<string, abstractderivative*> & get_abstract_derivative_map(){ return m_abstract_derivative_map; }
		protected:
			map_ex<string,abstractderivative*>  m_abstract_derivative_map;
			map_ex<string, maturity*> m_maturity_map;
		protected:
			underlying * m_pUnderlying;
			time_duration    m_MaturityTime;
		};
	}
}
#endif //_FINANCIAL_INSTRUMENT_H_

