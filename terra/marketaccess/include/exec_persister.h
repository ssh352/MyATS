#ifndef __EXEC_PERSISTER2_H__
#define __EXEC_PERSISTER2_H__

#ifdef _WIN32
# include <Winsock2.h>
#endif


#include "exec.h"
#include "LockFreeWorkQueue.h"
#include "tbb/concurrent_vector.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class exec_persister
			{
			public:
				static exec_persister* instance();
				virtual ~exec_persister();

				bool load(std::string &connection_name);

				bool start();
				bool stop();

				void add_exec(exec* e);

				const char* get_directory_name() { return m_directoryName.c_str(); }
				void set_directory_name(const char* pszDirectoryName);

				bool is_alive();
				void is_alive(bool b);

				std::thread m_thread;
				tbb::concurrent_vector<std::string> m_exec_vec;
			protected:
				virtual void Process();


			private:
				exec_persister();

				void persist_exec(exec* e);
				void load_exec(exec* e);

				bool exec_to_string(exec* e, char* szLine);
				exec* string_to_exec(std::string &pszLine, std::string &connection_name);


			private:
				static exec_persister* ms_pInstance;

				std::string m_directoryName;
				std::string m_fullName;
				boost::filesystem::ofstream m_ofstream;

				terra::common::LockFreeWorkQueue<exec> m_queue;
				//std::map<std::string, exec*> m_execSerialized;
				//std::set<std::string> m_referencesSerialized;

				bool m_isAlive;
				
				boost::shared_mutex m_mutex;

				std::map<std::string, int>_OrderWay_name_to_value;
				//std::map<std::string, int>_TradingType_name_TO_value;
				std::map<std::string, int>_OrderOpenClose_name_TO_value;
			};
		}
	}
}
#endif // __EXEC_PERSISTER_H__

