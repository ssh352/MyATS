#include <array>
#include "lts_source.h"
#include "lts_connection.h"
#include "feeditem.h"
#include "terra_logger.h"
#include <nn.h>
#include <pubsub.h>
using namespace terra::common;
namespace feed
{
	namespace lts
	{
		lts_source::lts_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, string pub, string url, string req_url)
			: feed_source("LTS", sourceName, hostname, service, brokerId, user, password, db, pub, url,req_url)
		{
			m_pConnection = new lts_connection(this);
		}

		//lts_source::~lts_source()
		//{
		//	if (m_pConnection)
		//	{
		//		this->release();
		//		delete m_pConnection;
		//		m_pConnection = nullptr;
		//	}
		//}

		void lts_source::init_source()
		{
			//
			feed_source::init_source();
			//

			get_queue()->setHandler(boost::bind(&lts_source::process_msg, this, _1));


#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif

			//std::thread t(std::bind(&lts_source::set_kernel_timer_thread, this));
			//m_thread.swap(t);
			load_database();
			
			m_pConnection->init();
			m_pConnection->create();
		}

#ifdef Linux
		void  lts_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&lts_source::process, this));
			m_queue.set_fd(efd);
		}
#endif

		void lts_source::process_msg(CSecurityFtdcDepthMarketDataField* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->InstrumentID) + "." + std::string(pMsg->ExchangeID);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::error("lts_cource cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("lts_source: instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}

		
		void lts_source::process()
		{

			get_queue()->Pops_Handle();

		}
		void lts_source::start_receiver()
		{
			CSecurityFtdcDepthMarketDataField msg;
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
		void lts_source::process_msg(CSecurityFtdcDepthMarketDataField* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;

			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->AskPrice1 != NO_PRICE ? pMsg->AskPrice1 : 0., pMsg->AskVolume1, feed_item);
			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->AskPrice2 != NO_PRICE ? pMsg->AskPrice2 : 0., pMsg->AskVolume2, feed_item);
			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->AskPrice3 != NO_PRICE ? pMsg->AskPrice3 : 0., pMsg->AskVolume3, feed_item);
			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->AskPrice4 != NO_PRICE ? pMsg->AskPrice4 : 0., pMsg->AskVolume4, feed_item);
			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->AskPrice5 != NO_PRICE ? pMsg->AskPrice5 : 0., pMsg->AskVolume5, feed_item);

			double closePrice = (!(pMsg->PreClosePrice == NO_PRICE || math2::is_zero(pMsg->PreClosePrice))) ? pMsg->PreClosePrice : (pMsg->ClosePrice != NO_PRICE ? pMsg->ClosePrice : 0);
			if (math2::not_zero(closePrice))
				feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->Volume;
			feed_item->set_daily_volume(dailyVolume);

			feed_item->set_turnover(pMsg->Turnover);
			//double hightest = pMsg->HighestPrice;

			//double lowest = pMsg->LowestPrice;

			double upperlmt = pMsg->UpperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->LowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->SettlementPrice;
			feed_item->set_settlement(selltement);

			feed_item->set_theoretical_open_price(pMsg->AuctionPrice);
			//double presettle = pMsg->PreSettlementPrice;

			//double preopeninterest = pMsg->PreOpenInterest;


			if (feed_item->market_time != pMsg->UpdateTime)
				feed_item->market_time = pMsg->UpdateTime;

		}
	}

}