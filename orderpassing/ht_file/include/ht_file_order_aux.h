#ifndef __HT_ORDER_FILE_H__
#define __HT_ORDER_FILE_H__

#include <string>
#include "order.h"
#include "quant_proxy.h"
#define  FAKE_ID_MIN  100000000
using namespace terra::marketaccess::orderpassing;

namespace ht_file
{
	class ht_file_connection;

	class ht_file_order_aux
	{
	public:
	

		static void anchor(ht_file_connection* pConnection, result_orde* pOrder,std::string &path,std::list<order *> &mlist);
		static order* anchor(ht_file_connection* pConnection, result_cancel* pOrder, std::string &path){};
		static order* anchor(ht_file_connection* pConnection, result_trader* pTrade, std::string &path){};


		static int get_order_ref(order* o) { return o->custome_ints[0]; }
		static void set_order_ref(order * o, int i) { o->custome_ints[0] = i; }

		static string& get_order_sys_id(order * o) { return o->custome_strings[0]; }
		static void set_order_sys_id(order * o, const char* psz) { o->custome_strings[0] = psz; }

		static string& get_order_local_id(order * o) { return o->custome_strings[1]; }
		static void set_order_local_id(order * o, const char* psz) { o->custome_strings[1] = psz; }

		static string& get_trader_id(order * o) { return o->custome_strings[2]; }
		static void set_trader_id(order * o, const char* psz) { o->custome_strings[2] = psz; }

		static bool get_IsETF(order * o) { return  o->custome_ints[1]==1; }
		static void set_IsETF(order* o, bool isETF) { isETF ? o->custome_ints[1] = 1 : o->custome_ints[1] = 0; }

	private:
		friend class ht_file_connection;
	};
}

#endif // __HT_ORDER_H__

