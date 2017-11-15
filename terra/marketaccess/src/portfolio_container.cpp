#include "portfolio_container.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			bool portfolio_container::add(portfolio* pPortfolio)
			{
				writeLock wlock(m_rwmutex);
				if (pPortfolio == NULL)
					return false;

				if (m_ptfByName.find(pPortfolio->get_name()) != m_ptfByName.end())
					return false;

				m_ptfByName[pPortfolio->get_name()] = pPortfolio;

				return true;
			}

			bool portfolio_container::remove(portfolio* pPortfolio)
			{
				writeLock wlock(m_rwmutex);
				if (pPortfolio == NULL)
					return false;

				if (m_ptfByName.find(pPortfolio->get_name()) != m_ptfByName.end())
				{
					m_ptfByName.erase(pPortfolio->get_name());
					return true;
				}
				return false;
			}

			bool portfolio_container::contains(portfolio* pPortfolio)
			{
				readLock rlock(m_rwmutex);
				if (pPortfolio == NULL)
					return false;

				return m_ptfByName.find(pPortfolio->get_name()) != m_ptfByName.end();
			}

			void portfolio_container::clear()
			{
				writeLock wlock(m_rwmutex);
				m_ptfByName.clear();
			}

			portfolio* portfolio_container::get_by_name(std::string name)
			{
				readLock rlock(m_rwmutex);
				portfolio_by_name::iterator it = m_ptfByName.find(name);
				return it != m_ptfByName.end() ? (*it).second : NULL;
			}
		}
	}

}