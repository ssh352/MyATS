#include "multi_feed_ats_instrument.h"
#include "feedsourcefactory.h"
#include "feedcommon.h"
namespace terra
{
	namespace ats
	{
		multi_feed_ats_instrument::multi_feed_ats_instrument(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections, int max_trading_type) :ats_instrument(pInstrument, portfolioName, feedsources, connections, max_trading_type)
		{
			//m_multi_feed_item = nullptr;
			match_feed_source(pInstrument, feedsources);
		}


		multi_feed_ats_instrument::~multi_feed_ats_instrument()
		{
		}

		string multi_feed_ats_instrument::match_feed_source(financialinstrument * pInstrument, std::vector<string> & feedsources)
		{
			if (pInstrument != nullptr)
			{
				for (auto & it : *feed_source_container::get_instance())
				{
					feed_source * pSource = it.second;
					//1.feedsources come from the client test.ini
					//2.m_FeedCodes come from the database
					auto feed = std::find(feedsources.begin(), feedsources.end(), pSource->get_name());
					if (feed != feedsources.end() && pInstrument->get_feed_codes().contain_key(pSource->get_name()) == true)
					{
						m_feed_source_list.push_back(pSource->get_name());
					}
				}
			}
			return "Multi";
		}
		void multi_feed_ats_instrument::stop_feed()
		{
			for (auto &it : m_feed_source_list)
			{
				//1.find the feed source
				feed_source * pSource = feed_source_container::get_instance()->get_by_key(it);
				if (pSource != nullptr && m_pInstrument != nullptr)
				{
					//2.find the feed item
					feed_item* sub_pFeedItem = pSource->get_feed_item(m_pInstrument->get_code());
					if (sub_pFeedItem != nullptr)
					{
						pSource->un_subscribe(sub_pFeedItem);
						this->m_subscribed--;
					}
				}
			}
			this->on_unsubscribed();
		}
		bool multi_feed_ats_instrument::start_feed()
		{
			if (this->m_subscribed >= 1)
			{
				this->on_subscribed();
				return true;
			}
			if (this->m_pInstrument == nullptr)
				return false;

			if (m_feed_source_list.size() > 1)
			{

				if (m_pInstrument != nullptr && m_pFeedItem == nullptr)
				{
					m_pFeedItem = new feed_item("", m_pInstrument->get_code(), m_pInstrument->get_code(), m_pInstrument->get_type());
				}
				for (auto &it : m_feed_source_list)
				{
					//1.find the feed source
					feed_source * pSource = feed_source_container::get_instance()->get_by_key(it);
					if (pSource != nullptr && m_pInstrument != nullptr)
					{
						//2.find the feed item
						feed_item* sub_pFeedItem = pSource->create_feed_item(m_pInstrument->get_code(), m_pInstrument->get_type());
						if (sub_pFeedItem != nullptr)
						{
							//sub_pFeedItem = pSource->create_feed_item(m_pInstrument->to_string(), m_Precision, m_SizeMultiplier, m_PrizeMultiplier);

							if (sub_pFeedItem->is_subsribed() == false)
								pSource->subscribe(sub_pFeedItem);
							this->on_subscribed();
							m_pFeedItem->copy_from(*sub_pFeedItem);

							sub_pFeedItem->handler_feed_field_update_event.push_back(boost::bind(&multi_feed_ats_instrument::feed_item_feed_field_update_event, this, _1, _2));
							sub_pFeedItem->handler_feed_update_event.push_back(boost::bind(&multi_feed_ats_instrument::feed_item_update_event, this, _1));
							sub_pFeedItem->publish();



							m_multi_feed_item_map.add(it, sub_pFeedItem);
							this->m_subscribed++;
						}
						//if (sub_pFeedItem->is_subsribed() == false)
						//	pSource->subscribe(sub_pFeedItem);
					}
				}
				return true;
			}
			else
			{
				feed_source * pSource = feed_source_container::get_instance()->get_by_key(m_strFeedSourceName);
				if (m_pFeedItem == nullptr)
				{
					if (pSource != nullptr && m_pInstrument != nullptr)
					{
						m_pFeedItem = pSource->create_feed_item(m_pInstrument->get_code(), m_pInstrument->get_type());
						pSource->subscribe(m_pFeedItem);
					}
					else
					{

					}
				}
				else
				{
					if (m_pFeedItem->is_subsribed() == false && pSource != nullptr)
					{
						pSource->subscribe(m_pFeedItem);
						return true;
					}
				}
			}
		}

		void multi_feed_ats_instrument::feed_item_update_event(feed_item * feed_item)
		{
			if (feed_item != nullptr&& m_pFeedItem != nullptr && feed_item->get_daily_volume() >= m_pFeedItem->get_daily_volume())
			{
				m_pFeedItem->on_feed_item_update_event();
			}
		}

		void multi_feed_ats_instrument::feed_item_feed_field_update_event(feed_item * feed_item, feed_field_update_event_args & e)
		{
			int depth = e.Depth;
			double value = e.Value;
			if (feed_item->get_daily_volume() < m_pFeedItem->get_daily_volume())
				return;
			if (feed_item != nullptr && m_pFeedItem != nullptr)
			{
				switch (e.Field)
				{
				case feed_field::BidPrice:
				{
					m_pFeedItem->set_market_bid(depth, value);
					if (depth == 0 && m_pFeedItem->get_implicit_pre_open() == false)
					{
						m_pFeedItem->set_bid_price(value);

						m_pFeedItem->set_ask_price(feed_item->get_ask_price());
					}

				}
				break;
				case feed_field::BidQuantity:
				{
					m_pFeedItem->set_market_bid_qty(depth, value);
					if (depth == 0 && m_pFeedItem->get_implicit_pre_open() == false)
					{
						m_pFeedItem->set_bid_quantity(value);
					}
				}
				break;
				case feed_field::AskPrice:
				{
					m_pFeedItem->set_market_ask(depth, value);
					if (depth == 0 && m_pFeedItem->get_implicit_pre_open() == false)
					{
						m_pFeedItem->set_ask_price(value);

						m_pFeedItem->set_bid_price(feed_item->get_bid_price());
					}
				}
				break;
				case feed_field::AskQuantity:
				{
					m_pFeedItem->set_market_ask_qty(depth, value);
					if (depth == 0 && m_pFeedItem->get_implicit_pre_open() == false)
					{
						m_pFeedItem->set_ask_quantity(value);
					}
				}
				break;
				case feed_field::LastPrice:
				{
					m_pFeedItem->set_last_price(value);
				}
				break;
				case feed_field::LastQuantity:
				{
					m_pFeedItem->set_last_quantity(value);
				}
				break;
				case feed_field::Close:
				{
					m_pFeedItem->set_close_price(value);
				}
				break;
				case feed_field::DailyVolume:
				{
					m_pFeedItem->set_daily_volume(value);
				}
				break;
				case feed_field::Turnover:
				{
					m_pFeedItem->set_turnover(value);
				}
				break;
				case feed_field::Perf:
				{
					m_pFeedItem->set_perf(value);
				}
				break;
				case feed_field::Settlement:
				{
					m_pFeedItem->set_settlement(value);
				}
				break;
				case feed_field::Moves:
				{
					m_pFeedItem->set_moves(value);
				}
				break;
				case feed_field::BidNbOrders:
				{
					m_pFeedItem->set_bid_nb_orders(value);
				}
				break;
				case feed_field::AskNbOrders:
				{
					m_pFeedItem->set_ask_nb_orders(value);
				}
				break;
				case feed_field::UpperLimit:
				{

					m_pFeedItem->set_upper_limit(value);
				}
				break;
				case feed_field::LowerLimit:
				{
					m_pFeedItem->set_lower_limit(value);
				}
				break;
				case feed_field::FQR:
				{
					m_pFeedItem->set_FQR_time(value);
				}
				break;
				case feed_field::TheoreticalOpenPrice:
				{
					m_pFeedItem->set_theoretical_open_price(value);
				}
				break;
				case feed_field::TheoreticalOpenVolume:
				{
					m_pFeedItem->set_theoretical_open_volume(value);
				}
				break;
				case feed_field::CustomInfo:
				{

				}
				break;
				default:
					break;
				}
				//printf_ex("multi_feed_ats_instrument::feed_item_feed_field_update_event m_multi_feed_item->dump \n");
				//m_multi_feed_item->dump();
			}
		}
	}
}
