#include "position.h"
#include "exec_gh.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			position::position(tradeitem* pInstrument, const char* pszPortfolioName)
			{
				m_pInstrument = pInstrument;
				//m_pPortfolio = pPortfolio;
				m_portfolioName = pszPortfolioName;

				m_persistedPosition = 0;

				m_yesterdayPosition = 0;
				m_todayPosition = 0;
				m_totalPosition = 0;
				m_bufferPosition = 0;

				m_yesterdayPrice = 0;

				m_todayBuyPosition = 0;
				m_todayBuyNominal = 0;
				m_todayBuyPrice = 0;

				m_todaySellPosition = 0;
				m_todaySellNominal = 0;
				m_todaySellPrice = 0;

				m_todayAveragePrice = 0;

				m_purredOutcomeQty = 0;
				m_purredIncomeQty = 0;

				m_nYesterdayPositionExternal = 0;
				m_nYesterdayPositionManual = 0;
			}

			position::~position()
			{
			}

			void position::set_yesterday_position(int p)
			{
				if (m_yesterdayPosition == p)
					return;

				m_yesterdayPosition = p;

				compute_position();
			}

			void position::update_yesterday_price()
			{
				switch (get_yesterday_price_type())
				{
				case YesterdayPriceType::Local: 
					set_yesterday_price(get_yesterday_price_local());
					break;
				case YesterdayPriceType::External: 
					set_yesterday_price(get_yesterday_price_external());
					break;
				default: break;
				}
			}

			void position::update_yesterday_position()
			{
				if (get_use_manual_position())
				{
					set_yesterday_position(get_yesterday_position_manual());
				}
				else
				{
					switch (get_yesterday_position_type())
					{
					case YesterdayPositionType::Local: 
						set_yesterday_position(get_yesterday_position_local());
						break;
					case YesterdayPositionType::External: 
						set_yesterday_position(get_yesterday_position_external());
						break;
					default: break;
					}
				}
			}

			void position::compute_buy_position()
			{
				m_todayBuyNominal = m_todayBuyPrice * m_todayBuyPosition;
				compute_position();
			}

			void position::compute_sell_position()
			{
				m_todaySellNominal = m_todaySellPrice * m_todaySellPosition;
				compute_position();
			}

			void position::compute_position()
			{
				compute_average_price();
				//logger::info("position::compute_position 1 tot=%d", m_totalPosition);
				m_todayPosition = m_todayBuyPosition - m_todaySellPosition + m_bufferPosition + m_purredIncomeQty - m_purredOutcomeQty;
				m_totalPosition = m_yesterdayPosition + m_todayPosition;
				//logger::info("position::compute_position 2 tot=%f,m_inc=%d,m_out=%d", m_totalPosition, m_purredIncomeQty, m_purredOutcomeQty);
			}

			void position::compute_average_price()
			{
				double sellMinusBuy = m_todaySellPosition - m_todayBuyPosition;
				if (sellMinusBuy != 0)
					m_todayAveragePrice = ((m_todaySellPosition * m_todaySellPrice - m_todayBuyPosition * m_todayBuyPrice)) / sellMinusBuy;
				else
					m_todayAveragePrice = 0;
			}

			void position::add_exec(exec* e)
			{
				/*if (m_referencesSerialized.find(e->get_reference()) != m_referencesSerialized.end())
				{
				// This deal has been already serialized (either reloaded from file, either already synchronized...)
				// -> We remove the deal persisted and add this one (We consider the last one received to be correct).
				exec* p = deal_gh::instance()->container().get_by_reference(e->get_reference());
				remove_exec(p);
				deal_gh::instance()->container().remove(p);
				}
				else
				{
				m_referencesSerialized.insert(e->get_reference());
				}*/

				//logger::info("position::add_exec instrument %s",e->get_instrument()->get_key());
				if (e->getWay() == OrderWay::Buy || e->getWay() == OrderWay::CoveredBuy)
				{
					double previousTodayBuyPosition = m_todayBuyPosition;
					m_todayBuyPosition += e->getQuantity();
					if (m_todayBuyPosition != 0)
						m_todayBuyPrice = (m_todayBuyPrice * previousTodayBuyPosition + e->getQuantity() * e->getPrice()) / m_todayBuyPosition;
					else
						m_todayBuyPrice = 0;

					compute_buy_position();
				}
				else if (e->getWay() == OrderWay::Sell || e->getWay() == OrderWay::CoveredSell)
				{

					double previousTodaySellPosition = m_todaySellPosition;
					m_todaySellPosition += e->getQuantity();
					if (m_todaySellPosition != 0)
						m_todaySellPrice = (m_todaySellPrice * previousTodaySellPosition + e->getQuantity() * e->getPrice()) / m_todaySellPosition;
					else
						m_todaySellPrice = 0;

					compute_sell_position();
				}
				else if (e->getWay() == OrderWay::ETFPur)
				{
					if (e->getTradeItem()->get_instr_type() == AtsType::InstrType::ETF)
						m_purredIncomeQty += e->getQuantity();
					else if (e->getTradeItem()->get_instr_type() == AtsType::InstrType::Stock)
						m_purredOutcomeQty += e->getQuantity();
					compute_position();

				}
				else if (e->getWay() == OrderWay::ETFPur)
				{

					if (e->getTradeItem()->get_instr_type() == AtsType::InstrType::ETF)
						m_purredOutcomeQty += e->getQuantity();
					else if (e->getTradeItem()->get_instr_type() == AtsType::InstrType::Stock)
						m_purredIncomeQty += e->getQuantity();
					compute_position();
				}


			}

			void position::remove_exec(exec* e)
			{
				if (e->getWay() == OrderWay::Buy)
				{
					double previousTodayBuyPosition = m_todayBuyPosition;
					m_todayBuyPosition -= e->getQuantity();
					if (m_todayBuyPosition != 0)
						m_todayBuyPrice = (m_todayBuyPrice * previousTodayBuyPosition - e->getQuantity() * e->getPrice()) / m_todayBuyPosition;
					else
						m_todayBuyPrice = 0;

					compute_buy_position();
				}
				else
				{
					double previousTodaySellPosition = m_todaySellPosition;
					m_todaySellPosition -= e->getQuantity();
					if (m_todaySellPosition != 0)
						m_todaySellPrice = (m_todaySellPrice * previousTodaySellPosition - e->getQuantity() * e->getPrice()) / m_todaySellPosition;
					else
						m_todaySellPrice = 0;

					compute_sell_position();
				}
			}
		}
	}

}