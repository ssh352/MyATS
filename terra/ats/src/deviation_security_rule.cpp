#include "deviation_security_rule.h"

//#include "..\..\..\..\..\CppProjects\toolkit\common\include\fields.h"

#include <boost/foreach.hpp>
#include <boost/format.hpp>

namespace terra
{
	namespace ats
	{
		namespace security
		{
			deviation_security_rule::deviation_security_rule(const char* mnemo, AtsType::InstrType::type t, deviation_type deviationType)
			{
				m_deviationType = deviationType;

				m_description = (boost::format("max deviation for %s %c") %mnemo %_InstrType_VALUES_TO_NAMES.at(t)).str();

			}

			bool deviation_security_rule::is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason)
			{
				
				return true;
			}

			bool deviation_security_rule::compare_deviations(std::pair<double, double> first, std::pair<double, double> second)
			{
				return first.first < second.first;
			}

			void deviation_security_rule::add_deviation(double min, double step)
			{
				m_deviations.push_back(std::pair<double, double>(min, step));
				m_deviations.sort(compare_deviations);
			}

			double deviation_security_rule::get_deviation(double price)
			{
				//std::pair<double, double> p;
				for(auto &it:m_deviations)
				{
					if (price > it.first)
						return it.second;
				}
				return 0;
			}
		}
	}

}