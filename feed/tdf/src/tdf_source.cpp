#include "tdf_source.h"
#include "tdf_connection.h"


namespace feed
{
	namespace tdf
	{
		//int tdf_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		TDF_MSG* msg = get_queue()->Pop();
		//		this->process_msg(msg);
		//	}
		//	return i;
		//}

		tdf_source::tdf_source(const string & sourceName, const string & hostname, const string & service, const string & user, const string & password, const string & dbName, int date, int time)
			:feed_source("TDF", sourceName, hostname, service, "", user, password, dbName)
		{

			
			m_date = date;
			m_time = time >= 0 ? time : 0xffffffff;
			m_pConnection = new tdf_connection(this, m_date, m_time);
			load_database();
		}

		//tdf_source::~tdf_source()
		//{
		//	if (m_pConnection)
		//		delete m_pConnection;
		//}

		void tdf_source::init_source()
		{
			//get_queue()->setHandler(boost::bind(&tdf_source::process_msg, this, _1));
			market_data_queue.setHandler(boost::bind(&tdf_source::process_market_data, this, _1));;
			index_queue.setHandler(boost::bind(&tdf_source::process_index, this, _1));;
			transaction_queue.setHandler(boost::bind(&tdf_source::process_transaction, this, _1));;
			orderqueue_queue.setHandler(boost::bind(&tdf_source::process_orderqueue, this, _1));;
			future_data_queue.setHandler(boost::bind(&tdf_source::process_future, this, _1));;
			order_queue.setHandler(boost::bind(&tdf_source::process_order, this, _1));;
			
			init_process(io_service_type::feed);

			/*std::thread t(std::bind(&tdf_source::set_kernel_timer_thread, this));
			m_thread.swap(t);*/


			m_pConnection->init();

			//Create();
		}
		//void  tdf_source::set_kernel_timer_thread()
		//{
		//	boost::asio::high_resolution_timer t(io, std::chrono::microseconds(20));
		//	t.async_wait(boost::bind(&tdf_source::process, this, boost::asio::placeholders::error, &t));
		//	io.run();
		//}
		void tdf_source::process()
		{
			//get_queue()->Pops_Handle();
			market_data_queue.Pops_Handle();
			index_queue.Pops_Handle();
			transaction_queue.Pops_Handle();
			orderqueue_queue.Pops_Handle();
			future_data_queue.Pops_Handle();
			order_queue.Pops_Handle();
		}

		//void tdf_source::process(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		//{
		//	//int i = 0;
		//	while (true)
		//	{
		//		//if (get_queue()->m_queue.read_available() > 0)
		//		//{
		//		//	for (auto &func : get_queue()->m_handler)
		//		//	{
		//		//		func();
		//		//	}
		//		//}
		//		//while (get_queue()->Pop_Handle())
		//		//++i;
		//		get_queue()->Pops_Handle();
		//		//if (i >= 10)
		//		{
		//			//i = 0;
		//			t->expires_at(t->expires_at() + std::chrono::microseconds(40));
		//			t->async_wait(boost::bind(&tdf_source::process, this, boost::asio::placeholders::error, t));
		//			return;
		//		}
		//	}
		//}
		//void tdf_source::release_source()
		//{
		//	m_pConnection->cleanup();
		//	//m_pListener->RemoveFDFromReadList(m_queue.GetFD());
		//}



		//void tdf_source::process_msg(TDF_MSG* pMsg)
		//{

		//	unsigned short nCount = pMsg->pAppHead->nItemCount;
		//	unsigned short nSize = pMsg->pAppHead->nItemSize;

		//	for (int i = 0; i < nCount; i++)
		//	{
		//		char itemName[33];
		//		memcpy(&itemName, pMsg->pData, 32);
		//		itemName[32] = '\0';

		//		std::string sFeedCode = std::string(itemName);

		//		////logger::info("tdf_source process_msg receives updates for %s", sFeedCode.c_str());

		//		//std::string instrName = m_feedCode2CodeMap[sFeedCode];
		//		//if (instrName.empty())
		//		//{
		//		//	//logger::info("tdf_source cannot find Code for FeedCode %s, will drop this instrument.", sFeedCode.c_str());
		//		//	return;
		//		//}

		//		//tdf_item* pItem = reinterpret_cast<tdf_item*>(find_item(instrName.c_str()));
		//		feed_item* pItem = get_feed_item(sFeedCode);
		//		if (pItem == nullptr)
		//		{
		//			loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
		//			return;
		//		}


		//		switch (pMsg->nDataType)
		//		{
		//		case MSG_DATA_MARKET:
		//			process_msg(&static_cast<TDF_MARKET_DATA*>((pMsg->pData))[i], pItem);
		//			break;
		//		case MSG_DATA_INDEX:
		//			process_msg(&static_cast<TDF_INDEX_DATA*>((pMsg->pData))[i], pItem);
		//			break;
		//		case MSG_DATA_TRANSACTION:
		//			process_msg(&static_cast<TDF_TRANSACTION*>((pMsg->pData))[i], pItem);
		//			break;
		//		case MSG_DATA_ORDERQUEUE:
		//			process_msg(&static_cast<TDF_ORDER_QUEUE*>((pMsg->pData))[i], pItem);
		//			break;
		//		case MSG_DATA_FUTURE:
		//			process_msg(&static_cast<TDF_FUTURE_DATA*>((pMsg->pData))[i], pItem);
		//			break;

		//		default:
		//			loggerv2::info("receives instrument %s , nDataType =%d", pItem->get_code().c_str(), pMsg->nDataType);
		//			break;
		//		}
		//		post(pItem);
		//	}
		//	delete pMsg->pAppHead; 
		//	TDF_MARKET_DATA* p1;
		//	TDF_INDEX_DATA* p2;
		//	TDF_TRANSACTION* p3;
		//	TDF_ORDER_QUEUE * p4;
		//	TDF_FUTURE_DATA* p5;
		//	TDF_ORDER* p6;
		//	switch (pMsg->nDataType)
		//	{
		//	case MSG_DATA_MARKET:
		//		p1 = static_cast<TDF_MARKET_DATA*>(pMsg->pData);
		//		delete[] p1;
		//		break;
		//	case MSG_DATA_INDEX:
		//		p2 = static_cast<TDF_INDEX_DATA*>(pMsg->pData);
		//		delete[] p2;
		//		break;
		//	case MSG_DATA_TRANSACTION:
		//		p3 = static_cast<TDF_TRANSACTION*>(pMsg->pData);
		//		delete[] p3;
		//		break;
		//	case MSG_DATA_ORDERQUEUE:
		//		p4 = static_cast<TDF_ORDER_QUEUE*>(pMsg->pData);
		//		delete[] p4;
		//		break;
		//	case MSG_DATA_FUTURE:
		//		p5 = static_cast<TDF_FUTURE_DATA*>(pMsg->pData);
		//		delete[]  p5;
		//		break;
		//	case MSG_DATA_ORDER:
		//		p6 = static_cast<TDF_ORDER*>(pMsg->pData);
		//		delete[] p6;
		//		break;

		//	default:
		//	
		//		break;
		//	}
		//;
		//	
		//}
		void tdf_source::process_market_data(TDF_MARKET_DATA* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->szWindCode);
			feed_item* pItem = get_feed_item(sFeedCode);
			if (pItem == nullptr)
			{
				loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
				return;
			}
			process_msg(pMsg, pItem);
			post(pItem);

		}
		void tdf_source::process_index(TDF_INDEX_DATA* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->szWindCode);
			feed_item* pItem = get_feed_item(sFeedCode);
			if (pItem == nullptr)
			{
				loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
				return;
			}
			//process_msg(pMsg, pItem);
			//post(pItem);
		}
		void tdf_source::process_transaction(TDF_TRANSACTION* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->szWindCode);
			feed_item* pItem = get_feed_item(sFeedCode);
			if (pItem == nullptr)
			{
				loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
				return;
			}
			//process_msg(pMsg, pItem);
			//post(pItem);
		}
		void tdf_source::process_orderqueue(TDF_ORDER_QUEUE* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->szWindCode);
			feed_item* pItem = get_feed_item(sFeedCode);
			if (pItem == nullptr)
			{
				loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
				return;
			}
			//process_msg(pMsg, pItem);
			//post(pItem);
		}
		void tdf_source::process_future(TDF_FUTURE_DATA* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->szWindCode);
			feed_item* pItem = get_feed_item(sFeedCode);
			if (pItem == nullptr)
			{
				loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
				return;
			}
			//process_msg(pMsg, pItem);
			//post(pItem);
		}
		void tdf_source::process_order(TDF_ORDER* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->szWindCode);
			feed_item* pItem = get_feed_item(sFeedCode);
			if (pItem == nullptr)
			{
				loggerv2::info("tdf_item: receive un-subscribed instrument %s", sFeedCode.c_str());
				return;
			}
			//process_msg(pMsg, pItem);
			//post(pItem);
		}

		void tdf_source::process_msg(TDF_MARKET_DATA* pMsg, feed_item* pItem)
		{
			//boost::lock_guard<boost::mutex> lock(m_mutex);
			//m_updateList.clear();
			for (int i = 0; i < TDF_FEED_MAX_DEPTH; i++)
			{
				/*if (i <= 4)
				{*/
				process_depth(i, pMsg->nBidVol[i], pMsg->nBidPrice[i] / m_factor, pMsg->nAskPrice[i] / m_factor, pMsg->nAskVol[i], pItem);
				/*}
				else
				{
				process_depth2(i, pMsg->nBidVol[i], pMsg->nBidPrice[i] / m_factor, pMsg->nAskPrice[i] / m_factor, pMsg->nAskVol[i]);
				}*/
			}
			if (pMsg->nPreClose > 0)
			{
				double closePrice = pMsg->nPreClose / m_factor;
				pItem->set_close_price(closePrice);
			}
			//if (m_closePrc != closePrice)
			//{
			//	m_closePrc = closePrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_HST_CLOSE, closePrice);
			//	m_updateList.set_value(RT_HST_CLOSE, pFeedItemValue);
			//}
			if (pMsg->nMatch > 0)
			{
				double lastPrice = pMsg->nMatch / m_factor;
				pItem->set_last_price(lastPrice);
			}
			
			//if (m_lastPrc != lastPrice)
			//{
			//	m_lastPrc = lastPrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
			//	m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);
			//	if (m_closePrc != 0)
			//	{
			//		double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
			//		if (m_percentChange != percentChange)
			//		{
			//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
			//			m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
			//		}
			//	}
			//}
			if (pMsg->nOpen > 0)
			{
				double openPrice = pMsg->nOpen / m_factor;
				pItem->set_theoretical_open_price(openPrice);
			}
			//if (m_openPrc != openPrice)
			//{
			//	m_openPrc = openPrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_THEO_OPENPRC, openPrice);
			//	m_updateList.set_value(RT_THEO_OPENPRC, pFeedItemValue);
			//}

			double upperLimit = pMsg->nHighLimited / m_factor;
			pItem->set_upper_limit(upperLimit);
			//if (m_upperLimit != upperLimit)
			//{
			//	m_upperLimit = upperLimit;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_UPPER_LMT_PRC, upperLimit);
			//	m_updateList.set_value(RT_UPPER_LMT_PRC, pFeedItemValue);
			//}

			double lowerLimit = pMsg->nLowLimited / m_factor;
			pItem->set_lower_limit(lowerLimit);
			//if (m_lowerLimit != lowerLimit)
			//{
			//	m_lowerLimit = lowerLimit;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_LOWER_LMT_PRC, lowerLimit);
			//	m_updateList.set_value(RT_LOWER_LMT_PRC, pFeedItemValue);
			//}

			//?? RT_NUM_MOVES ???
			if (pMsg->nNumTrades>0)
			{
				int lastQuantity = pMsg->nNumTrades;
				pItem->set_last_quantity(lastQuantity);
			}
			
			//if (m_lastQty != lastQuantity)
			//{
			//	m_lastQty = lastQuantity;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_VOL_1, lastQuantity);
			//	m_updateList.set_value(RT_TRD_VOL_1, pFeedItemValue);
			//}
			if (pMsg->iVolume > 0)
			{
				int dailyVolume = pMsg->iVolume;
				pItem->set_daily_volume(dailyVolume);
			}
			//if (m_dailyVolume != dailyVolume)
			//{
			//	m_dailyVolume = dailyVolume;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ACVOL_1, dailyVolume);
			//	m_updateList.set_value(RT_ACVOL_1, pFeedItemValue);
			//}

			// send updates to client
			//int n = m_updateList.size();
			//if (m_pItemEventHandler != NULL && n > 0)
			//{
			//	m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
			//}

			if (pItem->market_time != boost::lexical_cast<string>(pMsg->nTime))
				pItem->market_time = boost::lexical_cast<string>(pMsg->nTime);
		}

		void tdf_source::process_msg(TDF_FUTURE_DATA* pMsg, feed_item* pItem)
		{
			for (int i = 0; i < 5; i++)
			{
				process_depth(i, pMsg->nBidVol[i], pMsg->nBidPrice[i] / m_factor, pMsg->nAskPrice[i] / m_factor, pMsg->nAskVol[i], pItem);
			}
			//boost::lock_guard<boost::mutex> lock(m_mutex);
			//m_updateList.clear();

			double closePrice = pMsg->nPreClose / m_factor;
			pItem->set_close_price(closePrice);
			//if (m_closePrc != closePrice)
			//{
			//	m_closePrc = closePrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_HST_CLOSE, closePrice);
			//	m_updateList.set_value(RT_HST_CLOSE, pFeedItemValue);
			//}

			double lastPrice = pMsg->nMatch / m_factor;
			pItem->set_last_price(lastPrice);
			/*if (m_lastPrc != lastPrice)
			{
			m_lastPrc = lastPrice;

			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
			m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);

			if (m_closePrc != 0)
			{
			double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
			if (m_percentChange != percentChange)
			{
			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
			m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
			}
			}
			}*/

			double openPrice = pMsg->nOpen / m_factor;
			pItem->set_theoretical_open_price(openPrice);
			//if (m_openPrc != openPrice)
			//{
			//	m_openPrc = openPrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_THEO_OPENPRC, openPrice);
			//	m_updateList.set_value(RT_THEO_OPENPRC, pFeedItemValue);
			//}

			int dailyVolume = pMsg->iVolume;
			pItem->set_daily_volume(dailyVolume);
			//if (m_dailyVolume != dailyVolume)
			//{
			//	m_dailyVolume = dailyVolume;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ACVOL_1, dailyVolume);
			//	m_updateList.set_value(RT_ACVOL_1, pFeedItemValue);
			//}





			double upperLimit = pMsg->nHighLimited / m_factor;
			pItem->set_upper_limit(upperLimit);
			//if (m_upperLimit != upperLimit)
			//{
			//	m_upperLimit = upperLimit;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_UPPER_LMT_PRC, upperLimit);
			//	m_updateList.set_value(RT_UPPER_LMT_PRC, pFeedItemValue);
			//}

			//double lowerLimit = pMsg->nLowLimited / m_factor;
			pItem->set_lower_limit(upperLimit);
			//if (m_lowerLimit != lowerLimit)
			//{
			//	m_lowerLimit = lowerLimit;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_LOWER_LMT_PRC, lowerLimit);
			//	m_updateList.set_value(RT_LOWER_LMT_PRC, pFeedItemValue);
			//}


			// send updates to client
			//int n = m_updateList.size();
			//if (m_pItemEventHandler != NULL && n > 0)
			//{
			//	//logger::info("Calling TDF ITEM feed_item_update_cb");
			//	m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
			//}

			if (pItem->market_time != boost::lexical_cast<string>(pMsg->nTime))
				pItem->market_time = boost::lexical_cast<string>(pMsg->nTime);

		}

		void tdf_source::process_msg(TDF_INDEX_DATA* pMsg, feed_item* pItem)
		{
			//boost::lock_guard<boost::mutex> lock(m_mutex);
			//m_updateList.clear();
			double closePrice = pMsg->nPreCloseIndex / m_factor;
			pItem->set_close_price(closePrice);
			//if (m_closePrc != closePrice)
			//{
			//	m_closePrc = closePrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_HST_CLOSE, closePrice);
			//	m_updateList.set_value(RT_HST_CLOSE, pFeedItemValue);
			//}

			double lastPrice = pMsg->nLastIndex / m_factor;
			pItem->set_last_price(lastPrice);
			//if (m_lastPrc != lastPrice)
			//{
			//	m_lastPrc = lastPrice;

			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
			//	m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);

			//	if (m_closePrc != 0)
			//	{
			//		double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
			//		if (m_percentChange != percentChange)
			//		{
			//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
			//			m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
			//		}
			//	}
			//}

			double openPrice = pMsg->nOpenIndex / m_factor;
			pItem->set_theoretical_open_price(openPrice);
			//if (m_openPrc != openPrice)
			//{
			//	m_openPrc = openPrice;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_THEO_OPENPRC, openPrice);
			//	m_updateList.set_value(RT_THEO_OPENPRC, pFeedItemValue);
			//}

			int dailyVolume = pMsg->iTotalVolume;
			pItem->set_daily_volume(dailyVolume);
			//if (m_dailyVolume != dailyVolume)
			//{
			//	m_dailyVolume = dailyVolume;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_ACVOL_1, dailyVolume);
			//	m_updateList.set_value(RT_ACVOL_1, pFeedItemValue);
			//}



			// send updates to client
			//int n = m_updateList.size();
			//if (m_pItemEventHandler != NULL && n > 0)
			//{
			//	//logger::info("Calling TDF ITEM feed_item_update_cb");
			//	m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
			//}

			if (pItem->market_time != boost::lexical_cast<string>(pMsg->nTime))
				pItem->market_time = boost::lexical_cast<string>(pMsg->nTime);
		}

		void tdf_source::process_msg(TDF_TRANSACTION* pMsg, feed_item* pItem)
		{
			//logger::info("update by trade for %s %f %d", pMsg->szWindCode, pMsg->nPrice/FACTOR, pMsg->nVolume);
			//boost::lock_guard<boost::mutex> lock(m_mutex);
			//m_updateList.clear();
			
			if (pMsg->nPrice > 0)
			{
				double lastPrice = pMsg->nPrice / m_factor;
				pItem->set_last_price(lastPrice);
			}
			//if (m_lastPrc != lastPrice)
			//{
			//	m_lastPrc = lastPrice;

			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_PRC_1, lastPrice);
			//	m_updateList.set_value(RT_TRD_PRC_1, pFeedItemValue);

			//	if (m_closePrc != 0)
			//	{
			//		double percentChange = (lastPrice - m_closePrc) / m_closePrc * 100;
			//		if (m_percentChange != percentChange)
			//		{
			//			shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_PCTCHNG, percentChange);
			//			m_updateList.set_value(RT_PCTCHNG, pFeedItemValue);
			//		}
			//	}
			//}


			if (pMsg->nVolume > 0)
			{
				int lastQuantity = pMsg->nVolume;
				pItem->set_last_quantity(lastQuantity);
			}
			//if (m_lastQty != lastQuantity)
			//{
			//	m_lastQty = lastQuantity;
			//	shared_ptr<feed_item_value> pFeedItemValue = m_valueList.set_value(RT_TRD_VOL_1, lastQuantity);
			//	m_updateList.set_value(RT_TRD_VOL_1, pFeedItemValue);
			//}

			//int n = m_updateList.size();
			//if (m_pItemEventHandler != NULL && n > 0)
			//{
			//	//logger::info("Calling TDF ITEM feed_item_update_cb");
			//	m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
			//}
			if (pItem->market_time != boost::lexical_cast<string>(pMsg->nTime))
				pItem->market_time = boost::lexical_cast<string>(pMsg->nTime);
		}

		void tdf_source::process_msg(TDF_ORDER_QUEUE* pMsg, feed_item* pItem)
		{

			//logger::info("get order items %d", pMsg->nABItems);
			int quantity = 0;
			for (int i = 0; i < pMsg->nABItems; i++)
			{
				quantity += pMsg->nABVolume[i];
			}

			if (pMsg->nSide == 'A')
			{
				//logger::info("update ask size %f %d", pMsg->nPrice/FACTOR, quantity);
				process_depth(0, pItem->market_bid_qty(0), pItem->market_bid(0), pMsg->nPrice / m_factor, quantity, pItem);
			}
			else if (pMsg->nSide == 'B')
			{
				//logger::info("update bid size %f %d", pMsg->nPrice / FACTOR, quantity);
				process_depth(0, quantity, pMsg->nPrice / m_factor, pItem->market_ask(0), pItem->market_ask_qty(0), pItem);
			}

			//int n = m_updateList.size();
			//if (m_pItemEventHandler != NULL && n > 0)
			//{
			//	//logger::info("Calling TDF ITEM feed_item_update_cb");
			//	m_pItemEventHandler->feed_item_update_cb(this, &m_updateList);
			//}

			if (pItem->market_time != boost::lexical_cast<string>(pMsg->nTime))
				pItem->market_time = boost::lexical_cast<string>(pMsg->nTime);
		}

		bool tdf_source::subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || m_pConnection == nullptr)
				return false;
			if (feed_item->is_subsribed() == true)
				return true;
			add_feed_item(feed_item);
			feed_item->subscribe();
			//std::string sFeedCode = m_code2FeedCodeMap[feed_item->get_code()];
			//if (sFeedCode.empty())
			//{
			//	return false;
			//}
			return m_pConnection->subscribe_item(feed_item);
		}

	}

}