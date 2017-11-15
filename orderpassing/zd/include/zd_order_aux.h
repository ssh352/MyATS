#ifndef __ZD_ORDER_H__
#define __ZD_ORDER_H__

#include <string>
#include "order.h"
//#include "quote.h"
#include <ShZdFutureUserApiStruct.h>

using namespace terra::marketaccess::orderpassing;

#define  FAKE_ID_MIN  100000000

namespace zd
{
   class zd_connection;

   class zd_order_aux
   {
   public:
       zd_order_aux(){}
	   ~zd_order_aux(){}	   
	   static order* anchor(zd_connection* pConnection, CTShZdInputOrderField* pOrder);
	   static order* anchor(zd_connection* pConnection, CTShZdOrderField* pOrder);
	   static order* anchor(zd_connection* pConnection, CTShZdTradeField* pTrade);
	  //static order* anchor(cffex_connection* pConnection, CThostFtdcExecOrderField* pOrder);
	  //static order* anchor(cffex_connection* pConnection, CThostFtdcInputExecOrderField* pOrder);

	  //static quote* anchor(cffex_connection* pConnection, CThostFtdcInputQuoteField* pQuote);
	  //static quote* anchor(cffex_connection* pConnection, CThostFtdcQuoteField* pQuote);

	  static int get_ord_ref(order* o) { return o->custome_ints[0]; }
	  static void set_ord_ref(order * o, int i) { o->custome_ints[0] = i; }


	  static string& get_order_sys_id(order * o) { return o->custome_strings[0]; }
	  //static string& get_quote_sys_id(quote * q) { return q->custome_strings[0]; }
	  static void set_order_sys_id(order * o, const char* psz) { o->custome_strings[0] = psz; }
	  //static void set_quote_sys_id(quote * q, const char* psz) { q->custome_strings[0] = psz; }
	  
   private:      
      friend class zd_connection;
   };
}

#endif // __ZD_ORDER_H__

