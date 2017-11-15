#ifndef __XS_OF_ORDER2_H__
#define __XS_OF_ORDER2_H__

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include <string>
#include "order.h"
#include "quote.h"

#include <DFITCApiStruct.h>

//using namespace toolkit::common;
using namespace terra::marketaccess::orderpassing;

namespace xs_of
{
   class xs_of_connection;

   class xs_of_order_aux
   {
   public:
      //xs_of_order(xs_of_connection* pConnection);
      //virtual ~xs_of_order() {}
	   xs_of_order_aux(){};
	   ~xs_of_order_aux(){};

	  static order* anchor(xs_of_connection* pConnection, DFITCOrderRtnField* pOrder);
	  static order* anchor(xs_of_connection* pConnection, DFITCMatchRtnField* pOrder);
	  static order* anchor(xs_of_connection* pConnection, DFITCOrderCommRtnField* pOrder);
	  static order* anchor(xs_of_connection* pConnection, DFITCMatchedRtnField* pOrder);

	  static quote* anchor(xs_of_connection* pConnection, DFITCQuoteRtnField* pQuote);
	  //static quote* anchor(cffex_connection* pConnection, CThostFtdcQuoteField* pQuote);

	  static void set_locId(order * o, int value){ o->custome_ints[0] = value; }
	  static int get_locId(order * o){ return o->custome_ints[0]; }
	  static void set_spdId(order * o, int value){ o->custome_ints[1] = value; }
	  static int get_spdId(order * o){ return o->custome_ints[1]; }

	  static void set_spdId(quote * q, int value){ q->custome_ints[1] = value; }
	  static int get_spdId(quote * q){ return q->custome_ints[1]; }
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
	  //

	  //ptime& get_rebuild_time() { return m_rebuildTime; }
	  //void set_rebuild_time(ptime l) { m_rebuildTime = l; }

	 /* const std::chrono::system_clock::time_point& get_rebuild_time_point() const { return m_rebuildTimePoint; }
	  void set_rebuild_time_point(const std::chrono::system_clock::time_point& tp) { m_rebuildTimePoint = tp; }*/

   private:
      

	  //ptime m_rebuildTime;
	  //std::chrono::system_clock::time_point m_rebuildTimePoint;

   private:
      friend class xs_of_connection;
   };
}

#endif // __XS_ORDER_H__

