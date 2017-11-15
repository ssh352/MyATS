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
		/// \brief EES����ṹ
		struct EESMarketDepthQuoteData
		{
		EESQuoteDateType            TradingDay;     ///<������
		EESQuoteInstrumentIDType    InstrumentID;   ///<��Լ����
		EESQuoteExchangeIDType      ExchangeID;     ///<����������
		EESQuoteExchangeInstIDType  ExchangeInstID; ///<��Լ�ڽ������Ĵ���
		EESQuotePriceType           LastPrice;      ///<���¼�
		EESQuotePriceType           PreSettlementPrice; ///<�ϴν����
		EESQuotePriceType           PreClosePrice;    ///<������
		EESQuoteLargeVolumeType     PreOpenInterest; ///<��ֲ���
		EESQuotePriceType           OpenPrice;       ///<����
		EESQuotePriceType           HighestPrice;    ///<��߼�
		EESQuotePriceType           LowestPrice;     ///<��ͼ�
		EESQuoteVolumeType          Volume;          ///<����
		EESQuoteMoneyType           Turnover;        ///<�ɽ����
		EESQuoteLargeVolumeType     OpenInterest;    ///<�ֲ���
		EESQuotePriceType           ClosePrice;      ///<������
		EESQuotePriceType           SettlementPrice; ///<���ν����
		EESQuotePriceType           UpperLimitPrice; ///<��ͣ���
		EESQuotePriceType           LowerLimitPrice; ///<��ͣ���
		EESQuoteRatioType           PreDelta;        ///<����ʵ��
		EESQuoteRatioType           CurrDelta;       ///<����ʵ��
		EESQuoteTimeType            UpdateTime;      ///<����޸�ʱ��
		EESQuoteMillisecType        UpdateMillisec;  ///<����޸ĺ���
		EESQuotePriceType           BidPrice1;       ///<�����һ
		EESQuoteVolumeType          BidVolume1;      ///<������һ
		EESQuotePriceType           AskPrice1;       ///<������һ
		EESQuoteVolumeType          AskVolume1;      ///<������һ
		EESQuotePriceType           BidPrice2;       ///<����۶�
		EESQuoteVolumeType          BidVolume2;      ///<��������
		EESQuotePriceType           AskPrice2;       ///<�����۶�
		EESQuoteVolumeType          AskVolume2;      ///<��������
		EESQuotePriceType           BidPrice3;       ///<�������
		EESQuoteVolumeType          BidVolume3;      ///<��������
		EESQuotePriceType           AskPrice3;       ///<��������
		EESQuoteVolumeType          AskVolume3;      ///<��������
		EESQuotePriceType           BidPrice4;       ///<�������
		EESQuoteVolumeType          BidVolume4;      ///<��������
		EESQuotePriceType           AskPrice4;       ///<��������
		EESQuoteVolumeType          AskVolume4;      ///<��������
		EESQuotePriceType           BidPrice5;       ///<�������
		EESQuoteVolumeType          BidVolume5;      ///<��������
		EESQuotePriceType           AskPrice5;       ///<��������
		EESQuoteVolumeType          AskVolume5;      ///<��������
		EESQuotePriceType           AveragePrice;    ///<���վ���
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

