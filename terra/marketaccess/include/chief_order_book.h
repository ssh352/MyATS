#ifndef TERRA_ORDER_BOOK2_H__
#define TERRA_ORDER_BOOK2_H__

//#include "iorderbook.h"
#include <list>
#include "order.h"
#include "terra_safe_tbb_hash_map.h"


using namespace tbb;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class chief_order_book //:public iorderbook
			{
				typedef terra_safe_tbb_hash_map<int, order *> m_chief_order_book;
			public:
				chief_order_book(){}
				virtual ~chief_order_book(){}
				bool add(order * order);
				bool remove(order * order);
				bool contains(order * order);

				order * get_by_id(int id);

				int count();

				int get_nb_open_orders();

				void killall();
				void killall(std::string portfolio);
				void killall(std::string portfolio, int Type);
				void clear();
				void setOrderRecord(order *order)
				{
					m_order = order;
				}

				void get_all_order(std::list<order*> &vec);

				order *m_order;
			private:
				m_chief_order_book m_container;
			};

		}
	}
}


#endif