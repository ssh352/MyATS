#ifndef __EXEC_GH2_H__
#define __EXEC_GH2_H__

#include "chief_exec_book.h"
#include "singleton.hpp"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			class exec_gh :public SingletonBase<exec_gh>
			{
			public:
				chief_exec_book &GetBook(){ return m_book; }
			private:
				chief_exec_book m_book;
			};

		}
	}
}


#endif