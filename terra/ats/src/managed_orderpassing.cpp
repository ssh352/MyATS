#include "managed_orderpassing.h"
#include "define.h"
#include <exec_book_notifier.h>
#include <order_book_notifier.h>
#include <conn_status_change_notifier.h>
#include "boost/property_tree/ini_parser.hpp"
#include "tradeItem_gh.h"
#include "order_gh.h"
#include "order_reference_provider.h"
#include "exec_persister.h"
#include "connection_gh.h"
#include "order.h"
#include <boost/lexical_cast.hpp>

#ifdef CON_CTP
#include "cffex/include/ctp_connection.h"
#endif
#ifdef CON_LTS
#include "lts/include/lts_connection.h"
#endif
#ifdef CON_XS
#include "xspeed/include/xs_connection.h"
#endif
#ifdef CON_XS2
#include "xspeed_of/include/xs_of_connection.h"
#endif
#ifdef CON_FS
#include "fs/include/fs_connection.h"
#endif
#ifdef CON_SL
#include "sl/include/sl_connection.h"
#endif
#ifdef CON_X1
#include "x1/include/x1_connection.h"
#endif
#ifdef CON_ES
#include "es/include/es_connection.h"
#endif
#ifdef CON_FEMAS
#include "femas/include/femas_connection.h"
#endif
#ifdef CON_IB
#include "ib/include/ib_connection.h"
#endif
#ifdef CON_ZD
#include "zd/include/zd_connection.h"
#endif
#ifndef Linux
#ifdef CON_LTSFILE
#include "lts_file/include/lts_file_connection.h"
#endif
#ifdef CON_GX
#include "gx/include/gx_connection.h"
#endif
#ifdef CON_GXFILE
#include "gx_file/include/gx_file_connection.h"
#endif
#ifdef CON_LTSFILE
#include "ht_file/include/ht_file_connection.h"
#endif
#ifdef CON_FS
#include "sf/include/sf_connection.h"
#endif
#endif

using namespace terra::marketaccess::orderpassing;

namespace terra
{
	namespace ats
	{
		managed_orderpassing::managed_orderpassing()
		{
			m_isAlive = true;


			/*m_mutex = new RTMutex();
			m_mutex->Create();*/
		}

		managed_orderpassing::~managed_orderpassing()
		{
			/*if (m_mutex)
				m_mutex->Destroy();*/
		}

		int managed_orderpassing::Initialize(const std::string& connectionFile/*, const std::string& logFile*/, const std::string& todayDir)
		{
			return Initialize(connectionFile/*, logFile*/, todayDir, "");
		}

		void write_path(const std::string &path, char c)
		{
			std::string fn = "../LastDaily.txt";
			std::ofstream f1(fn.data(), std::ios::out);
			if (!f1.bad())
			{
				f1 << path << "," << c << std::endl;
				f1.close();
			}
		}

		bool read_path(std::string &path, string& session)
		{
			std::string fn = "../LastDaily.txt";
			std::ifstream f1(fn.data(), std::ios::in);
			if (f1.is_open())
			{
				std::string buf;
				std::vector<std::string>vec;

				getline(f1, buf);
				f1.close();

				boost::split(vec, buf, boost::is_any_of(","));
				if (vec.size() < 2)
					return false;

				/*if (vec[0].size() < 10 || vec[1].at(0) != 'N'||vec[0].at(vec[0].size()-2)!='N')
				{
				return false;
				}*/
				path = vec[0];
				session = vec[1];
				//write_path(path,'D');
				return true;
			}
			else
				return false;
		}

		int managed_orderpassing::Initialize(const std::string& connectionFile/*, const std::string& logFile*/, const std::string& todayDir, const std::string& db)
		{
			//const char* pszLogFile = logFile.c_str();

			//std::stringstream ss;
			std::string m_path, session;

			bool read_res = read_path(m_path, session);
			bool is_night_session = todayDir.size() > 0 && todayDir.at(todayDir.size() - 1) == 'N';

			if (is_night_session)//夜盘是新一个交易日的开始
			{
				if (m_path != todayDir || !read_res)
				{
					write_path(todayDir, 'N'); //update file
				}
			}
			else
			{
				if (m_path != todayDir)
				{
					if (read_res && session == "N")
					{
						boost::filesystem::path src = boost::filesystem::path(m_path);
						boost::filesystem::path dst = boost::filesystem::path(todayDir);
						if (boost::filesystem::exists(src))
						{

							boost::system::error_code ec;
							for (directory_entry& x : directory_iterator(src))
							{

								const boost::filesystem::path newDst = dst / x.path().filename();
								boost::filesystem::copy_file(x, newDst, boost::filesystem::copy_option::overwrite_if_exists, ec);
								if (ec)
								{
									loggerv2::error(ec.message().c_str());
								}
							}
						}
					}
					write_path(todayDir, 'D');
				}

			}


			loggerv2::info("-------------------------------- Wrapper STARTED --------------------------------");
			exec_persister::instance()->set_directory_name(todayDir.c_str());

			int nbSources = 0;
			if (!boost::filesystem::exists(connectionFile))
			{
				loggerv2::error("file does not exist :%s", connectionFile.data());
				return 0;
			}
			if (connectionFile.length() < 1)
				return -1;

			boost::filesystem::path p(connectionFile);
			if (!boost::filesystem::exists(p))
			{
				printf_ex("ats_config::load config_file:%s not exist!\n", connectionFile.c_str());
				return false;
			}
			boost::property_tree::ptree root;
			boost::property_tree::ini_parser::read_ini(connectionFile, root);

			std::string names = root.get<string>("CONNECTION.names", "");

			//std::string ctime = root.get<string>("GLOBAL.CutOffTime", "");
			/*time_duration cutoffTime;
			if (ctime.size() != 0)
			cutoffTime = duration_from_string(p.string());
			else
			cutoffTime = time_duration(18, 0, 0);
			*/
			int orderStartInt;

			if (is_night_session)
			{
				orderStartInt = root.get<int>("GLOBAL.t1_order_ref", 50000);
			}
			else
			{
				orderStartInt = root.get<int>("GLOBAL.order_ref_int", 0);
			}




			order_reference_provider::get_instance().initialize((todayDir + "/orderRef.ini").c_str(), orderStartInt);
			std::vector<std::string> namesArray;
			boost::split(namesArray, names, boost::is_any_of(","));
			for (std::string name : namesArray)
			{
				std::string type = root.get<string>(name + ".type", "");
				//string stock_account = root.get<string>(name + ".stock_account", "");
				//string stock_channel = root.get<string>(name + ".stock_channel", "");
				connection * con = nullptr;
				bool checkCancel = false;
				if (type == "CTP")
				{
#ifdef CON_CTP
					checkCancel = true;
					con = new cffex::cffex_connection(false);
#endif	
				}
				else if (type == "LTS")
				{
#ifdef CON_LTS
					con = new lts::lts_connection(false);
#endif	
				}
				else if (type == "LTS_FILE")
				{
#ifdef CON_LTSFILE
#ifndef Linux
					con = new lts_file::lts_file_connection(false);
					((lts_file::lts_file_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif	
#endif	
				}
				else if (type == "GX")
				{
#ifdef CON_GX
#ifndef Linux
					con = new gx::gx_connection(false);
					((gx::gx_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif
#endif	
				}
				else if (type == "XS")
				{
#ifdef CON_XS
					con = new xs::xs_connection(todayDir, false);
#endif	
				}
				else if (type == "XS2")
				{
#ifdef CON_XS2
					con = new xs_of::xs_of_connection(false);
#endif	
				}
				else if (type == "X1")
				{
#ifdef CON_X1
#ifdef Linux
					con = new x1::x1_connection(todayDir, false);
#endif
#endif
				}
				else if (type == "IB")
				{
#ifdef CON_IB
					//#ifndef Linux
					con = new ib::ib_connection(false);
					//#endif
#endif	
				}
				else if (type == "FS")
				{
#ifdef CON_FS
					con = new fs::fs_connection(false);
					((fs::fs_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif	
				}
				else if (type == "SL")
				{
#ifdef CON_SL
					con = new sl::sl_connection(false);
					((sl::sl_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif	
				}
				else if (type == "ES")
				{
#ifdef CON_ES
					con = new es::es_connection(false);
					((es::es_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif	
				}
				else if (type == "FEMAS")
				{
#ifdef CON_FEMAS
					con = new femas::femas_connection(false);
					((femas::femas_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif	
				}
				else if (type == "SF")
				{
#ifndef Linux
#ifdef CON_SF
					con = new sf::sf_connection(false);
#endif
#endif
				}
				else if (type == "ZD")
				{
#ifdef CON_ZD
					con = new zd::zd_connection(false);
					((zd::zd_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif
				}
				else if (type == "GX_FILE")
				{
#ifdef CON_GXFILE
#ifndef Linux
					con = new gx_file::gx_file_connection(false);
					((gx_file::gx_file_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif
#endif
				}
				else if (type == "HT_FILE")
				{
#ifdef CON_HTFILE
#ifndef Linux
					con = new ht_file::ht_file_connection(false);
					((ht_file::ht_file_connection*)con)->init_user_info((char*)todayDir.c_str());
#endif
#endif
				}
				if (con != nullptr)
				{

					con->setStatus(ConnectionStatus::Disconnected);

					AddConnectionEventHandler(con, connection_CB::instance());
					AddConnectionStatusEventHandler(con, connection_status_CB::instance());
					con->init_connection();
				}

				if (con != nullptr)
				{
					if (db.size() == 0)
						con->setup(name, connectionFile);
					else
						con->setup(name, connectionFile, db.c_str());
				}

				if (con != nullptr)
				{
					connection_gh::get_instance().container().add(con);
					nbSources++;
					exec_persister::instance()->load(con->getName());
				}
				else
				{
					loggerv2::error("Connection %s Not Initialized!", name.c_str());
				}

			}

			std::thread td(std::bind(&managed_orderpassing::Process, this));
			m_Thread.swap(td);
			return nbSources;

		}

		void managed_orderpassing::Terminate()
		{
			m_isAlive = false;
			m_Thread.join();


		}




		void managed_orderpassing::Process()
		{
			exec_persister::instance()->start();
			time_t now;
			time_t lastIdleCheck = time(NULL);

			while (m_isAlive)
			{
				now = time(NULL);
				if (now >= lastIdleCheck + 1)
				{
					lastIdleCheck = now;
					auto ConMap = connection_gh::get_instance().container().get_map();
					auto it = ConMap.begin();
					for (; it != ConMap.end(); ++it)
					{
						it->second->process_idle();
					}
				}
				lastIdleCheck = now;
				sleep_by_milliseconds(300);

			}

			exec_persister::instance()->stop();
			order_reference_provider::get_instance().close();
		}

		void managed_orderpassing::AddConnectionEventHandler(connection* m_pImpl, connection_CB* handler)
		{
			m_pImpl->add_connection_event_handler(handler);

			loggerv2::info("add_orderbook_event_handler %d", handler);
			m_pImpl->add_orderbook_event_handler(handler);

			loggerv2::info("ad add_quotebook_event_handler %d", handler);
			m_pImpl->add_quotebook_event_handler(handler);

			loggerv2::info("add_execbook_event_handler %d", handler);
			m_pImpl->add_execbook_event_handler(handler);
			loggerv2::info("add_connection_event_handler %d", handler);
			m_pImpl->add_connection_event_handler(handler);
		}

		void managed_orderpassing::AddConnectionStatusEventHandler(connection* m_pImpl, connection_status_CB* handler)
		{
			m_pImpl->add_connection_status_event_handler(handler);
		}

		//-------------------------------------------------------------------
		connection_status_CB* connection_status_CB::instance()
		{
			if (ms_pInstance == nullptr)
				ms_pInstance = new connection_status_CB();
			return ms_pInstance;
		}

		connection_status_CB::connection_status_CB()
		{

		}

		void connection_status_CB::connection_status_cb(connection* con, ConnectionStatus::type newStatus, const char* pszMessage)
		{

			conn_status_change_notifier::get_instance().on_status_changed(con);
		}

		connection_status_CB* connection_status_CB::ms_pInstance;

		//-----------------------------------
		connection_CB::connection_CB()
		{

		}

		void connection_CB::add_order_cb(order* o)
		{
			order_gh::get_instance().GetBook()->add(o);
			order_book_notifier::get_instance().add_order_cb(o);
		}
		void  connection_CB::update_tradingaccount_cb(tradingaccount* ta)
		{

		}

		void connection_CB::update_order_cb(order* o)
		{
			order_book_notifier::get_instance().update_order_cb(o);
		}

		void connection_CB::add_exec_cb(exec* e)
		{
			exec_book_notifier::get_instance().add_exec_cb(e);
		}

		void connection_CB::add_exec_cb(exec* pExec, order* o)
		{
			//To Implement
		}

		void connection_CB::new_instrument_cb(void* con, tradeitem* i)
		{

		}

		connection_CB* connection_CB::instance()
		{
			if (ms_pInstance == nullptr)
				ms_pInstance = new connection_CB();
			return ms_pInstance;
		}

		void connection_CB::add_quote_cb(quote* o)
		{
			quote_gh::get_instance().GetBook()->add(o);
			quote_book_notifier::get_instance().add_quote_cb(o);
		}

		void connection_CB::update_quote_cb(quote* o)
		{
			quote_book_notifier::get_instance().update_quote_cb(o);
		}
		void connection_CB::set_quote_sys_id_cb(tradeitem* i, const string & id)
		{

		}
		connection_CB* connection_CB::ms_pInstance;

		//----------------------------------------------------
		order_observer_CB* order_observer_CB::instance()
		{
			if (ms_pInstance == nullptr)
				ms_pInstance = new order_observer_CB();
			return ms_pInstance;
		}

		order_observer_CB::order_observer_CB()
		{

		}

		void order_observer_CB::add_order_cb(order* pOrder)
		{

		}

		void order_observer_CB::update_order_cb(order* pOrder)
		{
			//iorderobserver *ob = pOrder->getObserver();
			//pOrder->getObserver()->update_order_cb(pOrder);
		}

		void order_observer_CB::inactive_order_cb(order* pOrder)
		{
			//pOrder->getObserver()->inactive_order_cb(pOrder);
		}

		void order_observer_CB::add_exec_cb(exec* pExec)
		{
			//order* order = order_gh::get_instance().GetBook()->GetById(pExec->getOrderId());
			//order->getObserver()->add_exec_cb(pExec);
		}

		void order_observer_CB::clear_orders()
		{

		}

		void order_observer_CB::clear_execs()
		{

		}

		order_observer_CB* order_observer_CB::ms_pInstance;
	}
}
