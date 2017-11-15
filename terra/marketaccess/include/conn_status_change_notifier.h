#ifndef __CONNSTATUSNOTIFIER_H__
#define __CONNSTATUSNOTIFIER_H__

#include "connection.h"
#include "singleton.hpp"


namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			class conn_status_change_notifier :public SingletonBase<conn_status_change_notifier>
			{
			public:
				typedef std::function<void(connection *)> ConnStatusEventHandler;

				void on_status_changed(connection *con)
				{
					boost_read_lock rlock(m_rwmutex);
					
					for (auto& it : m_func)
					{
						it(con);
					}
				}

				void add(ConnStatusEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.push_back(handle);
				}

				//void insert(int i, ConnStatusEventHandler handle)
				//{
				//	boost_write_lock wlock(m_rwmutex);
				//	m_func.push_back(handle);
				//}

				//void del(int i)
				//{
				//	//boost_write_lock wlock(m_rwmutex);
				//	//m_func.erase(i);
				//}

				void clear_all()
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.clear();
				}

			private:
				int m_nOpenExec;
				int m_nCloseExec;
				std::list<ConnStatusEventHandler> m_func;
				boost::shared_mutex m_rwmutex;

			};

		}
	}
}


#endif