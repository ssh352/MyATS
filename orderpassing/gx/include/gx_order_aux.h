#ifndef __GX_ORDER_H__
#define __GX_ORDER_H__

#include <string>
#include "order.h"
//#include "SecurityFtdcUserApiStruct.h"


using namespace terra::marketaccess::orderpassing;
//_USING_LTS_OLD_
namespace gx
{
	class gx_connection;

	class gx_order_aux
	{
	public:
		static order* anchor(gx_connection* pConnection,const string & clientID,const string & instrumentID, int quantity, double price,const string & date);
	private:
		friend class gx_connection;
	};
}

#endif // __GX_ORDER_H__

