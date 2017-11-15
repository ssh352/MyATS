#ifndef _IA_OPTION_V2_H_
#define _IA_OPTION_V2_H_
#pragma once
#include "multi_feed_ats_instrument.h"
using namespace terra::ats;
namespace simpleats
{
	class ia_option : public multi_feed_ats_instrument
		{
		public:
			ia_option(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string>& connections, int max_trading_type);
			virtual ~ia_option();
		};	
}
#endif //_IA_OPTION_V2_H_

