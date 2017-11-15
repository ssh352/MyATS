#include "feeditem.h"
#include "feedlevel.h"


namespace terra
{
	namespace feedcommon
	{
		feed_item::feed_item(const std::string& strSourceName, std::string& code, string& feed_code, AtsType::InstrType::type instrType)
		{
			m_feed_source_name = strSourceName;
			m_code = code;
			//m_precision = precision;
			//sizeMultiplier = sizeMultiplier;
			//priceMultiplier = priceMultiplier;

			m_feedcode = feed_code;
			m_type = instrType;
			m_implicitPreOpen = false;

			m_up = false;//		
			m_CalculatedPerf = true;
			reset();
					

			Subscribed = false;
			m_max_depth = feed_limits::MAXDEPTH;
			last_update_date_time = date_time_publisher_gh::get_instance()->now();
			market_time = "";
			FQR_ID = "";
		}

		feed_item::~feed_item()
		{

		}
		void feed_item::reset()
		{
			//BidLevels.resize(feed_limits::MAXDEPTH);
			//AskLevels.resize(feed_limits::MAXDEPTH);
			//LastLevels.resize(feed_limits::MAXDEPTH);
			//for (int i = 0; i < feed_limits::MAXDEPTH; i++)
			//{
			//	BidLevels[i] = new feed_level();
			//	AskLevels[i] = new feed_level();
			//	LastLevels[i] = new feed_level();
			//}
						
			theoretical_open_price = 0.0;
			theoretical_open_volume = 0;
			m_daily_volume = 0;
			Moves = 0;
			m_close = 0;
			m_settlement = 0;
			m_perf = 0;
						
			bid_nb_orders = 0;
			ask_nb_orders = 0;			
			Barycenter = 0.0;
		}
		void feed_item::compute_bary_center(double alpha)
		{
			if (get_bid_quantity() + get_ask_quantity() == 0)
				Barycenter = 0;
			else
			{
				Barycenter = (get_bid_price() * get_ask_quantity() + get_ask_price() * get_bid_quantity()) / (get_bid_quantity() + get_ask_quantity());
				Barycenter = alpha * Barycenter + (1 - alpha) * (get_bid_price() + get_ask_price()) / 2;
			}
		}
		void feed_item::compute_bary_center()
		{
			if (get_bid_quantity() > 0 && get_ask_quantity() > 0 && !is_suspended())
				Barycenter = (get_bid_price() * get_ask_quantity() + get_ask_price() * get_bid_quantity()) / (get_bid_quantity() + get_ask_quantity());
			else if (get_bid_quantity() > 0)
				Barycenter = get_bid_price();
			else if (get_bid_quantity() > 0)
				Barycenter = get_ask_price();
			else
				Barycenter = last_or_close();
		}
		double feed_item::market_bid(int depth)
		{
			return depth < feed_limits::MAXDEPTH ? BidLevels[depth].Price : 0;
		}
		double feed_item::market_ask(int depth)
		{
			return depth < feed_limits::MAXDEPTH ? AskLevels[depth].Price : 0;
		}
		int feed_item::market_bid_qty(int depth)
		{
			return depth < feed_limits::MAXDEPTH ? BidLevels[depth].Qty : 0;
		}
		int feed_item::market_ask_qty(int depth)
		{
			return depth < feed_limits::MAXDEPTH ? AskLevels[depth].Qty : 0;
		}
		void feed_item::set_market_bid(int depth, double value)
		{
			if (depth >= 0 && depth < this->get_max_depth())
			{
				if (math2::eq(BidLevels[depth].Price, value) == false)
				{
					BidLevels[depth].Price = value;
					m_feed_field_event_args.Depth = depth;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::BidPrice;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
		}
		void feed_item::set_market_ask(int depth, double value)
		{
			if (depth >= 0 && depth < this->get_max_depth())
			{
				if (math2::eq(AskLevels[depth].Price, value) == false)
				{
					AskLevels[depth].Price = value;
					m_feed_field_event_args.Depth = depth;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::AskPrice;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
		}
		void feed_item::set_market_bid_qty(int depth, int value)
		{
			if (depth >= 0 && depth < this->get_max_depth())
			{
				if (math2::eq(BidLevels[depth].Qty, value) == false)
				{
					BidLevels[depth].Qty = value;
					m_feed_field_event_args.Depth = depth;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::BidQuantity;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
		}
		void feed_item::set_market_ask_qty(int depth, int value)
		{
			if (depth >= 0 && depth < this->get_max_depth())
			{
				if (math2::eq(AskLevels[depth].Qty, value) == false)
				{
					AskLevels[depth].Qty = value;
					m_feed_field_event_args.Depth = depth;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::AskQuantity;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
		}
		void feed_item::pre_open()
		{
			int i = 0, j = 0;

			BaseBidLevel.Price = BidLevels[0].Price;
			BaseBidLevel.Qty = BidLevels[0].Qty;
			BaseAskLevel.Price = AskLevels[0].Price;
			BaseAskLevel.Qty = AskLevels[0].Qty;
			while (i < feed_limits::MAXDEPTH - 1 && j < feed_limits::MAXDEPTH - 1 && (math2::sup_eq(BaseBidLevel.Price, BaseAskLevel.Price) || math2::eq(BaseBidLevel.Price, 0) || math2::eq(BaseAskLevel.Price, 0)))
			{
				int match = 0;
				if (BaseBidLevel.Qty < BaseAskLevel.Qty)
					match = BaseBidLevel.Qty;
				else
					match = BaseAskLevel.Qty;

				// BidQty == 0 || AskQty == 0 -> we do not apply UseTheo.
				if (match == 0)
					break;

				BaseBidLevel.Qty -= match;
				BaseAskLevel.Qty -= match;

				if (BaseBidLevel.Qty == 0)
				{
					BaseBidLevel.Price = BidLevels[i + 1].Price;
					BaseBidLevel.Qty = BidLevels[i + 1].Qty;
					i++;
				}
				if (BaseAskLevel.Qty == 0)
				{
					BaseAskLevel.Price = AskLevels[i + 1].Price;
					BaseAskLevel.Qty = AskLevels[i + 1].Qty;
					j++;
				}
			}
		}
		void feed_item::dump()
		{
			printf_ex("feed_item::dump enter\n");			
			printf_ex("code:%s\n", this->m_code.c_str());
			printf_ex("source:%s\n", this->m_feed_source_name.c_str());
			printf_ex("subscribe:%d\n", this->Subscribed);
			printf_ex("closeprice:%f\n", this->get_close_price());
			printf_ex("lastprice:%f\n", this->get_last_price());
			printf_ex("bidprice:%f\n", this->market_bid(0));
			printf_ex("askprice:%f\n", this->market_ask(0));
			printf_ex("bidqty:%d\n", this->market_bid_qty(0));
			printf_ex("askqty:%d\n", this->market_ask_qty(0));
			printf_ex("volume:%d\n", this->get_daily_volume());
			printf_ex("Barycenter:%f\n", this->Barycenter);
			printf_ex("feed_item::dump exit\n");
		}	
		void feed_item::copy_from(feed_item & item)
		{
			for (int i = 0; i < feed_limits::MAXDEPTH; i++)
			{
				if (math2::not_zero(item.BidLevels[i].Qty) && math2::not_zero(item.BidLevels[i].Price))
				{
					this->BidLevels[i].Qty   = item.BidLevels[i].Qty;
					this->BidLevels[i].Price = item.BidLevels[i].Price;
				}
				if (math2::not_zero(item.AskLevels[i].Qty) && math2::not_zero(item.AskLevels[i].Price))
				{
					this->AskLevels[i].Qty = item.AskLevels[i].Qty;
					this->AskLevels[i].Price = item.AskLevels[i].Price;
				}
				if (math2::not_zero(item.LastLevels[i].Qty) && math2::not_zero(item.LastLevels[i].Price))
				{
					this->LastLevels[i].Qty = item.LastLevels[i].Qty;
					this->LastLevels[i].Price = item.LastLevels[i].Price;
				}
			}
			if (math2::not_zero(item.BaseBidLevel.Qty) && math2::not_zero(item.BaseBidLevel.Price))
			{
				this->BaseBidLevel.Qty = item.BaseBidLevel.Qty;
				this->BaseBidLevel.Price = item.BaseBidLevel.Price;
			}
			if (math2::not_zero(item.BaseAskLevel.Qty) && math2::not_zero(item.BaseAskLevel.Price))
			{
				this->BaseAskLevel.Qty = item.BaseAskLevel.Qty;
				this->BaseAskLevel.Price = item.BaseAskLevel.Price;
			}
			if (math2::not_zero(item.theoretical_open_price))
			{
				this->theoretical_open_price = item.theoretical_open_price;
			}
			if (math2::not_zero(item.theoretical_open_volume))
			{
				this->theoretical_open_volume = item.theoretical_open_volume;
			}
			if (item.get_daily_volume() !=0 )
			{
				this->m_daily_volume = item.get_daily_volume();
			}
			if (item.Moves!= 0)
			{
				this->Moves = item.Moves;
			}
			if (math2::not_zero(item.get_close_price()))
			{
				this->m_close = item.get_close_price();
			}
			if (math2::not_zero(item.get_settlement()))
			{
				this->m_settlement = item.get_settlement();
			}
			if (math2::not_zero(item.get_perf()))
			{
				this->m_perf = item.get_perf();
			}
			if (math2::not_zero(item.get_ask_nb_orders()))
			{
				this->ask_nb_orders = item.get_ask_nb_orders();
			}
			if (math2::not_zero(item.get_bid_nb_orders()))
			{
				this->bid_nb_orders = item.get_bid_nb_orders();
			}
		}
	}
}
