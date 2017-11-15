#include "xs_source.h"
#include "xs_connection.h"
#include <nn.h>
#include <pubsub.h>
using namespace terra::common;
namespace feed
{
	namespace xs
	{
		xs_source::xs_source(const string &sourceName, const string &hostname, const string &service, const string &user, const string &password, const string &dbName, string pub, string url, string req_url)
			: feed_source("XS", sourceName, hostname, service, "", user, password, dbName, pub, url,req_url)
		{
			m_pConnection = new xs_connection(this);
		}

		//xs_source::~xs_source()
		//{

		//}
		void xs_source::init_source()
		{
			//
			feed_source::init_source();
			//

			get_op_queue()->setHandler(boost::bind(&xs_source::process_sop_msg, this, _1));
			get_stock_queue()->setHandler(boost::bind(&xs_source::process_stock_msg, this, _1));

			/*std::thread t(std::bind(&xs_source::set_kernel_timer_thread, this));
			m_thread.swap(t);*/
			//init_process(io_service_type::feed);
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
		void  xs_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&xs_source::process, this));
			m_op_queue.set_fd(efd);
			m_stock_queue.set_fd(efd);
		}
#endif

		//void xs_source::release_source()
		//{
		//	m_pConnection->cleanup();
		//}
		void xs_source::process_sop_msg(DFITCSOPDepthMarketDataField* pMsg)
		{
			std::string sItemFeedcode = std::string(pMsg->staticDataField.exchangeID) + std::string(pMsg->staticDataField.securityID);
			//std::string instrName = get_code_by_feedcode(sItemFeedcode);
			//if (instrName.empty())
			//	return;
			feed_item* afeed_item = get_feed_item(sItemFeedcode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("xs_source::process_msg instrument %s not found", sItemFeedcode.c_str());
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		void xs_source::process_stock_msg(DFITCStockDepthMarketDataField* pMsg)
		{
			std::string sItemFeedcode = std::string(pMsg->staticDataField.exchangeID) + std::string(pMsg->staticDataField.securityID);
			//std::string instrName = get_code_by_feedcode(sItemFeedcode);
			//if (instrName.empty())
			//	return;
			feed_item* afeed_item = get_feed_item(sItemFeedcode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("xs_source::process_msg instrument %s not found", sItemFeedcode.c_str());
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		
		void xs_source::process()
		{

			get_op_queue()->Pops_Handle();
			get_stock_queue()->Pops_Handle();

		}
		void xs_source::start_receiver()
		{
			char buffer[1024];		
			int rc = -1;
			//while (true)
			{
				memset(buffer, 0, sizeof(buffer));
				rc = nn_recv(m_sub_handle, buffer, sizeof(buffer), 0);
				if (rc == sizeof(DFITCSOPDepthMarketDataField))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);						
					get_op_queue()->CopyPush((DFITCSOPDepthMarketDataField*)buffer);
				}
				else if (rc == sizeof(DFITCStockDepthMarketDataField))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);						
					get_stock_queue()->CopyPush((DFITCStockDepthMarketDataField*)buffer);
				}
			}
		}
		void xs_source::process_msg(DFITCSOPDepthMarketDataField* pMsg, feed_item * feed_item)
		{
			//feed_decoder::process_msg(feed_item);

			// process depth
			process_depth(0, pMsg->sharedDataField.bidQty1, pMsg->sharedDataField.bidPrice1 != NO_PRICE ? pMsg->sharedDataField.bidPrice1 : 0, pMsg->sharedDataField.askPrice1 != NO_PRICE ? pMsg->sharedDataField.askPrice1 : 0, pMsg->sharedDataField.askQty1, feed_item);

			double closePrice = (!(pMsg->staticDataField.preClosePrice == NO_PRICE || pMsg->staticDataField.preClosePrice == 0)) ? pMsg->staticDataField.preClosePrice : (pMsg->staticDataField.preClosePrice != NO_PRICE ? pMsg->staticDataField.preClosePrice : 0);
			feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->sharedDataField.latestPrice != NO_PRICE ? pMsg->sharedDataField.latestPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->sharedDataField.tradeQty;
			feed_item->set_daily_volume(dailyVolume);

			feed_item->set_turnover(pMsg->sharedDataField.turnover);
			feed_item->set_theoretical_open_price(pMsg->specificDataField.auctionPrice);

			//double hightest = pMsg->sharedDataField.highestPrice;

			//double lowest = pMsg->sharedDataField.lowestPrice;

			double upperlmt = pMsg->staticDataField.upperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->staticDataField.lowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			feed_item->set_settlement(pMsg->specificDataField.settlePrice);

			//double presettle = pMsg->specificDataField.settlePrice;

			//feed_decoder::post_msg(feed_item);
		}

		void xs_source::process_msg(DFITCStockDepthMarketDataField* pMsg, feed_item * feed_item)
		{
			// process depth
			process_depth(0, pMsg->sharedDataField.bidQty1, pMsg->sharedDataField.bidPrice1 != NO_PRICE ? pMsg->sharedDataField.bidPrice1 : 0, pMsg->sharedDataField.askPrice1 != NO_PRICE ? pMsg->sharedDataField.askPrice1 : 0, pMsg->sharedDataField.askQty1, feed_item);

			double closePrice = (!(pMsg->staticDataField.preClosePrice == NO_PRICE || pMsg->staticDataField.preClosePrice == 0)) ? pMsg->staticDataField.preClosePrice : (pMsg->staticDataField.preClosePrice != NO_PRICE ? pMsg->staticDataField.preClosePrice : 0);
			feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->sharedDataField.latestPrice != NO_PRICE ? pMsg->sharedDataField.latestPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->sharedDataField.tradeQty;
			feed_item->set_daily_volume(dailyVolume);

			//double hightest = pMsg->sharedDataField.highestPrice;

			//double lowest = pMsg->sharedDataField.lowestPrice;

			double upperlmt = pMsg->staticDataField.upperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->staticDataField.lowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);
		}
	}
}
