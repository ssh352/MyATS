#include "xs2_source.h"
#include "xs2_connection.h"

#include <nn.h>
#include <pubsub.h>
using namespace terra::common;
namespace feed
{
	namespace xs2
	{
		xs2_source::xs2_source(const string &sourceName, const string &hostname, const string &service, const string &user, const string &password, const string &dbName, const string& is_FQR, string pub, string url,string req_url)
			: feed_source("XS2", sourceName, hostname, service, "", user, password, dbName, pub, url,req_url)
		{
			m_pConnection = new xs2_connection(this);
			if (is_FQR == "FQR")
			{
				FQR = true;
			}
			else
			{
				FQR = false;
			}
		}

		void xs2_source::receive_FQR(DFITCQuoteSubscribeRtnField * pForQuoteRsp)
		{
			std::string sFeedCode = std::string(pForQuoteRsp->instrumentID);

			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("cffex_source::FQR instrument %s not found", pForQuoteRsp->instrumentID);
				return;
			}
			afeed_item->FQR_ID = pForQuoteRsp->quoteID;
			afeed_item->set_FQR_time(microsec_clock::local_time().time_of_day().total_seconds());

		}

		void xs2_source::init_source()
		{
			//
			feed_source::init_source();
			//

			get_queue()->setHandler(boost::bind(&xs2_source::process_msg, this, _1));
#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif
			load_database();
						
			m_pConnection->init();
			m_pConnection->create();
		}

#ifdef Linux
		void  xs2_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&xs2_source::process, this));
			m_queue.set_fd(efd);
		}
#endif
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
		void xs2_source::process_msg(DFITCDepthMarketDataField* pMsg)
		{
			std::string sItemFeedcode = std::string(pMsg->instrumentID);

			feed_item* afeed_item = get_feed_item(sItemFeedcode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("xs2_source::process_msg instrument %s not found", sItemFeedcode.c_str());
				return;
			}
			update_item(pMsg, afeed_item);
			return post(afeed_item);
		}

		void xs2_source::process()
		{
			get_queue()->Pops_Handle();
		}

		void xs2_source::update_item(DFITCDepthMarketDataField* pMsg, feed_item * feed_item)
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
			feed_item->set_turnover(pMsg->turnover);
			//double hightest = pMsg->highestPrice;

			//double lowest = pMsg->lowestPrice;

			double upperlmt = pMsg->upperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->lowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->settlementPrice;
			feed_item->set_settlement(selltement);

			//double presettle = pMsg->preSettlementPrice;

			//double preopeninterest = pMsg->preOpenInterest;
		}
	}
}

