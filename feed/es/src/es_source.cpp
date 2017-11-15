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
			TapAPIContract				Contract;						///< 合约
			TAPISTR_10					CurrencyNo;						///< 币种编号
			TAPICHAR					TradingState;					///< 交易状态。1,集合竞价;2,集合竞价撮合;3,连续交易;4,交易暂停;5,闭市
			TAPIDTSTAMP					DateTimeStamp;					///< 时间戳
			TAPIQPRICE					QPreClosingPrice;				///< 昨收盘价
			TAPIQPRICE					QPreSettlePrice;				///< 昨结算价
			TAPIQVOLUME					QPrePositionQty;				///< 昨持仓量
			TAPIQPRICE					QOpeningPrice;					///< 开盘价
			TAPIQPRICE					QLastPrice;						///< 最新价
			TAPIQPRICE					QHighPrice;						///< 最高价
			TAPIQPRICE					QLowPrice;						///< 最低价
			TAPIQPRICE					QHisHighPrice;					///< 历史最高价
			TAPIQPRICE					QHisLowPrice;					///< 历史最低价
			TAPIQPRICE					QLimitUpPrice;					///< 涨停价
			TAPIQPRICE					QLimitDownPrice;				///< 跌停价
			TAPIQVOLUME					QTotalQty;						///< 当日总成交量
			TAPIQPRICE					QTotalTurnover;					///< 当日成交金额
			TAPIQVOLUME					QPositionQty;					///< 持仓量
			TAPIQPRICE					QAveragePrice;					///< 均价
			TAPIQPRICE					QClosingPrice;					///< 收盘价
			TAPIQPRICE					QSettlePrice;					///< 结算价
			TAPIQVOLUME					QLastQty;						///< 最新成交量
			TAPIQPRICE					QBidPrice[20];					///< 买价1-20档
			TAPIQVOLUME					QBidQty[20];					///< 买量1-20档
			TAPIQPRICE					QAskPrice[20];					///< 卖价1-20档
			TAPIQVOLUME					QAskQty[20];					///< 卖量1-20档
			TAPIQPRICE					QImpliedBidPrice;				///< 隐含买价
			TAPIQVOLUME					QImpliedBidQty;					///< 隐含买量
			TAPIQPRICE					QImpliedAskPrice;				///< 隐含卖价
			TAPIQVOLUME					QImpliedAskQty;					///< 隐含卖量
			TAPIQPRICE					QPreDelta;						///< 昨虚实度
			TAPIQPRICE					QCurrDelta;						///< 今虚实度
			TAPIQVOLUME					QInsideQty;						///< 内盘量
			TAPIQVOLUME					QOutsideQty;					///< 外盘量
			TAPIQPRICE					QTurnoverRate;					///< 换手率
			TAPIQVOLUME					Q5DAvgQty;						///< 五日均量
			TAPIQPRICE					QPERatio;						///< 市盈率
			TAPIQPRICE					QTotalValue;					///< 总市值
			TAPIQPRICE					QNegotiableValue;				///< 流通市值
			TAPIQDIFF					QPositionTrend;					///< 持仓走势
			TAPIQPRICE					QChangeSpeed;					///< 涨速
			TAPIQPRICE					QChangeRate;					///< 涨幅
			TAPIQPRICE					QChangeValue;					///< 涨跌值
			TAPIQPRICE					QSwing;							///< 振幅
			TAPIQVOLUME					QTotalBidQty;					///< 委买总量
			TAPIQVOLUME					QTotalAskQty;					///< 委卖总量
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

