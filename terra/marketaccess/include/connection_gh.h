#ifndef __CONNECTION_GH_H__
#define __CONNECTION_GH_H__

#include "connection_container.h"
#include "singleton.hpp"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class connection_gh :public SingletonBase<connection_gh>
			{
			public:
				connection_container& container() { return m_container; }

			private:
				connection_container m_container;
			};
		}
	}
}
#endif // __CONNECTION_GH_H__
