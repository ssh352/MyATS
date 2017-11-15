#ifndef __GX_CONNECTION_H__
#define __GX_CONNECTION_H__

#if 0 //def _WIN32
#include <afxwin.h>
#endif

#include "gx_order_aux.h"
//#include "SecurityFtdcTraderApi.h"
//#define POLL_NUM 10
//#define LTS_CONNECTION_THREADED
#include <ctpbase_connection.h>
#include "lockfree_classpool_workqueue.h"
#include "gx_quant_proxy.h"

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;

namespace gx
{
	class user_info
	{
	public:
		string clientID;
		string userID;
		int    way;
	};
	class gx_connection : public ctpbase_connection
	{
	public:

		typedef terra::common::lockfree_classpool_workqueue<user_info> gx_user_info_inbound_queue;
		typedef terra::common::LockFreeWorkQueue<string>         gx_pdu_inbound_queue;
	
		gx_connection(bool checkSecurities = true);
		virtual ~gx_connection();

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

#if 0
		void OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pOrder, int errorId);
		void OnRtnOrderAsync(CSecurityFtdcOrderField* pOrder);
		void OnRtnTradeAsync(CSecurityFtdcTradeField* pTrade);
		void OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pOrder, int errorId);
#else
		void onRtnOrder(string * pdu);
		void OnRspOrderInsertAsync(int id);
		void OnRtnOrderAsync(string * pdu);
		void OnRtnTradeAsync(int id,const string & instrumentID, int quantity, double price, const string & tradeID, const string & tradeDate, const string & tradeTime);
		void OnRspOrderActionAsync(string * pdu);
#endif

		void OnRspQryInvestorPosition(const string & instrumentID, int quantity);

		void OnUserInfoAsync(user_info* pInfo);//Òì²½¼ÇÂ¼userinfo



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
		void create_user_info(const string & clientID,const string & userID,int way);		
		void append(user_info * info);
		string get_user_info_ex(const string & clientID);
		int get_way(const string & clientID);
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
		bool m_bAutoDetectCoveredSell;
		bool m_blts_wrapper = false;
		std::string m_str_ltsSrv;

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
		gx_quant_proxy * m_quant_proxy;
		std::string       m_sStockAccountName;		
		tbb::concurrent_unordered_map<string, user_info*> m_user_info_map;
		tbb::concurrent_unordered_map<int,string> m_order_id_map;
#if 1
		gx_user_info_inbound_queue m_userInfoQueue;
		gx_pdu_inbound_queue       m_pdu_queue;
#endif
		string m_user_info_file_name;
#endif			
		bool m_bIsDicoRdy = false;
		friend class gx_order_aux;
	};
}

#endif // __GX_CONNECTION_H__

