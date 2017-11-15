#ifndef __CTPBASE_CONNECTION_H__
#define __CTPBASE_CONNECTION_H__
#include "connection.h"
#include "abstract_processor.h"
#include <LockFreeClassPool.h>

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			typedef terra::common::LockFreeWorkQueue<order> order_outbound_queue;
			typedef terra::common::LockFreeWorkQueue<quote> quote_outbound_queue;
			class ctpbase_connection : public connection,public abstract_processor
			{
			public:
				ctpbase_connection(bool security=true);
				~ctpbase_connection();
				virtual bool init_config(const std::string &ini) override { return false; };
				virtual bool init_config(const std::string &name, const std::string &strConfigFile) override;
				
				inline order_outbound_queue* get_outbound_queue() { return &m_outboundQueue; }

				virtual void request_investor_full_positions()=0;
				virtual void process_idle() override;
				void release() override;		
				//
				string get_login_id(){ return m_sLoginId; }
				virtual void req_RiskDegree(){};
				//
			protected:
				order_outbound_queue m_outboundQueue;
				quote_outbound_queue m_outquoteboundQueue;
				//bool m_isAlive;
				bool m_debug;
				bool m_bCloseToday;
				lwtp m_last_time;
				lwtp m_last_check_wait_time;
				std::string m_sHostname;
				std::string m_sService;
				std::string m_sBrokerId;
				std::string m_sUsername;
				std::string m_sPassword;
				std::string m_sCurrentBizDate;
				//
				std::string m_sLoginId;
				//
				bool m_bRequestPosition;		
				bool m_bKey_with_exchange;
				char outReason[REASON_MAXLENGTH + 1];
				std::string m_sName;
				//std::string GetName(){ return m_sName; }
				//inline bool is_alive() { return m_isAlive; }
				//inline void is_alive(bool b) { m_isAlive = b; }
				string m_type;
				string m_code_type;
				string get_type(){ return m_type; }
				std::string getCurrentBizDate();
				int get_order_id(const char*, OrderWay::type way=OrderWay::Undef);
				void get_user_info(const char* psz, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType);
				void update_instr_on_ack_from_market_cb(order* o,int actQty=-1);
				void update_instr_on_nack_from_market_cb(order* o,int actQty=-1);
				void update_instr_on_cancel_from_market_cb(order* o,int cancelQty=-1);
				void update_instr_on_exec_from_market_cb(order* o, exec* e, bool onlyUpdatePending = false);
				bool compute_userId(order* o, char* userID, int n);
				bool compute_userId(quote* o, char* userID, int n);
				virtual void load_instruments(const std::string& name, const std::string& ini, const char* sqlfile) override;
				virtual void load_instruments_type(const std::string& name, const std::string& ini, terra::common::abstract_database* db, InstrType::type instType);
				int market_create_order(order* ord, char* pszReason) override;
				int market_cancel_order(order* o, char* pszReason) override;
				virtual int market_create_order_async(order* ord, char* pszReason) = 0;
				virtual int market_cancel_order_async(order* o, char* pszReason) = 0;

				int market_create_quote(quote* ord, char* pszReason) override;
				int market_cancel_quote(quote* o, char* pszReason) override;
				virtual int market_create_quote_async(quote* ord, char* pszReason) { return 0; };
				virtual int market_cancel_quote_async(quote* o, char* pszReason) { return 0; };
				//virtual int process_outbound_msg_cb();

				void process_outbound_order(order* o);
				void process_outbound_quote(quote* q);

				std::string compute_second_key(std::string code, std::string exchangeId);
				//void init_process();

				//std::thread m_thread;

				//boost::asio::io_service io;
				//void process_loop(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
				//void set_kernel_timer_thread();
				//virtual void process() = 0;
				//static const int POLL_NUM = 10;
				virtual void cancel_num_warning(tradeitem* i) override;
				virtual void cancel_num_ban(tradeitem* i) override;

			};

			inline int ctpbase_connection::market_create_order(order* ord, char* pszReason)
			{
				if (m_status != AtsType::ConnectionStatus::Connected)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "connection not ready.\n");
					ord->set_status(AtsType::OrderStatus::Reject);
					return 0;
				}
				m_outboundQueue.Push(ord);
				return 1;
			}

			inline int ctpbase_connection::market_cancel_order(order* ord, char* pszReason)
			{
				m_outboundQueue.Push(ord);
				return 1;
			}


			inline int ctpbase_connection::market_create_quote(quote* ord, char* pszReason)
			{
				if (m_status != AtsType::ConnectionStatus::Connected)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "connection not ready.\n");
					ord->set_status(AtsType::OrderStatus::Reject);
					return 0;
				}
				m_outquoteboundQueue.Push(ord);
				return 1;
			}

			inline int ctpbase_connection::market_cancel_quote(quote* ord, char* pszReason)
			{
				m_outquoteboundQueue.Push(ord);
				return 1;
			}
			inline std::string ctpbase_connection::compute_second_key(std::string code, std::string exchangeId)
			{
				std::string key;
				if (m_bKey_with_exchange)
					key = code + "." + exchangeId + "@" + getName();
				else
					key = code + "@" + getName();
				return key;
			}

			

		}
	}
}
#endif
