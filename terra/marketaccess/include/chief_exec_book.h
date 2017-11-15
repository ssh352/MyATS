#ifndef TERRA_EXEC_BOOK2_H__
#define TERRA_EXEC_BOOK2_H__

#include <unordered_map>
#include "exec.h"

#include "tbb/concurrent_hash_map.h"


using namespace tbb;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class chief_exec_book
			{
				typedef tbb::concurrent_hash_map<std::string, exec *> m_chief_exe_book;
			public:
				chief_exec_book(){}
				~chief_exec_book(){}
				bool add(exec* e);
				bool remove(exec* e);
				bool contains(const char* reference);
				void clear();
				int size();

				exec* get_by_reference(string& reference);

				
				const tbb::concurrent_hash_map<std::string, exec *>& get_map() { return m_container; }
				//

			private:
				tbb::concurrent_hash_map<std::string, exec *>  m_container;
			};

		}
	}
}


#endif