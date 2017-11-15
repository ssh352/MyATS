#include "chief_order_book.h"

using namespace AtsType;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			bool chief_order_book::add(order* pOrder)
			{
				if (pOrder == NULL)
					return false;

				m_chief_order_book::accessor wa;
				if (!m_container.find(wa, pOrder->get_id()))
				{
					m_container.insert(wa, pOrder->get_id());
					wa->second = pOrder;
					return true;
				}
				return false;
			}

			bool chief_order_book::remove(order* pOrder)
			{
				if (pOrder == NULL)
					return false;

				m_container.erase(pOrder->get_id());
				return true;
			}

			bool chief_order_book::contains(order* pOrder)
			{
				m_chief_order_book::const_accessor ra;
				return m_container.find(ra, pOrder->get_id());
			}

			void chief_order_book::clear()
			{
				m_container.clear();
			}

			order* chief_order_book::get_by_id(int id)
			{
				m_chief_order_book::const_accessor ra;
				bool it = m_container.find(ra, id);
				return it == true ? (ra->second) : nullptr;
			}

			void chief_order_book::get_all_order(std::list<order*> &mlist)
			{
				/*for (auto &it : m_container)
				{
				vec[++i] = it.second;
				}*/
				//m_chief_order_book::const_accessor ra;
				//for (auto it = m_container.begin(); it != m_container.end(); ++it)
				//{
				//	if (m_container.find(ra, it->first))
				//	{
				//		mlist.push_back(it->second);
				//		ra.release();//其实可以不加
				//	}
				//	
				//}
				auto cpy_all = [&mlist](m_chief_order_book::const_accessor &ra)
				{
					mlist.push_back(ra->second);
				};

				m_container.for_each(cpy_all);
			}

			int chief_order_book::count()
			{
				return m_container.size();
			}

			int chief_order_book::get_nb_open_orders()
			{
				int n = 0;
				auto get_all_nb_open = [&n](m_chief_order_book::const_accessor &ra)
				{
					if (ra->second != nullptr)
					{
						auto status = ra->second->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack || status == OrderStatus::WaitMarket || status == OrderStatus::WaitServer)
							++n;
					}

				};

				m_container.for_each(get_all_nb_open);
				//m_chief_order_book::const_accessor ra;
				//for (auto it = m_container.begin(); it != m_container.end();++it)
				//{
				//	if (m_container.find(ra, it->first))
				//	{
				//		if (it->second == NULL)
				//		{
				//			continue;
				//		}

				//		auto status = it->second->get_status();
				//		if (status == OrderStatus::Ack || status == OrderStatus::Nack || status == OrderStatus::WaitMarket || status == OrderStatus::WaitServer)
				//			n++;
				//		ra.release();//其实可以不加
				//	}

				//}
				return n;
			}

			void chief_order_book::killall()
			{
				auto killall_lam = [](m_chief_order_book::const_accessor &ra)
				{
					if (ra->second != nullptr)
					{
						auto status = ra->second->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack)
						{
							if (ra->second->get_binding_quote() == nullptr)
								ra->second->Cancel();
						}
					}

				};

				m_container.for_each(killall_lam);
				//m_chief_order_book::const_accessor ra;
				//for (auto it = m_container.begin(); it != m_container.end(); ++it)
				//{
				//	if (m_container.find(ra, it->first))
				//	{
				//		auto order = it->second;
				//		if (order == NULL)
				//		{
				//			continue;
				//		}

				//		auto status = order->get_status();
				//		if (status == OrderStatus::Ack || status == OrderStatus::Nack)
				//		{
				//			if (order->get_binding_quote() == nullptr)
				//			order->Cancel();
				//		}
				//		ra.release();//其实可以不加
				//	}
				//}
			}

			void chief_order_book::killall(std::string portfolio)
			{
				auto killall_lam = [&portfolio](m_chief_order_book::const_accessor &ra)
				{
					auto order = ra->second;
					if (order != nullptr)
					{
						if (order->get_portfolio() == portfolio)
						{
							auto status = ra->second->get_status();
							if (status == OrderStatus::Ack || status == OrderStatus::Nack)
							{
								if (ra->second->get_binding_quote() == nullptr)
									ra->second->Cancel();
							}
						}
					}

				};

				m_container.for_each(killall_lam);
				//m_chief_order_book::const_accessor ra;
				//for (auto it = m_container.begin(); it != m_container.end(); ++it)
				//{
				//	if (m_container.find(ra, it->first))
				//	{
				//		auto order = it->second;
				//		if (order == NULL)
				//		{
				//			continue;
				//		}
				//		if (order->get_portfolio() != portfolio)
				//		{
				//			continue;
				//		}

				//		auto status = order->get_status();
				//		if (status == OrderStatus::Ack || status == OrderStatus::Nack)
				//		{
				//			order->Cancel();
				//		}
				//		ra.release();//其实可以不加
				//	}
				//}

			}

			void chief_order_book::killall(std::string portfolio, int Type)
			{
				auto killall_lam = [&portfolio,&Type](m_chief_order_book::const_accessor &ra)
				{
					auto order = ra->second;
					if (order != nullptr)
					{
						if (order->get_portfolio() == portfolio&&order->get_trading_type()==Type)
						{
							auto status = ra->second->get_status();
							if (status == OrderStatus::Ack || status == OrderStatus::Nack)
							{
								if (ra->second->get_binding_quote() == nullptr)
									ra->second->Cancel();
							}
						}
					}

				};

				m_container.for_each(killall_lam);
				//	m_chief_order_book::const_accessor ra;

				//	for (auto it = m_container.begin(); it != m_container.end(); ++it)
				//	{
				//		if (m_container.find(ra, it->first))
				//		{
				//			auto order = it->second;
				//			if (order == NULL)
				//			{
				//				continue;
				//			}
				//			if (order->get_portfolio() != portfolio || order->get_trading_type() != Type)
				//			{
				//				continue;
				//			}

				//			auto status = order->get_status();
				//			if (status == OrderStatus::Ack || status == OrderStatus::Nack)
				//			{
				//				order->Cancel();
				//			}
				//			ra.release();//其实可以不加
				//		}
				//	}

			}
		}
	}
}
