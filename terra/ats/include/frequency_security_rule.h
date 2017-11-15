#ifndef __FREQUENCY_SECURITY_RULE2_H__
#define __FREQUENCY_SECURITY_RULE2_H__

#include <string>
#include <list>
#include "iorder_security_rule.h"
//#include "terra_time.h"
#include "feeditem.h"
using namespace terra::common;
namespace terra
{
	namespace ats
	{
		namespace security
		{
			class frequency_security_rule : public iorder_security_rule
			{
			public:
				frequency_security_rule(const char* mnemo, AtsType::InstrType::type t, int maxNbOrder, int maxInterval);
				virtual ~frequency_security_rule() {}

				virtual const char* get_description() { return m_description.c_str(); }

				virtual bool is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason);


			protected:
				void incr_it();


			protected:
				int m_maxInterval;
				std::list<ptime> m_list;
				std::list<ptime>::iterator m_it;

				std::string m_description;

				//time_value m_lastError;
			};
		}
	}
}
#endif // __FREQUENCY_SECURITY_RULE_H__
