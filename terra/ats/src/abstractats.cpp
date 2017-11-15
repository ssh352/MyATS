#include "abstractats.h"
#include "atsinstrument.h"
#include "atsmanager.h"
#include "defaultdatetimepublisher.h"
#include "atsconfig.h"
#include "order_gh.h"
#include "feedsourcefactory.h"
#include "connection.h"
#include "connection_gh.h"
#include "managed_orderpassing.h"

using namespace terra::common;
using namespace terra::feedcommon;
namespace terra
{
	namespace ats
	{
		abstract_ats::abstract_ats(string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections)
		{
			m_started = false;
			m_workFlowInitialized = false;
			m_auto_status = false;
			m_portfolioName = portfolioName;
			m_feedsources = feedsources;
			m_connections = connections;
			this->m_today_pnl_bary = 0.0;
			this->m_today_pnl_mid = 0.0;
			this->m_today_pnl_last = 0.0;
			this->m_yesterday_pnl_bary = 0.0;
			this->m_yesterday_pnl_last = 0.0;
			this->m_yesterday_pnl_mid = 0.0;
			this->m_fees_broker = 0.0;
			this->m_fees_exchange = 0.0;
			if (!boost::filesystem::exists(ats_config::get_instance()->get_data_directory()))
			{
				boost::filesystem::create_directory(ats_config::get_instance()->get_data_directory());
			}
			boost::filesystem::path p;
			p.clear();
			p.append(ats_config::get_instance()->get_data_directory());
			p.append(portfolioName);
			m_strAtsDirectory = p.string();
			if (!boost::filesystem::exists(m_strAtsDirectory))
			{
				boost::filesystem::create_directory(m_strAtsDirectory);
			}
			time_duration span(0, 0, 3);
			m_buffer_time = span;
			m_price_finished = false;
			m_eod_finished = false;
			m_intra_data = nullptr;
			m_base_ats_data = nullptr;
			m_ats_daily_data = nullptr;
			//m_IsCancellationRequested = false;
			m_yesterdayPositionType = YesterdayPositionType::Local;
		}


		abstract_ats::~abstract_ats()
		{
			if (m_intra_data != nullptr)
			{
				delete m_intra_data;
				m_intra_data = nullptr;
			}
			if (m_ats_daily_data != nullptr)
			{
				delete m_ats_daily_data;
				m_ats_daily_data = nullptr;
			}
			if (m_base_ats_data != nullptr)
			{
				delete m_base_ats_data;
				m_base_ats_data = nullptr;
			}
		}
		void abstract_ats::start()
		{
			if (m_started == true)
				return;

			if (this->m_intra_data != nullptr)
			{
				this->m_intra_data->load_data(m_stream_writer.get_file_name());
			}

			//		m_stream_writer.open();

			bool feedNotReady = true;
			int countDown = 0;
			while (feedNotReady && countDown < 10)
			{
				feedNotReady = false;

				for (auto & it : *feed_source_container::get_instance())
				{
					if (it.second->get_status() == AtsType::FeedSourceStatus::Down)
					{
						feedNotReady = true;
						loggerv2::error("FeedSource %s is not ready.", it.first.c_str());
						sleep_by_milliseconds(ats_config::get_instance()->get_ats_timer());
						break;
					}
				}
				countDown++;
			}

			for (auto &it : connection_gh::get_instance().container().get_map())
			{
				it.second->StatusChanged = boost::bind(&abstract_ats::connection_status_changed, this, _1);
			}

			for (auto & it : *feed_source_container::get_instance())
			{
				it.second->Status_Changed_Handler = boost::bind(&abstract_ats::feed_source_status_changed, this, _1);
			}

			for (auto &it : AtsInstrumentList)
			{
				it->start_feed();
			}

			m_started = true;
		}
		void abstract_ats::stop()
		{
			if (m_started == false)
				return;

			/*for (auto &it : connection_gh::get_instance().container().get_map())
			{
			it.second->StatusChanged.clear();
			}

			for (auto & it : *feed_source_container::get_instance())
			{
			delete it.second->Status_Changed_Handler;
			}*/

			for (auto &it : AtsInstrumentList)
			{
				it->stop_feed();
			}

			if (m_stream_writer.is_open())
				m_stream_writer.close();

			if (this->m_portfolioName != this->m_default_name)
			{
				save_config();
				save_config_daily();
			}

			m_started = false;
		}
		void abstract_ats::start_workflow()
		{
			if (!m_workFlowInitialized)
			{
				initialize_workflow();
				m_workFlowInitialized = true;
				//t_workflow = std::thread(std::bind(&abstract_ats::work_flow_interate, this));
				ats_work_flow.set_sleep_timer(ats_config::get_instance()->get_ats_timer());
				ats_work_flow.run();
				//t.detach();
			}
		}
		void abstract_ats::do_pricing()
		{
			for (auto &it : AtsInstrumentList)
			{
				if (it->get_feed_item() != nullptr)
				{
					it->get_feed_item()->compute_bary_center();
				}
			}
		}
		void abstract_ats::stop_workflow()
		{
			//m_IsCancellationRequested = true;
			ats_work_flow.stop();
		}
	/*	void abstract_ats::work_flow_interate()
		{
			int total_ms = 0;
			int last_num = 0;
			while (!m_IsCancellationRequested)
			{
				ptime last = date_time_publisher_gh::get_instance()->now();
				
				if (print_screen)
				{
					++num_workflow;
					if (num_workflow >= last_num + 100)
					{
						last_num += 100;
						std::cout << "current phase: " << _TradingPhase_VALUES_TO_NAMES.at(currentTradingPhase) << " task number: " << last_num << "  consume ms: "<<total_ms<<std::endl;
						total_ms = 0;
					}
				}

				try
				{
					last = date_time_publisher_gh::get_instance()->now();
					this->_atsWorkflow.iterate();
					total_ms += (date_time_publisher_gh::get_instance()->now() - last).total_milliseconds();

				}
				catch (std::exception& ex)
				{
					std::cout << ex.what() << std::endl;
					loggerv2::error(ex.what());
				}
				sleep_by_milliseconds(ats_config::get_instance()->get_ats_timer());
			}
		}*/
		void abstract_ats::shift_eod_cb()
		{
			if (this->m_auto_status)
			{
				loggerv2::error("stop_ats:%s  EndOfDay CB and Automaton still ON...", this->m_portfolioName.c_str());
				this->stop_automaton(true);
			}

			for (auto &it : AtsInstrumentList)
			{
				if (it->get_position() != nullptr)
				{
					it->get_position()->set_yesterday_position_local(it->get_position()->get_total_position());
					it->get_position()->set_yesterday_price_local(it->get_position()->m_dYesterdayPriceTemp);
				}
			}
		}
		void abstract_ats::shift_price_cb()
		{
			loggerv2::info("%s: Shift Today Prices...", get_name().c_str());
			for (auto &it : AtsInstrumentList)
			{
				if (it->get_position() != nullptr && it->get_feed_item() != nullptr)
				{
					it->get_position()->m_dYesterdayPriceTemp = it->get_feed_item()->get_bary_center();
					loggerv2::info("%s: %f", it->get_instrument()->get_code().c_str(), it->get_position()->m_dYesterdayPriceTemp);
				}
			}
		}
		void abstract_ats::compute_pnl()
		{

			double temp_yesterday_pnl_bary = 0;
			double temp_today_pnl_bary = 0;
			double temp_yesterday_pnl_mid = 0;
			double temp_today_pnl_mid = 0;
			double temp_yesterday_pnl_last = 0;
			double temp_today_pnl_last = 0;
			double temp_fees_exchange = 0;
			double temp_fees_broker = 0;
			for (auto &it : AtsInstrumentList)
			{

				it->compute_pnl();
				it->compute_fees();

				temp_yesterday_pnl_bary += it->get_yesterday_phl_bary_center();
				temp_today_pnl_bary += it->get_today_phl_bary_center();
				temp_yesterday_pnl_mid += it->get_yesterday_pnl_mid();
				temp_today_pnl_mid += it->get_today_pnl_mid();
				temp_yesterday_pnl_last += it->get_yesterday_pnl_last();
				temp_today_pnl_last += it->get_today_pnl_last();
				temp_fees_exchange += it->get_exchange_fees();
				temp_fees_broker += it->get_broker_fees();
			}
			m_yesterday_pnl_bary = temp_yesterday_pnl_bary;
			m_today_pnl_bary = temp_today_pnl_bary;
			m_yesterday_pnl_mid = temp_yesterday_pnl_mid;
			m_today_pnl_mid = temp_today_pnl_mid;
			m_yesterday_pnl_last = temp_yesterday_pnl_last;
			m_today_pnl_last = temp_today_pnl_last;
			m_fees_exchange = temp_fees_exchange;
			m_fees_broker = temp_fees_broker;

		}
		void abstract_ats::compute_trading_phase()
		{
			//current_task = "compute trading phase";
			currentTradingPhase = TradingPeriodManager.get_trading_phase(date_time_publisher_gh::get_instance()->get_time_of_day());
			if (is_close() && get_auto_status())
			{

				loggerv2::warn("stop_ats:[%s] Non trading hours", get_name().c_str());
				stop_automaton(true);

			}
		}
		void abstract_ats::set_yesterday_position_type(YesterdayPositionType::type type)
		{
			m_yesterdayPositionType = type;
			for (auto &it : AtsInstrumentList)
			{
				if (it->get_position() != nullptr)
				{
					it->get_position()->set_yesterday_position_type(type);
				}
			}
		}
		void abstract_ats::on_name_changed()
		{
			for (auto &it : AtsInstrumentList)
			{
				it->update_portfolio(this->get_name());
			}
			string dumpDirectory = ats_config::get_instance()->get_dump_directory();

			boost::filesystem::path p;
			p.clear();
			p.append(dumpDirectory);
			string fileName = this->get_name() + "_" + to_iso_string(date_time_publisher_gh::get_instance()->today()) + ".csv";
			p.append(fileName);
			fileName = p.string();
			m_stream_writer.set_file_name(fileName);
		}
		void abstract_ats::check_callbacks()
		{
			time_duration now = date_time_publisher_gh::get_instance()->get_time_of_day();
			time_duration s = TradingPeriodManager.get_current_stop_time(now);
			if (now >= s && now < (s + m_buffer_time) && now >(m_stop_time + m_buffer_time))
			{
				m_stop_time = now;
				loggerv2::error("stop_ats:check_callbacks");
				stop_automaton(true);
			}
			// Price
			if (!m_price_finished && now >= TradingPeriodManager.get_shift_price_time() && now < TradingPeriodManager.get_shift_price_time() + m_buffer_time)
			{
				m_price_finished = true;
				this->shift_price_cb();
			}

			// EOD
			if (!m_eod_finished && now > TradingPeriodManager.get_shift_eod_time() && now < TradingPeriodManager.get_shift_eod_time() + m_buffer_time)
			{
				m_eod_finished = true;
				shift_eod_cb();
				save_config();
				save_config_daily();
			}
			if (!m_eod_finished && now > TradingPeriodManager.get_shift_eod_time_n() && now < TradingPeriodManager.get_shift_eod_time_n() + m_buffer_time)
			{
				save_config();
				save_config_daily();
				order_gh::get_instance().GetBook()->killall();
			}
		}
		ats_instrument * abstract_ats::find(const string & code)
		{
			for (auto &it : AtsInstrumentList)
			{
				if (it->get_instrument() != nullptr)
				{
					if (it->get_instrument()->get_code() == code)
						return it;
				}
			}
			return nullptr;
		}
		void abstract_ats::load_config_daily()
		{
			if (m_ats_daily_data != nullptr)
				m_ats_daily_data->load_daily();
		}
		void abstract_ats::save_config_daily()
		{
			if (m_ats_daily_data != nullptr)
				m_ats_daily_data->save_daily();
		}
		void abstract_ats::load_config()
		{
			if (m_base_ats_data != nullptr)
				m_base_ats_data->load(m_strAtsDirectory);
		}
		void abstract_ats::save_config()
		{
			if (m_base_ats_data != nullptr)
				m_base_ats_data->save(m_strAtsDirectory);
		}
		void abstract_ats::start_automaton()
		{
			if (m_auto_status)
				return;
			m_auto_status = true;
		}
		void abstract_ats::stop_automaton(bool playSound)
		{
			if (!m_auto_status)
				return;
			loggerv2::error("stopping ats %s", this->get_name().c_str());
			m_auto_status = false;
			if (playSound)
			{

			}
		}
		void abstract_ats::feed_source_status_changed(feed_source * source)
		{
			if (source != nullptr)
			{
				if (source->get_status() == AtsType::FeedSourceStatus::Down)
				{
					if (m_auto_status == true)
					{
						loggerv2::error("stop_ats:FeedSource %s is DOWN", source->get_name().c_str());
						this->stop_automaton(true);
					}
				}
			}
		}
		void abstract_ats::connection_status_changed(terra::marketaccess::orderpassing::connection * conn)
		{
			if (conn != nullptr)
				return;
			if (conn->getStatus() == ConnectionStatus::Connected)
			{

			}
			else
			{
				if (m_auto_status == true)
				{
					loggerv2::error("stop_ats:Connection %s is DOWN", conn->getName().c_str());
					this->stop_automaton(true);
				}
			}
		}
	}
}