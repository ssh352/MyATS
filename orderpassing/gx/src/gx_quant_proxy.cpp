#include "gx_quant_proxy.h"
#include "tlhelp32.h"
#include <stdio.h>
#include <string.h>
#include <WtsApi32.h>
#include <psapi.h>
#include "terra_logger.h"
#include "gx_connection.h"
#include "string_tokenizer.h"
#include "ELInterop.h"
#include <boost/locale.hpp>
using namespace terra::common;
namespace gx
{
	bool gx_Msg_Callback(const TCHAR * msg, void * obj_handle)
	{
		gx_quant_proxy * proxy = (gx_quant_proxy*)obj_handle;
		if (proxy != nullptr)
		{
			//wprintf(L"gx_Msg_Callback:%s\n", msg);
			std::string & str = boost::locale::conv::from_utf(msg, "GBK");
			if (proxy->m_con != nullptr)
			{
				proxy->m_con->onRtnOrder(new string(str));
			}
		}
		return true;
	}
	bool gx_quant_proxy::connect()
	{
		return ELInteropRegisterMsgCallback(gx_Msg_Callback, this);
	}
	bool gx_quant_proxy::disconnect()
	{		
		return ELInteropUnregisterMsgCallback();
	}
	gx_quant_proxy::~gx_quant_proxy()
	{
		ELInteropUnregisterMsgCallback();
	}
	/*
	TCHAR *cmd = TEXT("ACCOUNT=410001174092;ACTION=BUY;SYMBOL=000060.SZ;QUANTITY=100;TYPE=MARKET;DURATION=IC5");
	TCHAR clientID[1024];
	for (int i = 0; i < 1; i++)
	{
	ELInteropSendOrder(cmd, clientID, 1024);
	}
	*/
	bool gx_quant_proxy::ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description)
	{
		wstring direction = L"none";
		switch (way)
		{
		case OrderWay::Buy:
		{
			direction = L"BUY";		
			break;
		}
		case OrderWay::Sell:
		{
			direction = L"SELL";
			break;
		}
		default:
			break;
		}
		
		std::wstring account = boost::locale::conv::to_utf<wchar_t>(this->m_con->get_stock_account_name(), "gbk");
		std::wstring code    = boost::locale::conv::to_utf<wchar_t>(feedCode, "gbk");
		
		if (strncmp(feedCode.c_str(),"6",1)==0 || feedCode == "510050")
		{
			code = code + L".SH"; 
		}
		else
		{
			code = code + L".SZ";
		}

		char price_buf[64];
		memset(price_buf, 0, sizeof(price_buf));
		sprintf(price_buf, "%f", price);
		std::wstring p = boost::locale::conv::to_utf<wchar_t>(price_buf, "gbk");
	
		memset(m_buffer, 0, sizeof(m_buffer));
		wsprintf(m_buffer, L"ACCOUNT=%s;ACTION=%s;SYMBOL=%s;QUANTITY=%d;LIMITPRICE=%s;TYPE=LIMIT;DURATION=GFD", (LPWSTR)account.c_str(), (LPWSTR)direction.c_str(), (LPWSTR)code.c_str(), quantity, p.c_str());		
		//m_order_queue.push(m_buffer);
		
    	TCHAR clientID[1024];
		bool bRet = ELInteropSendOrder(m_buffer, clientID, 1024);
		wprintf(L"--%s,clientID:%s,bRet:%d\n",m_buffer,clientID,bRet);
		if (bRet == true)
		{
			std::string & client = boost::locale::conv::from_utf(clientID, "GBK");
			this->m_con->create_user_info(client, description,(int)way);
		}
		return bRet;
	}
	bool gx_quant_proxy::cancel(const string & clientID)
	{
		std::wstring p = boost::locale::conv::to_utf<wchar_t>(clientID.c_str(), "gbk");
		return ELInteropCancelOrder(p.c_str());		
	}	
	bool gx_quant_proxy::query(const string & clientID)
	{
		std::wstring p = boost::locale::conv::to_utf<wchar_t>(clientID.c_str(), "gbk");
		TCHAR result[1024];
		return ELInteropQueryOrderState(p.c_str(),result,1024);
	}
	bool gx_quant_proxy::query_position(const string & account,const string symbol)
	{
		std::wstring p = boost::locale::conv::to_utf<wchar_t>(account.c_str(), "gbk");		

		std::wstring q = boost::locale::conv::to_utf<wchar_t>(symbol.c_str(), "gbk");

		return ELInteropQueryPosition(p.c_str(),q.c_str());
	}
}
