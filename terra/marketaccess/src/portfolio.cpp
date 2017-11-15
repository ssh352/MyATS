#include "portfolio.h"
#include "position.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			position* portfolio::get_position(tradeitem* i)
			{
				auto p = m_container.find(i);
				if (p == m_container.end())
				{
					auto ptr = new position(i, m_name.c_str());
					m_container[i] = ptr;
					return ptr;
				}
				return p->second;
			}
		}
	}
}
