#ifndef __LTS_ORDER_FILE_H__
#define __LTS_ORDER_FILE_H__

#include <string>
#include "order.h"
#include "SecurityFtdcUserApiStruct.h"

#define  FAKE_ID_MIN  100000000
using namespace terra::marketaccess::orderpassing;
//_USING_LTS_OLD_
namespace lts_file
{
	class lts_file_connection;

	class lts_file_order_aux
	{
	public:
		//lts_order(lts_file_connection* pConnection);
		//virtual ~lts_order() {}


		static order* anchor(lts_file_connection* pConnection, CSecurityFtdcInputOrderField* pOrder);
		static order* anchor(lts_file_connection* pConnection, CSecurityFtdcOrderField* pOrder, bool isETFSotck = false);
		static order* anchor(lts_file_connection* pConnection, CSecurityFtdcTradeField* pTrade, bool isETFSotck = false);


		// get/set
		//int get_order_ref() { return m_orderRef; }
		//void set_order_ref(int i) { m_orderRef = i; }


		static int get_order_ref(order* o) { return o->custome_ints[0]; }
		static void set_order_ref(order * o, int i) { o->custome_ints[0] = i; }

		//const char* get_order_local_id() { return m_orderLocalId.c_str(); }
		//void set_order_local_id(const char* psz) { m_orderLocalId = psz; }

		//const char* get_trader_id() { return m_traderId.c_str(); }
		//void set_trader_id(const char* psz) { m_traderId = psz; }



		//const char* get_order_sys_id() { return m_orderSysId.c_str(); }
		//void set_order_sys_id(const char* psz) { m_orderSysId = psz; }


		static string& get_order_sys_id(order * o) { return o->custome_strings[0]; }
		static void set_order_sys_id(order * o, const char* psz) { o->custome_strings[0] = psz; }

		static string& get_order_local_id(order * o) { return o->custome_strings[1]; }
		static void set_order_local_id(order * o, const char* psz) { o->custome_strings[1] = psz; }

		static string& get_trader_id(order * o) { return o->custome_strings[2]; }
		static void set_trader_id(order * o, const char* psz) { o->custome_strings[2] = psz; }


		//ptime& get_rebuild_time() { return m_rebuildTime; }
		//void set_rebuild_time(ptime& v) { m_rebuildTime = v; }


		//bool get_IsETF() const{ return m_bisETF; }
		//void set_IsETF(bool isETF) { m_bisETF = isETF; }

		static bool get_IsETF(order * o) { return  o->custome_ints[1]==1; }
		static void set_IsETF(order* o, bool isETF) { isETF ? o->custome_ints[1] = 1 : o->custome_ints[1] = 0; }


	private:
		//int m_orderRef;

		//std::string m_orderLocalId; //mandatory for security
		//std::string m_traderId;     //mandatory for security
		//std::string m_orderSysId;
		//ptime m_rebuildTime;
		

		//bool m_bisETF = false;

	private:
		friend class lts_file_connection;
	};
}

#endif // __LTS_ORDER_H__

