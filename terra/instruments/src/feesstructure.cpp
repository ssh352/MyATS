#include "feesstructure.h"
namespace terra
{
	namespace instrument
	{

		feesstructure::feesstructure()
		{
			m_bNotCloseToday = false;
		}


		feesstructure::~feesstructure()
		{
		}
		void  feesstructure::save(boost::property_tree::ptree & pt)
		{
			pt.put("FeesFloatExchange", this->get_fees_float_exchange());
			pt.put("FeesFloatBroker", this->get_fees_float_broker());
			pt.put("FeesFixExchange", this->get_fees_fix_exchange());
			pt.put("FeesFixBroker", this->get_fees_fix_broker());
			pt.put("FeesSellAmount", this->get_fees_sell_amount());
			//
			pt.put("NotCloseToday",this->get_not_close_today());
		}
		void feesstructure::load(boost::property_tree::ptree & pt)
		{
			this->set_fees_float_exchange(pt.get<double>("FeesFloatExchange", 0));
			this->set_fees_float_broker(pt.get<double>("FeesFloatBroker", 0));
			this->set_fees_fix_exchange(pt.get<double>("FeesFixExchange", 0));
			this->set_fees_fix_broker(pt.get<double>("FeesFixBroker", 0));
			this->set_fees_sell_amount(pt.get<double>("FeesSellAmount", 0));
			//
			this->set_not_close_today(pt.get<bool>("NotCloseToday", 0));
		}
	}
}
