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
		证券代码,证券名称,方向,数量,价格,备注
		*/
		bool ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description);
		/*撤单*/
		bool cancel(const string & clientID);	
		bool query(const string & clientID);
		bool query_position(const string & account,const string symbol="none");
	private:
		TCHAR m_buffer[1024];
	};
}
#endif //_GX_QUANT_PROXY_H_
