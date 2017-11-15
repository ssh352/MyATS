#ifndef TERRA_QUOTE_BOOK2_H__
#define TERRA_QUOTE_BOOK2_H__

#include <vector>
#include <list>
#include "quote.h"
#include "terra_safe_tbb_hash_map.h"


using namespace tbb;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class chief_quote_book //:public iquotebook
			{
				typedef terra_safe_tbb_hash_map<int, quote *> m_chief_quote_book;
			public:
				chief_quote_book(){}
				virtual ~chief_quote_book(){}
				bool add(quote * quote);
				bool remove(quote * quote);
				bool contains(quote * quote);

				quote * get_by_id(int id);

				int count();

				int get_nb_open_quotes();

				void killall();
				void killall(std::string portfolio);
				void killall(std::string portfolio, int Type);
				void clear();
				void setquoteRecord(quote *quote)
				{
					m_quote = quote;
				}

				void get_all_quote(std::list<quote*> &mlist);

				quote *m_quote;
			private:
				m_chief_quote_book m_container;
			};

		}
	}
}


#endif