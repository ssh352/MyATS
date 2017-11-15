#include "common.h"
#include <thread>
#include "AtsType_types.h"
#include "lockfree_classpool_workqueue.h"
#include "int_provider.h"
using namespace AtsType;
#pragma once
namespace gx_file
{
	class gx_file_connection;	
	class gx_file_quant_proxy
	{

	public:
		gx_file_quant_proxy()
		{
			m_hwnd = 0;
		}
		virtual~gx_file_quant_proxy(){}
	public:
		virtual bool connect();
		virtual bool disconnect();
		virtual void start();
        virtual void stop(); 
		gx_file_connection *m_con = nullptr;
		/*
		证券代码,证券名称,方向,数量,价格,备注
		*/
		int ReqOrderInsert(const string & code, const string & exchange, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description);
		/*撤单f2*/
		bool cancel(int orderRef);
		/*下单f2*/
		bool req_order_insert();						
		string get_request_id();
		int    get_local_entrust_no();
		void init_user_info(char * user_info_file);
		/*撤单（F9）导出*/
		bool req_real_cancel_trade();
		bool rtn_real_cancel_trade();
		//
		bool req_real_trade(int &n);
		bool req_account_position();
		//
	protected:
		void post_keyboard_msg();		
	protected:
		std::string m_strAppName;		
		int         m_hwnd;
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_queue;
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_order_queue;
		//根据国信泰九的sendOrd-sdfx,下单文件路径是一样的
		std::string m_strOrderPath;
		terra::common::int_provider m_intProvider;
		string      intProvider_file;
		//
		std::vector<string>  m_ack_line_vector;
		std::vector<string>  m_position_line_vector;
		//
	};
}
