#include "atsinstrument.h"
#include "currency.h"
#include "tradeItem_gh.h"
#include "feedsourcefactory.h"
#include "connection.h"
#include "connection_gh.h"
#include "time_order_container.h"
#include "price_order_container.h"
#include <portfolio_container.h>
#include <portfolio_gh.h>
using namespace terra::marketaccess::orderpassing;
namespace terra
{
	namespace ats
	{
		ats_instrument::ats_instrument(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections, int max_trading_type)
		{
			//m_Precision = 2;
			//m_SizeMultiplier = 1;
			//m_PrizeMultiplier = 1;
			this->m_pFeedItem = nullptr;
			this->m_pPosition = nullptr;
			this->m_pTradeItem = nullptr;
			m_pInstrument = pInstrument;
			m_strPortfolioName = portfolioName;
			m_strFeedSourceName = match_feed_source(pInstrument, feedsources);
			m_strConnectionName = match_connection(pInstrument, connections);			
			m_pTradeItem = tradeitem_gh::get_instance().container().get_by_key(m_pInstrument->get_code(), m_strConnectionName);
			if (m_pTradeItem == NULL)
			{
				loggerv2::error("m_pTradeItem is null,code:%s,Name:%s", m_pInstrument->get_code().c_str(), m_strConnectionName.c_str());
			}
			this->m_broker_fees = 0.0;
			this->m_exchange_fees = 0.0;
			m_today_pnl_bary_center = 0.0;
			m_yesterday_phl_bary_center = 0.0;
			m_order_map_array.resize(max_trading_type);
			this->max_trading_type = max_trading_type;
			for (int i = 0; i < max_trading_type; i++)
			{
				m_order_map_array[i] = new order_map();
				size_to_send = 0;
			}
			this->market_maker_item_ask = nullptr;
			this->market_maker_item_bid = nullptr;
			m_subscribed = 0;
			TodayPnlLast = 0;
			YesterdayPnlLast = 0;
			TodayPnlMid = 0;
			YesterdayPnlMid = 0;
		}
		string ats_instrument::match_feed_source(financialinstrument * pInstrument, std::vector<string> & feedsources)
		{
			if (pInstrument != nullptr)
			{				
				for(auto &it:*feed_source_container::get_instance())
				{
					feed_source * pSource = it.second;
					auto feed = std::find(feedsources.begin(), feedsources.end(), pSource->get_name());
					if (feed != feedsources.end() && pInstrument->get_feed_codes().contain_key(pSource->get_type()) == true)
					{
						return pSource->get_name();
					}			
				}
			}
			return "";
		}
		string ats_instrument::match_connection(financialinstrument * pInstrument, std::vector<string> & conns)
		{
			if (pInstrument == nullptr)
				return "";			
			for (auto &it : connection_gh::get_instance().container().get_map())
			{
				connection* conn = it.second;
				auto con = std::find(conns.begin(), conns.end(), conn->getName());
				if ( con != conns.end() && tradeitem_gh::get_instance().container().get_by_key(m_pInstrument->get_code(), conn->getName()))
				{
					return conn->getName();
				}				
			}
			return "";
		}
		bool ats_instrument::start_feed()
		{
			m_subscribed++;
			if (m_subscribed > 1)
			{
				on_subscribed();
				return true;
			}
			if (m_pInstrument == nullptr)
			{
				return false;
			}
			feed_source * pSource = feed_source_container::get_instance()->get_by_key(m_strFeedSourceName);
			if (m_pFeedItem == nullptr)
			{
				if (pSource != nullptr && m_pInstrument != nullptr)
				{
					m_pFeedItem = pSource->create_feed_item(m_pInstrument->get_code(), m_pInstrument->get_type());
					pSource->subscribe(m_pFeedItem);
				}
				else
				{

				}
			}
			else
			{
				if (m_pFeedItem->is_subsribed() == false && pSource != nullptr)
				{
					pSource->subscribe(m_pFeedItem);
					return true;
				}
			}
			return true;
		}
		void ats_instrument::stop_feed()
		{
			m_subscribed--;
			if (m_subscribed == 0 && m_pFeedItem != nullptr)
			{
				feed_source * pSource = feed_source_container::get_instance()->get_by_key(m_strFeedSourceName);
				if ( pSource != nullptr)
				{
					pSource->un_subscribe(m_pFeedItem);
				}
				on_unsubscribed();
			}			
		}
		void ats_instrument::on_subscribed()
		{
			if (subscribed_event_handler.empty() == false)
			{
				for (auto &it : subscribed_event_handler)
					it(this);
				//subscribed_event_handler(this);
			}
		}
		void ats_instrument::on_unsubscribed()
		{
			if (unsubscribed_event_handler.empty() == false)
			{

				for (auto &it : unsubscribed_event_handler)
					it(this);
				//subscribed_event_handler(this);

				//unsubscribed_event_handler(this);
			}
		}
		double ats_instrument::get_nominal()
		{
			if (this->get_feed_item() != nullptr && this->m_pInstrument != nullptr)
			{
				return m_pInstrument->get_point_value() * m_pInstrument->get_to_reference()*get_feed_item()->last_or_close();
			}
			return 0.0;
		}
		void ats_instrument::compute_pnl()
		{
			if (get_feed_item() != nullptr && m_pPosition != nullptr && m_pInstrument != nullptr && get_feed_item()->is_bid_ask_active())
			{
				double barycenter = get_feed_item()->Barycenter;
				m_yesterday_phl_bary_center = (barycenter - m_pPosition->get_yesterday_price()) * m_pPosition->get_yesterday_position() * m_pInstrument->get_point_value()* m_pInstrument->get_to_reference();
				m_today_pnl_bary_center = ((barycenter - m_pPosition->get_today_buy_price()) * m_pPosition->get_today_buy_position() - (barycenter - m_pPosition->get_today_sell_price()) * m_pPosition->get_today_sell_position()) * m_pInstrument->get_point_value() * m_pInstrument->get_to_reference();

				YesterdayPnlMid = (get_feed_item()->mid() - m_pPosition->get_yesterday_price()) * m_pPosition->get_yesterday_position() * m_pInstrument->get_point_value() * m_pInstrument->get_to_reference();
				TodayPnlMid = ((get_feed_item()->mid() - m_pPosition->get_today_buy_price()) * m_pPosition->get_today_buy_position() - (get_feed_item()->mid() - m_pPosition->get_today_sell_price()) * m_pPosition->get_today_sell_position()) * m_pInstrument->get_point_value() * m_pInstrument->get_to_reference();

				double last = get_feed_item()->last_or_close();
				YesterdayPnlLast = (last - m_pPosition->get_yesterday_price()) * m_pPosition->get_yesterday_position() * m_pInstrument->get_point_value()* m_pInstrument->get_to_reference();
				TodayPnlLast = ((last - m_pPosition->get_today_buy_price()) * m_pPosition->get_today_buy_position() - (last - m_pPosition->get_today_sell_price()) * m_pPosition->get_today_sell_position()) * m_pInstrument->get_point_value() * m_pInstrument->get_to_reference();
			}
		}
		void ats_instrument::compute_fees()
		{
			if (m_pPosition != nullptr && get_feed_item() != nullptr)
			{
				m_exchange_fees = (m_pPosition->get_today_buy_nominal() + m_pPosition->get_today_sell_nominal())* m_pInstrument->get_fees_float_exchange() * m_pInstrument->get_point_value();
				m_exchange_fees += (m_pPosition->get_today_buy_position() + m_pPosition->get_today_sell_position())* m_pInstrument->get_fees_fix_exchange();
				m_exchange_fees += m_pPosition->get_today_sell_nominal() * m_pInstrument->get_fees_sell_amount() * m_pInstrument->get_point_value();
				m_exchange_fees = m_exchange_fees * m_pInstrument->get_to_reference();

				m_broker_fees = (m_pPosition->get_today_buy_nominal() + m_pPosition->get_today_sell_nominal())* m_pInstrument->get_fees_float_broker() * m_pInstrument->get_point_value();
				m_broker_fees += (m_pPosition->get_today_buy_position() + m_pPosition->get_today_sell_position())* m_pInstrument->get_fees_fix_broker();
				m_broker_fees = m_broker_fees * m_pInstrument->get_to_reference();

				UnitBuyFees = compute_trade_fees(get_feed_item() == nullptr ? 0 : get_feed_item()->get_ask_price(), OrderWay::Buy);
				BuyFees = UnitBuyFees * m_pInstrument->get_point_value() * m_pInstrument->get_to_reference();
				UnitSellFees = compute_trade_fees(get_feed_item() == nullptr ? 0 : get_feed_item()->get_bid_price(), OrderWay::Sell);
				SellFees = UnitSellFees * m_pInstrument->get_point_value() * m_pInstrument->get_to_reference();
			}
		}
		double ats_instrument::compute_trade_fees(double price, OrderWay::type way)
		{
			if (m_pInstrument != nullptr)
			{
				double fees = price * (m_pInstrument->get_fees_float_exchange() + m_pInstrument->get_fees_float_broker());
				if (way == OrderWay::Sell || way == OrderWay::CoveredSell)
				{
					fees += price * m_pInstrument->get_fees_sell_amount();
					
				}
				fees += (m_pInstrument->get_fees_fix_exchange() + m_pInstrument->get_fees_fix_broker()) / m_pInstrument->get_point_value();
				return fees;
			}
			return 0.0;
		}
		void ats_instrument::update_portfolio(string portfolioName)
		{
			portfolio_gh & gh = portfolio_gh::get_instance();
			portfolio_container & container = gh.container();
			portfolio * folio = container.get_by_name(portfolioName);
			if (folio == nullptr)
			{
				//loggerv2::error("ats_instrument::update_portfolio will implement in the future!");
				auto it = new terra::marketaccess::orderpassing::portfolio(portfolioName.c_str());
				container.add(it);
				folio = it;
			}
			if (m_pTradeItem != nullptr)// && folio != nullptr)
			{
				m_pPosition = folio->get_position(m_pTradeItem);
				m_pPosition->set_yesterday_position_type(YesterdayPositionType::Local);
				m_pPosition->set_use_manual_position(false);
				m_pPosition->set_yesterday_price_type(YesterdayPriceType::Local);
			}
		}

		bool ats_instrument::create_order(double price, OrderWay::type way, int quantity, iorderobserver * observer, int tradingType, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode)
		{
			if (quantity <= 0 || m_pInstrument == nullptr)
				return false;			
			if (m_pTradeItem != nullptr && m_pTradeItem->getConnection() != nullptr)
			{
				order* pOrder = m_pTradeItem->getConnection()->create_order(m_pTradeItem, way, quantity, price);
				pOrder->set_restriction(restriction);
				pOrder->set_trading_type(tradingType);
				if (m_pPosition != nullptr)
					pOrder->set_portfolio(m_pPosition->get_portfolio_name());
				pOrder->add_observer(observer);
				if (openClose != OrderOpenClose::Undef)
				{
					pOrder->set_open_close(openClose);
				}
				pOrder->set_price_mode(priceMode);
				if (pOrder->Create() != 0)
				{
					//if (pOrder->getObserver() != nullptr)
					//{
					//pOrder->getObserver()->add_order_cb(pOrder);
					//}
					return true;
				}
				if (pOrder->getObserver() != nullptr)
				{
					pOrder->getObserver()->inactive_order_cb(pOrder);
				}
				return false;
			}
			else
			{
				if (m_pTradeItem == nullptr)
				{
					if (m_pInstrument != nullptr)
					{
						loggerv2::error("ats_instrument::create_order,fail,reasion:m_pTradeItem is nullptr,code:%s!", m_pInstrument->get_code().c_str());
					}
					else
					{
						loggerv2::error("ats_instrument::create_order,fail,reasion:m_pTradeItem and m_pInstrument are nullptr!");
					}
				}
			}
			return false;
		}
		void ats_instrument::add_order_cb(order *order)
		{
			if (order == nullptr)
				return;
			//lock
			//boost_write_lock(m_rwmutex);
			if (this->m_order_map_array[order->get_trading_type()] != nullptr || !this->m_order_map_array[order->get_trading_type()]->contains(order->get_id()))
			{
				this->m_order_map_array[order->get_trading_type()]->insert(order->get_id(), order);

				if (order->get_trading_type() == AtsTradingType::Hitter)
				{
					if (order->get_way() == OrderWay::Buy || order->get_way() == OrderWay::CoveredBuy)
					{
						size_sent_buy += order->get_book_quantity();
					}
					if (order->get_way() == OrderWay::Sell || order->get_way() == OrderWay::CoveredSell)
					{
						size_sent_sell += order->get_book_quantity();
					}
				}
			}
		}
		void ats_instrument::update_order_cb(order *porder)
		{
			if (porder != nullptr && (porder->get_status() == OrderStatus::Reject || porder->get_status() == OrderStatus::Cancel || porder->get_status() == OrderStatus::Exec))
			{
				order_map * ordermap = this->m_order_map_array[porder->get_trading_type()];
				if (ordermap != nullptr)
				{
					//lock					
					//boost_write_lock(m_rwmutex);
					ordermap->erase(porder->get_id());															
				}
			}
		}
		void ats_instrument::inactive_order_cb(order *order)
		{

		}
		void ats_instrument::add_exec_cb(exec *exec)
		{
			if (exec == nullptr)
				return;
			//lock
			//boost_write_lock(m_rwmutex);
			if (exec->getTradingType() == AtsTradingType::Hitter)
			{
				if (exec->getWay() == OrderWay::Buy || exec->getWay() == OrderWay::CoveredBuy)
				{
					size_exec_buy += exec->getQuantity();
				}
				if (exec->getWay() == OrderWay::Sell || exec->getWay() == OrderWay::CoveredSell)
				{
					size_exec_sell += exec->getQuantity();
				}
			}
			if (fix_feed_by_exec == true && get_feed_item() != nullptr)
			{
				int i = 0;
				bool feedFixed = false;				
				if ((exec->getTradingType() == AtsTradingType::Hitter && (exec->getWay() == OrderWay::Buy || exec->getWay() == OrderWay::CoveredBuy)) || (exec->getTradingType() == AtsTradingType::Contrib && (exec->getWay() == OrderWay::Sell || exec->getWay() == OrderWay::CoveredSell)))
				{					
					while (i<get_feed_item()->get_max_depth() && math2::sup(get_feed_item()->get_ask_price(), get_feed_item()->market_ask(i)) && math2::not_zero(get_feed_item()->market_ask(i)))
					{
						i++;
					}
					if (i >= get_feed_item()->get_max_depth() || math2::not_zero(get_feed_item()->market_ask(i)))
						return;


					while (math2::sup_eq(exec->getPrice(), get_feed_item()->get_ask_price()) && !feedFixed)
					{
						if (math2::sup(exec->getPrice(), get_feed_item()->get_ask_price()))
						{							
							get_feed_item()->set_ask_quantity(0);
						}
						else
						{
							if (get_feed_item()->get_ask_quantity() - exec->getQuantity() > 0)
							{
								get_feed_item()->set_ask_quantity(get_feed_item()->get_ask_quantity() - exec->getQuantity());
							}
							else
							{
								get_feed_item()->set_ask_quantity(0);
							}							
							feedFixed = true;
						}


						if (i + 1<get_feed_item()->get_max_depth() && math2::not_zero(get_feed_item()->market_ask(i + 1)) && get_feed_item()->get_ask_quantity() <= 0)
						{							
							get_feed_item()->set_ask_price(get_feed_item()->market_ask(i + 1));
							get_feed_item()->set_ask_quantity(get_feed_item()->market_ask_qty(i + 1));
						}

					}					
				}
				if ((exec->getTradingType() == AtsTradingType::Hitter && (exec->getWay() == OrderWay::Sell || exec->getWay() == OrderWay::CoveredSell)) || (exec->getTradingType() == AtsTradingType::Contrib && (exec->getWay() == OrderWay::Buy || exec->getWay() == OrderWay::CoveredBuy)))
				{
					
					while (i < get_feed_item()->get_max_depth() && math2::inf(get_feed_item()->get_bid_price(), get_feed_item()->market_bid(i)) && math2::not_zero(get_feed_item()->market_bid(i)))
					{
						i++;
					}
					if (i >= get_feed_item()->get_max_depth() || math2::is_zero(get_feed_item()->market_bid(i)))
						return;

					while (math2::inf_eq(exec->getPrice(), get_feed_item()->get_bid_price()) && !feedFixed)
					{
						if (math2::inf(exec->getPrice(), get_feed_item()->get_bid_price()))
						{							
							get_feed_item()->set_bid_quantity(0);
						}
						else
						{							
							if (get_feed_item()->get_bid_quantity() - exec->getQuantity() > 0)
							{
								get_feed_item()->set_bid_quantity(get_feed_item()->get_bid_quantity() - exec->getQuantity());
							}
							else
							{
								get_feed_item()->set_bid_quantity(0);
							}
							feedFixed = true;
						}


						if (i + 1 < get_feed_item()->get_max_depth() && math2::not_zero(get_feed_item()->market_bid(i + 1)) && get_feed_item()->get_bid_quantity() <= 0)
						{
							get_feed_item()->set_bid_price(get_feed_item()->market_bid(i + 1));
							get_feed_item()->set_bid_quantity(get_feed_item()->market_bid_qty(i + 1));
						}
					}					
				}
				get_feed_item()->on_feed_item_update_event();
			}

		}
		void ats_instrument::clear_orders()
		{
			//lock
			//boost_write_lock(m_rwmutex);			
			for(auto &it :m_order_map_array)
			{				
				it->clear();			
			}
		}
		void ats_instrument::clear_execs()
		{

		}
		int ats_instrument::get_nb_open_orders()
		{
			//lock
			//boost_read_lock(m_rwmutex);
			int n = 0;									
			for(auto &it: m_order_map_array)
			{				
				n += it->size();				
			}
			return n;
		}
		int ats_instrument::get_manual_orders_qty(OrderWay::type way)
		{
			int n = 0;		
			order_map * ordermap = this->m_order_map_array[AtsTradingType::Manual];
			if (ordermap != nullptr)
			{
				//lock
				//boost_read_lock(m_rwmutex);				
				for(auto &it: *ordermap)
				{
					order * o = it.second;
					if (o->get_way() == way)
					{
						n += o->get_book_quantity();
					}					
				}
			}
			return n;
		}

		bool ats_instrument::create_quote(double bidprice, int bidquantity, double askprice, int askquantity, iquoteobserver * observer, int tradingType, OrderOpenClose::type bidopenClose /*= OrderOpenClose::Undef*/, OrderOpenClose::type askopenClose /*= OrderOpenClose::Undef*/,const string& fqr_id)
		{
			if (bidquantity <= 0 || askquantity<=0 || m_pInstrument == nullptr)
				return false;
			if (m_pTradeItem != nullptr && m_pTradeItem->getConnection() != nullptr)
			{
				quote* pQuote = m_pTradeItem->getConnection()->create_quote(m_pTradeItem, bidquantity, bidprice, askquantity, askprice,fqr_id);
				
				pQuote->set_trading_type(tradingType);
				if (m_pPosition != nullptr)
					pQuote->set_portfolio(m_pPosition->get_portfolio_name());
				pQuote->add_observer(observer);
				if (bidopenClose != OrderOpenClose::Undef)
				{
					pQuote->get_bid_order()->set_open_close(bidopenClose);
				}

				if (askopenClose != OrderOpenClose::Undef)
				{
					pQuote->get_ask_order()->set_open_close(askopenClose);
				}
				
				bool auto_replace = this->get_instrument()->get_exchange() == "CZCE";
				pQuote->get_ask_order()->set_bypass_crosscheck(auto_replace);
				pQuote->get_bid_order()->set_bypass_crosscheck(auto_replace);
				if (pQuote->Create() != 0)
				{
					//if (pOrder->getObserver() != nullptr)
					//{
					//pOrder->getObserver()->add_order_cb(pOrder);
					//}
					return true;
				}
				if (pQuote->getObserver() != nullptr)
				{
					pQuote->getObserver()->inactive_quote_cb(pQuote);
				}
				return false;
			}
			else
			{
				if (m_pTradeItem == nullptr)
				{
					if (m_pInstrument != nullptr)
					{
						loggerv2::error("ats_instrument::create_quote,fail,reasion:m_pTradeItem is nullptr,code:%s!", m_pInstrument->get_code().c_str());
					}
					else
					{
						loggerv2::error("ats_instrument::create_quote,fail,reasion:m_pTradeItem and m_pInstrument are nullptr!");
					}
				}
			}
			return false;
		}

		void ats_instrument::add_quote_cb(quote *quote)
		{
			cout<<"The method or operation is not implemented.";
		}

		void ats_instrument::update_quote_cb(quote *quote)
		{
			cout << "The method or operation is not implemented.";
		}

		void ats_instrument::inactive_quote_cb(quote *quote)
		{
			cout << "The method or operation is not implemented.";
		}

		void ats_instrument::clear_quotes()
		{
			cout << "The method or operation is not implemented.";
		}


	}
}
