#ifndef __MAX_NOMINAL_SECURITY_RULE2_H__
#define __MAX_NOMINAL_SECURITY_RULE2_H__

#include <string>
#include "iorder_security_rule.h"
#include "feeditem.h"
namespace terra
{
	namespace ats
	{
		namespace security
		{
			class max_nominal_security_rule : public iorder_security_rule
			{
			public:
				max_nominal_security_rule(const char* mnemo, AtsType::InstrType::type t, double maxNominal);
				virtual ~max_nominal_security_rule() {}

				virtual const char* get_description() { return m_description.c_str(); }

				virtual bool is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason);


			protected:
				double m_maxNominal;
				//double m_forex;
				std::string m_description;
			};
		}
	}
}
#endif // __MAX_NOMINAL_SECURITY_RULE_H__
