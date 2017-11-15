#ifndef _ABSTRACT_DERIVATIVE_H_
#define _ABSTRACT_DERIVATIVE_H_
#include "financialinstrument.h"
#include "instrumentcommon.h"
#pragma once
namespace terra
{
	namespace instrument
	{
		class abstractderivative : public financialinstrument
		{
		public:
			abstractderivative(std::string & code);
			~abstractderivative();
		public:
			ptime get_maturity()
			{
				return m_date_time;
			}
			string get_maturity_str()
			{
				return m_strMaturity;
			}
			/*
			maturity format:2015-12-18
			*/
			void set_maturity(std::string strMaturity)
			{
				m_strMaturity = strMaturity;
				//m_date_time.set_date(strMaturity);
				m_date_time = ptime(from_string(strMaturity));
			}
			virtual void on_maturity_time_changed() ;
			virtual void show();
		protected:
			ptime   m_date_time;
			std::string    m_strMaturity;
		};
	}
}
#endif //_ABSTRACT_DERIVATIVE_H_

