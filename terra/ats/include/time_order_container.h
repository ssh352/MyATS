#ifndef _TIME_ORDER_CONTAINER_H_
#define _TIME_ORDER_CONTAINER_H_
#pragma once
#include "common.h"
#include "order.h"
#include "atsinstrument.h"
#include "terra_safe_map.h"
using namespace terra::common;
using namespace terra::marketaccess::orderpassing;
namespace terra
{
	namespace ats
	{		
		class price_order_container;
		class time_order_container : public terra_safe_map<int, order*, std::greater<int>>
		{
		public:
			time_order_container(price_order_container * container,double price);
			~time_order_container();
		public:
			ats_instrument * get_instrument();
			int get_quantity(){ return m_quantity; }
			void compute_quantity();
			bool get_can_send_order(){ return m_can_send_order; }
			void compute_can_send_order();
			void create(int quantity, OrderOpenClose::type openclose);
			void cancel();
			void reduce_qty(int quantity, int residus, bool canModif);
			void increase_qty(int quantity, OrderOpenClose::type openclose);
			void add_order_cb(order* order);
			void inactive_order_cb(order* order);
			void update_order_cb(order* order);
			bool check_orders();
		protected:
			double m_price;
			int    m_quantity;
			bool   m_can_send_order;
			price_order_container * order_container;
		};
	}
}
#endif //_TIME_ORDER_CONTAINER_H_

