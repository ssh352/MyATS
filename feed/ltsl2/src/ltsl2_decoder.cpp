//#include "ltsl2_decoder.h"
//#include "feedenum.h"
//
//namespace feed
//{
//	namespace ltsl2
//	{
//		void ltsl2_decoder::process_msg(CSecurityFtdcL2MarketDataField* pMsg, feed_item * feed_item)
//		{
//			if (feed_item == nullptr || pMsg == nullptr)
//				return;
//
//			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->OfferPrice1 != NO_PRICE ? pMsg->OfferPrice1 : 0., pMsg->OfferVolume1, feed_item);
//			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->OfferPrice2 != NO_PRICE ? pMsg->OfferPrice2 : 0., pMsg->OfferVolume2, feed_item);
//			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->OfferPrice3 != NO_PRICE ? pMsg->OfferPrice3 : 0., pMsg->OfferVolume3, feed_item);
//			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->OfferPrice4 != NO_PRICE ? pMsg->OfferPrice4 : 0., pMsg->OfferVolume4, feed_item);
//			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->OfferPrice5 != NO_PRICE ? pMsg->OfferPrice5 : 0., pMsg->OfferVolume5, feed_item);
//
//			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || pMsg->ClosePrice == 0)) ? pMsg->ClosePrice : (pMsg->PreClosePrice != NO_PRICE ? pMsg->PreClosePrice : 0);
//			feed_item->set_close_price(closePrice);
//
//			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
//			feed_item->set_last_price(lastPrice);
//
//			int dailyVolume = pMsg->TotalTradeVolume;
//			feed_item->set_daily_volume(dailyVolume);
//
//			double hightest = pMsg->HighPrice;
//
//			double lowest = pMsg->LowPrice;										
//
//		}
//	}
//}
//
