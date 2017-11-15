#ifndef __PORTFOLIO2_H__
#define __PORTFOLIO2_H__

#include <map>
#include <string>
#include "tradeitem.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class position;

			class portfolio
			{
			public:
				portfolio(const char* pszName) : m_name(pszName) {}
				virtual ~portfolio() { /* position created to delete !!!*/ }


				const char* get_name() { return m_name.c_str(); }

				position* get_position(tradeitem* i);
				const std::map<tradeitem*, position*>& get_postion_by_instruments() { return m_container; }

			private:
				typedef std::map<tradeitem*, position*> position_container;

				std::string m_name;
				position_container m_container;
			};
		}
	}
}
#endif // __PORTFOLIO_H__
