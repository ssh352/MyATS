#ifndef _TRADING_PERIOD_H_
#define _TRADING_PERIOD_H_
#pragma once
#include "instrumentenum.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class tradingperiod
		{
		public:
			tradingperiod(time_duration startTime, time_duration stopTime, AtsType::TradingPhase::type phase, int autoStopInterval);
			~tradingperiod();
		protected:
			time_duration m_startTime;
			time_duration m_stopTime;
			AtsType::TradingPhase::type m_enTradingPhase;
			int       m_iAutoStopInterval;
		public:
			time_duration & get_start_time(){ return m_startTime; }
			time_duration & get_stop_time(){ return m_stopTime; }
			AtsType::TradingPhase::type get_trading_phase(){ return m_enTradingPhase; }
			int get_auto_stop_interval(){ return m_iAutoStopInterval; }
			void set_start_time(time_duration & value){ m_startTime = value; }
			void set_stop_time(time_duration & value){ m_stopTime = value; }
			void set_trading_phase(AtsType::TradingPhase::type value){ m_enTradingPhase = value; }
			void set_auto_stop_interval(int value){ m_iAutoStopInterval = value; }
		};
	}
}
#endif //_TRADING_PERIOD_H_

