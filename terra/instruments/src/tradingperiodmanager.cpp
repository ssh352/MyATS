#include "tradingperiodmanager.h"
#include "AtsType_types.h"
namespace terra
{
	namespace instrument
	{


		tradingperiodmanager::tradingperiodmanager()
		{
			m_pRefCurreTradingPeriod = nullptr;
		}


		tradingperiodmanager::~tradingperiodmanager()
		{
			clear();
		}
		void tradingperiodmanager::clear()
		{		
			for(auto &it: m_TradingPeriodList)
			{
				tradingperiod* pTradingPeriod = it;
				delete pTradingPeriod;
				pTradingPeriod = nullptr;				
			}
			m_TradingPeriodList.clear();
		}
		bool tradingperiodmanager::add_trading_period(time_duration startTime, time_duration stopTime, AtsType::TradingPhase::type phase, int autoStopInterval)
		{
			if (this->m_TradingPeriodList.size() > 0)
			{
				std::vector<tradingperiod*>::iterator it;
				it = this->m_TradingPeriodList.end();
				it--;
				tradingperiod* pTradingPeriod = (*it);
				if (startTime < pTradingPeriod->get_stop_time())
				{
					return false;
				}
			}
			this->m_TradingPeriodList.push_back(new tradingperiod(startTime, stopTime, phase, autoStopInterval));
			return true;
		}
		time_duration tradingperiodmanager::get_current_stop_time(time_duration now)
		{
			check_current_period(now);

			if (m_pRefCurreTradingPeriod == nullptr)
				return time_duration();
			time_duration interval = seconds(m_pRefCurreTradingPeriod->get_auto_stop_interval());
			//m_pRefCurreTradingPeriod->get_stop_time() = m_pRefCurreTradingPeriod->get_stop_time() - interval;
			//return _curreTradingPeriod.StopTime.Add(new time_duration(0, 0, -_curreTradingPeriod.AutoStopInterval));
			//loggerv2::error("tradingperiodmanager::GetCurrentStopTime need further implement in the future");
			return m_pRefCurreTradingPeriod->get_stop_time() - interval;
		}

		time_duration tradingperiodmanager::get_current_start_time(time_duration now)
		{
			check_current_period(now);

			if (m_pRefCurreTradingPeriod == nullptr)
				return time_duration();
			
			//m_pRefCurreTradingPeriod->get_stop_time() = m_pRefCurreTradingPeriod->get_stop_time() - interval;
			//return _curreTradingPeriod.StopTime.Add(new time_duration(0, 0, -_curreTradingPeriod.AutoStopInterval));
			//loggerv2::error("tradingperiodmanager::GetCurrentStopTime need further implement in the future");
			return m_pRefCurreTradingPeriod->get_start_time();
		}

		AtsType::TradingPhase::type tradingperiodmanager::get_trading_phase(time_duration now)
		{
			check_current_period(now);

			if (m_pRefCurreTradingPeriod == nullptr)
				return AtsType::TradingPhase::CLOSE;

			return m_pRefCurreTradingPeriod->get_trading_phase();
		}
		void tradingperiodmanager::check_current_period(time_duration now)
		{
			if (this->m_TradingPeriodList.size() <= 0)
			{
				m_pRefCurreTradingPeriod = nullptr;
				return;
			}
			if (m_pRefCurreTradingPeriod != nullptr)
			{
				if (m_pRefCurreTradingPeriod->get_stop_time() > m_pRefCurreTradingPeriod->get_start_time() && now >= m_pRefCurreTradingPeriod->get_start_time() &&
					now < m_pRefCurreTradingPeriod->get_stop_time())
					return;
				if (m_pRefCurreTradingPeriod->get_stop_time() < m_pRefCurreTradingPeriod->get_start_time() &&
					(now >= m_pRefCurreTradingPeriod->get_start_time() || now < m_pRefCurreTradingPeriod->get_stop_time()))
					return;
				loggerv2::error("Trading Phase changed Previous Period: %s, start time %s, stop time %s", AtsType::_TradingPhase_VALUES_TO_NAMES.at(m_pRefCurreTradingPeriod->get_trading_phase()), to_simple_string(m_pRefCurreTradingPeriod->get_start_time()).c_str(), to_simple_string(m_pRefCurreTradingPeriod->get_stop_time()).c_str());
			}
					
			for(auto &it: m_TradingPeriodList)
			{
				if (it->get_stop_time() > it->get_start_time() && now >= it->get_start_time() &&
					now < it->get_stop_time())
				{
					m_pRefCurreTradingPeriod = it;
					loggerv2::error("Trading Phase changed Current Period: %s, start time %s, stop time %s", AtsType::_TradingPhase_VALUES_TO_NAMES.at(m_pRefCurreTradingPeriod->get_trading_phase()), to_simple_string(m_pRefCurreTradingPeriod->get_start_time()).c_str(), to_simple_string(m_pRefCurreTradingPeriod->get_stop_time()).c_str());
					return;
				}
				if (it->get_stop_time() < it->get_start_time() && (now >= it->get_start_time() || now < it->get_stop_time()))
				{
					m_pRefCurreTradingPeriod = it;
					loggerv2::error("Trading Phase changed Current Period: %s, start time %s, stop time %s", AtsType::_TradingPhase_VALUES_TO_NAMES.at(m_pRefCurreTradingPeriod->get_trading_phase()), to_simple_string(m_pRefCurreTradingPeriod->get_start_time()).c_str(), to_simple_string(m_pRefCurreTradingPeriod->get_stop_time()).c_str());
					return;
				}				
			}
			m_pRefCurreTradingPeriod = nullptr;
		}
		bool tradingperiodmanager::load(string strFile)
		{
			boost::filesystem::path p(strFile);
			if (!boost::filesystem::exists(p))
			{
				loggerv2::error("tradingperiodmanager::load dbfile:%s not exist!", strFile.c_str());
				return false;
			}
			TradingPhaseFile = strFile;
			boost::property_tree::ptree root;
			boost::property_tree::read_json(strFile, root);
			//1.读根节点信息
			//string str = root.get<string>("ShiftPriceTime");
			//time_duration _ShiftPriceTime(duration_from_string(str));
			this->ShiftPriceTime = duration_from_string(root.get<string>("ShiftPriceTime","00:00:00"));

			//str = root.get<string>("ShiftEodTime");
			//time_duration _ShiftEodTime(duration_from_string(str));
			this->ShiftEodTime = duration_from_string(root.get<string>("ShiftEodTime", "00:00:00"));
			this->ShiftEodTimeN = duration_from_string(root.get<string>("ShiftEodTimeN", "00:00:00"));

			//2.读数组
			ptree p1 = root.get_child("TradingPeriodList");
			for (auto& it : p1)
			{
				//遍历读出数据  			
	
				time_duration startTime(duration_from_string(it.second.get<string>("StartTime")));
				time_duration stopTime(duration_from_string(it.second.get<string>("StopTime")));
				tradingperiod * pPeriod = new tradingperiod(startTime, stopTime, (AtsType::TradingPhase::type)it.second.get<int>("Phase"), it.second.get<int>("AutoStopInterval"));
				this->m_TradingPeriodList.push_back(pPeriod);
			}
			return true;
		}
		void tradingperiodmanager::save(string strFile)
		{
			boost::property_tree::ptree pt_root;
			boost::property_tree::ptree children;
			boost::property_tree::ptree child;
			for (auto & v : this->m_TradingPeriodList)
			{
				child.put("StartTime", to_simple_string(v->get_start_time()));
				child.put("StopTime", to_simple_string(v->get_stop_time()));
				child.put("Phase", v->get_trading_phase());
				child.put("AutoStopInterval", v->get_auto_stop_interval());
				children.push_back(std::make_pair("", child));
			}
			pt_root.add_child("TradingPeriodList", children);
			pt_root.put("ShiftPriceTime", to_simple_string(get_shift_price_time()));
			pt_root.put("ShiftEodTime", to_simple_string(get_shift_eod_time()));
			pt_root.put("ShiftEodTimeN", to_simple_string(get_shift_eod_time_n()));
			boost::property_tree::write_json(strFile, pt_root);
		}
	}
}