#include "connection.h"
#include "iconnection_status_event_handler.h"
#include "iorderbook_event_handler.h"
#include "iexecbook_event_handler.h"
#include "order_reference_provider.h"

#include "order_gh.h"
#include "exec_gh.h"
#include "portfolio_gh.h"
#include "terra_logger.h"
#include "abstract_database.h"
#include "position.h"
#include "exec_persister.h"

#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
//#include <stdio.h>
#include <fstream>

#include "resynchronizationmode.h"
#include "iorderobserver.h"

#include <boost/property_tree/ptree.hpp>    
#include <boost/property_tree/ini_parser.hpp>  
#include <boost/filesystem.hpp>
//#include <map>

#include <istream>
#if _MSC_VER
#define snprintf _snprintf
#endif
using namespace std;
const std::string unknown = "UNKNOWN";
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			connection::connection(bool checkSecurities)
			{
				m_Container.push_back(this);
				n_sName = "???";

				m_orderPoolSize = 0;
				m_orderPoolIncr = 0;
				m_orderPoolCurrent = -1;
				//m_orderPool = NULL;


				m_quotePoolSize = 0;
				m_quotePoolIncr = 0;
				m_quotePoolCurrent = -1;
				//m_quotePool = NULL;

				//m_nNew = 0;
				//m_nMod = 0;
				//m_nCan = 0;
				//m_nAck = 0;
				//m_nRej = 0;
				//m_nNack = 0;
				//m_nExe = 0;

				m_checkSecurities = checkSecurities;

				m_bTradingAllowed = true;

				m_status = ConnectionStatus::Disconnected;

				m_resynchroType = Full;

				// by default, initialize reject_frequency_control with very conservative values (1 reject max in 1 hour)
				set_reject_frequency_control(1, 3600);
				// by default, initialize sned_frequency_control with very conservative values (100 send max in 1 second)
				set_send_frequency_control(1, 3600);

				m_maxRejectNumberPerOrder = 100;

				m_database = NULL;
				m_isbusy = false;
				m_account_num = 0;
			}

			connection::~connection()
			{
				/*if (m_orderPool != NULL)
					delete[] m_orderPool;
				if (m_database != NULL)
				{
					m_database->close_databse();
					delete m_database;
				}*/
			}

			void connection::add_connection_status_event_handler(iconnection_status_event_handler* handler)
			{
				if (handler == NULL)
					return;

				m_connectionStatusEventHandlers.push_back(handler);
			}

			void connection::remove_connection_status_event_handler(iconnection_status_event_handler* handler)
			{
				// TODO: remove handler
				//m_connectionStatusEventHandlers.xxx(handler);
			}

			void connection::add_orderbook_event_handler(iorderbook_event_handler* handler)
			{
				if (handler == NULL)
					return;

				m_orderbookEventHandlers.push_back(handler);
			}

			void connection::add_quotebook_event_handler(iquotebook_event_handler* handler)
			{
				if (handler == NULL)
					return;

				m_quotebookEventHandlers.push_back(handler);
			}

			void connection::remove_orderbook_event_handler(iorderbook_event_handler* handler)
			{
				// TODO: remove Handler
				//m_orderbookEventHandlers.(pHandler);
			}

			void connection::add_execbook_event_handler(iexecbook_event_handler* handler)
			{
				if (handler == NULL)
					return;

				m_execbookEventHandlers.push_back(handler);
			}

			void connection::remove_execbook_event_handler(iexecbook_event_handler* handler)
			{
				// TODO: remove Handler
				//m_execbookEventHandlers.(pHandler);
			}

			void connection::add_connection_event_handler(iconnection_event_handler* handler)
			{
				if (handler == NULL)
					return;
				loggerv2::info("m_connectionEventHandlers.push_back(handler) %d", handler);
				m_connectionEventHandlers.push_back(handler);
			}

			void connection::remove_connection_event_handler(iconnection_event_handler* handler)
			{

			}


			void connection::on_status_changed(ConnectionStatus::type newStatus, const char* pszReason)
			{
				if (m_status != newStatus)
				{
					/*if (pszReason != NULL)
						loggerv2::info("connection status changed: %s -> %s [%s]", connection_status_translator::get(m_Status), connection_status_translator::get(newStatus), pszReason);
						else
						loggerv2::info("connection status changed: %s -> %s", connection_status_translator::get(m_Status), connection_status_translator::get(newStatus));*/


					m_status = newStatus;


					for (iconnection_status_event_handler* pHandler : m_connectionStatusEventHandlers)
					{
						pHandler->connection_status_cb(this, newStatus, pszReason);

					}
				}
			}


			//
			// setup
			//
			bool connection::setup()
			{
				return false;
			}
			void connection::pre_setup(string name, string strConfigFile)
			{
				if (strConfigFile.length() < 1)
					return;
				boost::filesystem::path p(strConfigFile);
				if (!boost::filesystem::exists(p))
				{
					printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
					return;
				}
				boost::property_tree::ptree root;
				boost::property_tree::ini_parser::read_ini(strConfigFile, root);


				n_sName = name;
				// resynchro type
				//p.clear();
				//p.append(root.get<string>(name+".resynchro", ""));
				//string buf = p.string();
				//const char* pszResynchroType = buf.data();

				//const char* pszResynchroType = ini->get_string(name, "resynchro", "");
				m_resynchroType = ResynchronizationModeTranslator::get_instance().GetResynchronizationMode(root.get<string>(name + ".resynchro", "").c_str());

				// max number of rejects per order
				//p.clear();
				//p.append(root.get<string>(name + ".max_reject_number_per_order", ""));
				m_maxRejectNumberPerOrder = root.get<int>(name + ".max_reject_number_per_order", 100);
				//m_maxRejectNumberPerOrder = ini->get_int(name, "max_reject_number_per_order", 100);

				// max number of rejects per interval
				//p.clear();
				//p.append(root.get<string>(name + ".max_reject_number", ""));
				int maxRejectNumber = root.get<int>(name + ".max_reject_number", 1);
				//int maxRejectNumber = ini->get_int(name, "max_reject_number", 1);

				//p.clear();
				//p.append(root.get<string>(name + ".max_reject_interval", ""));
				int maxRejectInterval = root.get<int>(name + ".max_reject_interval", 3600);
				//int maxRejectInterval = ini->get_int(name, "max_reject_interval", 3600);
				set_reject_frequency_control(maxRejectNumber, maxRejectInterval);

				int maxSendNumber = root.get<int>(name + ".max_send_number", 45);
				//int maxRejectNumber = ini->get_int(name, "max_reject_number", 1);

				//p.clear();
				//p.append(root.get<string>(name + ".max_reject_interval", ""));
				int maxSendInterval = root.get<int>(name + ".max_send_interval", 1);
				//int maxRejectInterval = ini->get_int(name, "max_reject_interval", 3600);
				set_send_frequency_control(maxSendNumber, maxSendInterval);

				//p.clear();
				//p.append(root.get<string>(name + ".account_file", ""));
				std::string sAcc = root.get<string>(name + ".account_file", "");
				//std::string sAcc = ini->get_string(name, "account_file", "account.csv");
				if (!sAcc.empty())
					m_accountFile = sAcc.c_str();
				loadAccount(m_accountFile.c_str());

				//p.clear();
				//p.append(root.get<string>(name + ".portfolio_file", ""));
				std::string sPort = root.get<string>(name + ".portfolio_file", "");
				//std::string sPort = ini->get_string(name, "portfolio_file", "portfolio.csv");
				if (!sPort.empty())
					m_portfolioFile = sPort.c_str();
				loadPortfolio(m_portfolioFile.c_str());

				//p.clear();
				//p.append(root.get<string>(name + ".order_pool_size", ""));
				m_orderPoolSize = root.get<int>(name + ".order_pool_size", 128 * 1024);
				m_quotePoolSize = root.get<int>(name + ".order_pool_size", 128 * 1024);
				//m_orderPoolSize = ini->get_int(name, "order_pool_size", 128 * 1024);

				//p.clear();
				//p.append(root.get<string>(name + ".order_pool_incr", ""));
				m_orderPoolIncr = root.get<int>(name + ".order_pool_incr", 64 * 1024);
				m_quotePoolIncr = root.get<int>(name + ".order_pool_incr", 64 * 1024);
				//m_orderPoolIncr = ini->get_int(name, "order_pool_incr", 64 * 1024);
				init_order_pool(m_orderPoolSize);
				init_quote_pool(m_quotePoolSize);
				//p.clear();
				//p.append(root.get<string>(name + ".cross_check", ""));
				//string str = p.string();
				//if (str == "true")
				//	m_crossCheck = true;
				//else
				//	m_crossCheck = false;
				m_crossCheck = root.get<bool>(name + ".cross_check", true);
				m_notify_cancelnum = root.get<bool>(name + ".notify_cancel", true);
				//m_crossCheck = ini->get_bool(name, "cross_check", false);


			}
			bool connection::setup(string name, string strConfigFile)
			{
				pre_setup(name, strConfigFile);

				// specific
				if (!init_config(name, strConfigFile))
				{
					loggerv2::error("init_config failed");
					return false;
				}

				//load_instruments
				if (strConfigFile.length() < 1)
					return false;
				boost::filesystem::path p(strConfigFile);
				if (!boost::filesystem::exists(p))
				{
					printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
					return false;
				}
				boost::property_tree::ptree root;
				boost::property_tree::ini_parser::read_ini(strConfigFile, root);
				p.clear();
				p.append(root.get<string>(name + ".sql_dico_file", ""));
				string buf = p.string();
				const char* sqlfile = buf.data();

				//const char* sqlfile = ini->get_string(name, "sql_dico_file", "");
				loggerv2::info("loading instruments.");
				load_instruments(name, strConfigFile, sqlfile);

				return true;
			}

			bool connection::setup(string name, string strConfigFile, const char * db)
			{
				pre_setup(name, strConfigFile);

				// specific
				if (!init_config(name, strConfigFile))
				{
					loggerv2::error("init_config failed");
					return false;
				}

				load_instruments(name, strConfigFile, db);

				return true;
			}
			//
			// order pool
			//
			void connection::init_order_pool(int newSize)
			{
				/*if (m_orderPool != NULL)
				{
					for (int i = m_orderPoolCurrent; i < m_orderPoolSize; i++)
						delete m_orderPool[i];

					delete[] m_orderPool;
				}

				m_orderPoolCurrent = 0;
				m_orderPoolSize = newSize;

				m_orderPool = new order*[m_orderPoolSize];
				for (int i = 0; i < m_orderPoolSize; i++)
				{
					m_orderPool[i] = new order(this);
				}*/
				m_orderPool.resize(m_orderPoolSize);
				for (int i = 0; i < m_orderPoolSize; i++)
				{
					m_orderPool[i] = new order(this);
				}

			}

			void connection::init_quote_pool(int newSize)
			{

				//if (m_quotePool != NULL)
				//{
				//	// delete remaining quotes
				//	for (int i = m_quotePoolCurrent; i < m_quotePoolSize; i++)
				//		delete m_quotePool[i];

				//	delete[] m_quotePool;
				//}

				//m_quotePoolCurrent = 0;
				//m_quotePoolSize = newSize;

				//m_quotePool = new quote*[m_quotePoolSize];
				m_quotePool.resize(m_quotePoolSize);
				for (int i = 0; i < m_quotePoolSize; i++)
				{
					m_quotePool[i] = new quote(this);
				}
			}

			// misc
			//
			void connection::add_pending_order(order* o)
			{
				int orderId = o->get_id();

				if (!order_gh::get_instance().GetBook()->add(o))
					o = order_gh::get_instance().GetBook()->get_by_id(orderId);


				//readLock rlock(m_lock);
				tbb::concurrent_hash_map<int, order*>::const_accessor ra;
				if (m_activeOrders.find(ra, orderId))
				{
					loggerv2::error("connection::add_pending_order - duplicate ID order %d", orderId);
					ra.release();
				}
				else
				{
					ra.release();
					tbb::concurrent_hash_map<int, order*>::accessor wa;
					//m_activeOrders.emplace(orderId, o);//emplace和insert不同，不能改写已有的key-value对
					loggerv2::info("connection,add order %d to hashmap", orderId);
					m_activeOrders.insert(wa, orderId);
					wa->second = o;
					wa.release();
					for (iorderbook_event_handler* handler : m_orderbookEventHandlers)
					{
						handler->add_order_cb(o);
					}

					if (m_crossCheck)
					{
						o->get_instrument()->get_order_book()->add_order(abs(o->get_id()), o->get_price(), /*o->get_book_quantity(), */o->get_way());
					}

					o->on_add_order();
					//}
				}


			}

			void connection::add_pending_quote(quote* q)
			{
				int orderId = q->get_id();

				if (!quote_gh::get_instance().GetBook()->add(q))
					q = quote_gh::get_instance().GetBook()->get_by_id(orderId);


				//readLock rlock(m_lock);
				tbb::concurrent_hash_map<int, quote*>::const_accessor ra;
				if (m_activeQuotes.find(ra, orderId))
				{
					loggerv2::error("connection::add_pending_quote - duplicate ID quote %d", orderId);
					ra.release();
				}
				else
				{
					ra.release();
					tbb::concurrent_hash_map<int, quote*>::accessor wa;
					//m_activeOrders.emplace(orderId, o);//emplace和insert不同，不能改写已有的key-value对
					loggerv2::info("connection,add quote %d to hashmap", orderId);
					m_activeQuotes.insert(wa, orderId);
					wa->second = q;
					wa.release();
					for (iquotebook_event_handler* handler : m_quotebookEventHandlers)
					{
						handler->add_quote_cb(q);
					}
					for (iorderbook_event_handler* handler : m_orderbookEventHandlers)
					{
						handler->add_order_cb(q->get_bid_order());
						handler->add_order_cb(q->get_ask_order());
					}
					if (m_crossCheck)
					{
						//o->get_instrument()->get_order_book()->add_order(abs(o->get_id()), o->get_price(), /*o->get_book_quantity(), */o->get_way());
						if (q->get_bid_order() != nullptr)
						{
							q->get_instrument()->get_order_book()->add_order(abs(q->get_bid_order()->get_id()), q->get_bid_order()->get_price(), /*o->get_book_quantity(), */q->get_bid_order()->get_way());
						}
						if (q->get_ask_order() != nullptr)
						{
							q->get_instrument()->get_order_book()->add_order(abs(q->get_ask_order()->get_id()), q->get_ask_order()->get_price(), /*o->get_book_quantity(), */q->get_ask_order()->get_way());
						}
					}

					q->on_add_quote();
					if (q->get_bid_order() != nullptr)
					{
						q->get_bid_order()->on_add_order();
					}
					if (q->get_ask_order() != nullptr)
					{
						q->get_ask_order()->on_add_order();
					}
					//}
				}


			}

			void connection::update_pending_order(order* o)
			{
				if (o == nullptr)
					return;

				OrderStatus::type status = o->get_status();

				if (status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
				{
					// move order from alive list to dead list
					int orderId = o->get_id();
					//m_activeOrders.unsafe_erase(orderId);
					m_activeOrders.erase(orderId);
					m_deadOrders.emplace(orderId, o);

					//loggerv2::info("connection::update_pending_order active to dead order,%d\n",orderId);

					if (m_crossCheck)
					{
						//撤单/成交/rej/nack回调，移除order_book中的项
						o->get_instrument()->get_order_book()->remove_order(abs(o->get_id()));// , o->get_price(), o->get_book_quantity(), o->get_way());
					}
				}

			
				if (o->get_binding_quote()!=nullptr)
				{
					update_pending_quote(o->get_binding_quote());
				}
				else
				{
					for (iorderbook_event_handler* handler : m_orderbookEventHandlers)
					{
						handler->update_order_cb(o);
					}
				}
				
			}

			void connection::update_pending_quote(quote* q)
			{
				OrderStatus::type status = q->get_status();

				if (status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
				{
					// move order from alive list to dead list
					int orderId = q->get_id();
					//m_activeOrders.unsafe_erase(orderId);
					m_activeQuotes.erase(orderId);
					m_deadQuotes.emplace(orderId, q);

					if (m_crossCheck)
					{
						//撤单/成交/rej/nack回调，移除order_book中的项
						if (q->get_bid_order())
						{
							q->get_instrument()->get_order_book()->remove_order(abs(q->get_bid_order()->get_id()));// , o->get_price(), o->get_book_quantity(), o->get_way());
						}
						if (q->get_ask_order())
						{
							q->get_instrument()->get_order_book()->remove_order(abs(q->get_ask_order()->get_id()));// , o->get_price(), o->get_book_quantity(), o->get_way());
						}
					}
				}

				for (iquotebook_event_handler* handler : m_quotebookEventHandlers)
				{
					handler->update_quote_cb(q);
				}

				for (iorderbook_event_handler* handler : m_orderbookEventHandlers)
				{
					handler->update_order_cb(q->get_bid_order());
					handler->update_order_cb(q->get_ask_order());
				}
			}

			order* connection::get_pending_order(int orderId)
			{
				//order_map::iterator it = m_activeOrders.find(orderId);
				tbb::concurrent_hash_map<int, order*>::const_accessor ra;
				if (m_activeOrders.find(ra, orderId))// != m_activeOrders.end())//it != m_activeOrders.end())
				{
					return ra->second;//m_activeOrders[orderId];//it->second;
				}
				else
				{
					//it = m_deadOrders.find(orderId);
					if (m_deadOrders.find(orderId) != m_deadOrders.end())//it != m_deadOrders.end())
					{
						loggerv2::warn("connection::get_pending_order - message received on dead order [%d] ...", orderId);
						return m_deadOrders[orderId];
						//return it->second;
					}
				}
				return NULL;
			}



			void connection::set_reject_frequency_control(int maxRejectNumber, int maxRejectInterval)
			{
				m_maxRejectInterval = maxRejectInterval;
				m_list.assign(maxRejectNumber, lw_min_time);
				m_it = m_list.begin();
			}

			void connection::set_send_frequency_control(int maxSendNumber, int maxSendInterval)
			{
				m_maxSendInterval = maxSendInterval;
				m_send_list.assign(maxSendNumber, lw_min_time);
				m_send_it = m_send_list.begin();
			}

			void connection::incr_reject_frequency_it()
			{
				m_it++;
				if (m_it == m_list.end())
					m_it = m_list.begin();
			}

			void connection::check_reject_frequency()
			{
				//ptime now = microsec_clock::local_time();
				lwtp now = get_lwtp_now();
				if (*m_it == lw_min_time)
				{
					// not initialized yet
					*m_it = now;
					incr_reject_frequency_it();
				}
				else
				{
					if ((now - *m_it) < std::chrono::milliseconds(m_maxRejectInterval * 1000))
					{
						// we exceed maxFrequency -> we prevent further order sending on the connection
						if (m_bTradingAllowed)
						{
							int gap = (int)std::chrono::duration_cast<std::chrono::seconds>(now - *m_it).count();
							loggerv2::error("[%s] connection::check_reject_frequency - %d rejects received in %.3f seconds", getName().c_str(), m_list.size(), gap);
							m_bTradingAllowed = false;
						}
					}
					else
					{
						// we did not exceed maxFrequency -> push current time and move to next element.
						*m_it = now;
						incr_reject_frequency_it();
					}
				}
			}

			void connection::check_reject_number(order* o)
			{
				if (o->get_active() && (o->get_nb_cancel() > m_maxRejectNumberPerOrder || o->get_nb_modif() > m_maxRejectNumberPerOrder))
				{
					// inactive order as we received too many rejects on it.
					o->set_active(false);


					{
						o->on_inactive_order();
					}

					for (iorderbook_event_handler* handler : m_orderbookEventHandlers)
					{
						//handler->add_inactive_order_cb(o);
						handler->add_order_cb(o);
						handler->update_order_cb(o);
					}
				}
			}

			void connection::process_should_pending(order *o)
			{
				if (o->get_open_close() == OrderOpenClose::Close || o->get_open_close() == OrderOpenClose::CloseToday)
				{
					if (o->get_way() == AtsType::OrderWay::Buy)
					{
						o->get_instrument()->add_should_pending_long_close_qty(o->get_quantity());
					}
					else if (o->get_way() == AtsType::OrderWay::Sell)
					{
						o->get_instrument()->add_should_pending_short_close_qty(o->get_quantity());
					}
				}
			}

			void connection::process_should_pending_in_nack(order *o)
			{
				if (o->get_open_close() == OrderOpenClose::Close || o->get_open_close() == OrderOpenClose::CloseToday)
				{
					if (o->get_way() == AtsType::OrderWay::Buy)
					{
						o->get_instrument()->add_should_pending_long_close_qty(-1 * o->get_quantity());
					}
					else if (o->get_way() == AtsType::OrderWay::Sell)
					{
						o->get_instrument()->add_should_pending_short_close_qty(-1 * o->get_quantity());
					}
				}
			}

			int connection::create(order* o, char* pszReason)
			{

				m_statistics.incr_new();
				if (prepare_create_order(o, pszReason) == 0)
					return 0;
				lwtp ltime = get_lwtp_now();
				o->set_last_time(ltime);

				if (m_status != AtsType::ConnectionStatus::Connected)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "connection not ready.\n");
					o->set_status(AtsType::OrderStatus::Reject);
					return 0;
				}
				add_pending_order(o);
				int res = market_create_order(o, pszReason);
				//if (res)
				//{
				//	add_pending_order(o);
				//}

				return res;
			}

			int connection::create(quote* q, char* pszReason)
			{
				m_statistics.incr_new();
				if (prepare_create_order(q->get_bid_order(), pszReason) == 0)
				{
					return 0;
				}

				if (prepare_create_order(q->get_ask_order(), pszReason) == 0)
				{
					return 0;
				}
				q->m_nId = q->get_bid_order()->get_id();

				q->set_last_action(OrderAction::Created);

				q->set_status(OrderStatus::WaitServer);
				auto ltime = get_lwtp_now();
				q->set_last_time(ltime);
				q->get_bid_order()->set_last_time(ltime);
				q->get_ask_order()->set_last_time(ltime);
				//o->dump_info();

				int res = market_create_quote(q, pszReason);
				if (res)
				{
					add_pending_quote(q);
					add_pending_order(q->get_bid_order());
					add_pending_order(q->get_ask_order());

				}

				return res;
			}

			int connection::modify(order* o, char* pszReason)
			{
				o->set_last_action(OrderAction::Modified);

				if (m_status != ConnectionStatus::Connected)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "connection failed");
					o->rollback();
					update_pending_order(o);
					return 0;
				}


				// security checks
				if (m_bTradingAllowed == false)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "[%d] trading not allowed", o->get_id());
					o->rollback();
					update_pending_order(o);
					return 0;
				}
				if (o->get_status() != OrderStatus::Ack && o->get_status() != OrderStatus::Nack)
				{
					auto it = _OrderStatus_VALUES_TO_NAMES.find(o->get_status());
					snprintf(pszReason, REASON_MAXLENGTH, "[%d] wrong status [%s]", o->get_id(), it->second);
					//
					// do not rollback here as we are waiting for ack to come back ???
					//
					o->rollback();
					update_pending_order(o);
					return 0;
				}
				if (o->get_binding_quote() != nullptr)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "should cancel quote [%d] not order[%d] ", o->get_binding_quote()->get_id() , o->get_id());
					//
					// do not rollback here as we are waiting for ack to come back ???
					//
					o->rollback();
					update_pending_order(o);
					return 0;
				}
				if (o->m_nQuantity <= 0 || (o->m_PriceMode == OrderPriceMode::Limit && o->m_dPrice <= 0))
				{
					snprintf(pszReason, REASON_MAXLENGTH, "[%d] wrong quantity [%d] / price [%f]", o->get_id(), o->m_nQuantity, o->m_dPrice);
					o->rollback();
					update_pending_order(o);
					return 0;
				}

				m_statistics.incr_mod();

				o->m_nModif++;
				o->set_status(OrderStatus::WaitServer);
				o->m_nBookQuantity = o->m_nQuantity - o->m_nExecQuantity;
				//o->m_lastTime.set_date();
				o->m_lastTime = get_lwtp_now();
				o->dump_info();


				// specific
				int res = market_alter_order(o, pszReason);
				if (res)
				{
					// to notify viewers (orderbook, etc...) that order has been modified (wait status...)?
					//++m_nMod;
					update_pending_order(o);
				}

				return res;
			}

			int connection::cancel(order* o, char* pszReason)
			{
				auto item = o->get_instrument();
				if (item->get_cancel_forbid() == true)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "tradeItem %s has been forbid to cancel,cancel num is:%d", item->getCode().c_str(), item->get_cancel_num());
					return 0;
				}

				auto stat = o->get_status();
				if (stat != OrderStatus::Ack && stat != OrderStatus::Nack)
				{
					if (stat == OrderStatus::Exec || stat == OrderStatus::Cancel || stat == OrderStatus::Reject || stat == OrderStatus::Nack)
					{
						snprintf(pszReason, REASON_MAXLENGTH, "[%d] wrong status [%s],can not cancle a order in exec/cancel/rej/nack stat", o->get_id(), _OrderStatus_VALUES_TO_NAMES.at(o->get_status()));
						return 0;
					}

					if (!(stat == OrderStatus::WaitServer&&o->m_preStatus == OrderStatus::Undef&&o->get_last_action() == OrderAction::Created))
					{

						snprintf(pszReason, REASON_MAXLENGTH, "[%d] wrong status [%s]", o->get_id(), _OrderStatus_VALUES_TO_NAMES.at(o->get_status()));
						//loggerv2::error("connection::cancel order roll back");
						//o->dump_info();
						//o->rollback();
						//loggerv2::error("connection::cancel after roll_back:");
						//o->dump_info();
						//update_pending_order(o);
						return 0;
					}
					else
					{
						auto now = get_lwtp_now();
						auto dur = now - o->m_lastTime;
						if (dur < std::chrono::seconds(5))
						{
							snprintf(pszReason, REASON_MAXLENGTH, "[%d] cancle rej:order sent time is less than 5 sec,wait [%s]", o->get_id(), _OrderStatus_VALUES_TO_NAMES.at(o->get_status()));
							return 0;
						}
						else
						{
							o->set_status(OrderStatus::Nack);
						}

					}
				}

				o->set_last_action(OrderAction::Cancelled);

				//m_statistics.incr_can();

				o->m_nCancel++;
				o->set_status(OrderStatus::WaitServer);
				o->m_nBookQuantity = 0;
				o->m_lastTime = get_lwtp_now();
				o->dump_info();

				// specific
				int res = market_cancel_order(o, pszReason);
				if (res)
				{
					//++m_nCan;
					update_pending_order(o);
				}

				return res;
			}

			int connection::cancel(quote* q, char* pszReason)
			{
				auto item = q->get_instrument();
				if (item->get_cancel_forbid() == true)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "tradeItem %s has been forbid to cancel,cancel num is:%d", item->getCode().c_str(), item->get_cancel_num());
					return 0;
				}


				auto stat = q->get_status();
				if (stat != OrderStatus::Ack && stat != OrderStatus::Nack)
				{
					if (stat == OrderStatus::Exec || stat == OrderStatus::Cancel || stat == OrderStatus::Reject || stat == OrderStatus::Nack)
					{
						snprintf(pszReason, REASON_MAXLENGTH, "[%d] wrong status [%s],can not cancle a quote in exec/cancel/rej/nack stat", q->get_id(), _OrderStatus_VALUES_TO_NAMES.at(q->get_status()));
						return 0;
					}

					if (!(stat == OrderStatus::WaitServer && q->m_preStatus == OrderStatus::Undef && q->get_last_action() == OrderAction::Created))
					{

						snprintf(pszReason, REASON_MAXLENGTH, "[%d] wrong status [%s]", q->get_id(), _OrderStatus_VALUES_TO_NAMES.at(q->get_status()));
						q->rollback();
						update_pending_quote(q);
						return 0;
					}
					else
					{
						auto now = get_lwtp_now();
						auto dur = now - q->m_lastTime;
						if (dur < std::chrono::seconds(5))
						{
							snprintf(pszReason, REASON_MAXLENGTH, "[%d] cancle rej:order sent time is less than 5 sec,wait [%s]", q->get_id(), _OrderStatus_VALUES_TO_NAMES.at(q->get_status()));
							return 0;
						}
						else
						{
							q->set_status(OrderStatus::Nack);
						}

					}
				}

				q->set_last_action(OrderAction::Cancelled);

				//m_statistics.incr_can();

				q->m_nCancel++;
				q->set_status(OrderStatus::WaitServer);
				q->get_bid_order()->m_nBookQuantity = 0;
				q->get_ask_order()->m_nBookQuantity = 0;
				q->m_lastTime = get_lwtp_now();
				q->dump_info();
				if (q->get_ask_order() != nullptr)
				{
					q->get_ask_order()->m_lastTime = q->m_lastTime;
				}
				if (q->get_bid_order() != nullptr)
				{
					q->get_bid_order()->m_lastTime = q->m_lastTime;
				}

				// specific
				int res = market_cancel_quote(q, pszReason);
				if (res)
				{
					//++m_nCan;
					update_pending_quote(q);
				}

				return res;
			}

			int connection::market_create_order(order* o, char* pszReason)
			{
				//snprintf(pszReason, REASON_MAXLENGTH, "not supported");
				return 0;
			}

			int connection::market_alter_order(order* o, char* pszReason)
			{
				//snprintf(pszReason, REASON_MAXLENGTH, "not supported");
				return 0;
			}

			int connection::market_cancel_order(order* o, char* pszReason)
			{
				//snprintf(pszReason, REASON_MAXLENGTH, "not supported");
				return 0;
			}

			void connection::on_ack_from_market_cb(order* o)
			{
				//loggerv2::info("last action = %d", o->get_last_action());
				//
				// update bookQty here ??
				//
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
				//
				//
				if (o->get_book_quantity() < 0)
				{
					o->set_book_quantity(0);
					loggerv2::error("BookQty error, should not be negative on_ack_from_market_cb");
				}
				/*if (o->get_last_action() == OrderAction::Cancelled)
					o->set_book_quantity(0);*/

				//to do ...
				if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
				{
					o->set_status(o->get_book_quantity() > 0 ? OrderStatus::Ack : OrderStatus::Cancel);
				}
				//o->save_previous_values();

				loggerv2::info("OnAckFromMarketCB : ACK %c (%06d) open[%s] source[%d] bookQty[%d]",
					o->get_last_action() == OrderAction::Created ? 'N' : o->get_last_action() == OrderAction::Modified ? 'M' : o->get_last_action() == OrderAction::Cancelled ? 'C' : '?',
					o->get_id(),
					_OrderOpenClose_VALUES_TO_NAMES.at(o->get_open_close()),
					o->get_trading_type(),
					o->get_book_quantity());

				o->on_update_order();

				update_pending_order(o);

				m_statistics.incr_ack();

				for (auto iter : m_connectionEventHandlers)
					iter->send_statistics(this);

				tbb::concurrent_hash_map<int, order*>::const_accessor ra;
				if (m_overTimeOrders.find(ra,o->get_id()))//这个下单回报来自之前的超时单，需要把这个单重新放入m_activeOrders和自成交检查队列
				{
					ra.release();
					m_overTimeOrders.erase(o->get_id());

					tbb::concurrent_hash_map<int, order*>::accessor wa;
					m_activeOrders.insert(wa, o->get_id());
					wa->second = o;
					wa.release();

					if (m_crossCheck)
					{
						o->get_instrument()->get_order_book()->add_order(abs(o->get_id()), o->get_price(), /*o->get_book_quantity(), */o->get_way());
					}
				}

			}

			void connection::on_nack_from_market_cb(order* o, const char* pszReason, bool is_process_should_pending)
			{
				const char* type = "";
				if (pszReason == NULL)
					pszReason = "unknown";

				if (o->get_last_action() == OrderAction::Created)
				{
					// REJ
					// create rejected by market
					if (is_process_should_pending)
						process_should_pending_in_nack(o);
					o->set_status(OrderStatus::Reject);
					o->set_book_quantity(0);

					type = "REJ";
					m_statistics.incr_rej();

					for (auto iter : m_connectionEventHandlers)
						iter->send_statistics(this);

				}
				else
				{
					// NACK
					// modif / cancel rejected by market

					if (o->get_status() == OrderStatus::Exec)
					{
						// We received a full exec in between, so the cancel is rejected.
						// -> Nothing to do (status already exec, bookQty already 0)
						o->set_book_quantity(0);

					}
					else if (o->get_status() == OrderStatus::Cancel)//临时方案
					{
						//   // order is already in cancel state (for example, we receive a cancel from market on an order we tried to cancel just before.
						//   // -> Nothing to do
						o->set_book_quantity(0);
					}
					else
					{
						// example: a cancel is rejected by market (outside market hours...)
						// -> rollBack to previous values (quantity, price, bookQuantity).
						o->rollback();

						// is the order still in market ?
						if (o->get_book_quantity() > 0)
							o->set_status(OrderStatus::Nack);

						else
							o->set_status(OrderStatus::Cancel);
					}

					type = "NACK";
					m_statistics.incr_nack();

					for (auto iter : m_connectionEventHandlers)
						iter->send_statistics(this);

				}

				//o->set_last_time();
				o->set_lastreason(pszReason);

				loggerv2::warn("OnNAckFromMarketCB : %s (%06d) Open[%s] Source[%d] Reason[%s]", type, o->get_id(),
					_OrderOpenClose_VALUES_TO_NAMES.at(o->get_open_close()),
					o->get_trading_type(), pszReason);


				o->on_update_order();

				update_pending_order(o);

				// check that we do not exceed max number of reject per order
				check_reject_number(o);

				// check that we do not exceed max reject frequency
				check_reject_frequency();

			}

			void connection::on_nack_quote_from_market_cb(quote* o, const char* pszReason)
			{
				if (o == nullptr)
					return;
				const char* type = "";
				if (pszReason == NULL)
					pszReason = "unknown";

				if (o->get_last_action() == OrderAction::Created)
				{
					// REJ
					// create rejected by market

					o->set_status(OrderStatus::Reject);
					if (o->get_bid_order())
					{
					o->get_bid_order()->set_book_quantity(0);
					}
					if (o->get_ask_order())
					{
					o->get_ask_order()->set_book_quantity(0);
					}

					type = "REJ";
					m_statistics.incr_rej();

					for (auto iter : m_connectionEventHandlers)
						iter->send_statistics(this);

				}
				else
				{
					// NACK
					// modif / cancel rejected by market

					if (o->get_status() == OrderStatus::Exec)
					{
						// We received a full exec in between, so the cancel is rejected.
						// -> Nothing to do (status already exec, bookQty already 0)
						if (o->get_bid_order())
						{
						o->get_bid_order()->set_book_quantity(0);
						}
						if (o->get_ask_order())
						{
						o->get_ask_order()->set_book_quantity(0);
						}
					}
					else if (o->get_status() == OrderStatus::Cancel)//临时方案
					{
						//   // order is already in cancel state (for example, we receive a cancel from market on an order we tried to cancel just before.
						//   // -> Nothing to do
						if (o->get_bid_order())
						{
						o->get_bid_order()->set_book_quantity(0);
						}
						if (o->get_ask_order())
						{
						o->get_ask_order()->set_book_quantity(0);
					}
					}
					else
					{
						// example: a cancel is rejected by market (outside market hours...)
						// -> rollBack to previous values (quantity, price, bookQuantity).
						o->rollback();


						// is the order still in market ?
						if (o->get_bid_order())
						{
						if (o->get_bid_order()->get_book_quantity() > 0)
							o->get_bid_order()->set_status(OrderStatus::Nack);
						else
							o->get_bid_order()->set_status(OrderStatus::Cancel);
						}
						if (o->get_ask_order())
						{
						if (o->get_ask_order()->get_book_quantity() > 0)
							o->get_ask_order()->set_status(OrderStatus::Nack);
						else
							o->get_ask_order()->set_status(OrderStatus::Cancel);
					}
					}

					type = "NACK";
					m_statistics.incr_nack();

					for (auto iter : m_connectionEventHandlers)
						iter->send_statistics(this);

				}

				//o->set_last_time();
				o->set_lastreason(pszReason);


				loggerv2::warn("OnNAckFromMarketCB : %s (%06d) Source[%d] Reason[%s]", type, o->get_id(), o->get_trading_type(), pszReason);



				//if (!isResynchro)
				{
					//std::string TradeingType;
					//auto it = _TradingType_VALUES_TO_NAMES.find(o->get_trading_type());
					//if (it != _TradingType_VALUES_TO_NAMES.end())
					//	TradeingType = it->second;
					loggerv2::warn("OnNAckFromMarketCB : %s (%06d) Source[%d] Reason[%s]", type, o->get_id(), o->get_trading_type(), pszReason);
				}


			
				o->on_update_quote();

				update_pending_quote(o);

				check_reject_frequency();

			}

			void connection::on_cancel_from_market_cb(order* o)
			{
				//
				// unsolicited ack (mass-cancel, cancel IAC, cancel FOK...).
				//
				tradeitem* i = o->get_instrument();
				if (o->get_restriction() == AtsType::OrderRestriction::None)
				{
					i->add_cancle_num();
				}
				if (m_notify_cancelnum)
				{
					loggerv2::warn("tradeItm:%s has been cancelled %d times", i->getCode().c_str(), i->get_cancel_num());
				}
				else
				{
					loggerv2::info("tradeItm:%s has been cancelled %d times", i->getCode().c_str(), i->get_cancel_num());
				}

				cancel_num_check(i);

				if (o->get_restriction() == AtsType::OrderRestriction::None)
				{
					m_statistics.incr_can();
				}

				o->set_status(OrderStatus::Cancel);
				o->set_book_quantity(0);

				loggerv2::info("on_cancel_from_market_cb : CANCEL C (%06d) Source[%d]", o->get_id(), o->get_trading_type());


				o->on_update_order();

				update_pending_order(o);

			}

			void connection::on_ack_quote_from_market_cb(quote* q)
			{
				on_ack_from_market_cb(q->get_bid_order());
				on_ack_from_market_cb(q->get_ask_order());

				q->on_update_quote();
				update_pending_quote(q);
				m_statistics.incr_ack();

				for (auto iter : m_connectionEventHandlers)
					iter->send_statistics(this);

				if (q->get_bid_order()->get_book_quantity() == 0 && q->get_ask_order()->get_book_quantity() == 0)
					q->set_status(OrderStatus::Cancel);
				else if (q->get_bid_order()->get_status() == OrderStatus::Exec && q->get_ask_order()->get_status() == OrderStatus::Exec)
					q->set_status(OrderStatus::Exec);
				else
					q->set_status(OrderStatus::Ack);
			}

			void connection::on_cancel_quote_from_market_cb(quote* q)
			{
				tradeitem* i = q->get_instrument();

				i->add_cancle_num();
				if (m_notify_cancelnum)
				{
					loggerv2::warn("tradeItm:%s has been canceled %d times", i->getCode().c_str(), i->get_cancel_num());
				}
				else
				{
					loggerv2::info("tradeItm:%s has been canceled %d times", i->getCode().c_str(), i->get_cancel_num());
				}

				cancel_num_check(i);
				m_statistics.incr_can();

				q->set_status(OrderStatus::Cancel);
				if (q->get_bid_order())
					q->get_bid_order()->set_book_quantity(0);
				if (q->get_ask_order())
					q->get_ask_order()->set_book_quantity(0);

				loggerv2::info("on_cancel_quote_from_market_cb : CANCEL C (%06d) Source[%d]", q->get_id(), q->get_trading_type());

				q->on_update_quote();
				if (q->get_bid_order())
					q->get_bid_order()->on_update_order();
				if (q->get_ask_order())
					q->get_ask_order()->on_update_order();

				update_pending_quote(q);
				update_pending_order(q->get_bid_order());
				update_pending_order(q->get_ask_order());
			}

			void connection::on_exec_from_market_cb(order* o, exec* e, bool &duplicat)
			{
				// 0 - check if we have already received an execution with this market reference.
				exec* old_e = exec_gh::get_instance().GetBook().get_by_reference(e->getReference());
				if (old_e != NULL)
				{
					if (old_e->getOrderId() != o->get_id())
					{
						loggerv2::warn("on_exec_from_market_cb cross trade,orderId:%d,old.orderId:%d,e->getReference():%s", o->get_id(), old_e->getOrderId(), e->getReference().c_str());
					}
					else
					{
						if (!old_e->is_persisted()) //&& *e == *old_e)
						{
							duplicat = true;
							loggerv2::error("on_exec_from_market_cb - reject execution, exec_ref[%s] already exist,orderId:%d,duplicat:%d", e->getReference().c_str(), o->get_id(), duplicat);
							return;
						}
					}
				}


				// 1 - update order
				int execQty = e->getQuantity();
				double execPrice = e->getPrice();

				int cumulatedQty = o->get_exec_quantity() + execQty;
				double averagePrc = (o->get_exec_quantity() * o->get_exec_price() + execQty * execPrice) / (cumulatedQty);

				int totQty = o->get_quantity();

				bool isETFRP = (o->get_way() == OrderWay::ETFPur || o->get_way() == OrderWay::ETFRed) && o->get_instrument()->get_instr_type() == AtsType::InstrType::ETF;
				if (isETFRP)
					totQty = execQty;

				int bookQty = totQty - cumulatedQty;

				if (bookQty <= 0)
				{
					o->set_status(OrderStatus::Exec);
				}

				if (bookQty < 0)
				{
					bookQty = 0;
					o->set_quantity(o->get_quantity() + execQty);
					loggerv2::info("BookQty is negative, rebuild, on_exec_from_market_cb - bookQty<0, exec_ref[%s]", e->getReference().c_str());
				}

				
				o->set_exec_quantity(cumulatedQty);
				o->set_exec_price(averagePrc);

				o->on_update_order();

				update_pending_order(o);

				for (iexecbook_event_handler* handler : m_execbookEventHandlers)
				{
					//for zmq broker
					handler->add_exec_cb(e, o);

					//classical call back
					handler->add_exec_cb(e);
				}

				if (o->get_observers() != nullptr)
					o->get_observers()->add_exec_cb(e);
				if (o->get_binding_quote() != nullptr)
				{
					o->get_binding_quote()->on_add_exec(e);
				}
				m_statistics.incr_exe();
				o->set_book_quantity(bookQty);

				for (auto iter : m_connectionEventHandlers)
					iter->send_statistics(this);

				if (old_e == NULL)
				{
					// 3 - update position
					portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(o->get_portfolio());
					if (pPortfolio == NULL)
					{
						pPortfolio = new portfolio(o->get_portfolio().c_str());
						portfolio_gh::get_instance().container().add(pPortfolio);
					}
					position* pPosition = pPortfolio->get_position(e->getTradeItem());
					pPosition->add_exec(e);

					//2015 09 08
					for (auto iter : m_connectionEventHandlers)
						iter->send_portfolio(o->get_portfolio(), n_sName, o->get_instrument());

					exec_gh::get_instance().GetBook().add(e);
					exec_persister::instance()->add_exec(e);
				}

				loggerv2::info("UpdateOrder:Ord(%06d) Way[%s] Instr[%s] Price[%f] ExecQty[%d] TotOrdQty[%d] bookqty[%d] Time[%s] Ptf[%s] ExecRef[%s] Action[%d]",
					o->get_id(),
					_OrderWay_VALUES_TO_NAMES.at(o->get_way()),//o->get_way() == W_BUY ? "BUY" : "SELL",
					o->get_instrument()->getCode().c_str(),
					//o->get_instrument()->get_precision(),
					execPrice,
					execQty,
					o->get_quantity(),
					bookQty,
					e->get_time().c_str(),
					//o->get_trading_type(),
					o->get_portfolio().c_str(),
					e->getReference().c_str(),
					o->get_last_action()
					);

				duplicat = false;
			}

			void connection::on_exec_from_market_cb(order* o, exec* e)
			{
				bool dul;
				on_exec_from_market_cb(o, e, dul);
			}

			int connection::getPortfolioNum(const string& portfolio)
			{
				boost::bimap<std::string, int>::left_iterator it = m_portfolioMap.left.find(portfolio);
				if (it != m_portfolioMap.left.end())
				{
					return it->second;
				}
				else
				{
					loggerv2::warn("Cannot find portfolio %s", portfolio.c_str());
					int nmax = 0;
					boost::bimap<std::string, int>::right_iterator itr = m_portfolioMap.right.begin();
					for (; itr != m_portfolioMap.right.end(); ++itr)
					{
						if (itr->first >= nmax)
							nmax = itr->first;
					}
					//insert itr
					m_portfolioMap.insert(boost::bimap<std::string, int>::value_type(portfolio, ++nmax));

					//save to file.
					std::ofstream in;
					in.open(m_portfolioFile.c_str(), std::ios::app | std::ios::out);
					in << std::endl;
					in << portfolio << "," << nmax << std::endl;
					in.close();
					loggerv2::info("Portfolio <%s , %d> has been added into file %s", portfolio.c_str(), nmax, m_portfolioFile.c_str());
					return nmax;
				}

				return 0;

			}

			const std::string& connection::getPortfolioName(int portfolionum)
			{
				boost::bimap<std::string, int>::right_iterator it = m_portfolioMap.right.find(portfolionum);
				if (it != m_portfolioMap.right.end())
				{
					//loggerv2::info("portfolio found %s", it->second.c_str());
					return (it->second);
				}

				loggerv2::error("unknown portfolio num:%d", portfolionum);
				return unknown;
			}

			void connection::loadAccount(const char* pfileName)
			{
				loggerv2::info("loading accounts file %s", pfileName);

				std::ifstream in(pfileName);

				if (!in.is_open())
				{
					loggerv2::info("Could not open the account file %s.", pfileName);


					std::ofstream in1;
					in1.open(pfileName, std::ios::app | std::ios::out);

					in1 << 1 << std::endl;
					in1.close();
					m_account_num = 1;
					loggerv2::info("account file %s has been added into file %d, init value is", pfileName, m_account_num);
					return;
					//create new file
				}




				std::string line;
				std::string account;
				//int i;
				std::vector<std::string> svect;

				//while (getline(in, line))
				//{
				//	//loggerv2::info("%s",line.c_str());
				//	boost::split(svect, line, boost::is_any_of(","));
				//	if (svect.size() == 2)
				//	{
				//		account = svect[0];
				//		i = atoi(svect[1].c_str());
				//		loggerv2::info("%s %d", account.c_str(), i);
				//		m_accountMap.insert(boost::bimap<std::string, int>::value_type(account, i));
				//	}
				//}
				getline(in, line);

				m_account_num = atoi(line.c_str());
				//loggerv2::info("loading accounts : %d accounts loaded", m_accountMap.left.size());

				in.close();
			}

			void connection::loadPortfolio(const char* pfileName)
			{
				loggerv2::info("load portfolio from %s", pfileName);

				std::ifstream in(pfileName);
				if (!in.is_open())
				{
					loggerv2::info("Could not open the portfolio file %s.", pfileName);
					return;
				}
				std::string line;

				std::string portfolio;
				int i;
				std::vector<std::string> svect;

				while (getline(in, line))
				{
					//loggerv2::info("%s",line.c_str());
					boost::split(svect, line, boost::is_any_of(","));
					if (svect.size() == 2)
					{
						portfolio = svect[0];
						i = atoi(svect[1].c_str());
						loggerv2::info("%s %d", portfolio.c_str(), i);
						m_portfolioMap.insert(boost::bimap<std::string, int>::value_type(portfolio, i));
					}
				}


				loggerv2::info("loading portfolio : %d portfolio loaded", m_portfolioMap.left.size());

				in.close();
			}

			void connection::on_trading_account_cb(tradingaccount* ta)
			{
				for (iorderbook_event_handler* handler : m_orderbookEventHandlers)
				{
					handler->update_tradingaccount_cb(ta);
				}

			}

			AtsType::OrderOpenClose::type connection::compute_open_close(order* ord, bool hasCloseToday)
			{
				tradeitem* i = ord->get_instrument();
				//if (m_debug)
				i->dumpinfo();

				int ordQty = ord->get_quantity();

				if (!i->getInstrument()->get_not_close_today())
				{
					switch (ord->get_way())
					{
					case AtsType::OrderWay::Sell: //short
					{

						if (hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX && i->get_yst_long_position() - i->get_pending_short_close_qty() - i->get_yst_comb_long() - ordQty >= 0 && i->get_yst_long_position() - i->get_should_pending_short_close_qty() - i->get_yst_comb_long() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close YstLong[%d] pendingShort[%d] YstCombLong[%d] ordQty[%d]", i->get_yst_long_position(), i->get_pending_short_close_qty(),i->get_yst_comb_long(), ordQty);
							return OrderOpenClose::Close;
						}
						if (hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX && i->get_today_long_position() - i->get_pending_short_close_today_qty() - ordQty >= 0 && i->get_today_long_position() - i->get_should_pending_short_close_qty() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close todayLong[%d] pendingShortToday[%d] ordQty[%d]", i->get_today_long_position(), i->get_pending_short_close_today_qty(), ordQty);
							return OrderOpenClose::CloseToday;
						}
						if (!(hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX) && i->get_tot_long_position() - i->get_pending_short_close_qty() - i->get_yst_comb_long() - ordQty >= 0 && i->get_tot_long_position() - i->get_should_pending_short_close_qty() - i->get_yst_comb_long() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close totLong[%d] pendingShort[%d] CombLong[%d] ordQty[%d]", i->get_tot_long_position(), i->get_pending_short_close_qty(), i->get_yst_comb_long(), ordQty);
							return OrderOpenClose::Close;
						}
						return OrderOpenClose::Open;
					}
					break;
					case AtsType::OrderWay::Buy: //long
					{
						if (hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX && i->get_yst_short_position() - i->get_pending_long_close_qty() - ordQty >= 0 && i->get_yst_short_position() - i->get_should_pending_long_close_qty() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close YstShort[%d] pendingLong[%d] ordQty[%d]", i->get_yst_short_position(), i->get_pending_long_close_qty(), ordQty);
							return OrderOpenClose::Close;
						}
						if (hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX && i->get_today_short_position() - i->get_pending_long_close_today_qty() - ordQty >= 0 && i->get_today_short_position() - i->get_should_pending_long_close_qty() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close todayShort[%d] pendingLongToday[%d] ordQty[%d]", i->get_today_short_position(), i->get_pending_long_close_today_qty(), ordQty);
							return OrderOpenClose::CloseToday;
						}
						if (!(hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX) && i->get_tot_short_position() - i->get_pending_long_close_qty() - i->get_yst_comb_short() - ordQty >= 0 && i->get_tot_short_position() - i->get_should_pending_long_close_qty() - i->get_yst_comb_short() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close totShort[%d] pendingLong[%d] CombLong[%d] ordQty[%d]", i->get_tot_short_position(), i->get_pending_long_close_qty(), i->get_yst_comb_short(), ordQty);
							return OrderOpenClose::Close;
						}
						return OrderOpenClose::Open;
					}
					break;
					default:
						return OrderOpenClose::Open;;
					}

				}
				else
				{
					switch (ord->get_way())
					{
					case AtsType::OrderWay::Sell: //short
					{
						//if (hasCloseToday && i->get_today_long_position() - i->get_pending_short_close_today_qty() - ordQty >= 0)
						if (i->get_today_long_position() > 0)
						{
							loggerv2::info("cffex_connection::compute_open_close todayLong[%d] pendingShortToday[%d] ordQty[%d]", i->get_today_long_position(), i->get_pending_short_close_today_qty(), ordQty);
							return 	OrderOpenClose::Open;
						}

						if (hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX && i->get_yst_long_position() - i->get_pending_short_close_qty() - ordQty >= 0 && i->get_yst_long_position() - i->get_should_pending_short_close_qty() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close YstLong[%d] pendingShort[%d] ordQty[%d]", i->get_yst_long_position(), i->get_pending_short_close_qty(), ordQty);
							return OrderOpenClose::Close;
						}

						if (!(hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX) && i->get_tot_long_position() - i->get_pending_short_close_qty() - i->get_yst_comb_long() - ordQty >= 0 && i->get_tot_long_position() - i->get_should_pending_short_close_qty() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close totLong[%d] pendingShort[%d] CombLong[%d] ordQty[%d]", i->get_tot_long_position(), i->get_pending_short_close_qty(), i->get_yst_comb_long(), ordQty);
							return OrderOpenClose::Close;
						}
						return OrderOpenClose::Open;
					}
					break;
					case AtsType::OrderWay::Buy: //long
					{
						//if (hasCloseToday && i->get_today_short_position() - i->get_pending_long_close_today_qty() - ordQty >= 0)
						if (i->get_today_short_position() > 0)
						{
							loggerv2::info("cffex_connection::compute_open_close todayShort[%d] pendingLongToday[%d] ordQty[%d]", i->get_today_short_position(), i->get_pending_long_close_today_qty(), ordQty);
							return OrderOpenClose::Open;
						}

						if (hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX && i->get_yst_short_position() - i->get_pending_long_close_qty() - ordQty >= 0 && i->get_yst_short_position() - i->get_should_pending_long_close_qty() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close YstShort[%d] pendingLong[%d] ordQty[%d]", i->get_yst_short_position(), i->get_pending_long_close_qty(), ordQty);
							return OrderOpenClose::Close;
						}

						if (!(hasCloseToday && i->get_close_order() == CLOSE_PRIORITY::FIX) && i->get_tot_short_position() - i->get_pending_long_close_qty() - i->get_yst_comb_short() - ordQty >= 0 && i->get_tot_short_position() - i->get_should_pending_long_close_qty() - i->get_yst_comb_short() - ordQty >= 0)
						{
							loggerv2::info("cffex_connection::compute_open_close totShort[%d] pendingLong[%d] CombShort[%d] ordQty[%d]", i->get_tot_short_position(), i->get_pending_long_close_qty(), i->get_yst_comb_short(), ordQty);
							return OrderOpenClose::Close;
						}
						return OrderOpenClose::Open;
					}
					break;
					default:
						return OrderOpenClose::Open;
					}


				}


			}

			void connection::set_is_last(bool value)
			{
				loggerv2::error("connection::set_is_last reload the dico file again!\n");
				m_bIsLast = value;
			}

			int connection::prepare_create_order(order* o, char* pszReason)
			{
				if (is_busy())
				{
					snprintf(pszReason, REASON_MAXLENGTH, "prepare_create_order fail,connection is busy");
					loggerv2::error("Connection %s is Busy, Can not create order", getName().data());
					return 0;
				}

				o->m_nId = order_reference_provider::get_instance().get_next_int();

				o->set_last_action(OrderAction::Created);

				if (m_bTradingAllowed == false)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "trading not allowed");
					o->set_lastreason(pszReason);
					loggerv2::error("connection::create %s", pszReason);
					return 0;
				}

				tradeitem *i = o->get_instrument();
				if (!i)
				{
					snprintf(pszReason, REASON_MAXLENGTH, "instrument NULL");
					o->set_lastreason(pszReason);
					loggerv2::error("connection::create %s", pszReason);
					return 0;
				}
				else if (m_crossCheck && !o->get_bypass_crosscheck())
				{
					int outid = 0;
					double best = 0.0;

					if (!i->get_order_book()->cross_check_validate(o->get_price(), o->get_way(), outid, best))
					{
						tbb::concurrent_hash_map<int, order*>::const_accessor ra;

						if (!m_activeOrders.find(ra, outid))
						{
							i->get_order_book()->remove_order(outid);
						}
						else
						{
							snprintf(pszReason, REASON_MAXLENGTH, "cross check,price:%lf,orderid:%d", best, outid);
							o->set_lastreason(pszReason);
							loggerv2::error("connection:: create order (id:%s,price:%lf,qty:%d,orderid:%d) rejected by cross_check.best price come from order:%d price %f bookquantity %d status %s",
								i->getCode().c_str(), o->get_price(), o->get_quantity(), o->get_id(), outid, ra->second->get_price(), ra->second->get_book_quantity(), _OrderStatus_VALUES_TO_NAMES.at(ra->second->get_status()));
							o->set_status(OrderStatus::Reject);
							ra->second->dump_info();
							return 0;
						}
						ra.release();
					}
				}


				if (o->m_nQuantity <= 0 || (o->m_PriceMode == OrderPriceMode::Limit && o->m_dPrice <= 0))
				{
					snprintf(pszReason, REASON_MAXLENGTH, "wrong quantity [%d] / price [%f]", o->m_nQuantity, o->m_dPrice);
					//o->set_lastreason(pszReason);
					loggerv2::error("connection::create %s", pszReason);
					return 0;
				}

				o->m_nBookQuantity = o->m_nQuantity;
				o->set_status(OrderStatus::WaitServer);
				o->m_preAction = OrderAction::Created;
				o->m_preStatus = OrderStatus::Nack;
				o->m_lastTime = get_lwtp_now();
				o->dump_info();
				return 1;
			}


			}
		}
	}

