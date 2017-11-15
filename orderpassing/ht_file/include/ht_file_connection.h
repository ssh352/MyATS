#ifndef __HT_CONNECTION_FILE_H__
#define __HT_CONNECTION_FILE_H__


#include "ht_file_order_aux.h"
#include <ctpbase_connection.h>
#include "quant_proxy.h"
#include "tbb/concurrent_hash_map.h"
using namespace terra::marketaccess::orderpassing;
using namespace AtsType;

namespace ht_file
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

	class ht_file_connection : public ctpbase_connection
	{
	public:

		//terra::common::SingleLockFreeClassPool<CSecurityFtdcInputOrderField> xs_create_pool;
		//terra::common::SingleLockFreeClassPool<CSecurityFtdcInputOrderActionField> xs_cancel_pool;

		typedef terra::common::lockfree_classpool_workqueue<user_info> ht_user_info_inbound_queue;
	
		ht_file_connection(bool checkSecurities = true);
		virtual ~ht_file_connection();

		// connection methods
		virtual void init_connection();
		virtual void release();

		void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
		void request_investor_full_positions();

		void request_instruments();
		//virtual void request_trading_account();

		//virtual void process_idle();

		virtual void connect();
		virtual void disconnect();
		
		OrderOpenClose::type compute_open_close(order* ord, bool hasCloseToday = false) override;
		void OnRtnOrderAsync(result_orde& pOrder);
		void OnRtnCancelOrderAsyn(result_cancel &pData);
		void OnRtnTradeAsync(result_trader &pTrade);

		void OnRtnOrderHisAsync(result_orde& pOrder);
		void OnRtnCancelOrderHisAsyn(result_cancel &pData){};
		void OnRtnTradeHisAsync(result_trader &pTrade){};

#if 0
		void OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pOrder, int errorId);
		void OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pOrder, int errorId);
#endif
		
		void OnUserInfoAsync(user_info* pInfo);//异步记录userinfo
		
		//bool isDicoReady(){ return m_bIsDicoRdy; }
		
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
		string get_account(){ return this->m_sUsername;};
		void insert_spId2order(int eid, int oid);
		int get_spId2order(int eid);
#endif
	protected:

		virtual bool init_config(const std::string &name, const std::string &ini) override;		
		void process() override;		
		//virtual void cancel_num_warning(tradeitem* i) override;
		//virtual void cancel_num_ban(tradeitem* i) override;

	private:
		std::string m_sHostnameQry;
		std::string m_sServiceQry;
		std::string m_sPasswordQry;

		std::string m_sHostnameTrd;
		std::string m_sServiceTrd;
		std::string m_sPasswordTrd;

		std::string m_sProductInfo;
		std::string m_sAuthCode;
		
		std::unordered_map<int, order*> m_cancelOrdMap;
		tbb::concurrent_hash_map<int, int> m_spId2orderId;
#if 0 //def _WIN32
		HWND m_hwnd = NULL;
		//BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam);
#else
		ht_quant_proxy * m_quant_proxy;
		std::string       m_sStockAccountName;		
		tbb::concurrent_unordered_map<int, user_info*> m_user_info_map;
		//
		tbb::concurrent_unordered_map<int, user_info*> m_order_ref_map;
		//
		ht_user_info_inbound_queue m_userInfoQueue;
		string m_user_info_file_name;
#endif		
		friend class ht_file_order_aux;
	};
}

#endif // __HT_CONNECTION_H__

