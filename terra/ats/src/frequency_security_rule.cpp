#include "frequency_security_rule.h"
#include "terra_logger.h"
#include "boost/format.hpp"
using namespace terra::common;

namespace terra
{
	namespace ats
	{
		namespace security
		{
			frequency_security_rule::frequency_security_rule(const char* mnemo, AtsType::InstrType::type t, int maxNbOrder, int maxInterval)
			{
				m_maxInterval = maxInterval;
				m_list.assign(maxNbOrder, min_date_time);
				m_it = m_list.begin();

				m_description = (boost::format("max frequency for %s %c = %d order per %d sec") % mnemo %_InstrType_VALUES_TO_NAMES.at(t) % maxNbOrder %maxInterval).str();

				//m_lastError = time_value::zero;
			}

			bool frequency_security_rule::is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason)
			{
				//time_value now = time_value::gettimeofday();
				ptime now(microsec_clock::local_time());
				if (*m_it == min_date_time)
				{
					// not initialized yet -> accept order
					*m_it = now;
					incr_it();
					return true;
				}

				int gap = (now - *m_it).total_milliseconds();
				if (gap < m_maxInterval*1000 )
				{
					// dump error message 
					//if ((now - m_lastError).get_msec() > 200)
					//{
					//   m_lastError = now;
					sprintf(pszReason, "frequency_security_rule - %s max frequency reached: %d orders in %.3f secs",
						o->get_instrument()->getCode().data(),
						(int)m_list.size(),
						gap / 1000.0);
					//}
					return false;
				}

				// we did not exceed maxFrequency -> push current time and move to next element.
				*m_it = now;
				incr_it();
				return true;
			}

			void frequency_security_rule::incr_it()
			{
				m_it++;
				if (m_it == m_list.end())
					m_it = m_list.begin();
			}
		}
	}
}
