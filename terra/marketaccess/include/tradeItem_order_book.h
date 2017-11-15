#pragma once
#ifndef __ORDER_BOOK2_H__
#define __ORDER_BOOK2_H__

#include "AtsType_types.h"
#include <map>
#include "common.h"

using namespace AtsType;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			


			//contains two side of the order_book_levels
			class tradeItem_order_book
			{
				typedef std::map<int, double> _price_levels;

			public:
				tradeItem_order_book(){}
				~tradeItem_order_book() {}

				//to implment
				//void add_qty(int orderid, double price, int qty, OrderWay::type ordway);
				//void remove_qty(int orderid, double price, int qty, OrderWay::type ordway);
				void add_order(int orderid, double price, /*int qty, */OrderWay::type ordway);
				void remove_order(int id);

				double get_best_bid_price(int &outid);

				double get_best_aks_price(int &outid);

				/*return true if cross check is good*/
				bool cross_check_validate(double price, OrderWay::type ordway, int &outid, double &bestprice);
				boost::shared_mutex m_lock;

			private:
				_price_levels m_bid_levels;//Î¯Âò
				_price_levels m_ask_levels;//Î¯Âô

			};

		}
	}
}
#endif