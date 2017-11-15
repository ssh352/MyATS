#include "ltsl2_source.h"
#include "ltsl2_connection.h"
#include "feeditem.h"
#include "feedcommon.h"
#include <nn.h>
#include <pubsub.h>
using namespace terra::common;
namespace feed
{
	namespace ltsl2
	{

		ltsl2_source::ltsl2_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & udp, string pub, string url, string req_url)
			: feed_source("LTSL2", sourceName, hostname, service, brokerId, user, password, db, pub, url,req_url)
		{
			m_pConnection = new ltsl2_connection(this);
			m_sUdp = udp;
		}

		//ltsl2_source::~ltsl2_source()
		//{

		//}
		void ltsl2_source::init_source()
		{
			//
			feed_source::init_source();
			//

			get_queue()->setHandler(boost::bind(&ltsl2_source::process_msg, this, _1));

			init_process(io_service_type::feed);
			/*std::thread t(std::bind(&ltsl2_source::process, this));
			t.detach();*/
			load_database();

			m_pConnection->init();
			m_pConnection->create();
			return;
		}

		/*void ltsl2_source::release()
		{
			m_pConnection->cleanup();
		}*/
		void ltsl2_source::start_receiver()
		{
			CSecurityFtdcL2MarketDataField msg;
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
		void ltsl2_source::process_msg(CSecurityFtdcL2MarketDataField* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->InstrumentID) + "." + std::string(pMsg->ExchangeID);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::info("lts_cource cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("ltsl2_source: instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		//void ltsl2_source::process()
		//{
		//	//int i = 0;
		//	while (true)
		//	{
		//		//if (get_queue()->m_queue.read_available() > 0)
		//		//{
		//		//	for(auto &func : get_queue()->m_handler)
		//		//	{
		//		//		func();
		//		//	}
		//		//}
		//		//while (get_queue()->Pop_Handle())
		//		//	++i;
		//		get_queue()->Pops_Handle();
		//		//if (i >= 10)
		//		{
		//			//i = 0;
		//			sleep_by_milliseconds(std::chrono::microseconds(500));
		//		}
		//	}
		//}
		void ltsl2_source::process()
		{
			get_queue()->Pops_Handle();
		}
		//int ltsl2_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		CSecurityFtdcL2MarketDataField* msg = get_queue()->Pop();
		//		this->process_msg(msg);
		//	}
		//	return i;
		//}
		void ltsl2_source::process_msg(CSecurityFtdcL2MarketDataField* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;

			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->OfferPrice1 != NO_PRICE ? pMsg->OfferPrice1 : 0., pMsg->OfferVolume1, feed_item);
			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->OfferPrice2 != NO_PRICE ? pMsg->OfferPrice2 : 0., pMsg->OfferVolume2, feed_item);
			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->OfferPrice3 != NO_PRICE ? pMsg->OfferPrice3 : 0., pMsg->OfferVolume3, feed_item);
			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->OfferPrice4 != NO_PRICE ? pMsg->OfferPrice4 : 0., pMsg->OfferVolume4, feed_item);
			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->OfferPrice5 != NO_PRICE ? pMsg->OfferPrice5 : 0., pMsg->OfferVolume5, feed_item);

			double closePrice = (!(pMsg->PreClosePrice == NO_PRICE || math2::is_zero(pMsg->PreClosePrice))) ? pMsg->PreClosePrice : (pMsg->ClosePrice != NO_PRICE ? pMsg->ClosePrice : 0);
			if (math2::not_zero(closePrice))
				feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->TotalTradeVolume;
			feed_item->set_daily_volume(dailyVolume);
			feed_item->set_turnover(pMsg->TotalTradeValue);
			feed_item->set_theoretical_open_price(pMsg->AuctionPrice);
			//double hightest = pMsg->HighPrice;

			//double lowest = pMsg->LowPrice;

			double upperlmt = pMsg->UpperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->LowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			//double selltement = pMsg->SettlementPrice;
			//feed_item->set_settlement(selltement);

			//double presettle = pMsg->PreSettlementPrice;

			//double preopeninterest = pMsg->PreOpenInterest;

			if (feed_item->market_time != pMsg->TimeStamp)
				feed_item->market_time = pMsg->TimeStamp;

		}
	}
}

