#include "chief_quote_book.h"

using namespace AtsType;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			bool chief_quote_book::add(quote* pquote)
			{
				if (pquote == NULL)
					return false;

				m_chief_quote_book::accessor wa;
				if (!m_container.find(wa, pquote->get_id()))
				{
					m_container.insert(wa, pquote->get_id());
					wa->second = pquote;
					return true;
				}
				return false;
			}

			bool chief_quote_book::remove(quote* pquote)
			{
				if (pquote == NULL)
					return false;

				m_container.erase(pquote->get_id());
				return true;
			}

			bool chief_quote_book::contains(quote* pquote)
			{
				m_chief_quote_book::const_accessor ra;
				return m_container.find(ra, pquote->get_id());
			}

			void chief_quote_book::clear()
			{
				m_container.clear();
			}

			quote* chief_quote_book::get_by_id(int id)
			{
				m_chief_quote_book::const_accessor ra;
				bool it = m_container.find(ra, id);
				return it == true ? (ra->second) : nullptr;
			}

			void chief_quote_book::get_all_quote(std::list<quote*> &mlist)
			{
				mlist.clear();

				/*for (auto &it : m_container)
				{
					vec[++i] = it.second;
				}*/

				auto cpy_all = [&mlist](m_chief_quote_book::const_accessor &ra)
				{
					mlist.push_back(ra->second);
				};

				m_container.for_each(cpy_all);
			}

			int chief_quote_book::count()
			{
				return m_container.size();
			}

			int chief_quote_book::get_nb_open_quotes()
			{
				int n = 0;
				auto get_all_nb_open = [&n](m_chief_quote_book::const_accessor &ra)
				{
					if (ra->second != nullptr)
					{
						auto status = ra->second->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack || status == OrderStatus::WaitMarket || status == OrderStatus::WaitServer)
							++n;
					}

				};

				m_container.for_each(get_all_nb_open);
				/*for (auto it = m_container.begin(); it != m_container.end(); ++it)
				{
					m_chief_quote_book::const_accessor ra;
					if (m_container.find(ra, it->first))
					{
						if (it->second == NULL)
						{
							continue;
						}

						auto status = it->second->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack || status == OrderStatus::WaitMarket || status == OrderStatus::WaitServer)
							n++;

						ra.release();
					}
				}*/
				return n;
			}
			void chief_quote_book::killall()
			{
				/*for (auto it = m_container.begin(); it != m_container.end(); ++it)
				{
					m_chief_quote_book::const_accessor ra;
					if (m_container.find(ra, it->first))
					{
						auto quote = it->second;
						if (quote == NULL)
						{
							continue;
						}

						auto status = quote->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack)
						{
							quote->Cancel();
						}
						ra.release();
					}
				}*/
				auto killall_lam = [](m_chief_quote_book::const_accessor &ra)
				{
					if (ra->second != nullptr)
					{
						auto status = ra->second->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack)
						{
							ra->second->Cancel();
						}
					}

				};

				m_container.for_each(killall_lam);
			}

			void chief_quote_book::killall(std::string portfolio)
			{
				/*for (auto it = m_container.begin(); it != m_container.end(); ++it)
				{
					m_chief_quote_book::const_accessor ra;
					if (m_container.find(ra, it->first))
					{
						auto quote = it->second;
						if (quote == NULL)
						{
							continue;
						}
						if (quote->get_portfolio() != portfolio)
						{
							continue;
						}

						auto status = quote->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack)
						{
							quote->Cancel();
						}
						ra.release();
					}
				}*/
				auto killall_lam = [&portfolio](m_chief_quote_book::const_accessor &ra)
				{
					auto quote = ra->second;
					if (quote != nullptr)
					{
						if (quote->get_portfolio() == portfolio)
						{
							auto status = ra->second->get_status();
							if (status == OrderStatus::Ack || status == OrderStatus::Nack)
							{
								quote->Cancel();
							}
						}
					}

				};

				m_container.for_each(killall_lam);

			}

			void chief_quote_book::killall(std::string portfolio, int Type)
			{
				/*for (auto it = m_container.begin(); it != m_container.end(); ++it)
				{
					m_chief_quote_book::const_accessor ra;
					if (m_container.find(ra, it->first))
					{
						auto quote = it->second;
						if (quote == NULL)
						{
							continue;
						}
						if (quote->get_portfolio() != portfolio || quote->get_trading_type() != Type)
						{
							continue;
						}

						auto status = quote->get_status();
						if (status == OrderStatus::Ack || status == OrderStatus::Nack)
						{
							quote->Cancel();
						}
						ra.release();
					}
				}*/
				auto killall_lam = [&portfolio, &Type](m_chief_quote_book::const_accessor &ra)
				{
					auto quote = ra->second;
					if (quote != nullptr)
					{
						if (quote->get_portfolio() == portfolio&&quote->get_trading_type() == Type)
						{
							auto status = ra->second->get_status();
							if (status == OrderStatus::Ack || status == OrderStatus::Nack)
							{
								quote->Cancel();
							}
						}
					}

				};

				m_container.for_each(killall_lam);

			}
		}
	}
}
