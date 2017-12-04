#include "atsmanager.h"
#include "feedsourcefactory.h"
#include "atsconfig.h"
#include "position_persister.h"
#include "managed_orderpassing.h"
#include "order_security_controller.h"

using namespace terra::marketaccess::orderpassing;
namespace terra
{
	namespace ats
	{
		ats_manager* ats_manager::g_ATSManager = nullptr;

		ats_manager::ats_manager()
		{
			net_mq_publisher = nullptr;
			//m_feed_decoder_work_flow.register_workflow(new feed_task());
			//m_feed_decoder_work_flow.set_name("feed_decoder_thread");
		}


		ats_manager::~ats_manager()
		{
		}
		void ats_manager::initialize_logger()
		{
			std::string logFile = ats_config::get_instance()->get_log_file();
			loggerv2::init(logFile);
			//boost::shared_ptr<terra::common::memory_log_sink> sink(new terra::common::memory_log_sink(boost::make_shared<sinks::text_file_backend>()));
			//log_sink = sink;
			//logging::core::get()->add_sink(sink);
			return ;
		}
		void ats_manager::initialize_referential()
		{
			printf_ex("initialize referential...start\n");
			referential::get_instance()->load(ats_config::get_instance()->get_referential_path(), ats_config::get_instance()->get_currency_file(), ats_config::get_instance()->get_db_file());
			//TradingPeriodManager.load(ats_config::get_instance()->get_trading_phase_file());
			printf_ex("initialize referential...end\n");
		}
		void ats_manager::initialize_feed_sources()
		{
			printf_ex("initialize feed sources...start\n");
			feed_source_factory::get_instance()->initialize_feed_sources(ats_config::get_instance()->get_feed_source_file(), ats_config::get_instance()->get_db_file());
			//io_service_gh::get_instance().start_feed_io();
			printf_ex("initialize feed sources...end\n");
		}
		void ats_manager::initialize_order_passing()
		{
			printf_ex("initialize order passing...start\n");
			std::string connectionFile, logFile, todayDir, db;
			connectionFile = ats_config::get_instance()->get_connection_file();
			//logFile = ats_config::get_instance()->get_wrapper_log_file();
			todayDir = ats_config::get_instance()->get_daily_directory();
			db = ats_config::get_instance()->get_db_file();
			managed_orderpassing::get_instance().Initialize(connectionFile/*, logFile*/, todayDir, db);
			//io_service_gh::get_instance().start_trader_io();
			printf_ex("initialize order passing...end\n");
		}
		void ats_manager::initialize_position()
		{
			printf_ex("initialize position...start\n");
			position_persister::load(ats_config::get_instance()->get_position_file().c_str());
			printf_ex("initialize position...end\n");
		}
		void ats_manager::initialize_security()
		{
			printf_ex("initialize security...start\n");
			terra::ats::security::order_security_controller::get_instance().load(ats_config::get_instance()->get_security_file().c_str());			
			printf_ex("initialize security...end\n");
		}
		void ats_manager::close_all()
		{
			std::map<std::string, connection*>  map = connection_gh::get_instance().container().get_map();			
			for(auto &it:map)
			{
				connection* conn = it.second;				
				conn->disconnect();
				conn->release();				
			}						
			for(auto &it:*feed_source_container::get_instance())
			{
				feed_source * pSource = it.second;
				pSource->remove_all_item();	
				pSource->release_source();			
			}
		}

		void ats_manager::active_feed()
		{
			//m_feed_select_listener.SetTimer(0, 1 * 5000);
			feed_source_container::iterator it = feed_source_container::get_instance()->begin();
			while (it != feed_source_container::get_instance()->end())
			{
				feed_source * pSource = it->second;
				//pSource->set_status_callback(feed_source_status_callback);
				//pSource->set_decoder_callback(feed_item_data_callback);
				if (pSource->get_type()!="TDF")
					pSource->init_source();
				it++;
			}
			//m_feed_decoder_work_flow.run();
		}

		void ats_manager::active_io_service()
		{
#ifdef Linux
			io_service_gh::get_instance().set_is_bind_feed_trader_core(ats_config::get_instance()->get_is_bind_feed_trader_core());
#endif
			io_service_gh::get_instance().start_feed_io(ats_config::get_instance()->get_feed_io_cpu_core());
			io_service_gh::get_instance().start_trader_io(ats_config::get_instance()->get_trader_io_cpu_core());
			io_service_gh::get_instance().init_other_io(ats_config::get_instance()->get_other_io_cpu_core());

			//io_service_gh::get_instance().start_other_io();
		}

		void ats_manager::active_twap_thread_pool()
		{
			//twap_thread_pool::get_instance().init();
		}
		//void feed_task::execute()
		//{
		//	//cta_ats_server::get_instance().get_feed_select_listener().SelectAndProcess();		
		//}

	}
}