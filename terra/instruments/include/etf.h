#ifndef _ETF_H_
#define _ETF_H_
#pragma once
#include "financialinstrument.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class stock;
		class etfcomponent
		{
		public:
			etfcomponent(stock * pRefStock, int iShares, int iMustCashReplace, double dCashReplaceAmt);
			~etfcomponent(){}
			void show();
		protected:
			stock * m_pRefStock;
			int      m_iShares;
			int      m_iMustCashReplace;
			double   m_dCashReplaceAmt;
		public:
			stock * get_stock(){ return m_pRefStock; }
			int get_shares(){ return m_iShares; }			
			int get_must_cash_replace(){ return m_iMustCashReplace; }
			double get_cash_replace_amt(){ return m_dCashReplaceAmt; }
			void   set_stock(stock * value){ m_pRefStock = value; }
			void   set_shares(int value){ m_iShares = value; }
			void   set_must_cash_replace(int value){ m_iMustCashReplace = value; }
			void   set_cash_replace_amt(double value){ m_dCashReplaceAmt = value; }
			string get_code();
		};
		class etf : public financialinstrument
		{
		public:
			etf(std::string & code);
			~etf();
		public:
			void  add(etfcomponent * pETFComponent);
			void  show();
		protected:
			int m_iUnitSize;
			double m_dCashDiffPerUnit;
			map_ex<string, etfcomponent*>  m_etf_component_map;
		public:
			int get_unit_size(){ return m_iUnitSize; }
			double get_cash_diff_per_unit(){ return m_dCashDiffPerUnit; }
			void set_unit_size(int value){ m_iUnitSize = value; }			
			void set_cash_diff_per_unit(double value){ m_dCashDiffPerUnit = value; }
			map_ex<string, etfcomponent*> & get_etf_component_map(){ return m_etf_component_map; }
		};
	}
}
#endif //_ETF_H_

