#ifndef __CONNECTION_STATUS_EVENT_HANDLER2_H__
#define __CONNECTION_STATUS_EVENT_HANDLER2_H__

#include "connection.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class iconnection_status_event_handler
			{
			public:
				virtual void connection_status_cb(connection* c, AtsType::ConnectionStatus::type newStatus, const char* pszReason = "") = 0;
			};
		}
	}
}
#endif // __CONNECTION_STATUS_EVENT_HANDLER_H__

