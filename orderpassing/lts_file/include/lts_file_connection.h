#ifndef __LTS_CONNECTION_FILE_H__
#define __LTS_CONNECTION_FILE_H__

#if 0 //def _WIN32
#include <afxwin.h>
#endif

#include "lts_file_trdapi.h"
#include "lts_file_reqapi.h"
#include "lts_file_order_aux.h"
#include "SecurityFtdcTraderApi.h"
//#define POLL_NUM 10
//#define LTS_CONNECTION_THREADED
#include <ctpbase_connection.h>

#include "quant_proxy.h"

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;

namespace lts_file
{
	class user_info
	{
	public:
		int    OrderID=-1;
		int    VolumeTotalOriginal=-1;
		double LimitPrice=0;
		//新版
		string UserID;
		int    OrderRef=-1;//本地没有维护，由服务器维护
	};
	class lts_file_connection : public ctpbase_connection

	{

	public:

		terra::common::SingleLockFreeClassPool<CSecurityFtdcInputOrderField> xs_create_pool;
		terra::common::SingleLockFreeClassPool<CSecurityFtdcInputOrderActionField> xs_cancel_pool;

		typedef terra::common::lockfree_classpool_workqueue<user_info> lts_user_info_inbound_queue;
	
		lts_file_connection(bool checkSecurities = true);
		virtual ~lts_file_connection();


		bool get_both_status() { return  (m_pltsReqApi->get_status() && m_pltsTrdApi->get_status()); }
		bool get_req_status() { return m_pltsReqApi->get_status(); }


		// connection methods
		virtual void init_connection();
		virtual void release();

		void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
		void request_investor_full_positions();

		void request_instruments();
		virtual void request_trading_account();

		//virtual void process_idle();

		virtual void connect();
		virtual void disconnect();
		//order* create_order(tradeitem *instrument, OrderWay::type way, int qty, double price);

		bool connect_req_api() { return m_pltsReqApi->connect(); }
		bool connect_trd_api() { return m_pltsTrdApi->connect(); }
		bool disconnect_reqApi() { return m_pltsReqApi->disconnect(); }

		void init_req_api()
		{
			m_pltsReqApi->init();
		}
		void init_trd_api()
		{
			m_pltsTrdApi->init();
		}


		OrderOpenClose::type compute_open_close(order* ord, bool hasCloseToday = false) override;



		void treat_freeze_order(terra::marketaccess::orderpassing::order* ord);

		void OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pOrder, int errorId);
		void OnRtnOrderAsync(CSecurityFtdcOrderField* pOrder);
		void OnRtnTradeAsync(CSecurityFtdcTradeField* pTrade);
		void OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pOrder, int errorId);
		void OnUserInfoAsync(user_info* pInfo);//异步记录userinfo



		bool isDicoReady(){ return m_bIsDicoRdy; }


		int market_create_order_async(order* o, char* pszReason) override;
		int market_cancel_order_async(order* o, char* pszReason) override;

#if 0 //def _WIN32
		void set_HWND(HWND hwnd){ m_hwnd = hwnd; }
		void post_keyboard_msg();
#else
		string & get_stock_account_name(){ return m_sStockAccountName; }
		void init_user_info(char * user_info_file);
		virtual void process_idle() { return; }
		void create_user_info(order * o);		
		//
		void update_user_info(int orderId,string userID,int orderRef);
		string get_user_id(int orderRef);
		//
		void append(user_info * info);
		void get_user_info_ex(order * o);
#endif
	protected:

		virtual bool init_config(const std::string &name, const std::string &ini) override;
		//virtual order* create_order() { return new lts_order(this); }

		void process() override;
		
		virtual void cancel_num_warning(tradeitem* i) override;
		virtual void cancel_num_ban(tradeitem* i) override;
		
	protected:
		

		//std::thread m_thread;

		//boost::asio::io_service io;
		//void Process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
		//void set_kernel_timer_thread();


	private:


		lts_file_trdapi* m_pltsTrdApi;
		lts_file_reqapi* m_pltsReqApi;


		bool m_bAutoDetectCoveredSell;
		bool m_blts_wrapper = false;
		std::string m_str_ltsSrv;
		CSecurityFtdcTraderApi* pAPI;



		std::string m_sHostnameQry;
		std::string m_sServiceQry;
		std::string m_sPasswordQry;

		std::string m_sHostnameTrd;
		std::string m_sServiceTrd;
		std::string m_sPasswordTrd;

		std::string m_sProductInfo;
		std::string m_sAuthCode;


		std::unordered_map<int, order*> m_cancelOrdMap;

		std::vector<std::string> m_etfName;

		bool m_reqstatus;
		bool m_trdstatus;

#if 0 //def _WIN32
		HWND m_hwnd = NULL;
		//BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam);
#else
		lts_quant_proxy * m_quant_proxy;
		std::string       m_sStockAccountName;		
		tbb::concurrent_unordered_map<int, user_info*> m_user_info_map;
		//
		tbb::concurrent_unordered_map<int, user_info*> m_order_ref_map;
		//
		lts_user_info_inbound_queue m_userInfoQueue;
		string m_user_info_file_name;
#endif



		bool m_bIsDicoRdy = false;


		friend class lts_file_trdapi;
		friend class lts_file_reqapi;
		friend class lts_file_order_aux;

	};
}

#endif // __LTS_CONNECTION_H__

