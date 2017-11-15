#ifndef __ORDERBOOKGH2_H__
#define __ORDERBOOKGH2_H__

#include "chief_order_book.h"
#include "chief_quote_book.h"
#include "singleton.hpp"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class order_gh :public SingletonBase<order_gh>
			{
			public:
				chief_order_book *GetBook(){ return &m_book; }
			private:
				chief_order_book m_book;
			};


			class quote_gh :public SingletonBase<quote_gh>
			{
			public:
				chief_quote_book *GetBook(){ return &m_book; }
			private:
				chief_quote_book m_book;
			};
		}
	}
}


#endif