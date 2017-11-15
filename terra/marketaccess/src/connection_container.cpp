#include "connection_container.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			bool connection_container::add(connection* pConnection)
			{
				if (pConnection == NULL)
					return false;

				if (m_connectionByName.find(pConnection->getName()) != m_connectionByName.end())
				{
					loggerv2::error("connection_container::add - name [%s] already exist", pConnection->getName().c_str());
					return false;
				}

				m_connectionByName[pConnection->getName()] = pConnection;

				return true;
			}

			bool connection_container::remove(connection* pConnection)
			{
				if (pConnection == NULL)
					return false;

				if (m_connectionByName.find(pConnection->getName()) != m_connectionByName.end())
				{
					m_connectionByName.erase(pConnection->getName());
					return true;
				}
				return false;
			}

			bool connection_container::contains(connection* pConnection)
			{
				if (pConnection == NULL)
					return false;

				return m_connectionByName.find(pConnection->getName()) != m_connectionByName.end();
			}

			void connection_container::clear()
			{
				m_connectionByName.clear();
			}

			connection* connection_container::get_by_name(const char* name)
			{
				connection_by_name::iterator it = m_connectionByName.find(name);
				return it != m_connectionByName.end() ? (*it).second : NULL;
			}
		}
	}
}
