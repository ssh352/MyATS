#ifndef _TRADING_PERIOD_MANAGER_H_
#define _TRADING_PERIOD_MANAGER_H_
#pragma once
#include "instrumentenum.h"
#include "instrumentcommon.h"
#include "tradingperiod.h"
namespace terra
{
	namespace instrument
	{
		class tradingperiodmanager
		{
		public:
			tradingperiodmanager();
			~tradingperiodmanager();
		public:
			bool add_trading_period(time_duration startTime, time_duration stopTime, AtsType::TradingPhase::type phase, int autoStopInterval);
			time_duration get_current_stop_time(time_duration now);
			time_duration get_current_start_time(time_duration now);
			AtsType::TradingPhase::type get_trading_phase(time_duration now);
			void check_current_period(time_duration now);
			void clear();
			bool load(string strFile);
			void save(string strFile);
			time_duration & get_shift_price_time(){ return ShiftPriceTime; }
			time_duration & get_shift_eod_time(){ return ShiftEodTime; }
			time_duration & get_shift_eod_time_n(){ return ShiftEodTimeN; }
			void        set_shift_price_time(time_duration & value){ ShiftPriceTime = value; }
			void        set_shift_eod_time(time_duration & value){ ShiftEodTime = value; }
			void        set_shift_eod_time_n(time_duration & value){ ShiftEodTimeN = value; }
			std::vector<tradingperiod*> & get_trading_period_list(){ return m_TradingPeriodList; }
		protected:
			time_duration ShiftPriceTime;
			time_duration ShiftEodTime;
			time_duration ShiftEodTimeN;
			string   TradingPhaseFile;
			std::vector<tradingperiod*> m_TradingPeriodList;
			tradingperiod * m_pRefCurreTradingPeriod;
		};
	}
}
#endif //_TRADING_PERIOD_MANAGER_H_

