//#include "lts_decoder.h"
//#include "feedenum.h"
//namespace feed
//{
//	namespace lts
//	{
//		void lts_decoder::process_msg(CSecurityFtdcDepthMarketDataField* pMsg, feed_item * feed_item)
//		{
//			if (feed_item == nullptr || pMsg == nullptr)
//				return;
//
//			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->AskPrice1 != NO_PRICE ? pMsg->AskPrice1 : 0., pMsg->AskVolume1, feed_item);
//			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->AskPrice2 != NO_PRICE ? pMsg->AskPrice2 : 0., pMsg->AskVolume2, feed_item);
//			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->AskPrice3 != NO_PRICE ? pMsg->AskPrice3 : 0., pMsg->AskVolume3, feed_item);
//			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->AskPrice4 != NO_PRICE ? pMsg->AskPrice4 : 0., pMsg->AskVolume4, feed_item);
//			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->AskPrice5 != NO_PRICE ? pMsg->AskPrice5 : 0., pMsg->AskVolume5, feed_item);
//
//			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || pMsg->ClosePrice == 0)) ? pMsg->ClosePrice : (pMsg->PreClosePrice != NO_PRICE ? pMsg->PreClosePrice : 0);
//			feed_item->set_close_price(closePrice);
//
//			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
//			feed_item->set_last_price(lastPrice);
//
//			int dailyVolume = pMsg->Volume;
//			feed_item->set_daily_volume(dailyVolume);
//
//			double hightest = pMsg->HighestPrice;
//
//			double lowest = pMsg->LowestPrice;
//
//			double upperlmt = pMsg->UpperLimitPrice;
//			feed_item->set_upper_limit(upperlmt);
//
//			double lowerlmt = pMsg->LowerLimitPrice;
//			feed_item->set_lower_limit(lowerlmt);
//
//			double selltement = pMsg->SettlementPrice;
//			feed_item->set_settlement(selltement);
//
//			double presettle = pMsg->PreSettlementPrice;
//
//			double preopeninterest = pMsg->PreOpenInterest;
//		}
//	}
//
//}