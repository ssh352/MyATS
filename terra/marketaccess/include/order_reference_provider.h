#ifndef __ORDERREFERENCEPROVIDER2_H__
#define __ORDERREFERENCEPROVIDER2_H__

#include "singleton.hpp"
#include "int_provider.h"
using namespace terra::common;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class order_reference_provider :public SingletonBase<order_reference_provider>
			{
			public:
				bool initialize(const char* filename, int startInt);
				void close();

				void set_current_int(int n);
				int get_current_int();
				int get_next_int();


			private:
				terra::common::int_provider m_intProvider;
			};
		}
	}
}

#endif // __ORDERREFERENCEPROVIDER_H__
