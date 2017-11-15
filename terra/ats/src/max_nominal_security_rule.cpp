#include "max_nominal_security_rule.h"
#include "order_security_controller.h"
#include <boost/format.hpp>
namespace terra
{
	namespace ats
	{
		namespace security
		{
			max_nominal_security_rule::max_nominal_security_rule(const char* mnemo, AtsType::InstrType::type t, double maxNominal)
			{
				m_maxNominal = maxNominal;
				//m_forex = 0;

				m_description = (boost::format("max nominal per order for %s %c = %.0f") %mnemo %_InstrType_VALUES_TO_NAMES.at(t) %maxNominal).str();
			}

			bool max_nominal_security_rule::is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason)
			{


				double nominal = o->get_quantity() * o->get_price() * o->get_instrument()->get_point_value() /** m_forex*/;
				if (nominal > m_maxNominal)
				{
					sprintf(pszReason, "nominal_security_rule - %s above max nominal : %.0f > %.0f",
						o->get_instrument()->getCode().data(),
						nominal,
						m_maxNominal);
					return false;
				}
				return true;
			}
		}
	}
}
