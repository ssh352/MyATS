#ifndef __IORDERBOOK_EVENT_HANDLER2_H__
#define __IORDERBOOK_EVENT_HANDLER2_H__

#include "order.h"
//#include "tradingaccount.h"
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class iorderbook_event_handler
			{
			public:
				virtual void add_order_cb(order* o) = 0;
				virtual void update_order_cb(order* o) = 0;
				virtual void update_tradingaccount_cb(tradingaccount* ta) = 0;

				//virtual void add_inactive_order_cb(order* o) = 0;
			};

			class iquotebook_event_handler
			{
			public:
				virtual void add_quote_cb(quote* o) = 0;
				virtual void update_quote_cb(quote* o) = 0;
				

				//virtual void add_inactive_order_cb(order* o) = 0;
			};
		}
	}
}
#endif // __IORDERBOOK_EVENT_HANDLER_H__

