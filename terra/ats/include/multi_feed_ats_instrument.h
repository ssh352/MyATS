#include "atsinstrument.h"
#pragma once
namespace terra
{
	namespace ats
	{
		class multi_feed_ats_instrument : public ats_instrument
		{
		public:
			multi_feed_ats_instrument(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections,int max_trading_type);
			virtual ~multi_feed_ats_instrument();
		protected:
			vector<string> m_feed_source_list;
			//feed_item    * m_multi_feed_item;
			unordered_map_ex<string, feed_item*> m_multi_feed_item_map;
		public:
			virtual void stop_feed();
			virtual bool start_feed();
			//virtual feed_item * get_feed_item(){ return m_multi_feed_item; }
			string match_feed_source(financialinstrument * pInstrument, std::vector<string> & feedsources) override;
		protected:	
			
			void feed_item_feed_field_update_event(feed_item * feed_item, feed_field_update_event_args & e);
			void feed_item_update_event(feed_item * feed_item);
		};
	}
}

