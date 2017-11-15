#ifndef _STOCK_CLASS_H_
#define _STOCK_CLASS_H_
#pragma once
#include "instrumentclass.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class stock;
		class stockclass : public instrumentclass
		{
		public:
			stockclass(std::string & className, int pointValue, currency * pCurrency);
			virtual ~stockclass();
		public:
			void add(stock * pStock);
			stock * get_stock(std::string strCode);
			map_ex<std::string, stock*> & get_stock_instrument_map(){ return m_stock_instrument_map; }
			virtual void show();
		protected:
			map_ex<std::string, stock*>  m_stock_instrument_map;
		};
	}
}
#endif //_STOCK_CLASS_H_


