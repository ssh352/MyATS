//#include "xs_decoder.h"
//namespace feed
//{
//	namespace xs
//	{
//		void xs_decoder::process_msg(DFITCSOPDepthMarketDataField* pMsg, feed_item * feed_item)
//		{
//			//feed_decoder::process_msg(feed_item);
//
//			// process depth
//			process_depth(0, pMsg->sharedDataField.bidQty1, pMsg->sharedDataField.bidPrice1 != NO_PRICE ? pMsg->sharedDataField.bidPrice1 : 0, pMsg->sharedDataField.askPrice1 != NO_PRICE ? pMsg->sharedDataField.askPrice1 : 0, pMsg->sharedDataField.askQty1, feed_item);
//
//			double closePrice = (!(pMsg->staticDataField.preClosePrice == NO_PRICE || pMsg->staticDataField.preClosePrice == 0)) ? pMsg->staticDataField.preClosePrice : (pMsg->staticDataField.preClosePrice != NO_PRICE ? pMsg->staticDataField.preClosePrice : 0);
//			feed_item->set_close_price(closePrice);
//
//			double lastPrice = pMsg->sharedDataField.latestPrice != NO_PRICE ? pMsg->sharedDataField.latestPrice : 0;
//			feed_item->set_last_price(lastPrice);
//
//			int dailyVolume = pMsg->sharedDataField.tradeQty;
//			feed_item->set_daily_volume(dailyVolume);
//
//			double hightest = pMsg->sharedDataField.highestPrice;
//
//			double lowest = pMsg->sharedDataField.lowestPrice;
//			
//			double upperlmt = pMsg->staticDataField.upperLimitPrice;
//			feed_item->set_upper_limit(upperlmt);
//
//			double lowerlmt = pMsg->staticDataField.lowerLimitPrice;
//			feed_item->set_lower_limit(lowerlmt);
//						
//			feed_item->set_settlement(pMsg->specificDataField.settlePrice);
//
//			double presettle = pMsg->specificDataField.settlePrice;
//
//			//feed_decoder::post_msg(feed_item);
//		}
//
//		void xs_decoder::process_msg(DFITCStockDepthMarketDataField* pMsg, feed_item * feed_item)
//		{
//			// process depth
//			process_depth(0, pMsg->sharedDataField.bidQty1, pMsg->sharedDataField.bidPrice1 != NO_PRICE ? pMsg->sharedDataField.bidPrice1 : 0, pMsg->sharedDataField.askPrice1 != NO_PRICE ? pMsg->sharedDataField.askPrice1 : 0, pMsg->sharedDataField.askQty1, feed_item);
//
//			double closePrice = (!(pMsg->staticDataField.preClosePrice == NO_PRICE || pMsg->staticDataField.preClosePrice == 0)) ? pMsg->staticDataField.preClosePrice : (pMsg->staticDataField.preClosePrice != NO_PRICE ? pMsg->staticDataField.preClosePrice : 0);
//			feed_item->set_close_price(closePrice);
//
//			double lastPrice = pMsg->sharedDataField.latestPrice != NO_PRICE ? pMsg->sharedDataField.latestPrice : 0;
//			feed_item->set_last_price(lastPrice);
//
//			int dailyVolume = pMsg->sharedDataField.tradeQty;
//			feed_item->set_daily_volume(dailyVolume);
//
//			double hightest = pMsg->sharedDataField.highestPrice;
//
//			double lowest = pMsg->sharedDataField.lowestPrice;
//
//			double upperlmt = pMsg->staticDataField.upperLimitPrice;
//			feed_item->set_upper_limit(upperlmt);
//
//			double lowerlmt = pMsg->staticDataField.lowerLimitPrice;
//			feed_item->set_lower_limit(lowerlmt);					
//		}
//	}
//}
