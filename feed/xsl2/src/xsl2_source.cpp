#include "xsl2_source.h"
#include "xsl2_connection.h"
#include "feeditem.h"
#include "feedcommon.h"

using namespace terra::common;
namespace feed
{
	namespace xsl2
	{

		xsl2_source::xsl2_source(string & sourceName, string & hostname, string & service, string & brokerId, string & user, string & password, string & db, string & udp)
			: feed_source("xsl2", sourceName, hostname, service, brokerId, user, password, db)
		{
			m_pConnection = new xsl2_connection(this);
			m_sUdp = udp;
		}

		//xsl2_source::~xsl2_source()
		//{

		//}
		void xsl2_source::init_source()
		{
			get_5_queue()->setHandler(boost::bind(&xsl2_source::process_5_msg, this, _1));
			get_10_queue()->setHandler(boost::bind(&xsl2_source::process_10_msg, this, _1));

			init_process(io_service_type::feed);
			/*std::thread t(std::bind(&xsl2_source::process, this));
			t.detach();*/
			load_database();

			m_pConnection->init();
			m_pConnection->create();
			return;
		}
		void xsl2_source::process_5_msg(MDBestAndDeep* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->Contract);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::info("lts_cource cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("xsl2_source: instrument %s not found", pMsg->Contract);
				return;
			}
			process_5_msg(pMsg, afeed_item);
			return post(afeed_item);
		}		
		void xsl2_source::process_10_msg(MDTenEntrust* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->Contract);
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("xsl2_source: instrument %s not found", pMsg->Contract);
				return;
			}
			process_10_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		void xsl2_source::process()
		{
			get_5_queue()->Pops_Handle();
			get_10_queue()->Pops_Handle();
		}		
		/*
		////////////////////////////////////////////////
		///MDBestAndDeep：最优与五档深度行情
		////////////////////////////////////////////////
		typedef struct MDBestAndDeep{
		INT1		Type;
		UINT4		Length;							//报文长度
		UINT4		Version;						//版本从1开始
		UINT4		Time;							//预留字段
		INT1		Exchange[3];					//交易所
		INT1		Contract[80];					//合约代码
		BOOL		SuspensionSign;					//停牌标志
		REAL4		LastClearPrice;					//昨结算价
		REAL4		ClearPrice;						//今结算价
		REAL4		AvgPrice;						//成交均价
		REAL4		LastClose;						//昨收盘
		REAL4		Close;							//今收盘
		REAL4		OpenPrice;						//今开盘
		UINT4		LastOpenInterest;				//昨持仓量
		UINT4		OpenInterest;					//持仓量
		REAL4		LastPrice;						//最新价
		UINT4		MatchTotQty;					//成交数量
		REAL8		Turnover;						//成交金额
		REAL4		RiseLimit;						//最高报价
		REAL4		FallLimit;						//最低报价
		REAL4		HighPrice;						//最高价
		REAL4		LowPrice;						//最低价
		REAL4		PreDelta;						//昨虚实度
		REAL4		CurrDelta;						//今虚实度
		REAL4		BuyPriceOne;					//买入价格1
		UINT4		BuyQtyOne;						//买入数量1
		UINT4		BuyImplyQtyOne;					//买1推导量
		REAL4		BuyPriceTwo;
		UINT4		BuyQtyTwo;
		UINT4		BuyImplyQtyTwo;
		REAL4		BuyPriceThree;
		UINT4		BuyQtyThree;
		UINT4		BuyImplyQtyThree;
		REAL4		BuyPriceFour;
		UINT4		BuyQtyFour;
		UINT4		BuyImplyQtyFour;
		REAL4		BuyPriceFive;
		UINT4		BuyQtyFive;
		UINT4		BuyImplyQtyFive;
		REAL4		SellPriceOne;					//卖出价格1
		UINT4		SellQtyOne;						//买出数量1
		UINT4		SellImplyQtyOne;				//卖1推导量
		REAL4		SellPriceTwo;
		UINT4		SellQtyTwo;
		UINT4		SellImplyQtyTwo;
		REAL4		SellPriceThree;
		UINT4		SellQtyThree;
		UINT4		SellImplyQtyThree;
		REAL4		SellPriceFour;
		UINT4		SellQtyFour;
		UINT4		SellImplyQtyFour;
		REAL4		SellPriceFive;
		UINT4		SellQtyFive;
		UINT4		SellImplyQtyFive;
		INT1		GenTime[13];					//行情产生时间
		UINT4		LastMatchQty;					//最新成交量
		INT4		InterestChg;					//持仓量变化
		REAL4		LifeLow;						//历史最低价
		REAL4		LifeHigh;						//历史最高价
		REAL8		Delta;							//delta
		REAL8		Gamma;							//gama
		REAL8		Rho;							//rho
		REAL8		Theta;							//theta
		REAL8		Vega;							//vega
		INT1		TradeDate[9];					//行情日期
		INT1		LocalDate[9];					//本地日期
		}THYQuote;
		*/
		void xsl2_source::process_5_msg(MDBestAndDeep* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;

			process_depth(0, pMsg->BuyQtyOne, pMsg->BuyPriceOne != NO_PRICE ? pMsg->BuyPriceOne : 0., pMsg->SellPriceOne != NO_PRICE ? pMsg->SellPriceOne : 0., pMsg->SellQtyOne, feed_item);
			process_depth(1, pMsg->BuyQtyTwo, pMsg->BuyPriceTwo != NO_PRICE ? pMsg->BuyPriceTwo : 0., pMsg->SellPriceTwo != NO_PRICE ? pMsg->SellPriceTwo : 0., pMsg->SellQtyTwo, feed_item);
			process_depth(2, pMsg->BuyQtyThree, pMsg->BuyPriceThree != NO_PRICE ? pMsg->BuyPriceThree : 0., pMsg->SellPriceThree != NO_PRICE ? pMsg->SellPriceThree : 0., pMsg->SellQtyThree, feed_item);
			process_depth(3, pMsg->BuyQtyFour, pMsg->BuyPriceFour != NO_PRICE ? pMsg->BuyPriceFour : 0., pMsg->SellPriceFour != NO_PRICE ? pMsg->SellPriceFour : 0., pMsg->SellQtyFour, feed_item);
			process_depth(4, pMsg->BuyQtyFive, pMsg->BuyPriceFive != NO_PRICE ? pMsg->BuyPriceFive : 0., pMsg->SellPriceFive != NO_PRICE ? pMsg->SellPriceFive : 0., pMsg->SellQtyFive, feed_item);

			double closePrice = (!(pMsg->LastClose == NO_PRICE || math2::is_zero(pMsg->LastClose))) ? pMsg->LastClose : (pMsg->Close != NO_PRICE ? pMsg->Close : 0);
			if (math2::not_zero(closePrice))
				feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->MatchTotQty;
			feed_item->set_daily_volume(dailyVolume);

			//double hightest = pMsg->HighPrice;

			//double lowest = pMsg->LowPrice;

			double upperlmt = pMsg->RiseLimit;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->FallLimit;
			feed_item->set_lower_limit(lowerlmt);

			//double selltement = pMsg->SettlementPrice;
			//feed_item->set_settlement(selltement);

			//double presettle = pMsg->PreSettlementPrice;

			//double preopeninterest = pMsg->PreOpenInterest;

			if (feed_item->market_time != pMsg->GenTime)
				feed_item->market_time = pMsg->GenTime;

		}
		/*
		////////////////////////////////////////////////
		///MDTenEntrust：最优价位上十笔委托
		////////////////////////////////////////////////
		typedef struct MDTenEntrust{
		INT1		Type;							//行情域标识
		UINT4		Len;
		INT1		Contract[80];					//合约代码
		REAL8		BestBuyOrderPrice;				//价格
		UINT4		BestBuyOrderQtyOne;				//委托量1
		UINT4		BestBuyOrderQtyTwo;				//委托量2
		UINT4		BestBuyOrderQtyThree;			//委托量3
		UINT4		BestBuyOrderQtyFour;			//委托量4
		UINT4		BestBuyOrderQtyFive;			//委托量5
		UINT4		BestBuyOrderQtySix;				//委托量6
		UINT4		BestBuyOrderQtySeven;			//委托量7
		UINT4		BestBuyOrderQtyEight;			//委托量8
		UINT4		BestBuyOrderQtyNine;			//委托量9
		UINT4		BestBuyOrderQtyTen;				//委托量10
		REAL8		BestSellOrderPrice;				//价格
		UINT4		BestSellOrderQtyOne;			//委托量1
		UINT4		BestSellOrderQtyTwo;			//委托量2
		UINT4		BestSellOrderQtyThree;			//委托量3
		UINT4		BestSellOrderQtyFour;			//委托量4
		UINT4		BestSellOrderQtyFive;			//委托量5
		UINT4		BestSellOrderQtySix;			//委托量6
		UINT4		BestSellOrderQtySeven;			//委托量7
		UINT4		BestSellOrderQtyEight;			//委托量8
		UINT4		BestSellOrderQtyNine;			//委托量9
		UINT4		BestSellOrderQtyTen;			//委托量10
		INT1		GenTime[13];					//生成时间
		}TENENTRUST;
		*/
		void xsl2_source::process_10_msg(MDTenEntrust* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;
#if 0
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

			double hightest = pMsg->HighPrice;

			double lowest = pMsg->LowPrice;

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
#endif

		}
	}
}

