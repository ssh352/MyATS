#ifndef __OP_FS_ORDER_H__
#define __OP_FS_ORDER_H__

#include <string>
#include "order.h"

#include <SgitFtdcUserApiStruct.h>

//using namespace toolkit::common;
using namespace terra::marketaccess::orderpassing;

namespace fs
{
   class fs_connection;

   class fs_order_aux
   {
   public:
      /*fs_order(fs_connection* pConnection);
      virtual ~fs_order() {}*/


	  static order* anchor(fs_connection* pConnection, fstech::CThostFtdcInputOrderField* pOrder);
	  static order* anchor(fs_connection* pConnection, fstech::CThostFtdcOrderField* pOrder);
	  static order* anchor(fs_connection* pConnection, fstech::CThostFtdcTradeField* pTrade);

	  static int get_ord_ref(order* o) { return o->custome_ints[0]; }
	  static void set_ord_ref(order * o, int i) { o->custome_ints[0] = i; }


	  static string& get_order_sys_id(order * o) { return o->custome_strings[0]; }
	  static void set_order_sys_id(order * o, const char* psz) { o->custome_strings[0] = psz; }
      // get/set
      //int get_ord_ref() { return m_ordRef; }
      //void set_ord_ref(int i) { m_ordRef = i; }

	  //const std::string get_exchange_id()
	  //{
		 // std::string str;
		 // terra::instrument::financialinstrument *i = this->get_instrument()->getInstrument();
		 // if (i != nullptr)
		 // {
			//  //std::string  ex = i->m_strExchange;
			//  return i->get_exchange();
		 // }
		 // else
		 // {
			//  return str;
		 // }
	  //}// ->get_market()->get_name(); }
      //void set_exchange_id(const char* psz) { m_exchangeId = psz; }

      //const char* get_order_sys_id() { return m_orderSysId.c_str(); }
      //void set_order_sys_id(const char* psz) { m_orderSysId = psz; }


   private:
      //int m_ordRef;
      ////std::string m_exchangeId;
      //std::string m_orderSysId;


   private:
      friend class fs_connection;
   };
}

#endif // __OP_FS_ORDER_H__

