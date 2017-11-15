
#include "cffex_source.h"
#include "cffex_connection.h"
//
#include "nn.h"
#include "pubsub.h"
//
using namespace terra::common;
namespace feed
{
	namespace cffex
	{
		cffex_source::cffex_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & dbName, const string& is_FQR, string pub, string url,string req_url)
			: feed_source("CTP", sourceName, hostname, service, brokerId, user, password, dbName, pub, url,req_url)
		{
			m_pConnection = new cffex_connection(this);
			if (is_FQR == "FQR")
			{
				FQR = true;
			}
			else
			{
				FQR = false;
			}

		}

		/*cffex_source::~cffex_source()
		{
		if (m_pConnection)
		delete m_pConnection;
		}*/
		void cffex_source::init_source()
		{
			//
			feed_source::init_source();
			//

			get_queue()->setHandler(std::bind(&cffex_source::process_msg, this, std::placeholders::_1));

			//std::thread t(std::bind(&cffex_source::set_kernel_timer_thread, this));
			//m_thread.swap(t);
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
		void  cffex_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&cffex_source::process, this));
			m_queue.set_fd(efd);
		}
#endif
		//
		void cffex_source::start_receiver()
		{
			CThostFtdcDepthMarketDataField msg;
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
		//
		//void cffex_source::release_source()
		//{
		//	m_pConnection->cleanup();					
		//}
		void cffex_source::process_msg(CThostFtdcDepthMarketDataField* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->InstrumentID);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::error("cffex_source::process_msg cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("cffex_source::process_msg instrument %s not found", pMsg->InstrumentID);
				return;
			}
			update_item(pMsg, afeed_item);
			return post(afeed_item);
		}

	
		void cffex_source::process()
		{
			get_queue()->Pops_Handle();
		}


		void cffex_source::update_item(CThostFtdcDepthMarketDataField* pMsg, feed_item * feed_item)
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
			feed_item->set_turnover(pMsg->Turnover);

			double upperlmt = pMsg->UpperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->LowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->SettlementPrice;
			feed_item->set_settlement(selltement);

			//double presettle = pMsg->PreSettlementPrice;

			//double preopeninterest = pMsg->PreOpenInterest;

			if (feed_item->market_time != pMsg->UpdateTime)
				feed_item->market_time = pMsg->UpdateTime;
		}

		void cffex_source::receive_FQR(CThostFtdcForQuoteRspField * pForQuoteRsp)
		{
			std::string sFeedCode = std::string(pForQuoteRsp->InstrumentID);

			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::error("cffex_source::FQR instrument %s not found", pForQuoteRsp->InstrumentID);
				return;
			}
			afeed_item->FQR_ID = pForQuoteRsp->ForQuoteSysID;
			afeed_item->set_FQR_time(microsec_clock::local_time().time_of_day().total_seconds());
			
		}

	}
}