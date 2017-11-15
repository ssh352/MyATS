#include "iaoption.h"
namespace simpleats
{	
	ia_option::ia_option(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections, int max_trading_type) :multi_feed_ats_instrument(pInstrument, portfolioName, feedsources, connections, max_trading_type)
	{
	}


	ia_option::~ia_option()
	{

	}	
}
