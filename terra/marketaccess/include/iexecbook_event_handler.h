#ifndef __IEXECBOOK_EVENT_HANDLER2_H__
#define __IEXECBOOK_EVENT_HANDLER2_H__

#include "exec.h"
#include "order.h"
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class iexecbook_event_handler
			{
			public:
				virtual void add_exec_cb(exec* pExec) = 0;
				virtual void add_exec_cb(exec* pExec, order* o) = 0;
			};
		}
	}
}
#endif // __IEXECBOOK_EVENT_HANDLER_H__

