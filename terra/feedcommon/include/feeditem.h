#ifndef _V2_FEED_ITEM_H_
#define _V2_FEED_ITEM_H_
#include "feedcommon.h"
#include "feedenum.h"
#include "feedlevel.h"
#include "feedevent.h"
#include "defaultdatetimepublisher.h"
#include <AtsType_types.h>
using namespace terra::common;
namespace terra
{
	namespace feedcommon
	{
		class feed_level;
		//class feed_level_vector : public vector<feed_level*>
		//{
		//public:
		//	feed_level_vector(){}
		//	~feed_level_vector()
		//	{				
		//		for (auto & it : *this)
		//		{					
		//			delete it;					
		//		}
		//	}
		//};
		typedef std::array<feed_level, feed_limits::MAXDEPTH> feed_level_vector;
		class feed_item
		{			
		public:
			feed_item(const std::string& strSourceName, std::string& code, string& feed_code, AtsType::InstrType::type instrType);
			virtual ~feed_item();
		protected:
			std::string m_code;
			std::string m_feed_source_name;
			bool m_up;
			//int  m_precision;
			//int  m_sizeMultiplier;
			//int  m_priceMultiplier;
			bool m_CalculatedPerf;		
			list<feed_field> m_listen_fields;
			int    m_max_depth;
			double m_close;
			int    m_daily_volume;
			double m_turnover;
			//ΩÒΩ·À„
			double m_settlement;
			double m_upper_limit;
			double m_lower_limit;
			double m_perf;
			bool   m_implicitPreOpen;
			map_ex<int, string> m_custom_fields;
			feed_field_update_event_args m_feed_field_event_args;
			string m_feedcode;
			AtsType::InstrType::type m_type;
		public:
			bool   Subscribed;
			double Barycenter;
			string market_time;
			ptime last_update_date_time;
			ptime last_trade_date_time;
			int    Moves;
			bool   ImplicitPreopenTh;
			feed_level_vector BidLevels;
			feed_level_vector AskLevels;
			feed_level_vector LastLevels;
			feed_level BaseBidLevel;
			feed_level BaseAskLevel;
			int bid_nb_orders;
			int ask_nb_orders;
			double theoretical_open_price;
			int    theoretical_open_volume;
			//double kpi_feeder_in;
			int m_FQR_time;
			std::list<feed_update_event_handler> handler_feed_update_event;
			std::list<feed_state_change_event_handler> handler_feed_state_change_event;
			std::list<feed_field_update_event_handler> handler_feed_field_update_event;
			string FQR_ID;
		public:
			void copy_from(feed_item & item);
			bool is_up(){ return m_up; }
			void subscribe(){ Subscribed = true; }
			void un_subscribe(){ Subscribed = false; }			
			bool is_bid_ask_active()
			{
				return math2::not_zero(market_bid(0)) && math2::not_zero(market_ask(0));
			}
			bool is_suspended()
			{
				return !is_bid_ask_active() || math2::eq(market_bid(0), market_ask(0));
			}
			bool is_subsribed(){ return Subscribed; }
			string& get_feed_source_name(){ return m_feed_source_name; }
			int    get_max_depth(){ return m_max_depth; }
			string& get_code(){ return m_code; }	
			string& get_feed_code(){ return m_feedcode; }
			AtsType::InstrType::type get_type(){ return m_type; }
			map_ex<int, string> & get_custom_fields(){ return m_custom_fields; }			
			//int get_precision(){ return m_precision; }
			void set_moves(int value)
			{
				if (Moves != value)
				{
					Moves = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::Moves;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			void set_bid_nb_orders(int value)
			{
				if (bid_nb_orders != value)
				{
					bid_nb_orders = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::BidNbOrders;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_bid_nb_orders()
			{
				return bid_nb_orders;
			}
			void set_ask_nb_orders(int value)
			{
				if (ask_nb_orders != value)
				{
					ask_nb_orders = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::AskNbOrders;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_ask_nb_orders()
			{
				return ask_nb_orders;
			}
			void set_FQR_time(int value)
			{
				if (math2::eq(m_FQR_time, value) == false)
				{
					m_FQR_time = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::FQR;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_FQR_time()
			{
				return m_FQR_time;
			}
			void set_theoretical_open_price(double value)
			{
				if (math2::eq(theoretical_open_price, value) == false)
				{
					theoretical_open_price = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::TheoreticalOpenPrice;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_theoretical_open_price()
			{
				return theoretical_open_price;
			}
			void set_theoretical_open_volume(int value)
			{
				if (theoretical_open_volume != value)
				{
					theoretical_open_volume = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::TheoreticalOpenVolume;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_theoretical_open_volume()
			{
				return theoretical_open_volume;
			}
			double get_bid_price()
			{
				return BaseBidLevel.Price;
			}
			void set_bid_price(double value)
			{				
				if (math2::eq(BaseBidLevel.Price, value) == false)
				{
					BaseBidLevel.Price = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::BidPrice;
					on_feed_field_update_event(m_feed_field_event_args);					
				}
			}
			double get_ask_price()
			{
				return BaseAskLevel.Price;
			}
			void set_ask_price(double value)
			{
				if (math2::eq(BaseAskLevel.Price, value) == false)
				{
					BaseAskLevel.Price = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::AskPrice;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_bid_quantity()
			{
				return BaseBidLevel.Qty;
			}
			void set_bid_quantity(int value)
			{
				if (BaseBidLevel.Qty != value)
				{
					BaseBidLevel.Qty = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::BidQuantity;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_ask_quantity()
			{
				return BaseAskLevel.Qty;
			}
			void set_ask_quantity(int value)
			{
				if (BaseAskLevel.Qty != value)
				{
					BaseAskLevel.Qty = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::AskQuantity;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_last_price()
			{
				return LastLevels[0].Price;
			}
			void set_last_price(double value)
			{
				if (math2::eq(LastLevels[0].Price, value) == false)
				{
					LastLevels[0].Price = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::LastPrice;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int get_last_quantity()
			{
				return LastLevels[0].Qty;
			}
			void set_last_quantity(int value)
			{
				if (LastLevels[0].Qty != value)
				{
					LastLevels[0].Qty = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::LastQuantity;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_close_price(){ return m_close; }
			void   set_close_price(double value)
			{
				if (math2::eq(m_close, value) == false)
				{
					m_close = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::Close;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_settlement(){ return m_settlement; }
			void   set_settlement(double value)
			{
				if (math2::eq(m_settlement, value) == false)
				{
					m_settlement = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::Settlement;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_upper_limit(){ return m_upper_limit; }
			void   set_upper_limit(double value)
			{
				if (math2::eq(m_upper_limit, value) == false)
				{
					m_upper_limit = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::UpperLimit;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_lower_limit(){ return m_lower_limit; }
			void set_lower_limit(double value)
			{
				if (math2::eq(m_lower_limit, value) == false)
				{
					m_lower_limit = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::LowerLimit;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double get_perf(){ return m_perf; }
			void   set_perf(double value)
			{
				if (math2::eq(m_perf, value) == false)
				{
					m_perf = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::Perf;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			int    get_daily_volume(){ return m_daily_volume; }			
			void   set_daily_volume(int value)
			{
				if (m_daily_volume != value)
				{
					m_daily_volume = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::DailyVolume;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double    get_turnover(){ return m_turnover; }
			void   set_turnover(double value)
			{
				if (m_turnover != value)
				{
					m_turnover = value;
					m_feed_field_event_args.Depth = 0;
					m_feed_field_event_args.Value = value;
					m_feed_field_event_args.Field = feed_field::Turnover;
					on_feed_field_update_event(m_feed_field_event_args);
				}
			}
			double last_or_close()
			{
				return math2::not_zero(get_last_price()) ? get_last_price() : get_close_price();
			}
			bool has_last_or_a_close()
			{
				return math2::not_zero(last_or_close());
			}
			double mid()
			{
				double mid;
				if (get_bid_price() > 0 && get_ask_price() > 0)
					mid = (get_bid_price() + get_ask_price()) / 2;
				else if (get_bid_price() > 0)
					mid = get_bid_price();
				else if (get_ask_price() > 0)
					mid = get_ask_price();
				else if (get_last_price() > 0)
					mid = get_last_price();
				else
					mid = get_close_price();
				return mid;
			}
			void set_implicit_pre_open(bool bValue)
			{
				m_implicitPreOpen = bValue;
				pre_open();
			}
			bool get_implicit_pre_open(){ return m_implicitPreOpen; }
			void add_to_listen(feed_field feed)
			{
				m_listen_fields.push_back(feed);
			}
			void remove_from_listen(feed_field feed)
			{
				m_listen_fields.remove(feed);
			}
			void clear_listen()
			{
				m_listen_fields.clear();
			}
			void reset();
			void compute_bary_center(double alpha);
			void compute_bary_center();
			double get_bary_center(){ return Barycenter;}
			double market_bid(int depth);
			double market_ask(int depth);			
			int market_bid_qty(int depth);
			int market_ask_qty(int depth);
			void set_market_bid(int depth, double value);
			void set_market_ask(int depth, double value);
			void set_market_bid_qty(int depth, int value);
			void set_market_ask_qty(int depth, int value);
			void on_feed_update()
			{
				if (!handler_feed_update_event.empty())
				{
					for (auto &it : handler_feed_update_event)
						it(this);
				}
					//handler_feed_update_event(this);
			}
			void on_feed_field_update_event(feed_field_update_event_args & e)
			{			
				if (!handler_feed_field_update_event.empty())
				{
					for (auto &it : handler_feed_field_update_event)
						it(this,e);
				}
					//handler_feed_field_update_event(this,e);
			}
			void on_feed_state_change_event(feed_state_change_event_args & e)
			{				
				if (!handler_feed_state_change_event.empty())
				{
					for (auto &it : handler_feed_state_change_event)
						it(this, e);
				}
					//handler_feed_state_change_event(this, e);
			}
			void on_feed_item_update_event()
			{
				last_update_date_time = date_time_publisher_gh::get_instance()->now();
				if (m_implicitPreOpen)
					pre_open();
				on_feed_update();
			}


			bool check_price_range(double current)
			{
				if (math2::is_zero(get_lower_limit()))
				{
					return true;
				}
				if (math2::is_zero(get_upper_limit()))
				{
					return true;
				}
				return (current - get_lower_limit() >= 0) && (current - get_upper_limit() <= 0);

			}
			//string get_subject(){ return m_code; }
			virtual void pre_open();
			void dump();
			virtual void publish(){}
		};
	}
}
#endif
