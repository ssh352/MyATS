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
		//��ʼ���������շ�
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
		֤ȯ����,֤ȯ����,����,����,�۸�,��ע
		*/
		bool ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description);
		/*����f2*/
		bool cancel(int orderRef);
		/*�µ�f2*/
		bool req_order_insert();
		/*�ɽ��ر�f8*/
		bool req_trade();
		/*���гɽ��ر���F9������*/
		bool req_all_trades();
		bool rtn_all_trades();
		/*�����ֲ��ʽ𵼳���F10��*/
		bool req_account_position();
		string get_request_id();
		void init_user_info(char * user_info_file);
		/*������F9������*/
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
