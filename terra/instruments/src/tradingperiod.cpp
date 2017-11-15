#include "tradingperiod.h"
namespace terra
{
	namespace instrument
	{
		tradingperiod::tradingperiod(time_duration startTime, time_duration stopTime, AtsType::TradingPhase::type phase, int autoStopInterval)
		{
			m_startTime = startTime;
			m_stopTime = stopTime;
			m_enTradingPhase = phase;
			m_iAutoStopInterval = autoStopInterval;
		}

		tradingperiod::~tradingperiod()
		{
		}

	}
}
