#include "atsconfig.h"
#include "defaultdatetimepublisher.h"
#include <boost/property_tree/ptree.hpp>    
#include <boost/property_tree/ini_parser.hpp>  
#include <boost/algorithm/string.hpp> 
using namespace terra::common;
namespace terra
{
	namespace ats
	{
		ats_config* ats_config::g_AtsConfig = nullptr;

		ats_config::ats_config()
		{			
			m_iGeneralAtsPort  = 9095;
			m_iSpecificAtsPort = 9095;
			m_iGUITimer = 200;
			m_iATSTimer = 0;
		}


		ats_config::~ats_config()
		{
			
		}
		bool ats_config::init()
		{
			m_strAppPath = boost::filesystem::initial_path<boost::filesystem::path>().string();
			
			boost::filesystem::path p;
			
			p.clear();
			p.append(m_strAppPath);
			
			boost::filesystem::path parentPath = p.parent_path();
			m_strAppPath = parentPath.string();
			printf("app path:%s\n", m_strAppPath.c_str());

			p.clear();
			p.append(m_strAppPath);
			p.append("Config");
			p.append("Config.ini");
			m_strConfigFile = p.string();
			printf("config file is :%s\n", m_strConfigFile.c_str());
			return false;
		}
		///refer to aut_broker::setup in the toolkit\broker\src\aut_broker.cpp
		bool ats_config::load(string strConfigFile)
		{
			try
			{
				if (strConfigFile.length() < 1)
					return false;
				this->m_strConfigFile = strConfigFile;
				std::cout<<strConfigFile<<std::endl;
				boost::filesystem::path p(strConfigFile);
				if (!boost::filesystem::exists(p))
				{
					printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
					return false;
				}
				boost::property_tree::ptree root;
				boost::property_tree::ini_parser::read_ini(strConfigFile, root);

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("DIRECTORIES.Data", "Data"));
				m_strDataPath = p.string();
				if (!boost::filesystem::exists(m_strDataPath))
				{
					boost::filesystem::create_directory(m_strDataPath);
				}

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("DIRECTORIES.Dump", "Dump"));
				m_strDumpDirectory = p.string();
				if (!boost::filesystem::exists(m_strDumpDirectory))
				{
					boost::filesystem::create_directory(m_strDumpDirectory);
				}

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("DIRECTORIES.Daily", "Daily"));
				string dailyDir = p.string();

				if (!boost::filesystem::exists(dailyDir))
				{
					boost::filesystem::create_directory(dailyDir);
				}

				m_strCutOffTime = root.get<string>("DIRECTORIES.CutOffTime", "17:00:00");
				m_cut_off_time = time_duration(duration_from_string(m_strCutOffTime));

				std::string ct_night = root.get<string>("DIRECTORIES.CutOffTimeNight", "03:00:00");
				auto cut_off_time_night = time_duration(duration_from_string(ct_night));


				m_auto_stop = root.get<bool>("DIRECTORIES.enable_auto_stop", false);

				std::string str_Shutdown_Time = root.get<string>("DIRECTORIES.ShutDownTime", "16:00:00|03:30:00");
				std::vector<std::string> vec;
				boost::split(vec, str_Shutdown_Time, boost::is_any_of("|"));

				ptime now = boost::posix_time::second_clock::local_time();
				m_shutdown_time = now + time_duration(24, 0, 0, 0);
				for (auto &it : vec)
				{
					ptime pt = ptime(day_clock::local_day(), duration_from_string(it));
					if (pt <= now)
						pt += time_duration(24, 0, 0, 0);
					if (m_shutdown_time > pt)
						m_shutdown_time = pt;
				}

				p.clear();
				p.append(dailyDir);
				auto mdate = date_time_publisher_gh::get_instance()->today();
				bool is_ct_night = false;
				if (date_time_publisher_gh::get_instance()->get_time_of_day() < cut_off_time_night)
				{
					mdate = mdate - boost::gregorian::days(1);
					is_ct_night = true;
				}
				p.append(to_iso_string(mdate));
				m_strDailyPath = p.string();
				if (date_time_publisher_gh::get_instance()->get_time_of_day() > m_cut_off_time||is_ct_night)
				{
					m_strDailyPath += "N";
				}
				
				if (!boost::filesystem::exists(m_strDailyPath))
				{
					boost::filesystem::create_directory(m_strDailyPath);
				}


				/*
				if (date_time_ex.Now.TimeOfDay > CutOffTime)
				DailyDirectory = ConfigDirectory + iniFile.GetString(section, "Daily") + "/" + date_time_publisher_gh.Publisher.today.to_string("yyyyMMdd")+"N";
				else
				DailyDirectory = ConfigDirectory + iniFile.GetString(section, "Daily") + "/" + date_time_publisher_gh.Publisher.today.to_string("yyyyMMdd");
				*/

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("DIRECTORIES.Referential", "Config/Referential"));
				m_strReferentialPath = p.string();

				p.clear();
				p.append(get_referential_path());
				p.append(root.get<string>("REFERENTIAL.InstrumentDB", "InstrumentDico.db"));
				m_strDBFile = p.string();

				p.clear();
				p.append(get_referential_path());
				p.append(root.get<string>("REFERENTIAL.currency", "currency.json"));
				m_strCurrencyFile = p.string();

				//p.clear();
				//p.append(get_referential_path());
				//p.append(root.get<string>("REFERENTIAL.tradingphase",""));
				//m_strTradingPhaseFile = p.string();

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("CORE.feedsources", "Config/Core/feedsources.ini"));
				m_strFeedSourceFile = p.string();

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("CORE.connection", "Config/Core/connection.ini"));
				m_strConnectionFile = p.string();

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("CORE.position", ""));
				m_strPositionFile = p.string();

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("CORE.security", "Config/Core/security.ini"));
				m_strSecurityFile = p.string();

				p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("LOG.log", "Log/OptionATS_yyyyMMdd.log"));
				m_strLogFile = p.string();
				m_strLogFile = expand(m_strLogFile);

				string logpath = p.parent_path().string();
				if (!boost::filesystem::exists(logpath))
				{
					boost::filesystem::create_directory(logpath);
				}

				//p.clear();
				//p.append(m_strAppPath);
				//p.append(root.get<string>("LOG.wrapper",""));
				//m_strWrapperLogFile = p.string();
				//m_strWrapperLogFile = expand(m_strWrapperLogFile);

				//p.clear();
				//p.append(m_strAppPath);
				//p.append(root.get<string>("LOG.feedWrapperLog",""));
				//m_strFeedWrapperLogFile = p.string();
				//m_strFeedWrapperLogFile = expand(m_strFeedWrapperLogFile);

				m_iSpecificAtsPort = root.get<int>("CORE.SpecificAtsPort", 9095);
				m_iGUITimer = root.get<int>("PERFS.GUITimer", 500);
				m_iATSTimer = root.get<int>("PERFS.ATSTimer", 100);
				//m_work_flow_delay = root.get<int>("PERFS.WorkFlowDelay", 100);

				m_feed_io_cpu_core = root.get<int>("PERFS.feed_io_cpu_core", -1);
				m_is_bind_feed_trader_core = root.get<bool>("PERFS.is_bind_feed_trader_core", true);
				if (m_is_bind_feed_trader_core)
					m_trader_io_cpu_core = m_feed_io_cpu_core;
				else
					m_trader_io_cpu_core = root.get<int>("PERFS.trader_io_cpu_core", -1);
				printf("m_feed_io_cpu_core is %d\n",m_is_bind_feed_trader_core);
				m_other_io_cpu_core = root.get<int>("PERFS.other_io_cpu_core", -1);

				m_strAtsPublishPort = root.get<string>("BROKER.ats_publish", "tcp://*:9092");
				//m_sleeping_milisec  = root.get<int>("BROKER.SleepingMiliseconds", 0);

				/*p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("GUI.DesktopFile", ""));
				m_strDesktopFile = p.string();
				*/
				//m_bShowContribOrder = root.get<bool>("GUI.ShowContribOrder", false);

				/*p.clear();
				p.append(m_strAppPath);
				p.append(root.get<string>("SOUND.SoundPath", ""));
				m_strSoundPath = p.string();*/
			}
			catch (std::exception &ex)
			{
				cout << ex.what();
				exit(1);
			}
			return true;
		}
		string ats_config::expand(string log)
		{
			//date_time_ex today = date_time_publisher_gh::get_instance()->today();
			
			int year = day_clock::local_day().year();
			int month = day_clock::local_day().month();
			int day = day_clock::local_day().day();
			/*int hh   = 0;
			int mi   = 0;
			int se   = 0;*/
			//today.get_date(&year, &month, &day,&hh,&mi,&se);
			char buffer[8];
			memset(buffer, 0, sizeof(buffer));
			int iIndex = log.find("yyyy");
			if (iIndex >=0)
			{
				sprintf(buffer, "%04d", year);
				log = log.replace(iIndex, 4, buffer);
			}
			iIndex = log.find("MM");
			if (iIndex >= 0)
			{
				sprintf(buffer, "%02d", month);
				log = log.replace(iIndex, 2, buffer);
			}
			iIndex = log.find("dd");
			if (iIndex >= 0)
			{
				sprintf(buffer, "%02d", day);
				log = log.replace(iIndex, 2, buffer);
			}
			return log;
		}
	}
}
