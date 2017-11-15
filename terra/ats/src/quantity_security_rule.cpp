#include "quantity_security_rule.h"
#include <boost/format.hpp> 
namespace terra
{
	namespace ats
	{
		namespace security
		{
			quantity_security_rule::quantity_security_rule(const char* mnemo, AtsType::InstrType::type t, int maxQuantity)
			{
				m_maxQuantity = maxQuantity;
				
				m_description = (boost::format("max quantity per order for %s %c = %d") % mnemo %_InstrType_VALUES_TO_NAMES.at(t) %maxQuantity).str();
			}

			bool quantity_security_rule::is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason)
			{
				if (o->get_quantity() > m_maxQuantity)
				{
					sprintf(pszReason, "quantity_security_rule - %s max quantity %d > %d", o->get_instrument()->getCode().data(), o->get_quantity(), m_maxQuantity);
					return false;
				}
				return true;
			}
		}
	}
}
