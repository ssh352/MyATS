//#include "tdf_item.h"
//#include "fields.h"
//
//namespace feed
//{
//	tdf_item::tdf_item(const char* subject, abstract_feed_source* pAbstractSource) :abstract_feed_item(subject, pAbstractSource)
//	{
//		m_nbLimits = 1;
//		for (int i = 0; i < TDF_FEED_MAX_DEPTH; i++)
//		{
//			m_bidNb[i] = 0;
//			m_bidQty[i] = 0;
//			m_bidPrc[i] = 0;
//			m_askPrc[i] = 0;
//			m_askQty[i] = 0;
//			m_askNb[i] = 0;
//		}
//		m_lastQty = 0;
//		m_lastPrc = 0;
//		m_dailyVolume = 0;
//		m_percentChange = 0;
//		m_upperLimit = 0;
//		m_lowerLimit = 0;
//		m_closePrc = 0;
//	}
//
//	tdf_item::~tdf_item()
//	{}
//
//	void tdf_item::process_msg(TDF_MARKET_DATA* pMsg)
//	{
//		boost::lock_guard<boost::mutex> lock(m_mutex);
//		m_updateList.clear();
//		for (int i = 0; i < TDF_FEED_MAX_DEPTH; i++)
//		{
//			if (i <= 4)
//			{
//				process_depth(i, pMsg->nBidVol[i], pMsg->nBidPrice[i] / FACTOR, pMsg->nAskPrice[i] / FACTOR, pMsg->nAskVol[i]);
//			}
//			else
//			{
//				process_depth2(i, pMsg->nBidVol[i], pMsg->nBidPrice[i] / FACTOR, pMsg->nAskPrice[i] / FACTOR, pMsg->nAskVol[i]);
//			}
//		}
//		double closePrice = pMsg->nPreClose / FACTOR;
//		if (m_closePrc != closePrice)
//		{
//			m_closePrc = closePrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_HST_CLOSE, closePrice);
//			m_updateList.set_value(RT_HST_CLOSE, pFeedItemValue);
//		}
//		double lastPrice = pMsg->nMatch / FACTOR;
//		if (m_lastPrc != lastPrice)
//		{
//			m_lastPrc = lastPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
//			m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);
//			if (m_closePrc != 0)
//			{
//				double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
//				if (m_percentChange != percentChange)
//				{
//					shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
//					m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
//				}
//			}
//		}
//		double openPrice = pMsg->nOpen / FACTOR;
//		if (m_openPrc != openPrice)
//		{
//			m_openPrc = openPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_THEO_OPENPRC, openPrice);
//			m_updateList.set_value(RT_THEO_OPENPRC, pFeedItemValue);
//		}
//
//		double upperLimit = pMsg->nHighLimited / FACTOR;
//		if (m_upperLimit != upperLimit)
//		{
//			m_upperLimit = upperLimit;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_UPPER_LMT_PRC, upperLimit);
//			m_updateList.set_value(RT_UPPER_LMT_PRC, pFeedItemValue);
//		}
//
//		double lowerLimit = pMsg->nLowLimited / FACTOR;
//		if (m_lowerLimit != lowerLimit)
//		{
//			m_lowerLimit = lowerLimit;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_LOWER_LMT_PRC, lowerLimit);
//			m_updateList.set_value(RT_LOWER_LMT_PRC, pFeedItemValue);
//		}
//
//		//?? RT_NUM_MOVES ???
//		int lastQuantity = pMsg->nNumTrades;
//		if (m_lastQty != lastQuantity)
//		{
//			m_lastQty = lastQuantity;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_VOL_1, lastQuantity);
//			m_updateList.set_value(RT_TRD_VOL_1, pFeedItemValue);
//		}
//
//		int dailyVolume = pMsg->iVolume;
//		if (m_dailyVolume != dailyVolume)
//		{
//			m_dailyVolume = dailyVolume;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ACVOL_1, dailyVolume);
//			m_updateList.set_value(RT_ACVOL_1, pFeedItemValue);
//		}
//
//		// send updates to client
//		int n = m_updateList.size();
//		if (m_pItemEventHandler != NULL && n > 0)
//		{
//			m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
//		} 
//	}
//
//	void tdf_item::process_msg(TDF_FUTURE_DATA* pMsg)
//	{
//		boost::lock_guard<boost::mutex> lock(m_mutex);
//		m_updateList.clear();
//
//		double closePrice = pMsg->nPreClose / FACTOR;
//		if (m_closePrc != closePrice)
//		{
//			m_closePrc = closePrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_HST_CLOSE, closePrice);
//			m_updateList.set_value(RT_HST_CLOSE, pFeedItemValue);
//		}
//
//		double lastPrice = pMsg->nMatch / FACTOR;
//		if (m_lastPrc != lastPrice)
//		{
//			m_lastPrc = lastPrice;
//
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
//			m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);
//
//			if (m_closePrc != 0)
//			{
//				double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
//				if (m_percentChange != percentChange)
//				{
//					shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
//					m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
//				}
//			}
//		}
//
//		double openPrice = pMsg->nOpen / FACTOR;
//		if (m_openPrc != openPrice)
//		{
//			m_openPrc = openPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_THEO_OPENPRC, openPrice);
//			m_updateList.set_value(RT_THEO_OPENPRC, pFeedItemValue);
//		}
//
//		int dailyVolume = pMsg->iVolume;
//		if (m_dailyVolume != dailyVolume)
//		{
//			m_dailyVolume = dailyVolume;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ACVOL_1, dailyVolume);
//			m_updateList.set_value(RT_ACVOL_1, pFeedItemValue);
//		}
//		
//		for (int i = 0; i < 5; i++)
//		{
//			process_depth(i, pMsg->nBidVol[i], pMsg->nBidPrice[i] / FACTOR, pMsg->nAskPrice[i] / FACTOR, pMsg->nAskVol[i]);
//		}
//
//
//
//		double upperLimit = pMsg->nHighLimited / FACTOR;
//		if (m_upperLimit != upperLimit)
//		{
//			m_upperLimit = upperLimit;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_UPPER_LMT_PRC, upperLimit);
//			m_updateList.set_value(RT_UPPER_LMT_PRC, pFeedItemValue);
//		}
//
//		double lowerLimit = pMsg->nLowLimited / FACTOR;
//		if (m_lowerLimit != lowerLimit)
//		{
//			m_lowerLimit = lowerLimit;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_LOWER_LMT_PRC, lowerLimit);
//			m_updateList.set_value(RT_LOWER_LMT_PRC, pFeedItemValue);
//		}
//
//
//		// send updates to client
//		int n = m_updateList.size();
//		if (m_pItemEventHandler != NULL && n > 0)
//		{
//			//logger::info("Calling TDF ITEM feed_item_update_cb");
//			m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
//		}
//
//	
//	
//	}
//
//	void tdf_item::process_msg(TDF_INDEX_DATA* pMsg)
//	{
//		boost::lock_guard<boost::mutex> lock(m_mutex);
//		m_updateList.clear();
//		double closePrice = pMsg->nPreCloseIndex / FACTOR;
//		if (m_closePrc != closePrice)
//		{
//			m_closePrc = closePrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_HST_CLOSE, closePrice);
//			m_updateList.set_value(RT_HST_CLOSE, pFeedItemValue);
//		}
//
//		double lastPrice = pMsg->nLastIndex / FACTOR;
//		if (m_lastPrc != lastPrice)
//		{
//			m_lastPrc = lastPrice;
//
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
//			m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);
//
//			if (m_closePrc != 0)
//			{
//				double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
//				if (m_percentChange != percentChange)
//				{
//					shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
//					m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
//				}
//			}
//		}
//
//		double openPrice = pMsg->nOpenIndex / FACTOR;
//		if (m_openPrc != openPrice)
//		{
//			m_openPrc = openPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_THEO_OPENPRC, openPrice);
//			m_updateList.set_value(RT_THEO_OPENPRC, pFeedItemValue);
//		}
//
//		int dailyVolume = pMsg->iTotalVolume;
//		if (m_dailyVolume != dailyVolume)
//		{
//			m_dailyVolume = dailyVolume;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ACVOL_1, dailyVolume);
//			m_updateList.set_value(RT_ACVOL_1, pFeedItemValue);
//		}
//
//
//
//		// send updates to client
//		int n = m_updateList.size();
//		if (m_pItemEventHandler != NULL && n > 0)
//		{
//			//logger::info("Calling TDF ITEM feed_item_update_cb");
//			m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
//		}
//
//
//	}
//
//	void tdf_item::process_msg(TDF_TRANSACTION* pMsg)
//	{
//		//logger::info("update by trade for %s %f %d", pMsg->szWindCode, pMsg->nPrice/FACTOR, pMsg->nVolume);
//		boost::lock_guard<boost::mutex> lock(m_mutex);
//		m_updateList.clear();
//		double lastPrice = pMsg->nPrice / FACTOR;
//		if (m_lastPrc != lastPrice)
//		{
//			m_lastPrc = lastPrice;
//
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
//			m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);
//
//			if (m_closePrc != 0)
//			{
//				double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
//				if (m_percentChange != percentChange)
//				{
//					shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
//					m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
//				}
//			}
//		}
//
//		
//		int lastQuantity = pMsg->nVolume;
//		if (m_lastQty != lastQuantity)
//		{
//			m_lastQty = lastQuantity;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_VOL_1, lastQuantity);
//			m_updateList.set_value(RT_TRD_VOL_1, pFeedItemValue);
//		}
//
//		int n = m_updateList.size();
//		if (m_pItemEventHandler != NULL && n > 0)
//		{
//			//logger::info("Calling TDF ITEM feed_item_update_cb");
//			m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
//		}
//
//	}
//	
//	void tdf_item::process_msg(TDF_ORDER_QUEUE* pMsg)
//	{
//		boost::lock_guard<boost::mutex> lock(m_mutex);
//		m_updateList.clear();
//		//logger::info("get order items %d", pMsg->nABItems);
//		int quantity = 0;
//		for (int i = 0; i < pMsg->nABItems;i++)
//		{
//			quantity += pMsg->nABVolume[i];
//		}
//
//		if (pMsg->nSide=='A')
//		{
//			//logger::info("update ask size %f %d", pMsg->nPrice/FACTOR, quantity);
//			process_depth(0, m_bidQty[0], m_bidPrc[0], pMsg->nPrice / FACTOR, quantity);
//		}
//		else if (pMsg->nSide == 'B')
//		{
//			//logger::info("update bid size %f %d", pMsg->nPrice / FACTOR, quantity);
//			process_depth(0, quantity,pMsg->nPrice/FACTOR,m_askPrc[0],m_askQty[0]);
//		}
//		
//		int n = m_updateList.size();
//		if (m_pItemEventHandler != NULL && n > 0)
//		{
//			//logger::info("Calling TDF ITEM feed_item_update_cb");
//			m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
//		}
//
//
//	}
//
//
//	void tdf_item::process_depth(int i, int bidQuantity, double bidPrice, double askPrice, int askQuantity)
//	{
//		//int bidQuantity = msg.nBidVol1;
//		if (m_bidQty[i] != bidQuantity)
//		{
//			m_bidQty[i] = bidQuantity;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_BID_VOL_1 + i, bidQuantity);
//			m_updateList.set_value(RT_BID_VOL_1 + i, pFeedItemValue);
//		}
//
//		//double bidPrice = msg.dBidPrice1;
//		if (m_bidPrc[i] != bidPrice)
//		{
//			m_bidPrc[i] = bidPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_BID_PRC_1 + i, bidPrice);
//			m_updateList.set_value(RT_BID_PRC_1 + i, pFeedItemValue);
//		}
//
//		//double askPrice = msg.dAskPrice1;
//		if (m_askPrc[i] != askPrice)
//		{
//			m_askPrc[i] = askPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ASK_PRC_1 + i, askPrice);
//			m_updateList.set_value(RT_ASK_PRC_1 + i, pFeedItemValue);
//		}
//
//		//int askQuantity = msg.nAskVol1;
//		if (m_askQty[i] != askQuantity)
//		{
//			m_askQty[i] = askQuantity;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ASK_VOL_1 + i, askQuantity);
//			m_updateList.set_value(RT_ASK_VOL_1 + i, pFeedItemValue);
//		}
//	}
//
//
//	void tdf_item::process_depth2(int i, int bidQuantity, double bidPrice, double askPrice, int askQuantity)
//	{
//		//int bidQuantity = msg.nBidVol1;
//		if (m_bidQty[i] != bidQuantity)
//		{
//			m_bidQty[i] = bidQuantity;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_BID_VOL_6 + i - 5, bidQuantity);
//			m_updateList.set_value(RT_BID_VOL_6 + i - 5, pFeedItemValue);
//		}
//
//		//double bidPrice = msg.dBidPrice1;
//		if (m_bidPrc[i] != bidPrice)
//		{
//			m_bidPrc[i] = bidPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_BID_PRC_6 + i - 5, bidPrice);
//			m_updateList.set_value(RT_BID_PRC_6 + i - 5, pFeedItemValue);
//		}
//
//		//double askPrice = msg.dAskPrice1;
//		if (m_askPrc[i] != askPrice)
//		{
//			m_askPrc[i] = askPrice;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ASK_PRC_6 + i - 5, askPrice);
//			m_updateList.set_value(RT_ASK_PRC_6 + i - 5, pFeedItemValue);
//		}
//
//		//int askQuantity = msg.nAskVol1;
//		if (m_askQty[i] != askQuantity)
//		{
//			m_askQty[i] = askQuantity;
//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ASK_VOL_6 + i - 5, askQuantity);
//			m_updateList.set_value(RT_ASK_VOL_6 + i - 5, pFeedItemValue);
//		}
//	}
//}
