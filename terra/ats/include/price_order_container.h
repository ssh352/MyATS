#ifndef _PRICE_ORDER_CONTAINER_H_
#define _PRICE_ORDER_CONTAINER_H_
#pragma once
#include "common.h"
#include "time_order_container.h"
#include "atsinstrument.h"
using namespace terra::common;
namespace terra
{
	namespace ats
	{
		class price_order_container :public iorderobserver
		{
			typedef terra_safe_map<double, time_order_container*>                        positive_map;
			typedef terra_safe_map<double, time_order_container*, std::greater<double>>  negative_map;
		public:
			price_order_container(ats_instrument * instrument, OrderWay::type way);
			~price_order_container();
		public:

			bool is_positive(){ return m_positive; };
			int get_traded_lots(){ return m_traded_lots; }
			bool get_active(){ return m_active; }
			void set_active(bool b){ m_active = b; }
			double get_market_maker_pnl(){ return m_market_maker_pnl; }
			virtual ats_instrument * get_instrument(){ return m_instrument; }
			virtual OrderWay::type get_way(){ return m_way; }
			virtual bool cancel(int index){ return cancel(index, index + 1); }

			int get_total_nb_orders();
			int get_size();

			int get_quantity();
			double get_first_price(bool usefair=false);
			double get_first_hedge_price();
			virtual double get_fair_price() = 0;
			double get_best_contrib_price();
			double get_improvement(double current, double toCompare);
			//bool check_price_range(double current);
			double get_next_price(double currentPrice, int nbTicks = 1);
			double get_prev_price(double currentPrice, int nbTicks = 1);
			double get_single_trade_pnl(double price);
			virtual void   do_contrib(bool autoOn) = 0;
			void  create(double price, int quantity, OrderOpenClose::type openclose);
			bool cancel(double price);
			bool cancel(int begin, int end);
			time_order_container*get_val(int i);
			double get_key(int i);

			void kill_all();
			virtual void add_order_cb(order * o);
			virtual void update_order_cb(order * o);
			virtual void inactive_order_cb(order * o);
			virtual void add_exec_cb(exec * e);
			virtual void clear_orders();
			virtual void clear_execs();
			void get_map(unordered_map_ex<double, time_order_container*> & map);
			
			template<typename Func>
			void for_each(Func func)
			{
				if (this->is_positive() == true)
				{
					m_positive_map.for_each(func);

				}
				else
				{
					m_negative_map.for_each(func);
				}
			}

		protected:
			ats_instrument * m_instrument;
			OrderWay::type   m_way;
			int              m_traded_lots;
			double           m_market_maker_pnl;
			bool             m_active;
			//boost::shared_mutex m_rwmutex;
			positive_map     m_positive_map;
			negative_map     m_negative_map;
			bool             m_positive;
		private:
			time_order_container * get_by_key(double price);
		};


	}
}
#endif


