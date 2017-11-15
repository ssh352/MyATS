#ifndef __OP_SL_ORDER_H__
#define __OP_SL_ORDER_H__

#include <string>
#include "order.h"
#include "EesTraderDefine.h"
//using namespace toolkit::common;
using namespace terra::marketaccess::orderpassing;
#define  FAKE_ID_MIN  100000000
namespace sl
{
   class sl_connection;

   class sl_order_aux
   {
   public:
	   
#if 1
	  static order* anchor(sl_connection* pConnection, EES_OrderAcceptField* pField);
	  static order* anchor(sl_connection* pConnection, EES_OrderMarketAcceptField* pField);	  
	  static order* anchor(sl_connection* pConnection, EES_OrderExecutionField* pExec);
#endif
	  static order* anchor(sl_connection* pConnection, EES_QueryAccountOrder* pQueryOrder);
	  static order* anchor(sl_connection* pConnection, EES_QueryOrderExecution* pQueryOrderExec);
	  /*
	  */
	  static int get_client_token(order* o) { return o->custome_ints[0]; }
	  static void set_client_token(order * o, int i) { o->custome_ints[0] = i; }

	  static int get_pre_book_qty(order * o){ return o->custome_ints[1]; }
	  static void set_pre_book_qty(order * o, int i){ o->custome_ints[1] = i; }
	  /*
	  */
	  static string& get_market_token(order * o) { return o->custome_strings[0]; }
	  static void set_market_token(order * o, const string & psz) { o->custome_strings[0] = psz; }

   private:
      friend class sl_connection;
   };
}

#endif // __OP_SL_ORDER_H__

