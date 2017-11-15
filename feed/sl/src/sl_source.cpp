#include "sl_source.h"
#include "sl_connection.h"
#include "feeditem.h"
#include "feedcommon.h"
#include <nn.h>
#include <pubsub.h>
using namespace terra::common;
namespace feed
{
	namespace sl
	{
		sl_source::sl_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & udp, string pub, string url, string req_url)
			: feed_source("SL", sourceName, hostname, service, brokerId, user, password, db,pub,url,req_url)
		{
			m_pConnection = new sl_connection(this);
			m_sUdp = udp;
			get_queue()->setHandler(boost::bind(&sl_source::process_msg, this, _1));
		}
		void sl_source::init_source()
		{			
			feed_source::init_source();

			//init_process();
#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif
			load_database();
			m_pConnection->init();
			m_pConnection->create();
		}
		void sl_source::process_msg(EESMarketDepthQuoteData* pMsg)
		{
			
			std::string sFeedCode = pMsg->InstrumentID;/* + "." + std::string(pMsg->ExchangeID)*/;			
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("lts_source: instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}		
		void sl_source::start_receiver()
		{
			EESMarketDepthQuoteData msg;
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
#ifdef Linux
		void  sl_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&sl_source::process, this));
			m_queue.set_fd(efd);
		}
#endif

		void sl_source::process()
		{
			get_queue()->Pops_Handle();
		}		
		/*
		/// \brief EES行情结构
		struct EESMarketDepthQuoteData
		{
		EESQuoteDateType            TradingDay;     ///<交易日
		EESQuoteInstrumentIDType    InstrumentID;   ///<合约代码
		EESQuoteExchangeIDType      ExchangeID;     ///<交易所代码
		EESQuoteExchangeInstIDType  ExchangeInstID; ///<合约在交易所的代码
		EESQuotePriceType           LastPrice;      ///<最新价
		EESQuotePriceType           PreSettlementPrice; ///<上次结算价
		EESQuotePriceType           PreClosePrice;    ///<昨收盘
		EESQuoteLargeVolumeType     PreOpenInterest; ///<昨持仓量
		EESQuotePriceType           OpenPrice;       ///<今开盘
		EESQuotePriceType           HighestPrice;    ///<最高价
		EESQuotePriceType           LowestPrice;     ///<最低价
		EESQuoteVolumeType          Volume;          ///<数量
		EESQuoteMoneyType           Turnover;        ///<成交金额
		EESQuoteLargeVolumeType     OpenInterest;    ///<持仓量
		EESQuotePriceType           ClosePrice;      ///<今收盘
		EESQuotePriceType           SettlementPrice; ///<本次结算价
		EESQuotePriceType           UpperLimitPrice; ///<涨停板价
		EESQuotePriceType           LowerLimitPrice; ///<跌停板价
		EESQuoteRatioType           PreDelta;        ///<昨虚实度
		EESQuoteRatioType           CurrDelta;       ///<今虚实度
		EESQuoteTimeType            UpdateTime;      ///<最后修改时间
		EESQuoteMillisecType        UpdateMillisec;  ///<最后修改毫秒
		EESQuotePriceType           BidPrice1;       ///<申买价一
		EESQuoteVolumeType          BidVolume1;      ///<申买量一
		EESQuotePriceType           AskPrice1;       ///<申卖价一
		EESQuoteVolumeType          AskVolume1;      ///<申卖量一
		EESQuotePriceType           BidPrice2;       ///<申买价二
		EESQuoteVolumeType          BidVolume2;      ///<申买量二
		EESQuotePriceType           AskPrice2;       ///<申卖价二
		EESQuoteVolumeType          AskVolume2;      ///<申卖量二
		EESQuotePriceType           BidPrice3;       ///<申买价三
		EESQuoteVolumeType          BidVolume3;      ///<申买量三
		EESQuotePriceType           AskPrice3;       ///<申卖价三
		EESQuoteVolumeType          AskVolume3;      ///<申卖量三
		EESQuotePriceType           BidPrice4;       ///<申买价四
		EESQuoteVolumeType          BidVolume4;      ///<申买量四
		EESQuotePriceType           AskPrice4;       ///<申卖价四
		EESQuoteVolumeType          AskVolume4;      ///<申卖量四
		EESQuotePriceType           BidPrice5;       ///<申买价五
		EESQuoteVolumeType          BidVolume5;      ///<申买量五
		EESQuotePriceType           AskPrice5;       ///<申卖价五
		EESQuoteVolumeType          AskVolume5;      ///<申卖量五
		EESQuotePriceType           AveragePrice;    ///<当日均价
		};
		*/
		void sl_source::process_msg(EESMarketDepthQuoteData* pMsg, feed_item * feed_item)
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
			feed_item->set_turnover(pMsg->Turnover);
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

			if (feed_item->market_time != pMsg->UpdateTime)
				feed_item->market_time = pMsg->UpdateTime;

		}
	}
}

