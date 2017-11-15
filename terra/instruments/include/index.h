#ifndef _INDEX_H_
#define _INDEX_H_
#include "financialinstrument.h"

#pragma once
namespace terra
{
	namespace instrument
	{
		class underlying;
		class stock;
		class index: public financialinstrument
		{
		public:
			index(string& code);
			~index();
		public:
			void add(stock * pStock, double nbShare);
			void set_divisor(double divisor)
			{
				m_dDivisor = divisor;
			}
			void show();
			
			double get_divisor(){return m_dDivisor;}
			void set_underlying(underlying* u){ m_pRefUnderLying = u; }
			underlying * get_underlying(){return m_pRefUnderLying;}
			map_ex<stock*, double>& get_stock_map(){ return m_refNbShares; }
		protected:
			underlying * m_pRefUnderLying;
			double        m_dDivisor;
			map_ex<stock*, double> m_refNbShares;
		};
	}
}
#endif //_INDEX_H_


