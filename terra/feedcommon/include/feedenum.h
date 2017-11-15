#ifndef _V2_FEED_ENUM_H_
#define _V2_FEED_ENUM_H_

namespace terra
{
	namespace feedcommon
	{
		enum feed_field
		{
			Unknown = -1,
			BidPrice,
			BidQuantity,
			AskPrice,
			AskQuantity,
			LastPrice,
			LastQuantity,
			Close,
			DailyVolume,
			Turnover,
			Perf,
			Settlement,
			Moves,
			BidNbOrders,
			AskNbOrders,
			UpperLimit,
			LowerLimit,
			FQR,//KpiFeed,
			TheoreticalOpenPrice,
			TheoreticalOpenVolume,
			CustomInfo
		};
		enum feed_item_status
		{
			UP,
			DOWN,
			STALE,
			REQUEST
		};
		//enum feed_source_type
		//{
		//	LTSL2 = 1,
		//	LTSUDP = 2,
		//	LTS = 3,
		//	TDF = 4,
		//	CTP = 5,//CFFEX
		//	ZMQFeed = 6,
		//	IB = 7,
		//	FEMAS = 8,
		//	XS = 9,
		//	LTSCFFEXUDP = 10,
		//	XS2 = 11,
		//	CFFEXUDP = 12,
		//	FS = 13
		//};
		/*enum feed_source_status
		{
			Up = 0,
			Down = 1,
		};*/

#define NO_PRICE  +1.7976931348623157e+308

		enum feed_limits
		{
			MAXDEPTH = 10,
			FEED_MAX_DEPTH = 5,
			FEED_QUEUE_MAX_LENGTH = 5000
		};

		/*enum instr_type
		{
			INSTR_UNDEF = 0,
			INSTR_INDEX,
			INSTR_STOCK,
			INSTR_CALL,
			INSTR_PUT,
			INSTR_FUTURE,
			INSTR_FUTURESPREAD,
			INSTR_REPO,
			INSTR_BOND,
			INSTR_FUND,
			INSTR_ETF,
			INSTR_MAX
		};*/
	}
}
#endif//_V2_FEED_ENUM_H_
