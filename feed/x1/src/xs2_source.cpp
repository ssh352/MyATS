#include "xs2_source.h"
#include "xs2_connection.h"
#include <nn.h>
#include <pubsub.h>

using namespace terra::common;
namespace feed
{
	namespace xs2
	{
		xs2_source::xs2_source(const string &sourceName, const string &hostname, const string &service, const string &user, const string &password, const string &dbName, string pub, string url)
			: feed_source("XS2", sourceName, hostname, service, "", user, password, dbName,pub,url)
		{
			m_pConnection = new xs2_connection(this);
		}

		//xs2_source::~xs2_source()
		//{
		//	if (m_pConnection)
		//	{
		//		delete m_pConnection;
		//	}
		//}

		void xs2_source::init_source()
		{
			//
			feed_source::init_source();
			//
			
			get_queue()->setHandler(boost::bind(&xs2_source::process_msg, this, _1));

			//std::thread t(std::bind(&xs2_source::set_kernel_timer_thread, this));
			//m_thread.swap(t);
			init_process(io_service_type::feed);
			load_database();

			//abstract_database* db = database_factory::create("sqlite", get_database_name().c_str());

			//if (db->open_database())
			//{
			//	loggerv2::info("connect to database %s", get_database_name().c_str());
			//}

			//std::array<std::string, 4> sInst = { "Stocks", "ETFs", "Options", "Futures" };

			//for (auto s : sInst)
			//{
			//	std::string sCmd = "select Code, FeedCodes from " + s + " where FeedCodes like '%@XS2%' ";
			//	std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
			//	//loggerv2::info("xs2_source : get futures number %d", res->size());

			//	for (std::vector<boost::property_tree::ptree>::iterator it = res->begin(); it != res->end(); ++it)
			//	{
			//		std::string temp = abstract_database::get_item((*it).get("FeedCodes", ""), "XS2");
			//		std::string code = (*it).get("Code", "");
			//		m_feedCode2CodeMap.insert(make_pair(temp, code));
			//		m_code2FeedCodeMap.insert(make_pair(code, temp));										
			//	}
			//	delete res;
			//}

			//db->close_databse();
			//loggerv2::info("xs2_source - %d instruments loaded from database", m_feedCode2CodeMap.size());

			m_pConnection->init();
			m_pConnection->create();
		}
    void xs2_source::start_receiver()
		{
			DFITCDepthMarketDataField msg;
			int rc = -1;
			//while (true)
			{
				rc = nn_recv(m_sub_handle, &msg, sizeof(msg), 0);
				if (rc == sizeof(msg))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					get_queue()->CopyPush(&msg);
				}
			}
		}
		/*		void xs2_source::release_source()
				{
				m_pConnection->cleanup();
				}	*/
		void xs2_source::process_msg(DFITCDepthMarketDataField* pMsg)
		{
			std::string sItemFeedcode = std::string(pMsg->instrumentID);

			/*			std::string instrName = get_code_by_feedcode(sItemFeedcode);
						if (instrName.empty())
						{
						loggerv2::error("xs2_source::process_msg cannot find Code for FeedCode %s.", sItemFeedcode.c_str());
						return;
						}	*/
			feed_item* afeed_item = get_feed_item(sItemFeedcode);
			if (afeed_item == NULL)
			{
				loggerv2::error("xs2_source::process_msg instrument %s not found", sItemFeedcode.c_str());
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}

		//void  xs2_source::set_kernel_timer_thread()
		//{
		//	boost::asio::high_resolution_timer t(io, std::chrono::microseconds(20));
		//	t.async_wait(boost::bind(&xs2_source::process, this, boost::asio::placeholders::error, &t));
		//	io.run();
		//}
		//void xs2_source::process()
		//{
		//	;
		//}

		//void xs2_source::process(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		//{
		//	//int i = 0;
		//	//while (true)
		//	{
		//		//if (get_queue()->m_queue.read_available() > 0)
		//		//{
		//		//	for(auto &func: get_queue()->m_handler)
		//		//	{
		//		//		func();
		//		//	}
		//		//}
		//		//while (get_queue()->Pop_Handle())
		//		//{
		//		//	++i;
		//		//}
		//		get_queue()->Pops_Handle();
		//		//if (i >= 10)
		//		{
		//			//i = 0;
		//			t->expires_at(t->expires_at() + std::chrono::microseconds(20));
		//			t->async_wait(boost::bind(&xs2_source::process, this, boost::asio::placeholders::error, t));
		//			return;
		//			//i = 0;
		//			//std::this_thread::sleep_for(std::chrono::microseconds(500));
		//		}
		//	}
		//}
		void xs2_source::process()
		{
			get_queue()->Pops_Handle();
		}
		//int xs2_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		DFITCDepthMarketDataField* msg = get_queue()->Pop();
		//		this->process_msg(msg);
		//	}
		//	return i;
		//}
		void xs2_source::process_msg(DFITCDepthMarketDataField* pMsg, feed_item * feed_item)
		{
			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->AskPrice1 != NO_PRICE ? pMsg->AskPrice1 : 0., pMsg->AskVolume1, feed_item);
			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->AskPrice2 != NO_PRICE ? pMsg->AskPrice2 : 0., pMsg->AskVolume2, feed_item);
			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->AskPrice3 != NO_PRICE ? pMsg->AskPrice3 : 0., pMsg->AskVolume3, feed_item);
			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->AskPrice4 != NO_PRICE ? pMsg->AskPrice4 : 0., pMsg->AskVolume4, feed_item);
			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->AskPrice5 != NO_PRICE ? pMsg->AskPrice5 : 0., pMsg->AskVolume5, feed_item);

			double closePrice = (!(pMsg->closePrice == NO_PRICE || pMsg->closePrice == 0)) ? pMsg->closePrice : (pMsg->preClosePrice != NO_PRICE ? pMsg->preClosePrice : 0);
			feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->lastPrice != NO_PRICE ? pMsg->lastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->Volume;
			feed_item->set_daily_volume(dailyVolume);

			double hightest = pMsg->highestPrice;

			double lowest = pMsg->lowestPrice;

			double upperlmt = pMsg->upperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->lowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->settlementPrice;
			feed_item->set_settlement(selltement);

			double presettle = pMsg->preSettlementPrice;

			double preopeninterest = pMsg->preOpenInterest;
		}
	}
}

