#ifndef __QUANTITY_SECURITY_RULE2_H__
#define __QUANTITY_SECURITY_RULE2_H__

#include <string>
#include "iorder_security_rule.h"
#include "feeditem.h"
namespace terra
{
	namespace ats
	{
		namespace security
		{
			class quantity_security_rule : public iorder_security_rule
			{
			public:
				quantity_security_rule(const char* mnemo, AtsType::InstrType::type t, int maxQuantity);
				virtual ~quantity_security_rule() {}

				virtual const char* get_description() { return m_description.c_str(); }

				virtual bool is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason);


			protected:
				int m_maxQuantity;
				std::string m_description;
			};
		}
	}
}
#endif // __QUANTITY_SECURITY_RULE_H__
