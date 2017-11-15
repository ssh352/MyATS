#ifndef _ABSTRACT_ATS_V2_H_
#define _ABSTRACT_ATS_V2_H_
//#include "atsworkflow.h"
#include "work_flow.h"
#include "AtsType_types.h"
#include "atsintradata.h"
#include "order.h"
#include "atsdailydata.h"
#include "baseatsdata.h"
#include "async_writer_new.h"
#include "feedsource.h"
#pragma once
#include <tradingperiodmanager.h>
using namespace terra::instrument;
using namespace terra::common;
using namespace terra::feedcommon;
using namespace AtsType;
using namespace terra::marketaccess::orderpassing;
namespace terra
{
	namespace ats
	{
		class ats_instrument;
		class abstract_ats
		{
		public:
			abstract_ats(std::string portfolioName, std::vector<std::string> & feedsources, std::vector<std::string> & connections);
			virtual ~abstract_ats();
		public:
			std::string& get_name(){ return m_portfolioName; }
			void   set_name(std::string& name){ m_portfolioName = name; }
			std::vector<std::string> & get_feed_sources(){ return m_feedsources; }
			std::vector<std::string> & get_connections(){ return m_connections; }

			double get_yesterday_pnl_bary(){ return m_yesterday_pnl_bary; }
			double get_today_pnl_bary(){ return m_today_pnl_bary; }
			double get_yesterday_pnl_mid(){ return m_yesterday_pnl_mid; }
			double get_today_pnl_mid(){ return m_today_pnl_mid; }
			double get_yesterday_pnl_last(){ return m_yesterday_pnl_last; }
			double get_today_pnl_last(){ return m_today_pnl_last; }
			double get_fees_exchange(){ return m_fees_exchange; }
			double get_fees_broker(){ return m_fees_broker; }
			bool   get_auto_status(){ return m_auto_status; }

			void start_automaton();
			void stop_automaton(bool playSound);

			void stop();
			virtual void start();

			void start_workflow();
			void stop_workflow();

			void set_yesterday_position_type(YesterdayPositionType::type type);
			YesterdayPositionType::type get_yesterday_position_type(){ return m_yesterdayPositionType; }

			virtual void load_config_daily();
			virtual void save_config_daily();
			virtual void load_config();
			virtual void save_config();
			
			virtual void shift_eod_cb();
			virtual void shift_price_cb();
			//pnl_task
			virtual void compute_pnl();
			//pricing_task
			virtual void do_pricing();
			//phase_task
			void compute_trading_phase();
			//heart_beat_check

			//dump_intraday_task
			virtual void dump_intra_day(){}
			//callback_task
			virtual void check_callbacks();
			tradingperiodmanager TradingPeriodManager;
		
			ats_instrument * find(const std::string & code);

			bool is_open()
			{
				return currentTradingPhase == AtsType::TradingPhase::OPEN;
			}

			bool is_pre_open()
			{
				return currentTradingPhase == AtsType::TradingPhase::PREOPEN;
			}

			bool is_close()
			{
				return currentTradingPhase == AtsType::TradingPhase::CLOSE || currentTradingPhase == AtsType::TradingPhase::PRECLOSE;
			}

			bool is_pre_close()
			{
				return currentTradingPhase == AtsType::TradingPhase::PRECLOSE;
			}

			bool is_open_or_pre_open()
			{
				return is_open() || is_pre_open();
			}

			bool is_open_or_pre_open_or_pre_close()
			{
				return is_open() || is_pre_open() || is_pre_close();
			}

			//bool check_thread_status()
			//{
			//	//return t_workflow.joinable();
			//}

			virtual void match_order(order* order){}

			bool get_started(){ return m_started; }
			atsintradata * get_intra_data(){ return m_intra_data; }
			AtsType::TradingPhase::type currentTradingPhase;
			int num_workflow = 0;
			//string current_task;
			bool print_screen = false;
		protected:
			void on_name_changed();
			virtual void initialize_workflow() = 0;
			void feed_source_status_changed(feed_source * source);
			void connection_status_changed(terra::marketaccess::orderpassing::connection * conn);
			//void work_flow_interate();
			//std::thread t_workflow;

			work_flow_thread ats_work_flow;
		protected:
			std::string m_default_name;
			
			bool m_started;
			//ats_workflow _atsWorkflow;
			bool   m_workFlowInitialized;
			//bool   m_IsCancellationRequested;
			std::string m_portfolioName;
			std::string m_strAtsDirectory;
			
			std::vector<std::string> m_feedsources;
			std::vector<std::string> m_connections;
			YesterdayPositionType::type m_yesterdayPositionType;			
			double m_yesterday_pnl_bary;
			double m_today_pnl_bary;
			double m_yesterday_pnl_mid;
			double m_today_pnl_mid;
			double m_yesterday_pnl_last;
			double m_today_pnl_last;
			double m_fees_exchange;
			double m_fees_broker;
			bool   m_auto_status;
			time_duration  m_buffer_time;
			time_duration  m_stop_time;
			bool       m_price_finished;
			bool       m_eod_finished;
			ats_daily_data* m_ats_daily_data;
			base_ats_data  *m_base_ats_data;
			atsintradata   *m_intra_data;
			async_writer_new m_stream_writer;
			
			
		public:
			std::vector<ats_instrument*> AtsInstrumentList;
		};
	}
}
#endif //_ABSTRACT_ATS_V2_H_

