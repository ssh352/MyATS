#ifndef __PORTFOLIO_GH2_H__
#define __PORTFOLIO_GH2_H__

#include "portfolio_container.h"
#include "singleton.hpp"
#include <unordered_map>
#include <string>
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class portfolio_gh :public SingletonBase<portfolio_gh>
			{

			public:
				portfolio_container& container() { return m_container; }
				std::unordered_map<string, std::unordered_map<string, string>>* get_postion_external(){ return &m_postion_external; }
			private:
				portfolio_container m_container;
				std::unordered_map<string, std::unordered_map<string, string>> m_postion_external;//<code,<porfolio,position>>
			};
		}
	}
}
#endif // __PORTFOLIO_GH_H__
