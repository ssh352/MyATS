//#ifndef __TDF_ITEM_H__
//#define __TDF_ITEM_H__
//
//#include "abstract_feed_item.h"
//#include "abstract_feed_source.h"
//
//#include "TDFAPI.h"
//#include "TDFAPIStruct.h"
//#include <boost/thread.hpp>
//using namespace toolkit::common;
//#define TDF_FEED_MAX_DEPTH 10
//#define FACTOR 10000.0
//
//namespace feed
//{
//	class tdf_item : public abstract_feed_item
//	{
//	public:
//		
//		tdf_item(const char* subject, abstract_feed_source* pAbstractSource);
//		virtual ~tdf_item();
//		void process_msg(TDF_MARKET_DATA* pMsg);
//		void process_msg(TDF_INDEX_DATA* pMsg);
//		void process_msg(TDF_TRANSACTION* pMsg);
//		void process_msg(TDF_ORDER_QUEUE* pMsg);
//		void process_msg(TDF_FUTURE_DATA* pMsg);
//
//
//	protected:
//
//		void process_depth(int i, int bidQty, double bidPrice, double askPrice, int askQuantity);
//		void process_depth2(int i, int bidQty, double bidPrice, double askPrice, int askQuantity);
//
//		int m_nbLimits;
//
//		int m_bidNb[TDF_FEED_MAX_DEPTH];
//		int m_bidQty[TDF_FEED_MAX_DEPTH];
//		double m_bidPrc[TDF_FEED_MAX_DEPTH];
//		double m_askPrc[TDF_FEED_MAX_DEPTH];
//		int m_askQty[TDF_FEED_MAX_DEPTH];
//		int m_askNb[TDF_FEED_MAX_DEPTH];
//
//		int m_lastQty;
//		double m_lastPrc;
//		int m_dailyVolume;
//		double m_percentChange;
//		double m_openPrc;
//		double m_closePrc;
//		double m_upperLimit;
//		double m_lowerLimit;
//
//		boost::mutex m_mutex;
//
//
//	};
//}
//
//#endif //__TDF_ITEM_H__