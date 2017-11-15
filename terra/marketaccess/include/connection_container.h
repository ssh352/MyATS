#ifndef __CONNECTION_CONTAINER_H__
#define __CONNECTION_CONTAINER_H__

#include <vector>
#include "connection.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class connection_container
			{
			public:
				connection_container() {}
				virtual ~connection_container() {}


				bool add(connection* pConnection);
				bool remove(connection* pConnection);
				bool contains(connection* pConnection);
				void clear();

				connection* get_by_name(const char* name);


			
				const std::map<std::string, connection*>& get_map() { return m_connectionByName; }
				//


			private:
				typedef std::map<std::string, connection*> connection_by_name;
				connection_by_name m_connectionByName;
			};
		}
	}
}
#endif // __CONNECTION_CONTAINER_H__
