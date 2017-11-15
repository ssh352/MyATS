#include "common.h"
#include <thread>
#include "AtsType_types.h"
#include "lockfree_classpool_workqueue.h"
//#include "int_provider.h"
using namespace AtsType;
#pragma once
namespace ht_file
{
	class ht_file_connection;	
	typedef struct _result_orde
	{
		std::string file_name;
		std::string local_entrust_no;
		std::string batch_no;
		std::string entrust_no;
		std::string fund_account;
		std::string result;
		std::string remark;
	}result_orde;

	typedef struct _result_cancel
	{
		std::string file_name;
		std::string local_withdraw_no;
		std::string fund_account;
		std::string entrust_no;
		std::string entrust_no_old;
		std::string result;
		std::string remark;
	}result_cancel;

	typedef struct _result_trader
	{
		std::string id; 
		std::string date;
		std::string exchange_type; 
		std::string fund_account; 
		std::string stock_account; 
		std::string stock_code; 
		std::string entrust_bs; 
		std::string business_price; 
		std::string business_amount; 
		std::string business_time; 
		std::string real_type; 
		std::string real_status;
		std::string entrust_no; 
		std::string business_balance; 
		std::string stock_name; 
		std::string position_str; 
		std::string entrust_prop; 
		std::string business_no; 
		std::string serial_no; 
		std::string business_times; 
		std::string report_no; 
		std::string orig_order_id;
	}result_trader;

	class ht_quant_proxy
	{

	public:
		ht_quant_proxy()
		{
			m_hwnd = 0;
		}
		virtual~ht_quant_proxy(){}
	public:
		virtual bool connect();
		virtual bool disconnect();
		virtual void start();
        virtual void stop(); 
		ht_file_connection *m_con = nullptr;
		/*
		证券代码,证券名称,方向,数量,价格,备注
		*/
		bool ReqOrderInsert(int id,const string & code, const string & exchange, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description);
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
		int    get_local_entrust_no();
		void init_user_info(char * user_info_file);
	protected:
		void post_keyboard_msg();
		void read_rsp(bool isold=false);
		void read_cancel(bool isold = false);
		void read_trader(bool isold = false);
	protected:
		std::string m_strAppName;
		std::string m_strAppPath;
		int         m_hwnd;
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_queue;
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_order_queue;
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > m_cancel_queue;
		std::string m_strOrderPath;
		std::string m_strTraderPath;
		//terra::common::int_provider m_intProvider;
		string      intProvider_file;

		int read_line_order_rtn = 1;
		int read_line_cancel_rtn = 1;
		int read_line_trader_rtn = 1;
		bool is_restart = true;
	};


}
