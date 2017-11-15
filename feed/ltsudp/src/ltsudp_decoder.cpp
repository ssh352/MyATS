//#include "ltsudp_decoder.h"
//#include "feedenum.h"
//
//namespace feed
//{
//	namespace ltsudp
//	{
//		void ltsudp_decoder::process_msg(CFAST_MD* pMsg, feed_item * feed_item)
//		{
//			if (feed_item == nullptr || pMsg == nullptr)
//				return;
//			process_depth(0, pMsg->BuyVolume1, pMsg->BuyPrice1 != NO_PRICE ? pMsg->BuyPrice1 : 0., pMsg->SellPrice1 != NO_PRICE ? pMsg->SellPrice1 : 0., pMsg->SellVolume1, feed_item);
//			process_depth(1, pMsg->BuyVolume2, pMsg->BuyPrice2 != NO_PRICE ? pMsg->BuyPrice2 : 0., pMsg->SellPrice2 != NO_PRICE ? pMsg->SellPrice2 : 0., pMsg->SellVolume2, feed_item);
//			process_depth(2, pMsg->BuyVolume3, pMsg->BuyPrice3 != NO_PRICE ? pMsg->BuyPrice3 : 0., pMsg->SellPrice3 != NO_PRICE ? pMsg->SellPrice3 : 0., pMsg->SellVolume3, feed_item);
//			process_depth(3, pMsg->BuyVolume4, pMsg->BuyPrice4 != NO_PRICE ? pMsg->BuyPrice4 : 0., pMsg->SellPrice4 != NO_PRICE ? pMsg->SellPrice4 : 0., pMsg->SellVolume4, feed_item);
//			process_depth(4, pMsg->BuyVolume5, pMsg->BuyPrice5 != NO_PRICE ? pMsg->BuyPrice5 : 0., pMsg->SellPrice5 != NO_PRICE ? pMsg->SellPrice5 : 0., pMsg->SellVolume5, feed_item);
//
//			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || pMsg->ClosePrice == 0)) ? pMsg->ClosePrice : 0;
//			feed_item->set_close_price(closePrice);
//
//			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
//			feed_item->set_last_price(lastPrice);
//
//			int dailyVolume = pMsg->TradeVolume;
//			feed_item->set_daily_volume(dailyVolume);
//
//			double hightest = pMsg->HighPrice;
//
//			double lowest = pMsg->LowPrice;
//		}
//	}
//}
