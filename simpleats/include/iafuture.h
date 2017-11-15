#ifndef _IA_FUTURE_V2_H_
#define _IA_FUTURE_V2_H_
#pragma once
#include "multi_feed_ats_instrument.h"
using namespace terra::ats;
namespace simpleats
{	
	class ia_future : public multi_feed_ats_instrument
	{
	public:
		ia_future(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections,int max_trading_type);
		virtual ~ia_future();
	};	
}
#endif //_IA_FUTURE_V2_H_


