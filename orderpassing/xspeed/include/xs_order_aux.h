#ifndef __XS_ORDER2_H__
#define __XS_ORDER2_H__

#include <string>
#include "order.h"

#include <DFITCSECApiStruct.h>

//using namespace toolkit::common;
using namespace terra::marketaccess::orderpassing;

namespace xs
{
   class xs_connection;

   class xs_order_aux
   {
   public:
	   xs_order_aux(){}
	   ~xs_order_aux() {}


	  static order* anchor(xs_connection* pConnection, DFITCSOPEntrustOrderRtnField* pOrder);
	  static order* anchor(xs_connection* pConnection, DFITCSOPTradeRtnField* pOrder);
	  static order* anchor(xs_connection* pConnection, DFITCSOPWithdrawOrderRtnField* pOrder);

	  static order* anchor(xs_connection* pConnection, DFITCStockEntrustOrderRtnField* pOrder);
	  static order* anchor(xs_connection* pConnection, DFITCStockTradeRtnField* pOrder);
	  static void set_locId(order * o, int value){ o->custome_ints[0] = value; }
	  static int get_locId(order * o){ return o->custome_ints[0]; }
	  static void set_spdId(order * o, int value){ o->custome_ints[1] = value; }
	  static int get_spdId(order * o){ return o->custome_ints[1]; }

   private:
      friend class xs_connection;
   };
}

#endif // __XS_ORDER_H__

