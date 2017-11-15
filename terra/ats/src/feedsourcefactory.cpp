#include "string_tokenizer.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "feedsourcefactory.h"
#include "lts_source.h"
#include "ltsl2_source.h"
#include "cffex_source.h"
#include "femas_source.h"
#include "xs_source.h"
#include "xs2_source.h"
#include "ltsudp_source.h"
#include "ltscffexudp_source.h"

#include "xele_source.h"
#include <boost/algorithm/string.hpp>

#ifndef Linux
//#include "ib_source.h"
#else
#include "sl2_source.h"
#endif
#include "fs_source.h"
#include "tdf_source.h"
#include "sl_source.h"
#include "sl2_source.h"
#include "es_source.h"
#include "xsl2_source.h"
#include "define.h"
#ifndef Linux
#include "sf_source.h"
//#include "zd_source.h"
#endif
#include "zd_source.h"
#include "ib_source.h"
using namespace feed;
namespace terra
{
	namespace ats
	{
		feed_source_factory* feed_source_factory::g_FeedSourceFactory = nullptr;

		int feed_source_factory::initialize_feed_sources(string feedSourceFile, string dbFile)
		{

			boost::filesystem::path p(feedSourceFile.c_str());
			if (!boost::filesystem::exists(p))
			{
				loggerv2::error("abstract_feed_source_factory : file [%s] does not exist...", feedSourceFile.c_str());
				return -1;
			}

			loggerv2::info("feed_source file [%s]", feedSourceFile.c_str());

			boost::filesystem::ifstream stream;
			stream.open(feedSourceFile.c_str());
			if (stream.bad() || stream.fail())
				return -1;

			string_tokenizer<1024> tokenizer;
			const char* szSeparators = ", ";

			int nbFeedSources = 0;
			std::string line;
			while (stream.good())
			{
				std::getline(stream, line);
				if (line.length() == 0 || line[0] == '#')
					continue;

				tokenizer.break_line(line.c_str(), szSeparators);
				//tokenizer.remove_empty_tokens();

				int n = tokenizer.size();
				if (n < 5)
				{
					loggerv2::error("abstract_feed_source_factory: invalid feed_source <%s> n[%d]", line.c_str(), n);
					continue;
				}

				int type = atoi(tokenizer[0]);
				if (tokenizer[1] == NULL)
					continue;

				string strPasswd;
				if (n > 6)
				{
					strPasswd = tokenizer[6];
				}

				string param1;
				if (n > 7)
				{
					param1 = tokenizer[7];
				}
				string param2;
				if (n > 8)
				{
					param2 = tokenizer[8];
				}
				//
				string param3;
				if (n > 9)
				{
					param3 = tokenizer[9];
				}
				string param4;
				if (n > 10)
				{
					param4 = tokenizer[10];
				}
				string param5;
				if (n > 11)
				{
					param5 = tokenizer[11];
				}
				//
				feed_source* pSource = nullptr;
				string strSourceName = tokenizer[1];
				string strHostName = tokenizer[2];
				string strPort = tokenizer[3];
				string strBrokerId = tokenizer[4];
				string strUserName = tokenizer[5];
				//if (dbFile.length() < 1)
				//{
				//	dbFile = strDB;
				//}
				if (dbFile.length() < 1)
				{
					loggerv2::error("feed_source_factory::initialize_feed_sources dbFile is null!");
					continue;
				}
				if (this->get_feed_source(strSourceName) != nullptr)
				{
					loggerv2::error("feed_source_factory::initialize_feed_sources sourceName:%s duplicated!", strSourceName.c_str());
					continue;
				}
				switch (type)
				{
				case 1://lts2
				{
#ifdef FEED_LTSL2
					pSource = new ltsl2::ltsl2_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1, param2, param3);
					nbFeedSources++;
#endif
					break;
				}
				case 2:
				{
#ifdef FEED_LTSUDP
					pSource = new ltsudp::ltsudp_source(strSourceName, strHostName, strPort, dbFile, param1, param2);
					nbFeedSources++;
#endif
					break;
				}
				case 3: //lts
				{
#ifdef FEED_LTS
					pSource = new lts::lts_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile,param2,param3,param4);
					nbFeedSources++;
#endif
					break;
				}
				case 4: //tdf
				{
#ifdef FEED_TDF
					pSource = new tdf::tdf_source(strSourceName, strHostName, strPort, strUserName, strPasswd, dbFile, atoi(param1.c_str()), atoi(param2.c_str()));
					nbFeedSources++;
#endif
					break;
				}
				case 5:
				{
#ifdef FEED_CTP
					pSource = new cffex::cffex_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1, param2, param3, param4);
					nbFeedSources++;
#endif
					break;
				}
				case 7:
				{
#ifdef FEED_IB
//#ifndef Linux
					pSource = new ib::ib_source(strSourceName, strHostName, strPort, strUserName, dbFile);
					nbFeedSources++;
//#endif
#endif
					break;
				}
				case 8:
				{
#ifdef FEED_FEMAS
					pSource = new femas::femas_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1, param2);
					nbFeedSources++;
#endif
					break;
				}
				case 9:
				{
#ifdef FEED_XS
#ifndef X1Con
					strUserName = tokenizer[4];
					strPasswd = tokenizer[5];
					pSource = new xs::xs_source(strSourceName, strHostName, strPort, strUserName, strPasswd, dbFile, param1, param2);
					nbFeedSources++;
#endif
#endif
					break;
				}
				case 10:
				{
#ifdef FEED_LTSCFFEXUDP
					pSource = new ltscffexudp::ltscffexudp_source(strSourceName, strHostName, strPort, dbFile, param1, param2);
					nbFeedSources++;
#endif
					break;
				}
				case 11:
				{
#ifdef FEED_XS2
					strUserName = tokenizer[4];
					strPasswd = tokenizer[5];
					pSource = new xs2::xs2_source(strSourceName, strHostName, strPort, strUserName, strPasswd, dbFile, param1, param2, param3, param4);
					nbFeedSources++;
#endif
					break;
				}
				case 12:
				{
#ifdef FEED_FS
					pSource = new fs::fs_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile);
					nbFeedSources++;
#endif
					break;
				}
				case 13://sl
				{
#ifdef FEED_SL
					pSource = new sl::sl_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1,param2,param3,param4);
					nbFeedSources++;
#endif
					break;
				}
				case 14:
				{
#ifdef FEED_ES
					pSource = new es::es_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1, param2, param3, param4);
					nbFeedSources++;
#endif
					break;
				}
				case 15://xsl2
				{
#ifdef FEED_XSL2
					pSource = new xsl2::xsl2_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1);
					nbFeedSources++;
#endif
					break;
				}
#ifdef FEED_SL2
#ifdef Linux
				case 16://sl2
				{
					pSource = new sl2::sl2_source(strSourceName, strHostName, strPort,dbFile, param1,param2,param3,param4);
					nbFeedSources++;
					break;
				}
#endif
#endif
#ifndef Linux
				case 17://sf
				{
#ifdef FEED_SF
					pSource = new sf::sf_source(strSourceName, strHostName, strPort, dbFile, param1);
					nbFeedSources++;
					break;
#endif
				}
#endif
#ifdef FEED_ZD
				case 18://zd
				{
					/*
					zd_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, const string& authServerIp, const string& authServerPort, const string& authCode);
					*/
					pSource = new zd::zd_source(strSourceName, strHostName, strPort, strBrokerId, strUserName, strPasswd, dbFile, param1, param2, param3, param4, param5);
					nbFeedSources++;
					break;
				}
#endif
				case 19:
				{
#ifdef FEED_XELE
						   if(n>=7)
						   {
							   strUserName = tokenizer[2];
							   strPasswd = tokenizer[3];
							   std::string FrontAddr = tokenizer[4];
							   std::string BCAddr = tokenizer[5];
							   std::string NIC = tokenizer[6];
							   
							   int mcore = -1;
							   if (n >= 8)
							   {
								   std::string cpucore = tokenizer[7];
								   mcore = atoi(cpucore.data());
							   }

							   std::string subFa = FrontAddr.substr(6, FrontAddr.size() - 6);
							   std::vector<std::string> vec;
							   boost::split(vec, subFa, boost::is_any_of(":"));
#ifdef __linux__
							   pSource = new xele::xele_source(strSourceName, FrontAddr, BCAddr, vec[0], vec[1], strUserName, strPasswd, NIC, dbFile,mcore);
							   nbFeedSources++;
#endif
						   }
						   else
						   {
							   std::cout << "FEED XELE:num of parm is less than 7"<<endl;
						   }
#endif
						   break;
				}
				default:
				{
					loggerv2::error("feed_source_factory::initialize_feed_sources didn't support feed source type:%d!", type);
					printf_ex("feed_source_factory::initialize_feed_sources didn't support feed source type:%d!\n", type);
					break;
				}
				}
			}
			stream.close();
			return nbFeedSources;
		}
		feed_source * feed_source_factory::get_feed_source(string strSourceName)
		{
			return feed_source_container::get_instance()->get_by_key(strSourceName);
		}
	}
}