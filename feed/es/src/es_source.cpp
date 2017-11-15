#include "es_source.h"
#include "es_connection.h"
#include "feeditem.h"
#include "feedcommon.h"
//
#include "nn.h"
#include "pubsub.h"
//
using namespace terra::common;
namespace feed
{
	namespace es
	{
		es_source::es_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & authCode, string pub, string url, string req_url)
			: feed_source("ES", sourceName, hostname, service, brokerId, user, password, db, pub, url,req_url)
		{
			m_pConnection = new es_connection(this);			
			get_queue()->setHandler(boost::bind(&es_source::process_msg, this, _1));
			m_strAuthCode = authCode;
		}
		void es_source::init_source()
		{			
			//
			feed_source::init_source();
			//
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

#ifdef Linux
		void  es_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "x1 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&es_source::process, this));
			m_queue.set_fd(efd);
		}
#endif
		//
		void es_source::start_receiver()
		{
			TapAPIQuoteWhole msg;
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
		void es_source::process_msg(TapAPIQuoteWhole* pMsg)
		{		
			if (pMsg == nullptr)
				return;
			std::string sFeedCode;
			switch (pMsg->Contract.Commodity.CommodityType)
			{
			case TAPI_COMMODITY_TYPE_FUTURES:
				sFeedCode = string(pMsg->Contract.Commodity.CommodityNo) + string(pMsg->Contract.ContractNo1);
				break;
			case TAPI_COMMODITY_TYPE_OPTION:
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%s%s%c%s", pMsg->Contract.Commodity.CommodityNo, pMsg->Contract.ContractNo1, pMsg->Contract.CallOrPutFlag1,pMsg->Contract.StrikePrice1);
				sFeedCode = buffer;
				break;
			default:
					break;
			}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == nullptr)
			{
				//loggerv2::info("es_source: instrument %s not found", sFeedCode.c_str());
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}		
		void es_source::process()
		{
			get_queue()->Pops_Handle();
		}		
		/*		
		struct TapAPIQuoteWhole
		{
			TapAPIContract				Contract;						///< ��Լ
			TAPISTR_10					CurrencyNo;						///< ���ֱ��
			TAPICHAR					TradingState;					///< ����״̬��1,���Ͼ���;2,���Ͼ��۴��;3,��������;4,������ͣ;5,����
			TAPIDTSTAMP					DateTimeStamp;					///< ʱ���
			TAPIQPRICE					QPreClosingPrice;				///< �����̼�
			TAPIQPRICE					QPreSettlePrice;				///< ������
			TAPIQVOLUME					QPrePositionQty;				///< ��ֲ���
			TAPIQPRICE					QOpeningPrice;					///< ���̼�
			TAPIQPRICE					QLastPrice;						///< ���¼�
			TAPIQPRICE					QHighPrice;						///< ��߼�
			TAPIQPRICE					QLowPrice;						///< ��ͼ�
			TAPIQPRICE					QHisHighPrice;					///< ��ʷ��߼�
			TAPIQPRICE					QHisLowPrice;					///< ��ʷ��ͼ�
			TAPIQPRICE					QLimitUpPrice;					///< ��ͣ��
			TAPIQPRICE					QLimitDownPrice;				///< ��ͣ��
			TAPIQVOLUME					QTotalQty;						///< �����ܳɽ���
			TAPIQPRICE					QTotalTurnover;					///< ���ճɽ����
			TAPIQVOLUME					QPositionQty;					///< �ֲ���
			TAPIQPRICE					QAveragePrice;					///< ����
			TAPIQPRICE					QClosingPrice;					///< ���̼�
			TAPIQPRICE					QSettlePrice;					///< �����
			TAPIQVOLUME					QLastQty;						///< ���³ɽ���
			TAPIQPRICE					QBidPrice[20];					///< ���1-20��
			TAPIQVOLUME					QBidQty[20];					///< ����1-20��
			TAPIQPRICE					QAskPrice[20];					///< ����1-20��
			TAPIQVOLUME					QAskQty[20];					///< ����1-20��
			TAPIQPRICE					QImpliedBidPrice;				///< �������
			TAPIQVOLUME					QImpliedBidQty;					///< ��������
			TAPIQPRICE					QImpliedAskPrice;				///< ��������
			TAPIQVOLUME					QImpliedAskQty;					///< ��������
			TAPIQPRICE					QPreDelta;						///< ����ʵ��
			TAPIQPRICE					QCurrDelta;						///< ����ʵ��
			TAPIQVOLUME					QInsideQty;						///< ������
			TAPIQVOLUME					QOutsideQty;					///< ������
			TAPIQPRICE					QTurnoverRate;					///< ������
			TAPIQVOLUME					Q5DAvgQty;						///< ���վ���
			TAPIQPRICE					QPERatio;						///< ��ӯ��
			TAPIQPRICE					QTotalValue;					///< ����ֵ
			TAPIQPRICE					QNegotiableValue;				///< ��ͨ��ֵ
			TAPIQDIFF					QPositionTrend;					///< �ֲ�����
			TAPIQPRICE					QChangeSpeed;					///< ����
			TAPIQPRICE					QChangeRate;					///< �Ƿ�
			TAPIQPRICE					QChangeValue;					///< �ǵ�ֵ
			TAPIQPRICE					QSwing;							///< ���
			TAPIQVOLUME					QTotalBidQty;					///< ί������
			TAPIQVOLUME					QTotalAskQty;					///< ί������
		};
		*/
		void es_source::process_msg(TapAPIQuoteWhole* pMsg, feed_item * feed_item)
		{
    		if (feed_item == nullptr || pMsg == nullptr)
				return;

			process_depth(0, pMsg->QBidQty[0], pMsg->QBidPrice[0] != NO_PRICE ? pMsg->QBidPrice[0] : 0., pMsg->QAskPrice[0] != NO_PRICE ? pMsg->QAskPrice[0] : 0., pMsg->QAskQty[0], feed_item);
			process_depth(1, pMsg->QBidQty[1], pMsg->QBidPrice[1] != NO_PRICE ? pMsg->QBidPrice[1] : 0., pMsg->QAskPrice[1] != NO_PRICE ? pMsg->QAskPrice[1] : 0., pMsg->QAskQty[1], feed_item);
			process_depth(2, pMsg->QBidQty[2], pMsg->QBidPrice[2] != NO_PRICE ? pMsg->QBidPrice[2] : 0., pMsg->QAskPrice[2] != NO_PRICE ? pMsg->QAskPrice[2] : 0., pMsg->QAskQty[2], feed_item);
			process_depth(3, pMsg->QBidQty[3], pMsg->QBidPrice[3] != NO_PRICE ? pMsg->QBidPrice[3] : 0., pMsg->QAskPrice[3] != NO_PRICE ? pMsg->QAskPrice[3] : 0., pMsg->QAskQty[3], feed_item);
			process_depth(4, pMsg->QBidQty[4], pMsg->QBidPrice[4] != NO_PRICE ? pMsg->QBidPrice[4] : 0., pMsg->QAskPrice[4] != NO_PRICE ? pMsg->QAskPrice[4] : 0., pMsg->QAskQty[4], feed_item);

			double closePrice = (!(pMsg->QClosingPrice == NO_PRICE || pMsg->QClosingPrice == 0)) ? pMsg->QClosingPrice : (pMsg->QPreClosingPrice != NO_PRICE ? pMsg->QPreClosingPrice : 0);
			feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->QLastPrice != NO_PRICE ? pMsg->QLastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->QTotalQty;
			feed_item->set_daily_volume(dailyVolume);
			feed_item->set_turnover(pMsg->QTotalTurnover);

			//double hightest = pMsg->QHighPrice;

			//double lowest = pMsg->QLowPrice;

			double upperlmt = pMsg->QLimitUpPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->QLimitDownPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->QSettlePrice;
			feed_item->set_settlement(selltement);

			if (feed_item->market_time != pMsg->DateTimeStamp)
				feed_item->market_time = pMsg->DateTimeStamp;
		}
	}
}

