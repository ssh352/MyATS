#ifndef __TRADEITEM_GH2_H__
#define __TRADEITEM_GH2_H__

#include "tradeItem_container.h"
#include "singleton.hpp"
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class tradeitem_gh :public SingletonBase<tradeitem_gh>
			{
			public:
				tradeitem_container& container() { return m_container; }

			private:
				tradeitem_container m_container;
			};
		}
	}
}
#endif // __INSTRUMENT_GH_H__
