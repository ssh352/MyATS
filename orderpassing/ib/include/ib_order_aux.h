 #ifndef __IB_ORDER_H__
#define __IB_ORDER_H__

#include <string>
#include "order.h"

#if 0
#include "../api/Shared/Order.h"
#include "../api/Shared/Contract.h"
#include "../api/Shared/Execution.h"
#include "../api/Shared/OrderState.h"
#else
#include "Order.h"
#include "Contract.h"
#include "Execution.h"
#include "OrderState.h"
#endif
//#include "SecurityFtdcUserApiStruct.h"

//using namespace toolkit::common;
//using namespace toolkit::marketaccess;
using namespace terra::marketaccess::orderpassing;
using namespace AtsType;

namespace ib
{
   class ib_connection;

   class ib_order_aux 
   {
   public:
      //ib_order(ib_connection* pConnection);
	  //virtual ~ib_order() {}
	   ib_order_aux(){}
	   ~ib_order_aux(){}
	  static order* anchor(ib_connection* pConnection, const Contract& contract, const Order& Ord, const OrderState& OrdStatus);
	  static order* anchor(ib_connection* pConnection, const Contract& contract, const Execution& execution);
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
	  //}
   
   private:
      friend class ib_connection;
   };
}

#endif // __IB_ORDER_H__

