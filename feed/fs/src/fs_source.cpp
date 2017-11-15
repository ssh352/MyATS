#include <array>
#include <boost/foreach.hpp>
#include "fs_source.h"
#include "fs_connection.h"
#include "database_factory.h"
#include "terra_logger.h"
using namespace terra::common;
namespace feed
{
	namespace fs
	{
		fs_source::fs_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & dbName)
			: feed_source("FS", sourceName, hostname, service, brokerId, user, password, dbName)
		{
			m_pConnection = new fs_connection(this);
		}

		//fs_source::~fs_source()
		//{
		//	if (m_pConnection)
		//		delete m_pConnection;
		//}
		void fs_source::init_source()
		{
			get_queue()->setHandler(boost::bind(&fs_source::process_msg, this, _1));

			//std::thread t(std::bind(&fs_source::set_kernel_timer_thread, this));
			//m_thread.swap(t);
			//init_process(io_service_type::feed);
#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif
			load_database();

			//abstract_database* db = database_factory::create("sqlite", get_database_name().c_str());
			//if (db->open_database())
			//{
			//	loggerv2::info("connect to database %s", get_database_name().c_str());
			//}
			//std::array<std::string, 3> sInst = { "Futures", "Stocks", "Options" };
			//for (auto s : sInst)
			//{
			//	std::string sCmd = "select Code, FeedCodes from " + s + " where FeedCodes like '%@FS%' ";
			//	std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
			//	//loggerv2::info("lts_source : get futures number %d", res->size());

			//	for (std::vector<boost::property_tree::ptree>::iterator it = res->begin(); it != res->end(); ++it)
			//	{
			//		std::string temp = abstract_database::get_item((*it).get("FeedCodes", ""), "FS");
			//		std::string code = (*it).get("Code", "");
			//		m_feedCode2CodeMap.insert(make_pair(temp, code));
			//		m_code2FeedCodeMap.insert(make_pair(code, temp));
			//	}
			//	delete res;
			//}
			//db->close_databse();
			//loggerv2::info("fs_source - %d instruments loaded from database", m_feedCode2CodeMap.size());
			m_pConnection->init();
			m_pConnection->create();
		}

#ifdef Linux
		void  fs_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&fs_source::process, this));
			m_queue.set_fd(efd);
		}
#endif

		//void fs_source::release_source()
		//{
		//	m_pConnection->cleanup();					
		//}
		void fs_source::process_msg(fstech::CThostFtdcDepthMarketDataField* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->InstrumentID);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::error("fs_source::process_msg cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("fs_source::process_msg instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}

		//void  fs_source::set_kernel_timer_thread()
		//{
		//	boost::asio::high_resolution_timer t(io, std::chrono::microseconds(20));
		//	t.async_wait(boost::bind(&fs_source::process, this, boost::asio::placeholders::error, &t));
		//	io.run();
		//}
		//void fs_source::process()
		//{
		//	;
		//}

		//		void fs_source::process(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		//		{
		//			//int i = 0;
		//			while (true)
		//			{
		///*				if (get_queue()->m_queue.read_available() > 0)
		//				{
		//					for(auto &func: get_queue()->m_handler)
		//					{
		//						func();
		//					}
		//				}			*/	
		//				/*while (get_queue()->Pop_Handle())
		//					++i;*/
		//				get_queue()->Pops_Handle();
		//				//if (i >= 10)
		//				{
		//					//i = 0;
		//					t->expires_at(t->expires_at() + std::chrono::microseconds(20));
		//					t->async_wait(boost::bind(&fs_source::process, this, boost::asio::placeholders::error, t));
		//					return;
		//				}
		//			}
		//		}

		void fs_source::process()
		{

			get_queue()->Pops_Handle();

		}
		//int fs_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		fstech::CThostFtdcDepthMarketDataField* msg = get_queue()->Pop();
		//		this->process_msg(msg);
		//	}
		//	return i;
		//}
		void fs_source::process_msg(fstech::CThostFtdcDepthMarketDataField* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;

			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->AskPrice1 != NO_PRICE ? pMsg->AskPrice1 : 0., pMsg->AskVolume1, feed_item);
			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->AskPrice2 != NO_PRICE ? pMsg->AskPrice2 : 0., pMsg->AskVolume2, feed_item);
			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->AskPrice3 != NO_PRICE ? pMsg->AskPrice3 : 0., pMsg->AskVolume3, feed_item);
			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->AskPrice4 != NO_PRICE ? pMsg->AskPrice4 : 0., pMsg->AskVolume4, feed_item);
			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->AskPrice5 != NO_PRICE ? pMsg->AskPrice5 : 0., pMsg->AskVolume5, feed_item);

			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || pMsg->ClosePrice == 0)) ? pMsg->ClosePrice : (pMsg->PreClosePrice != NO_PRICE ? pMsg->PreClosePrice : 0);
			feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->Volume;
			feed_item->set_daily_volume(dailyVolume);

			//double hightest = pMsg->HighestPrice;

			//double lowest = pMsg->LowestPrice;

			double upperlmt = pMsg->UpperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->LowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->SettlementPrice;
			feed_item->set_settlement(selltement);

			//double presettle = pMsg->PreSettlementPrice;

			//double preopeninterest = pMsg->PreOpenInterest;
		}

	}
}