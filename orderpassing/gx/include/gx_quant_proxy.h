#ifndef _GX_QUANT_PROXY_H_
#define _GX_QUANT_PROXY_H_
#include "common.h"
#include <thread>
#include "AtsType_types.h"
#include "LockFreeWorkQueue.h"
#include "int_provider.h"
using namespace AtsType;
#pragma once
namespace gx
{
	class gx_connection;	
	class gx_quant_proxy
	{

	public:
		gx_quant_proxy()
		{
			
		}
		virtual~gx_quant_proxy();
	public:
		virtual bool connect();
		virtual bool disconnect();
		gx_connection *m_con = nullptr;
		/*
		֤ȯ����,֤ȯ����,����,����,�۸�,��ע
		*/
		bool ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description);
		/*����*/
		bool cancel(const string & clientID);	
		bool query(const string & clientID);
		bool query_position(const string & account,const string symbol="none");
	private:
		TCHAR m_buffer[1024];
	};
}
#endif //_GX_QUANT_PROXY_H_
