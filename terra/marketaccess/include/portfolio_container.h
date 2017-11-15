#ifndef __PORTFOLIO_CONTAINER2_H__
#define __PORTFOLIO_CONTAINER2_H__

#include <unordered_map>
#include <string>
#include "portfolio.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
typedef boost::shared_lock<boost::shared_mutex> readLock;
typedef boost::unique_lock<boost::shared_mutex> writeLock;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class portfolio_container
			{
			public:
				portfolio_container() {}
				virtual ~portfolio_container() {}

				bool add(portfolio* pPortfolio);
				bool remove(portfolio* pPortfolio);
				bool contains(portfolio* pPortfolio);
				void clear();

				portfolio* get_by_name(std::string name);

				const std::unordered_map<std::string, portfolio*>& get_porfolio_by_name() { return m_ptfByName; }


			protected:
				typedef std::unordered_map<std::string, portfolio*> portfolio_by_name;
				portfolio_by_name m_ptfByName;
				boost::shared_mutex m_rwmutex;
			};
		}
	}
}
#endif // __PORTFOLIO_CONTAINER_H__
