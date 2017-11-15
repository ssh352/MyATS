#include "time_order_container.h"
#include "price_order_container.h"
//#include "orderqueue.h"
namespace terra
{
	namespace ats
	{
		time_order_container::time_order_container(price_order_container * container, double price)
		{
			order_container = container;
			m_price = price;
			m_quantity = 0;
			m_can_send_order = false;
		}


		time_order_container::~time_order_container()
		{
		}
		ats_instrument * time_order_container::get_instrument()
		{
			if (order_container != nullptr)
				return order_container->get_instrument();
			return nullptr;
		}
		void time_order_container::compute_quantity()
		{
			m_quantity = 0;
			int qty = m_quantity;
			auto com_qty_lambda = [&qty](std::map<int, order*>::reference pair)
			{
				if (pair.second->get_last_action() != OrderAction::Cancelled || pair.second->get_status() == OrderStatus::Nack)
				{
					qty += pair.second->get_book_quantity();
				}
			};

			this->for_each(com_qty_lambda);
			m_quantity = qty;

			/*for (auto & v : *this)
			{
			if (v.second->get_last_action() != OrderAction::Cancelled || v.second->get_status() == OrderStatus::Nack)
			{
			m_quantity += v.second->get_book_quantity();
			}
			}*/
		}
		void time_order_container::compute_can_send_order()
		{
			m_can_send_order = true;
			bool mcan = m_can_send_order;
			auto com_can_lambda = [&mcan](const std::map<int, order*>::reference pair)
			{
				if (pair.second->get_status() == OrderStatus::WaitMarket || pair.second->get_status() == OrderStatus::WaitServer)
				{
					mcan = false;
				}
			};
			this->for_each(com_can_lambda);
			m_can_send_order = mcan;
			/*for (auto & v : *this)
			{
			if (v.second->get_status() == OrderStatus::WaitMarket || v.second->get_status() == OrderStatus::WaitServer)
			{
			m_can_send_order = false;
			}
			}	*/
		}

		bool time_order_container::check_orders()
		{
			list<int> to_remove_ids;
			bool anychange = false;
			auto check_order = [&to_remove_ids](const std::map<int, order*>::reference pair)
			{
				if (pair.second->get_status() == OrderStatus::Exec || pair.second->get_status() == OrderStatus::Cancel || pair.second->get_status() == OrderStatus::Reject)
				{
					to_remove_ids.push_back(pair.first);
				}
			};
			this->for_each(check_order);
			if (to_remove_ids.size() > 0)
				anychange = true;
			for (auto& id : to_remove_ids)
			{
				this->erase(id);
			}
			return anychange;
			
			/*for (auto & v : *this)
			{
			if (v.second->get_status() == OrderStatus::WaitMarket || v.second->get_status() == OrderStatus::WaitServer)
			{
			m_can_send_order = false;
			}
			}	*/
		}
		void time_order_container::create(int quantity, OrderOpenClose::type openclose)
		{
			//order_queue::get_instance()->create_order(get_instrument(), m_price, order_container->get_way(), quantity,order_container, AtsTradingType::Contrib);
			get_instrument()->create_order(m_price, order_container->get_way(), quantity, order_container, AtsTradingType::Contrib,OrderRestriction::None,openclose);
		}
		void time_order_container::cancel()
		{
			auto com_cancel_lambda = [](const std::map<int, order*>::reference pair)
			{
				if (pair.second->get_status() == OrderStatus::Ack || pair.second->get_status() == OrderStatus::Nack)
				{
					pair.second->Cancel();
				}
			};
			this->for_each(com_cancel_lambda);
			/*for (auto & v : *this)
			{
			if (v.second->get_status() == OrderStatus::Ack || v.second->get_status() == OrderStatus::Nack)
			{
			v.second->Cancel();
			}
			}*/
			compute_quantity();
		}
		void time_order_container::reduce_qty(int quantity, int residus, bool canModif)
		{
			int toReduce = quantity;
			/*for (auto & v : *this)
			{
			if (toReduce <= residus)
			{
			continue;
			}
			if (v.second->get_book_quantity() <= toReduce + residus || !canModif)
			{
			if (!v.second->Cancel())
			{
			continue;
			}
			toReduce -= v.second->get_book_quantity();
			}
			if (canModif)
			{
			v.second->set_book_quantity(v.second->get_book_quantity() - toReduce);
			v.second->Modify();
			}
			}*/

			auto reduce_qty_lambda = [&toReduce, &residus, &canModif](std::map<int, order*>::reference pair)
			{
				if (toReduce > residus)
				{

					if (pair.second->get_book_quantity() <= toReduce + residus || !canModif)
					{
						if (!pair.second->Cancel())
						{
							;
						}
						else
							toReduce -= pair.second->get_book_quantity();
					}
					if (canModif)
					{
						pair.second->set_book_quantity(pair.second->get_book_quantity() - toReduce);
						pair.second->Modify();
					}
				}
			};
			this->for_each(reduce_qty_lambda);

			compute_quantity();
		}
		void time_order_container::increase_qty(int quantity, OrderOpenClose::type openclose)
		{
			if (quantity > 0)
			{
				create(quantity, openclose);
			}
		}
		void time_order_container::add_order_cb(order* order)
		{
			if (this->find(order->get_id()) == this->end())
			{
				this->insert(order->get_id(), order);
			}
			this->compute_quantity();
			if (order->get_status() == OrderStatus::WaitMarket || order->get_status() == OrderStatus::WaitServer)
				m_can_send_order = false;
			else
			{
				compute_can_send_order();
			}
		}
		void time_order_container::inactive_order_cb(order* order)
		{
			this->erase(order->get_id());
			compute_can_send_order();
			compute_quantity();
		}
		void time_order_container::update_order_cb(order* order)
		{
			if (order->get_status() == OrderStatus::Reject || order->get_status() == OrderStatus::Exec || order->get_status() == OrderStatus::Cancel)
			{
				this->erase(order->get_id());
			}
			if (order->get_status() == OrderStatus::WaitMarket || order->get_status() == OrderStatus::WaitServer)
				m_can_send_order = false;
			else
			{
				compute_can_send_order();
			}
			compute_quantity();
		}
	}
}