#ifndef __FEMAS_ORDER_H_OP_
#define __FEMAS_ORDER_H_OP_
#include <string>
#include "order.h"
#include "USTPFtdcUserApiStruct.h"
using namespace terra::marketaccess::orderpassing;
namespace femas
{
   class femas_connection;
   class femas_order_aux
   {
   public:
	   femas_order_aux(){}
	   ~femas_order_aux(){}

	  static order* anchor(femas_connection* pConnection, CUstpFtdcInputOrderField* pOrder);
      static order* anchor(femas_connection* pConnection, CUstpFtdcOrderField* pOrder);
      static order* anchor(femas_connection* pConnection, CUstpFtdcTradeField* pTrade);

	  // get/set
	  static std::string get_user_ord_local_id(order* o) { return o->custome_strings[0]; }
	  static void set_user_ord_local_id(order* o, std::string localId) { o->custome_strings[0] = localId; }

	  static std::string get_ord_local_id(order* o) { return o->custome_strings[1]; }
	  static void set_ord_local_id(order* o, std::string localId) { o->custome_strings[1] = localId; }	 	  

	  static std::string get_order_sys_id(order* o) {return o->custome_strings[2];}
	  static void set_order_sys_id(order* o, std::string sysId) { o->custome_strings[2] = sysId; }

   private:
      friend class femas_connection;
   };
}
#endif // __FEMAS_ORDER_H_OP_

