#include "tickrule.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{

		tickrule::tickrule(std::string & values) : m_strValues(values)
		{
			if (m_strValues.length() <= 0)
			{
				loggerv2::error("tickrule::tickrule tickrule is empty!");
				return;
			}
			std::vector<std::string> strs;
			boost::split(strs, m_strValues, boost::is_any_of("|"));
			for (std::string& fullcode : strs)
			{
				std::vector<std::string> strs1;
				boost::split(strs1, fullcode, boost::is_any_of("_"));
				this->m_ruleValues[atof(strs1[0].c_str())] = atof(strs1[1].c_str());
			}						
		}

		tickrule::~tickrule()
		{
		}

		double tickrule::tick_up(double price)
		{
			double limit = 0;
			double tick = 1;
			for (auto& iter : m_ruleValues)
			{
				if (price >= iter.first)
				{
					limit = iter.first;
					tick = iter.second;
				}
				else
					break;

			}
			return math2::floor_ex((price - limit) / tick)*tick + limit + tick;
		}
		double tickrule::tick_down(double price)
		{
			double limit = 0;
			double tick = 1;
			for (auto& iter : m_ruleValues)
			{
				if (price >= iter.first)
				{
					limit = iter.first;
					tick = iter.second;
				}
				else
					break;

			}
			double temp = math2::floor_ex((price - limit) / tick) * tick + limit;
			if (temp < price)
				return temp;
			return temp - tick;
		}

	}
}
