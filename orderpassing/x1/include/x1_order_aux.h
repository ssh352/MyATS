#ifndef __X1_ORDER2_H__
#define __X1_ORDER2_H__

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include <string>
#include "order.h"
#include "quote.h"
#include "X1FtdcApiStruct.h"

//using namespace toolkit::common;
using namespace terra::marketaccess::orderpassing;

namespace x1
{
   class x1_connection;

   class x1_order_aux
   {
   public:
      //x1_order(x1_connection* pConnection);
      //virtual ~x1_order() {}
	   x1_order_aux(){};
	   ~x1_order_aux(){};

	   static order* anchor(x1_connection* pConnection, CX1FtdcRspPriOrderField* pOrder);
	   static order* anchor(x1_connection* pConnection, CX1FtdcRspPriCancelOrderField* pOrder);
	   static order* anchor(x1_connection* pConnection, CX1FtdcRspMatchField* pOrder);

	  static order* anchor(x1_connection* pConnection, CX1FtdcRspOrderField* pOrder);
	  static order* anchor(x1_connection* pConnection, CX1FtdcRspPriMatchInfoField* pOrder);

	  static quote* anchor(x1_connection* pConnection, CX1FtdcQuoteRtnField* pQuote);
	  static quote* anchor(x1_connection* pConnection, CX1FtdcQuoteMatchRtnField* pQuote);
	  static quote* anchor(x1_connection* pConnection, CX1FtdcQuoteCanceledRtnField* pQuote);

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
      friend class x1_connection;
   };
}

#endif // __XS_ORDER_H__

