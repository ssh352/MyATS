#ifndef __CFFEX_ORDER_H__
#define __CFFEX_ORDER_H__

#include <string>
#include "order.h"
#include "quote.h"
#include <ThostFtdcUserApiStruct.h>


using namespace terra::marketaccess::orderpassing;

namespace cffex
{
   class cffex_connection;

   class cffex_order_aux
   {
   public:
      //cffex_order(cffex_connection* pConnection);
      //virtual ~cffex_order() {}
	   cffex_order_aux(){}
	   ~cffex_order_aux(){}


      static order* anchor(cffex_connection* pConnection, CThostFtdcInputOrderField* pOrder);
	  static order* anchor(cffex_connection* pConnection, CThostFtdcOrderField* pOrder);
	  static order* anchor(cffex_connection* pConnection, CThostFtdcTradeField* pTrade);
	  static order* anchor(cffex_connection* pConnection, CThostFtdcExecOrderField* pOrder);
	  static order* anchor(cffex_connection* pConnection, CThostFtdcInputExecOrderField* pOrder);

	  static quote* anchor(cffex_connection* pConnection, CThostFtdcInputQuoteField* pQuote);
	  static quote* anchor(cffex_connection* pConnection, CThostFtdcQuoteField* pQuote);

	  static int get_ord_ref(order* o) { return o->custome_ints[0]; }
	  static void set_ord_ref(order * o, int i) { o->custome_ints[0] = i; }


	  static string& get_order_sys_id(order * o) { return o->custome_strings[0]; }
	  static string& get_quote_sys_id(quote * q) { return q->custome_strings[0]; }
	  static void set_order_sys_id(order * o, const char* psz) { o->custome_strings[0] = psz; }
	  static void set_quote_sys_id(quote * q, const char* psz) { q->custome_strings[0] = psz; }


   private:
      //int m_ordRef;
      //std::string m_orderSysId;

      friend class cffex_connection;
   };
}

#endif // __CFFEX_ORDER_H__

