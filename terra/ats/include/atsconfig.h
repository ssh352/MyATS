#ifndef _ATS_CONFIG_V2_H_
#define _ATS_CONFIG_V2_H_
#include <boost/filesystem.hpp>
#include "feedcommon.h"
#pragma once
namespace terra
{
	namespace ats
	{
		class ats_config
		{
		private:
			ats_config();
			~ats_config();
			static ats_config * g_AtsConfig;
		public:
			static ats_config * get_instance()
			{
				if (g_AtsConfig == nullptr)
				{
					g_AtsConfig = new ats_config();
				}
				return g_AtsConfig;
			}
		protected:			
			string m_strAppPath;
			string m_strConfigFile;
			string m_strDataPath;
			string m_strDailyPath;
			string m_strDumpDirectory;
			//string m_strTradingPhaseFile;
			string m_strFeedSourceFile;
			string m_strConnectionFile;
			string m_strDBFile;
			string m_strReferentialPath;
			string m_strCurrencyFile;
			string m_strPositionFile;
			string m_strSecurityFile;
			string m_strLogFile;
			//string m_strWrapperLogFile;
			//string m_strFeedWrapperLogFile;
			string m_strAtsPublishPort;
			int m_iGeneralAtsPort;
			int m_iSpecificAtsPort;
			int m_iGUITimer;
			int m_iATSTimer;
			//int m_work_flow_delay;
			int m_sleeping_milisec;

			int m_feed_io_cpu_core = -1;
			int m_trader_io_cpu_core = -1;
			int m_other_io_cpu_core = -1;
			bool m_is_bind_feed_trader_core = true;

			string m_strCutOffTime;
			time_duration m_cut_off_time;

			ptime m_shutdown_time;
			bool m_auto_stop;
			//string m_strDesktopFile;
			//bool   m_bShowContribOrder;
			//string m_strSoundPath;
		public:
			bool init();
			bool load(string strConfigFile);
			string get_config_file(){ return m_strConfigFile; }
			string get_app_path(){ return m_strAppPath; }
			string get_data_directory(){ return m_strDataPath; }
			string get_daily_directory(){ return m_strDailyPath; }
			string get_dump_directory(){ return m_strDumpDirectory; }
			//string get_trading_phase_file(){ return m_strTradingPhaseFile; }
			string get_feed_source_file(){ return m_strFeedSourceFile; }
			string get_connection_file(){ return m_strConnectionFile; }
			string get_db_file(){ return m_strDBFile; }
			string get_referential_path(){ return m_strReferentialPath; }
			string get_currency_file(){ return m_strCurrencyFile; }
			string get_position_file(){ return m_strPositionFile; }
			string get_security_file(){ return m_strSecurityFile; }
			string get_log_file(){ return m_strLogFile; }
			//string get_wrapper_log_file(){ return m_strWrapperLogFile; }
			//string get_feed_wrapper_log_file(){ return m_strFeedWrapperLogFile; }
			int    get_gui_timer(){ return m_iGUITimer; }
			int    get_ats_timer(){ return m_iATSTimer; }
			string get_ats_publish_port(){ return m_strAtsPublishPort; }
			int    get_general_ats_port(){ return m_iGeneralAtsPort; }
			int    get_specific_ats_port(){ return m_iSpecificAtsPort; }
			//int    get_work_flow_delay(){ return m_work_flow_delay; }
			//int    get_sleeping_milisec(){ return m_sleeping_milisec; }

			int get_feed_io_cpu_core(){ return m_feed_io_cpu_core; }
			int get_trader_io_cpu_core(){ return m_trader_io_cpu_core; }
			int get_other_io_cpu_core(){ return m_other_io_cpu_core; }
			bool get_is_bind_feed_trader_core(){ return m_is_bind_feed_trader_core;}

			string expand(string log);
			time_duration get_cut_off_time(){ return m_cut_off_time; }
			ptime get_shutdown_time(){ return m_shutdown_time; }
			bool get_enable_auto_stop(){ return m_auto_stop; }
			//string get_desktop_file(){ return m_strDesktopFile; }
			//bool   get_show_contrib_order(){ return m_bShowContribOrder; }
			//string get_sound_path(){ return m_strSoundPath; }
		};
	}
}
#endif//_ATS_CONFIG_V2_H_

