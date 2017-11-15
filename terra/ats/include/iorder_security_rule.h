#ifndef __IORDER_SECURITY_RULE2_H__
#define __IORDER_SECURITY_RULE2_H__

#include "order.h"
#include "feeditem.h"
using namespace terra::marketaccess::orderpassing;
namespace terra
{
	namespace ats
	{
		namespace security
		{
			class iorder_security_rule
			{
			public:
				virtual ~iorder_security_rule() {}

				virtual bool is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason) = 0;
				virtual const char* get_description() = 0;
			};
		}
	}
}
#endif // __IORDER_SECURITY_RULE_H__
