#ifndef __MANAGED_ORDERPASSING_H__	
#define __MANAGED_ORDERPASSING_H__
#include <boost/atomic.hpp>
#include "singleton.hpp"
#include <boost/filesystem.hpp>
#include <thread>
#include "iconnection_status_event_handler.h"
#include "connection.h"
#include "connection_gh.h"

#include "iorderbook_event_handler.h"
#include "iexecbook_event_handler.h"
#include "iorderobserver.h"

#include "order.h"
#include "exec.h"


using namespace terra::common;
using namespace terra::marketaccess::orderpassing;

namespace terra
{
	namespace ats
	{
		class connection_status_CB : public iconnection_status_event_handler
		{
		public:
			static connection_status_CB* instance();
			connection_status_CB();
			virtual ~connection_status_CB() { if (ms_pInstance != NULL) delete ms_pInstance; }

			virtual void connection_status_cb(connection* c, ConnectionStatus::type newStatus, const char* pszMessage);
		private:
			static connection_status_CB* ms_pInstance;

		};

		class connection_CB : public iorderbook_event_handler, public iexecbook_event_handler, public iconnection_event_handler, public iquotebook_event_handler
		{
		public:
			static connection_CB* instance();
			connection_CB();
			virtual ~connection_CB() { if (ms_pInstance != NULL) delete ms_pInstance; }

			virtual void add_order_cb(order* o);
			virtual void update_order_cb(order* o);
			virtual void add_exec_cb(exec* e);
			virtual void add_exec_cb(exec* pExec, order* o);
			virtual void new_instrument_cb(void* con, tradeitem* i);
			virtual void update_tradingaccount_cb(tradingaccount* ta);

			virtual void add_quote_cb(quote* o);

			virtual void update_quote_cb(quote* o);

			virtual void set_quote_sys_id_cb(tradeitem* i, const string & id);

		private:
			static connection_CB* ms_pInstance;
		};

		class managed_orderpassing :public SingletonBase<managed_orderpassing>
		{
		public:
			managed_orderpassing();
			~managed_orderpassing();
			int Initialize(const std::string& connectionFile/*, const  std::string& logFile*/, const std::string& todayDir);
			int Initialize(const std::string& connectionFile/*, const std::string& logFile*/, const std::string& todayDir, const std::string& db);
			void Terminate();
			

			void AddConnectionEventHandler(connection*, connection_CB* handler);
			void AddConnectionStatusEventHandler(connection*, connection_status_CB* handler);

		private:
//			RTMutex* m_mutex;
			//bool init_logs(const char* pszLogFile);
			bool m_isAlive;
			void Process();
			
			std::thread m_Thread;
		};



		class order_observer_CB : public iorderobserver
		{
		public:
			static order_observer_CB* instance();
			order_observer_CB();
			virtual ~order_observer_CB() { if (ms_pInstance != NULL) delete ms_pInstance; }

			virtual void add_order_cb(order* pOrder);
			virtual void update_order_cb(order* pOrder);
			virtual void inactive_order_cb(order* pOrder);

			virtual void add_exec_cb(exec* pExec);

			virtual void clear_orders();
			virtual void clear_execs();
		private:
			static order_observer_CB* ms_pInstance;
		};

	}

}

#endif //__MANAGED_ORDERPASSING_H__	