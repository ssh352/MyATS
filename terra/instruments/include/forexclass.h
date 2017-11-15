#ifndef _FOREX_CLASS_H_
#define _FOREX_CLASS_H_
#pragma once
#include "instrumentclass.h"
namespace terra
{
	namespace instrument
	{
		class forex;
		class forexclass :public instrumentclass
		{
		public:
			forexclass(std::string & className, int pointValue, currency * pCurrency);
			forex * get_forex(std::string strCode)
			{
				return m_forex_instrument_map.get_by_key(strCode);
			}
			map_ex<std::string, forex*> & get_forex_instrument_map(){ return m_forex_instrument_map; }
			void add(forex * pForex);
			~forexclass();
		private:
			map_ex<std::string, forex*>  m_forex_instrument_map;
		};
	}
}

#endif