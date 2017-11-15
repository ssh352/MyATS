#include "chief_exec_book.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			bool chief_exec_book::add(exec* e)
			{
				if (e == NULL)
					return false;
				m_chief_exe_book::accessor wa;

				if (m_container.find(wa,e->getReference()))
					return false;

				
				m_container.insert(wa, e->getReference());
				wa->second = e;
				return true;
			}

			bool chief_exec_book::remove(exec* e)
			{
				if (e == NULL)
					return false;

				m_container.erase(e->getReference());
				return true;
			}

			bool chief_exec_book::contains(const char* reference)
			{
				m_chief_exe_book::const_accessor ra;
				return m_container.find(ra,reference) ;
			}

			void chief_exec_book::clear()
			{
				m_container.clear();
			}

			int chief_exec_book::size()
			{
				return m_container.size();
			}

			exec* chief_exec_book::get_by_reference(string& reference)
			{
				m_chief_exe_book::const_accessor ra;
				bool it = m_container.find(ra,reference);
				return it == true ? (ra->second) : NULL;
			}
		}
	}
}