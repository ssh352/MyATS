#ifndef _MATURITY_H_
#define _MATURITY_H_
#include "instrumentcommon.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#pragma once
namespace terra
{
	namespace instrument
	{
		class optionclass;
		class futureclass;
		class derivclassbase;
		class strikeparity;
		class option;
		class maturity
		{
		public:
			maturity(optionclass * poptionclass, date maturityDay);
			maturity(futureclass * pfutureclass, date maturityDay);
			~maturity();
		public:
			void init();
			void compute_days_off();
			void compute_time_to_maturity();
			void compute_actu();
			double get_actu(){ return m_dActu; }
			strikeparity * get_strike(double strike);
			void add(option * pOption);
			void compute_first_last_strikes();
			void show();
			//string to_string(){ return m_MaturityDay.get_string(terra::common::date_time_ex::date_format::FN3); }
			int get_off_days(){ return m_iOffDays; }
			double get_open_days(){ return m_dOpenDays; }
			ptime& get_maturity_datetime(){ return m_MaturityDay; }
			string get_day_string(){ return to_iso_extended_string(m_MaturityDay.date()); }
			double get_time_to_maturity(){ return m_dTimeToMaturity; }
			double get_first_strike(){return m_dFirstStrike;}			
			double get_last_strike(){return m_dLastStrike;}
			derivclassbase * get_derivclass(){ return m_pDerivClassBase; }
		protected:
			ptime      m_MaturityDay;
			derivclassbase *  m_pDerivClassBase;
			int               m_iOffDays;
			double            m_dOpenDays;
			double            m_dTimeToMaturity;
			double            m_dActu;
			double            m_dFirstStrike;
			double            m_dLastStrike;		
			map_ex<double, strikeparity*> m_strikeparitylist;
		public:
			map_ex<double, strikeparity*> & get_strike_parity_list(){ return m_strikeparitylist; }
		};
	}
}
#endif

