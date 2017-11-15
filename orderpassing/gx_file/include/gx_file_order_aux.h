#ifndef __GX_FILE_ORDER_FILE_H__
#define __GX_FILE_ORDER_FILE_H__

#include <string>
#include "order.h"

#define  FAKE_ID_MIN  100000000
using namespace terra::marketaccess::orderpassing;

namespace gx_file
{
	class gx_file_connection;

	class gx_file_order_aux
	{
	public:
		static order* anchor_order(gx_file_connection* pConnection, string line);
		static order* anchor_trade(gx_file_connection* pConnection, string line);

		static int get_order_local_id(order* o) { return o->custome_ints[0]; }
		static void set_order_local_id(order * o, int i) { o->custome_ints[0] = i; }			

	private:
		friend class gx_file_connection;
	};
}

#endif // __GX_FILE_ORDER_FILE_H__

