#ifndef __CONNECTION2_H__
#define __CONNECTION2_H__

#if _MSC_VER
#define snprintf _snprintf
#endif

#include "tradeitem.h"
#include "order.h"
#include "quote.h"
#include "connection_statistics.h"
#include "AtsType_types.h"

#include "orderdatadef.h"
#include "exec.h"
#include "tradingaccount.h"

#include "abstract_database.h"
#include "iconnection_event_handler.h"
#include <boost/bimap.hpp>
#include <list>
#include <vector>
#include "tbb/concurrent_unordered_map.h"
#include "terra_safe_tbb_hash_map.h"
#include "tbb/spin_rw_mutex.h"

typedef boost::bimap<std::string, int> boostbimap;
//typedef void(*EventHandler)(terra::marketaccess::orderpassing::connection *);
typedef std::function<void(terra::marketaccess::orderpassing::connection *)> EventHandler;



using namespace terra::common;
using namespace AtsType;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class iconnection_status_event_handler;
			class iorderbook_event_handler;
			class iquotebook_event_handler;
			class iexecbook_event_handler;
			class iconnection_event_handler;

			class connection
			{

			public:
				connection(bool);
				virtual ~connection();
				virtual void connect() = 0;
				virtual void disconnect() = 0;
				virtual void release() = 0;

				//virtual order* create_order(tradeitem *instrument, OrderWay::type way, int qty, double price) = 0;
				//virtual int get_nb_open_orders(tradeitem *instrument);
				virtual void on_status_changed(ConnectionStatus::type newStatus, const char* pszReason = NULL);
				EventHandler StatusChanged;
				/*void OnStatusChanged()
				{
				if (StatusChanged.empty())
				{
				StatusChanged(this);
				}
				}*/
				//void connection::on_trading_account_cb(tradingaccount* ta);
				//new member
				void setName(std::string& value){ n_sName = value; }
				virtual std::string& getName(){ return n_sName; }
				connection_statistics& get_statistics() { return m_statistics; }
				virtual ConnectionStatus::type getStatus(){ return m_status; }
				virtual bool getTradingAllowed(){ return m_bTradingAllowed; }
				virtual ResynchronizationMode getResynchronizationMode(){ return m_resynchroType; }

				virtual void setStatus(ConnectionStatus::type value){ m_status = value; }
				void setTradingAllowed(bool value){ m_bTradingAllowed = value; }
				void getResynchronizationMode(ResynchronizationMode value){ m_resynchroType = value; }

				void set_RiskDegree(double value){ m_riskDegree = value; }
				double get_RiskDegree(){ return m_riskDegree; }

				//old member
				virtual void init_connection() = 0;
				//virtual void release_connection() = 0;
				virtual void process_idle() {}

				virtual void request_trading_account() { return; }
				bool setup();
				bool setup(string section, std::string strConfigFile);
				bool setup(string section, std::string strConfigFile, const char * db);
				virtual void load_instruments(const std::string& name, const std::string &strConfigFile, const char* sqlfile) = 0;

				order* create_order(tradeitem* i, OrderWay::type way, int quantity, double price);
				//order* get_new_order_by_name(std::string inst, order_way way, int quantity, double price);
				quote* create_quote(tradeitem* i, int bid_quantity, double bid_price, int ask_quantity, double ask_price, const string& fqr_id);

				// order sending
				int create(order* o, char* pszReason);
				int modify(order* o, char* pszReason);
				int cancel(order* o, char* pszReason);

				int create(quote* o, char* pszReason);

				int prepare_create_order(order* o, char* pszReason);


				int cancel(quote* o, char* pszReason);

				// ack / exe
				void on_ack_from_market_cb(order* o);
				void on_nack_from_market_cb(order* o, const char* pszReason,bool is_process_should_pending = true);
				void on_cancel_from_market_cb(order* o);
				void on_exec_from_market_cb(order* o, exec* e);
				void on_exec_from_market_cb(order* o, exec* e, bool &duplicat);
				void on_trading_account_cb(tradingaccount* ta);

				void on_ack_quote_from_market_cb(quote* o);
				void on_cancel_quote_from_market_cb(quote* o);
				void on_nack_quote_from_market_cb(quote* q, const char* pszReason);

				// handlers
				void add_connection_status_event_handler(iconnection_status_event_handler* handler);
				void remove_connection_status_event_handler(iconnection_status_event_handler* handler);

				void add_orderbook_event_handler(iorderbook_event_handler* handler);
				void remove_orderbook_event_handler(iorderbook_event_handler* handler);

				void add_quotebook_event_handler(iquotebook_event_handler* handler);

				void add_execbook_event_handler(iexecbook_event_handler* handler);
				void remove_execbook_event_handler(iexecbook_event_handler* handler);

				void add_connection_event_handler(iconnection_event_handler* handler);
				void remove_connection_event_handler(iconnection_event_handler* handler);

				// get/set

				int get_nb_open_orders() { return m_activeOrders.size(); }
				//int get_nb_total_orders() { return m_deadOrders.size() + m_activeOrders.size(); }
				//int get_nb_execs() { return exec_gh::instance()->container().size(); }

	
				
				order* get_pending_order(int orderId);
				//boost::shared_mutex m_lock;
				virtual OrderOpenClose::type compute_open_close(order* ord, bool hasCloseToday);

				order* get_order_from_map(int orderId, int &ret);
				quote* get_quote_from_map(int orderId, int &ret);
				order* get_order_from_pool();

				quote* get_quote_from_pool();

				bool is_busy(){ 
					if (!m_isbusy)
						return m_isbusy; 
					else
					{
						lwtp now = get_lwtp_now();
						//if (*m_send_it == min_date_time || (now - *m_send_it).total_milliseconds() > m_maxSendInterval * 1000)
						if (*m_send_it == lw_min_time || now - *m_send_it > std::chrono::milliseconds(m_maxSendInterval * 1000))
							m_isbusy = false;
						return m_isbusy;	
					}
				}
				void cancel_num_check(tradeitem* item);
				unsigned int get_cancel_num_warning(){ return m_cancel_num_warning; }
				unsigned int get_cancel_num_ban(){ return m_cancel_num_ban; }
				int getAccountNum(){ return m_account_num; }
			public:
				bool getRequestInstruments() { return m_bRequestInstruments; }	                
				string getConfigFile(){ return m_sConfigFile; }		
				bool get_is_last(){ return m_bIsLast; }
				void set_is_last(bool value);
				void setRequestInstruments(bool value){ m_bRequestInstruments = value; }
			protected:
				bool m_bRequestInstruments = false;
                string m_sConfigFile;
				bool m_bIsLast = false;
			protected:

				//virtual order* create_order() = 0;

				virtual bool init_config(const std::string &name, const std::string &configfile) = 0;
				virtual bool init_config(const std::string &configfile) = 0;

				virtual int market_create_order(order* o, char* pszReason);
				virtual int market_alter_order(order* o, char* pszReason);
				virtual int market_cancel_order(order* o, char* pszReason);

				virtual int market_create_quote(quote* ord, char* pszReason){ return 0; }
				virtual int market_cancel_quote(quote* o, char* pszReason){ return 0; }

				//int getAccountNum(const string& account);
				int getPortfolioNum(const string& portfolio);
				const std::string& getPortfolioName(int portfolionum);
				//const std::string& getAccountName(int accountnum);

				void loadAccount(const char* pfileName);
				void loadPortfolio(const char* pfilneName);

				void add_pending_order(order* o);
				void update_pending_order(order* o);

				void add_pending_quote(quote* o);
				void update_pending_quote(quote* o);

				void check_reject_number(order* o);

				void check_reject_frequency();
				void incr_reject_frequency_it();

				void process_should_pending(order *o);
				void process_should_pending_in_nack(order *o);
				//cancle_num_limit
				unsigned int m_cancel_num_warning;
				unsigned int m_cancel_num_ban;

				virtual void cancel_num_warning(tradeitem* i) = 0;
				virtual void cancel_num_ban(tradeitem* i) = 0;


			private:
				void init_order_pool(int newSize);
				void init_quote_pool(int newSize);
				void set_reject_frequency_control(int maxRejectNumber, int maxRejectInterval);
				void pre_setup(std::string name, std::string configname);
				void set_send_frequency_control(int maxSendNumber, int maxSendInterval);
				//order* create_order(){ return new order(this);}

			protected:
				static connection* ms_instance;

				//typedef std::map<int, order*> order_map;
				terra_safe_tbb_hash_map<int, order*> m_activeOrders;
				tbb::concurrent_hash_map<int, order*> m_overTimeOrders;
				tbb::concurrent_unordered_map<int, order*> m_deadOrders;

				terra_safe_tbb_hash_map<int, quote*> m_activeQuotes;
				tbb::concurrent_unordered_map<int, quote*> m_deadQuotes;

				ConnectionStatus::type m_status;

				connection_statistics m_statistics;

				
				// securities
				bool m_checkSecurities;
				int m_maxRejectNumberPerOrder;
				double m_riskDegree = 0.0;

				int m_maxRejectInterval;
				std::list<lwtp> m_list;
				std::list<lwtp>::iterator m_it;


				int m_maxSendInterval;
				std::list<lwtp> m_send_list;
				std::list<lwtp>::iterator m_send_it;

				bool m_isbusy;

				// handlers
				typedef std::vector<iconnection_status_event_handler*> connection_status_event_handler_list;
				connection_status_event_handler_list m_connectionStatusEventHandlers;

				typedef std::vector<iorderbook_event_handler*> orderbook_event_handler_list;
				orderbook_event_handler_list m_orderbookEventHandlers;

				typedef std::vector<iquotebook_event_handler*> quotebook_event_handler_list;
				quotebook_event_handler_list m_quotebookEventHandlers;

				typedef std::vector<iexecbook_event_handler*> execbook_event_handler_list;
				execbook_event_handler_list m_execbookEventHandlers;

				typedef std::vector<iconnection_event_handler*> connection_event_handler_list;
				connection_event_handler_list m_connectionEventHandlers;

				abstract_database* m_database;
				int m_account_num;
				bool m_notify_cancelnum;

				bool m_crossCheck;
			private:
				std::string n_sName;

				bool m_bTradingAllowed;
				ResynchronizationMode m_resynchroType;
				std::list<connection*> m_Container;

				//old member
				int m_orderPoolSize;
				int m_orderPoolIncr;
				std::atomic<int> m_orderPoolCurrent;
				//order** m_orderPool;
				std::vector<order*> m_orderPool;

				int m_quotePoolSize;
				int m_quotePoolIncr;
				std::atomic<int> m_quotePoolCurrent;
				std::vector<quote*> m_quotePool;

				boostbimap m_portfolioMap;
				//boostbimap m_accountMap;
				
				std::string m_portfolioFile = "../portfolio.csv";
				std::string m_accountFile = "../account.csv";
				boost::shared_mutex m_rw_mutex_order;
				boost::shared_mutex m_rw_mutex_quote;
				//std::unordered_map<std::string, std::unordered_map<int, order*>> m_openOrderHashMap;
			};


			inline order* connection::get_order_from_pool()
			{
				if (m_orderPoolCurrent < m_orderPoolSize-1)
				{
					return m_orderPool[++m_orderPoolCurrent];
				}
				else
				{
					loggerv2::error("connection::get_new_order - orderPool is full, reset...");

					boost::unique_lock<boost::shared_mutex> lock(m_rw_mutex_order);//为了避免写锁妨碍正常功能，把写锁放这儿
					if (m_orderPoolCurrent < m_orderPoolSize - 1)//倘若第二个线程进入这一行，第一个进入这行的线程已经分配好内存，直接返回
					{
						return m_orderPool[++m_orderPoolCurrent];
					}
					
					m_orderPool.resize(m_orderPoolSize * 2);
					for (int i = m_orderPoolSize; i < m_orderPoolSize * 2; ++i)
					{
						m_orderPool[i] = new order(this);
					}

					m_orderPoolSize *= 2;

					if (m_orderPoolCurrent < m_orderPoolSize)
					{
						return m_orderPool[++m_orderPoolCurrent];
					}
					else
					{
						loggerv2::error("connection::get_new_order - cannot create order...");
						return new order(this);
					}
				}

			}

			inline quote* connection::get_quote_from_pool()
			{
				if (m_quotePoolCurrent < m_quotePoolSize-1)
				{
					return m_quotePool[++m_quotePoolCurrent];
				}
				else
				{
					loggerv2::error("connection::get_new_quote - quotePool is full, reset...");

					boost::unique_lock<boost::shared_mutex> lock(m_rw_mutex_quote);//注释同上
					if (m_quotePoolCurrent < m_quotePoolSize - 1)
					{
						return m_quotePool[++m_quotePoolCurrent];
					}
					
					m_quotePool.resize(m_quotePoolSize * 2);
					for (int i = m_quotePoolSize; i < m_quotePoolSize * 2; ++i)
					{
						m_orderPool[i] = new order(this);
					}

					m_quotePoolSize *= 2;

					if (m_quotePoolCurrent < m_quotePoolSize - 1)
					{
						return m_quotePool[++m_quotePoolCurrent];
					}
					else
					{
						loggerv2::error("connection::get_new_quote - cannot create quote...");
						return new quote(this);
					}
				}

			}

			inline order* connection::create_order(tradeitem* i, OrderWay::type way, int quantity, double price)
			{
				if (i == nullptr)
					return nullptr;
				order * o = get_order_from_pool();

				o->set_instrument(i);
				o->set_way(way);
				o->set_quantity(quantity);
				o->set_price(price);
				o->set_previous_quantity(quantity);
				o->set_previous_price(price);
				return o;
			}


			inline quote* connection::create_quote(tradeitem* i, int bid_quantity, double bid_price, int ask_quantity, double ask_price, const string& fqr_id)
			{
				
				order* bid_o = create_order(i, OrderWay::Buy, bid_quantity, bid_price);
				order* ask_o = create_order(i, OrderWay::Sell, ask_quantity, ask_price);
				quote* q = get_quote_from_pool();
				q->set_bid_order(bid_o);
				q->set_ask_order(ask_o);

				q->set_instrument(i);
				q->set_FQR_ID(fqr_id);
				return q;
			}
			inline  order* connection::get_order_from_map(int orderId, int &ret)
			{
				tbb::concurrent_hash_map<int, order*>::const_accessor ra;
				if (m_activeOrders.find(ra, orderId))
				{
					ret = 0;
					return ra->second;
				}

				if (m_deadOrders.find(orderId) != m_deadOrders.end())
				{
					ret = 1;
					return m_deadOrders[orderId];
				}
				else
				{
					ret = 2;
					return nullptr;
				}
			}

			inline  quote* connection::get_quote_from_map(int orderId, int &ret)
			{
				tbb::concurrent_hash_map<int, quote*>::const_accessor ra;
				if (m_activeQuotes.find(ra, orderId))
				{
					ret = 0;
					return ra->second;
				}

				if (m_deadQuotes.find(orderId) != m_deadQuotes.end())
				{
					ret = 1;
					return m_deadQuotes[orderId];
				}
				else
				{
					ret = 2;
					return nullptr;
				}
			}

			inline void connection::cancel_num_check(tradeitem* i)
			{
				if (i->get_cancel_num() >= get_cancel_num_ban())
				{
					cancel_num_ban(i);
				}
				else if (i->get_cancel_num() >= get_cancel_num_warning())
				{
					cancel_num_warning(i);
				}

			}


		}
	}
}


#endif

