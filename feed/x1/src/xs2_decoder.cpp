//#include "xs2_decoder.h"
//
//
//namespace feed
//{
//	namespace xs2
//	{
//		xs2_decoder::xs2_decoder()
//		{			
//		}
//
//		xs2_decoder::~xs2_decoder()
//		{
//		}
//
//		void xs2_decoder::process_msg(DFITCDepthMarketDataField* pMsg,feed_item * feed_item)
//		{
//			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->AskPrice1 != NO_PRICE ? pMsg->AskPrice1 : 0., pMsg->AskVolume1, feed_item);
//			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->AskPrice2 != NO_PRICE ? pMsg->AskPrice2 : 0., pMsg->AskVolume2, feed_item);
//			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->AskPrice3 != NO_PRICE ? pMsg->AskPrice3 : 0., pMsg->AskVolume3, feed_item);
//			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->AskPrice4 != NO_PRICE ? pMsg->AskPrice4 : 0., pMsg->AskVolume4, feed_item);
//			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->AskPrice5 != NO_PRICE ? pMsg->AskPrice5 : 0., pMsg->AskVolume5, feed_item);
//
//			double closePrice = (!(pMsg->closePrice == NO_PRICE || pMsg->closePrice == 0)) ? pMsg->closePrice : (pMsg->preClosePrice != NO_PRICE ? pMsg->preClosePrice : 0);
//			feed_item->set_close_price(closePrice);
//
//			double lastPrice = pMsg->lastPrice != NO_PRICE ? pMsg->lastPrice : 0;
//			feed_item->set_last_price(lastPrice);
//
//			int dailyVolume = pMsg->Volume;
//			feed_item->set_daily_volume(dailyVolume);
//			
//			double hightest = pMsg->highestPrice;
//
//			double lowest = pMsg->lowestPrice;
//
//			double upperlmt = pMsg->upperLimitPrice;
//			feed_item->set_upper_limit(upperlmt);
//
//			double lowerlmt = pMsg->lowerLimitPrice;
//			feed_item->set_lower_limit(lowerlmt);
//
//			double selltement = pMsg->settlementPrice;
//			feed_item->set_settlement(selltement);
//
//			double presettle = pMsg->preSettlementPrice;
//
//			double preopeninterest = pMsg->preOpenInterest;
//		}		
//	}
//}
//
