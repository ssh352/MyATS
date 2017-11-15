#ifndef __OP_ES_ORDER_H__
#define __OP_ES_ORDER_H__

#include <string>
#include "order.h"
#include "quote.h"
#include "TapTradeAPIDataType.h"
using namespace terra::marketaccess::orderpassing;
#define  FAKE_ID_MIN  100000000
namespace es
{
   class es_connection;

   class es_order_aux
   {
   public:
	   static order* anchor(es_connection* pConnection, TapAPIOrderInfo * pField);
	   static order* anchor(es_connection* pConnection, TapAPIFillInfo  * pTrade);	
	   //to do ...
	   static quote* anchor_quote(es_connection* pConnection, TapAPIOrderInfo * pField);
	  /*
	  */
	  static string& get_order_no(order * o) { return o->custome_strings[0]; }
	  static void set_order_no(order * o, const string & psz) { o->custome_strings[0] = psz; }
	  //to do ...
	  static string& get_order_no(quote * o) { return o->custome_strings[0]; }
	  static void set_order_no(quote * o, const string & psz) { o->custome_strings[0] = psz; }
   private:
      friend class es_connection;
   };
}

#endif // __OP_ES_ORDER_H__

