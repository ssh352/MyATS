#include "price_order_container.h"

namespace terra
{
	namespace ats
	{	
		price_order_container::price_order_container(ats_instrument * instrument, OrderWay::type way)
		{
			m_instrument = instrument;
			m_way = way;
			m_positive = true;
			switch (m_way)
			{
			case OrderWay::Buy:
			case OrderWay::CoveredBuy:
			{
				m_positive = false;
				break;
			}
			case OrderWay::Sell:
			case OrderWay::CoveredSell:
			{
				m_positive = true;
				break;
			}
			default:
				break;
			}
		}

		price_order_container::~price_order_container(){}

		int price_order_container::get_quantity()
		{
			//boost_read_lock lk(m_rwmutex);
			int value = 0;
			auto get_qty_lambda = [&value](std::map<double, time_order_container*>::reference pair)
			{
				value += pair.second->get_quantity();
			};

			if (this->is_positive() == true)
			{
				/*for (auto & v : m_positive_map)
				{
					value += v.second->get_quantity();
				}*/
				m_positive_map.for_each(get_qty_lambda);

			}
			else
			{
				/*for (auto & v : m_negative_map)
				{
					value += v.second->get_quantity();
				}*/
				m_negative_map.for_each(get_qty_lambda);
			}
			return value;
		}

		int price_order_container::get_total_nb_orders()
		{
			//boost_read_lock lk(m_rwmutex);
			int value = 0;
			auto get_size_lambda = [&value](std::map<double, time_order_container*>::reference pair)
			{
				value += pair.second->get_quantity();
			};
			if (this->is_positive() == true)
			{
				/*for (auto & v : m_positive_map)
				{
					value += v.second->size();
				}*/
				m_positive_map.for_each(get_size_lambda);
			}
			else
			{
				/*for (auto & v : m_negative_map)
				{
					value += v.second->size();
				}*/
				m_negative_map.for_each(get_size_lambda);
			}
			return value;
		}

		int price_order_container::get_size()
		{
			//boost_read_lock lk(m_rwmutex);
			
			if (this->is_positive() == true)
			{
				return m_positive_map.size();
			}
			else
			{
				return m_negative_map.size();
			}
			
		}

		double price_order_container::get_best_contrib_price()
		{
			//boost_read_lock lk(m_rwmutex);
			if (this->is_positive() == true)
			{
				if (m_positive_map.size() > 0)
				{					
					auto it = m_positive_map.begin();
					if (it != m_positive_map.end())
					{
						return it->first;
					}					
				}
			}
			else
			{
				if (m_negative_map.size() > 0)
				{
					auto it = m_negative_map.begin();
					if (it != m_negative_map.end())
					{
						return it->first;
					}
				}
			}
			if (m_way == OrderWay::Buy || m_way == OrderWay::CoveredBuy)
				return 0.0;
			return DBL_MAX;
		}

		void price_order_container::create(double price, int quantity, OrderOpenClose::type openclose)
		{
			//boost_write_lock lk(m_rwmutex);
			if (get_by_key(price) != nullptr)
				return;
			time_order_container * container = new time_order_container(this, price);
			if (this->is_positive() == true)
			{								
				m_positive_map.insert(price, container);				
			}
			else
			{						
				m_negative_map.insert(price, container);				
			}	
			loggerv2::info("*** IAContribItem::Create [%s] Create Limit [%f]", get_instrument()->get_instrument()->get_code().c_str(), price);
			container->create(quantity, openclose);
		}

		time_order_container * price_order_container::get_by_key(double price)
		{
			//boost_read_lock(m_rwmutex);
			if (this->is_positive() == true)
			{
				auto it = m_positive_map.find(price);
				if (it != m_positive_map.end())
				{
					return it->second;
				}
			}
			else
			{
				auto it = m_negative_map.find(price);
				if (it != m_negative_map.end())
				{
					return it->second;
				}
			}
			return nullptr;
		}

		void   price_order_container::kill_all()
		{
			//boost_read_lock lk(m_rwmutex);

			auto cancel_lambda = [&](std::map<double, time_order_container*>::reference pair)
			{
				pair.second->cancel();
			};

			if (this->is_positive() == true)
			{
				/*for (auto & v : m_positive_map)
				{
					v.second->cancel();
				}*/
				m_positive_map.for_each(cancel_lambda);
			}
			else
			{
				/*for (auto & v : m_negative_map)
				{
					v.second->cancel();
				}*/
				m_negative_map.for_each(cancel_lambda);
			}			
		}

		void price_order_container::add_order_cb(order * o)
		{
			if (this->m_instrument == nullptr)
				return;
			m_instrument->add_order_cb(o);

			//boost_read_lock lk(m_rwmutex);

			time_order_container * container = this->get_by_key(o->get_price());
			if (container == nullptr)
			{
				container = new time_order_container(this, o->get_price());
				if (this->is_positive() == true)
				{
					m_positive_map.insert(o->get_price(), container);
				}
				else
				{
					m_negative_map.insert(o->get_price(), container);
				}
			}

			container->add_order_cb(o);
		}

		void price_order_container::clear_orders()
		{
			//boost_write_lock lk(m_rwmutex);

			this->m_instrument->clear_orders();

			auto clear_lambda = [&](std::map<double, time_order_container*>::reference pair)
			{
				delete pair.second;
				pair.second = nullptr;
			};

			if (this->is_positive() == true)
			{
				/*for (auto & v : m_positive_map)
				{
					delete v.second;
					v.second = nullptr;
				}*/
				m_positive_map.for_each(clear_lambda);
				m_positive_map.clear();
			}
			else
			{
				//for (auto & v : m_negative_map)
				//{
				//	delete v.second;
				//	v.second = nullptr;
				//}
				m_negative_map.for_each(clear_lambda);
				m_negative_map.clear();
			}
		}

		void price_order_container::get_map(unordered_map_ex<double, time_order_container*> & _map)
		{
			//boost_read_lock lk(m_rwmutex);
			auto cpy_lambda = [&_map](std::map<double, time_order_container*>::reference pair)
			{
				_map.add(pair.first, pair.second);
			};


			if (this->is_positive() == true)
			{
				/*for (auto & v : m_positive_map)
				{
					map.add(v.first, v.second);
				}*/
				m_positive_map.for_each(cpy_lambda);
			}
			else
			{
				/*for (auto & v : m_negative_map)
				{
					map.add(v.first, v.second);
				}*/
				m_negative_map.for_each(cpy_lambda);
			}
		}

		double price_order_container::get_first_price(bool usefair)
		{
			if (m_instrument == nullptr || m_instrument->get_feed_item() == nullptr)
				return 0.0;
			
			if (m_way == OrderWay::Buy || m_way == OrderWay::CoveredBuy)
			{
				if (math2::is_zero(m_instrument->get_feed_item()->get_bid_price()) && usefair)
				{
					return m_instrument->get_instrument()->tick_down(get_fair_price());
				}
				return m_instrument->get_feed_item()->get_bid_price();
				
			}
			if (m_way == OrderWay::Sell || m_way == OrderWay::CoveredSell)
			{
				if (math2::is_zero(m_instrument->get_feed_item()->get_ask_price()) && usefair)
				{
					return  m_instrument->get_instrument()->tick_up(get_fair_price());
				}
				return m_instrument->get_feed_item()->get_ask_price();
			}
			return m_instrument->get_feed_item()->mid();
		}

		double price_order_container::get_first_hedge_price()
		{
			if (m_instrument == nullptr || m_instrument->get_feed_item() == nullptr)
				return 0.0;
			if (m_way == OrderWay::Buy || m_way == OrderWay::CoveredBuy)
			{
				return m_instrument->get_feed_item()->get_ask_price();
			}
			if (m_way == OrderWay::Sell || m_way == OrderWay::CoveredSell)
			{
				return m_instrument->get_feed_item()->get_bid_price();
			}
			return m_instrument->get_feed_item()->mid();
		}

		double price_order_container::get_improvement(double current, double toCompare)
		{
			if (m_way == OrderWay::Buy || m_way == OrderWay::CoveredBuy)
				return current - toCompare;
			return toCompare - current;
		}

		double price_order_container::get_next_price(double currentPrice, int nbTicks)
		{
			if (m_instrument == nullptr || m_instrument->get_instrument() == nullptr)
				return 0.0;
			if (m_way == OrderWay::Buy || m_way == OrderWay::CoveredBuy)
			{
				return m_instrument->get_instrument()->tick_down(currentPrice, nbTicks);
			}
			return m_instrument->get_instrument()->tick_up(currentPrice, nbTicks);
		}

		double price_order_container::get_prev_price(double currentPrice, int nbTicks)
		{
			if (m_instrument == nullptr || m_instrument->get_instrument() == nullptr)
				return 0.0;
			if (m_way == OrderWay::Buy || m_way == OrderWay::CoveredBuy)
			{
				return m_instrument->get_instrument()->tick_up(currentPrice, nbTicks);
			}
			return m_instrument->get_instrument()->tick_down(currentPrice, nbTicks);
		}

		double price_order_container::get_single_trade_pnl(double price)
		{
			return get_improvement(get_fair_price(), price);
		}

		bool price_order_container::cancel(double price)
		{
			//boost_read_lock lk(m_rwmutex);
			time_order_container * container = this->get_by_key(price);
			if (container != nullptr)
			{
				container->cancel();
				return true;
			}
			return false;
		}

		bool price_order_container::cancel(int begin, int end)
		{
			if (begin >= end)
				return true;

			//std::vector<time_order_container*> vec;
			int i = 0;
			//boost_read_lock lk(m_rwmutex);

			auto cancel_lambda = [&i,&begin,&end](std::map<double, time_order_container*>::reference pair)
			{
				if (i >= begin&&i <= end)
					pair.second->cancel();
				if (i > end)
					return;
				++i;
			};

			if (this->is_positive() == true)
			{
				/*vec.resize(m_positive_map.size());
				for (auto &it : m_positive_map)
				{
					vec[++i] = it.second;
				}*/
				m_positive_map.for_each(cancel_lambda);
			}
			else
			{
				/*vec.resize(m_negative_map.size());
				for (auto &it : m_negative_map)
				{
					vec[++i] = it.second;
				}*/
				m_negative_map.for_each(cancel_lambda);
			}
			//m_rwmutex.unlock();
			/*for (int i = begin; i < end; ++i)
			{
				if (i < 0 || i >= (int)vec.size())
					continue;
				vec[i]->cancel();
			}*/
			return true;
		}

		time_order_container* price_order_container::get_val(int i)
		{
			if (i < 0)
				return nullptr;
			//boost_read_lock lk(m_rwmutex);
			if (this->is_positive() == true)
			{
				/*int j = 0;
				for (auto &it : m_positive_map)
				{
				if (j == i)
				return it.second;
				++j;
				}*/
				auto it = m_positive_map.find_by_index(i);
				if (it == m_positive_map.end())
					return nullptr;
				else
					return it->second;
			}
			else
			{
				/*int j = 0;
				for (auto &it : m_negative_map)
				{
				if (j == i)
				return it.second;
				++j;
				}*/
				auto it = m_negative_map.find_by_index(i);
				if (it == m_negative_map.end())
					return nullptr;
				else
					return it->second;
			}
			return nullptr;
		}

		double price_order_container::get_key(int i)
		{
			if (i < 0)
				return -1.0;
			//boost_read_lock lk(m_rwmutex);
			if (this->is_positive() == true)
			{
				/*int j = 0;
				for (auto &it : m_positive_map)
				{
				if (j == i)
				return it.first;
				++j;
				}*/
				auto it = m_positive_map.find_by_index(i);
				if (it == m_positive_map.end())
					return -1.0;
				else
					return it->first;
			}
			else
			{
				/*int j = 0;
				for (auto &it : m_negative_map)
				{
				if (j == i)
				return it.first;
				++j;
				}*/
				auto it = m_negative_map.find_by_index(i);
				if (it == m_negative_map.end())
					return -1.0;
				else
					return it->first;
			}
			return -1.0;
		}

		void price_order_container::update_order_cb(order * o)
		{
			if (this->m_instrument == nullptr)
				return;
			this->m_instrument->update_order_cb(o);

			//boost_read_lock rk(m_rwmutex);
			time_order_container * container = this->get_by_key(o->get_price());
			if (container != nullptr)
			{
				container->update_order_cb(o);
				if (container->size() == 0)
				{
					//rk.unlock();
					//boost_write_lock lk(m_rwmutex);
					if (this->is_positive() == true)
					{
						m_positive_map.erase(o->get_price());
					}
					else
					{
						m_negative_map.erase(o->get_price());
					}
					//lk.unlock();
				}
			}
			else
			{
				if (o->get_status() == OrderStatus::Ack || o->get_status() == OrderStatus::Nack)
				{
					loggerv2::error("PriceOrderContainer::UpdateOrderCB [%s] Limit[%f] not found in Map, order cancelled...", o->get_instrument()->getCode().c_str(), o->get_price());
					o->Cancel();
				}

			}
		}

		void price_order_container::inactive_order_cb(order * o)
		{
			
			if (this->m_instrument == nullptr)
				return;
			this->m_instrument->inactive_order_cb(o);

			//boost_read_lock rk(m_rwmutex);
			time_order_container * container = this->get_by_key(o->get_price());
			if (container != nullptr)
			{
				container->inactive_order_cb(o);
				if (container->size() == 0)
				{
					//rk.unlock();
					//boost_write_lock lk(m_rwmutex);
					if (this->is_positive() == true)
					{
						m_positive_map.erase(o->get_price());
					}
					else
					{
						m_negative_map.erase(o->get_price());
					}
					//lk.unlock();
				}
			}
			else
			{
				if (o->get_status() == OrderStatus::Ack || o->get_status() == OrderStatus::Nack)
				{
					loggerv2::error("PriceOrderContainer::InactiveOrderCB [%s] Limit[%f] not found in Map, order cancelled...", o->get_instrument()->getCode().c_str(), o->get_price());
					o->Cancel();
				}

			}
		}

		void price_order_container::add_exec_cb(exec * e)
		{
			//boost_write_lock lk(m_rwmutex);
			if (this->m_instrument == nullptr)
				return;
			this->m_instrument->add_exec_cb(e);
		}

		void price_order_container::clear_execs()
		{
			this->m_instrument->clear_execs();
		}
	
	}
}