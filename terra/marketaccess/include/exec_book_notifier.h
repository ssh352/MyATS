#ifndef __EXECBOOKNOTIFIER_H__
#define __EXECBOOKNOTIFIER_H__

#include "exec.h"
#include "singleton.hpp"



namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			class exec_book_notifier :public SingletonBase<exec_book_notifier>
			{
			public:
				typedef std::function<void(exec *)> ExecUpdateEventHandler;
				
				void add_exec_cb(exec *exec)
				{
					if (exec->getOpenClose() == AtsType::OrderOpenClose::Open)
						m_nOpenExec += exec->getQuantity();
					else
						m_nCloseExec += exec->getQuantity();

					boost_read_lock rlock(m_rwmutex);
					auto it = m_func.begin();
					for (; it != m_func.end(); ++it)
					{
						(*it)(exec);
					}
				}

				void setOpen(int value){ m_nOpenExec = value; }
				void setClose(int value){ m_nCloseExec = value; }
				int getOpen(){ return m_nOpenExec; }
				int getClose(){ return m_nCloseExec; }
				void add(ExecUpdateEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					//m_func[i] = handle;
					m_func.push_back(handle);
				}

				void insert(ExecUpdateEventHandler handle)
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.push_back(handle);
				}

				void del()
				{
					boost_write_lock wlock(m_rwmutex);
					//m_func.erase(i);
				}

				void clear_all()
				{
					boost_write_lock wlock(m_rwmutex);
					m_func.clear();
				}

			private:
				int m_nOpenExec;
				int m_nCloseExec;
				std::list<ExecUpdateEventHandler> m_func;
				boost::shared_mutex m_rwmutex;

			};

		}
	}
}


#endif