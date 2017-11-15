#ifndef __ORDER_OBSERVER2_H__
#define __ORDER_OBSERVER2_H__

#include "order.h"
#include "exec.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class iorderobserver
			{
				
			public:
				iorderobserver(){};
				virtual ~iorderobserver(){};
				virtual void add_order_cb(order *order) = 0;
				virtual void update_order_cb(order *order) = 0;
				virtual void inactive_order_cb(order *order) = 0;

				virtual void add_exec_cb(exec *exec) = 0;

				virtual void clear_orders() = 0;
				virtual void clear_execs() = 0;
			};


			class iquoteobserver
			{

			public:
				iquoteobserver(){};
				virtual ~iquoteobserver(){};
				virtual void add_quote_cb(quote *quote) = 0;
				virtual void update_quote_cb(quote *quote) = 0;
				virtual void inactive_quote_cb(quote *quote) = 0;

				virtual void add_exec_cb(exec *exec) = 0;

				virtual void clear_quotes() = 0;
				virtual void clear_execs() = 0;
			};
		}
	}
}


#endif