#include "common.h"
#include <thread>
#include "AtsType_types.h"
#include "LockFreeWorkQueue.h"
#include "int_provider.h"
using namespace AtsType;
#pragma once
namespace lts_file
{
	class lts_file_connection;
	class quant_proxy
	{
	public:
		quant_proxy();
		virtual~quant_proxy();
	public:
		virtual bool connect(){ return false; }
		virtual bool disconnect(){ return false; }
		//开始启动数据收发
		virtual void start(){}
                virtual void stop(){}
	};

	class lts_quant_proxy : public quant_proxy
	{

	public:
		lts_quant_proxy()
		{
			m_hwnd = 0;
		}
		virtual~lts_quant_proxy(){}
	public:
		virtual bool connect();
		virtual bool disconnect();
		virtual void start();
                virtual void stop(); 
		lts_file_connection *m_con = nullptr;
		/*
		证券代码,证券名称,方向,数量,价格,备注
		*/
		bool ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description);
		/*撤单f2*/
		bool cancel(int orderRef);
		/*下单f2*/
		bool req_order_insert();
		/*成交回报f8*/
		bool req_trade();
		/*所有成交回报（F9）导出*/
		bool req_all_trades();
		bool rtn_all_trades();
		/*新增持仓资金导出（F10）*/
		bool req_account_position();
		string get_request_id();
		void init_user_info(char * user_info_file);
		/*撤单（F9）导出*/
		bool req_real_cancel_trade(int &n);
		bool rtn_real_cancel_trade(int &n);
	protected:
		void post_keyboard_msg();
		void read_rsp();
	protected:
		std::string m_strAppName;
		std::string m_strAppPath;
		int         m_hwnd;
		//char        m_buffer[256];
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_queue;
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_order_queue;
		std::string m_strOrderPath;
		terra::common::int_provider m_intProvider;
		string      intProvider_file;
	};
}
