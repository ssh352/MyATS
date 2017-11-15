#ifndef __GX_FILE_CONNECTION_FILE_H__
#define __GX_FILE_CONNECTION_FILE_H__


#include "gx_file_order_aux.h"
#include <ctpbase_connection.h>
#include "gx_file_quant_proxy.h"

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;

namespace gx_file
{
	class user_info
	{
	public:		
		int    LocalID = -1;
		string UserID;		
	};

	class gx_file_connection : public ctpbase_connection
	{
	public:

		//terra::common::SingleLockFreeClassPool<CSecurityFtdcInputOrderField> xs_create_pool;
		//terra::common::SingleLockFreeClassPool<CSecurityFtdcInputOrderActionField> xs_cancel_pool;

		typedef terra::common::lockfree_classpool_workqueue<user_info> ht_user_info_inbound_queue;
	
		gx_file_connection(bool checkSecurities = true);
		virtual ~gx_file_connection();

		// connection methods
		virtual void init_connection();
		virtual void release();

		void request_investor_position(terra::marketaccess::orderpassing::tradeitem* i);
		void request_investor_full_positions();

		void request_instruments();
		virtual void request_trading_account(){};

		//virtual void process_idle();

		virtual void connect();
		virtual void disconnect();
		
		OrderOpenClose::type compute_open_close(order* ord, bool hasCloseToday = false) override;

		void OnRtnOrderAsync(string line);
		void OnRtnTradeAsync(string line);
		void OnRspQryPosition(string line);
		
		void OnUserInfoAsync(user_info* pInfo);//Òì²½¼ÇÂ¼userinfo
	
		int market_create_order_async(order* o, char* pszReason) override;
		int market_cancel_order_async(order* o, char* pszReason) override;

#if 0 //def _WIN32
		void set_HWND(HWND hwnd){ m_hwnd = hwnd; }
		void post_keyboard_msg();
#else
		string & get_order_dir(){ return m_strOrderDir; }
		string & get_result_dir(){ return m_strResultDir; }
		void init_user_info(char * user_info_file);
		virtual void process_idle() { return; }
		void create_user_info(int localid,string userid);		
		//
		string get_user_id(int localid);
		//
		void append(user_info * info);
		string get_account(){ return this->m_sUsername;};
#endif
	protected:

		virtual bool init_config(const std::string &name, const std::string &ini) override;		
		void process() override;		
		virtual void cancel_num_warning(tradeitem* i) override {};
		virtual void cancel_num_ban(tradeitem* i) override{};

	private:
				
#if 0 //def _WIN32
		HWND m_hwnd = NULL;
		//BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam);
#else
		gx_file_quant_proxy * m_quant_proxy;
		std::string       m_strOrderDir;		
		std::string       m_strResultDir="c:\\";
		/*
		local id --> user info
		*/
		tbb::concurrent_unordered_map<int, user_info*> m_user_info_map;
		//
		//tbb::concurrent_unordered_map<int, user_info*> m_order_ref_map;
		//
		ht_user_info_inbound_queue m_userInfoQueue;
		string m_user_info_file_name;
		bool m_bTsession;
#endif		
		friend class gx_file_order_aux;
	};
}

#endif // __GX_FILE_CONNECTION_FILE_H__

