#pragma once
#include "tradeItem_order_book.h"

using namespace AtsType;
using namespace terra::common;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			void tradeItem_order_book::add_order(int orderid, double price, /*int qty, */OrderWay::type ordway)
			{
				boost_write_lock wlock(m_lock);
				switch (ordway)
				{
				case OrderWay::Buy:
				case OrderWay::CoveredBuy:
				{
					auto it = m_bid_levels.find(orderid);
					if (it == m_bid_levels.end())
					{
						//m_bid_levels.insert(std::make_pair(orderid, price_level(price, qty)));
						m_bid_levels.insert(std::make_pair(orderid,price));
					}
					/*else
					{
					it->second.add_qty(qty);
					if (it->second.get_qty() == 0)
					m_bid_levels.erase(orderid);
					}*/
				}
				break;
				case OrderWay::Sell:
				case OrderWay::CoveredSell:
				{
					auto it = m_ask_levels.find(orderid);
					if (it == m_ask_levels.end())
					{
						//m_ask_levels.insert(std::make_pair(orderid, price_level(price, qty)));
						m_ask_levels.insert(std::make_pair(orderid, price));
					}
					/*else
					{
					it->second.add_qty(qty);
					if (it->second.get_qty() == 0)
					m_ask_levels.erase(orderid);
					}*/
				}
				break;

				default:
					break;
				}

			}
			

			bool tradeItem_order_book::cross_check_validate(double price, OrderWay::type ordway, int &outid, double &best)
			{
				bool result;
				switch (ordway)
				{
				case OrderWay::Buy:
				case OrderWay::CoveredBuy:
				{
					if (m_ask_levels.empty())
						return true;

					best = get_best_aks_price(outid);
					result = (price < best);
					return result;
				}

				break;
				case OrderWay::Sell:
				case OrderWay::CoveredSell:
				{
					if (m_bid_levels.empty())
						return true;

					best = get_best_bid_price(outid);
					result = (price > best);
					return result;
				}

				break;

				default:
					break;
				}

				return true;
			}

			double tradeItem_order_book::get_best_bid_price(int &outid)
			{
				boost_read_lock rlock(m_lock);
				if (m_bid_levels.size() == 0)
					return -1 * (1 << 30);

				double max = m_bid_levels.begin()->second;// .get_price();
				outid = m_bid_levels.begin()->first;

				for(auto &it:m_bid_levels)
				{
					//if (it.second.get_price() > max&&it.second.get_qty() > 0)
					//{
					//	max = it.second.get_price();
					//	outid = it.first;
					//}
					if (it.second > max&&it.second > 0)
					{
						max = it.second;
						outid = it.first;
					}
				}
				return max;

			}
			double tradeItem_order_book::get_best_aks_price(int &outid)
			{
				boost_read_lock rlock(m_lock);
				if (m_ask_levels.size() == 0)
					return 1 << 30;

				double min = m_ask_levels.begin()->second;// .get_price();
				outid = m_ask_levels.begin()->first;

				for(auto &it:m_ask_levels)
				{
					//if (it.second.get_price() < min&&it.second.get_qty() > 0)
					//{
					//	min = it.second.get_price();
					//	outid = it.first;
					//}
					if (it.second < min&&it.second > 0)
					{
						min = it.second;
						outid = it.first;
					}
				}
				return min;
			}

			void tradeItem_order_book::remove_order(int id)
			{
				boost_write_lock wlock(m_lock);
				m_ask_levels.erase(id);
				m_bid_levels.erase(id);
			}
		}

	}
}