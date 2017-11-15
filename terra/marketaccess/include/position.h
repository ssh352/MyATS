#ifndef __POSITION2_H__
#define __POSITION2_H__

#include <string>
#include "tradeitem.h"
#include "portfolio.h"
#include "exec.h"
#include "terra_logger.h"
using namespace terra::common;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class position
			{
			public:
				position(tradeitem* i, const char* pszPortfolioName);
				virtual ~position();


				void add_exec(exec* e);
				void remove_exec(exec* e);


				// get/set
				tradeitem* get_instrument() { return m_pInstrument; }
				//portfolio* get_portfolio() { return m_pPortfolio; }
				const char* get_portfolio_name() { return m_portfolioName.c_str(); }

				//int get_persisted_position() { return m_persistedPosition; }
				//void set_persisted_position(int p) { m_persistedPosition = p; }

				int get_yesterday_position() { return m_yesterdayPosition; }
				
				double get_yesterday_price() { return m_yesterdayPrice; }
			

				int get_today_position() { return m_todayPosition; }
				int get_total_position() { return m_totalPosition; }

				int get_buffer_position() { return m_bufferPosition; }
				void set_buffer_position(int d) { m_bufferPosition = d; compute_position(); }

				int get_today_buy_position() { return m_todayBuyPosition; }
				int get_today_sell_position() { return m_todaySellPosition; }

				double get_today_buy_price() { return m_todayBuyPrice; }
				double get_today_sell_price() { return m_todaySellPrice; }
				double get_today_average_price() { return m_todayAveragePrice; }

				double get_today_buy_nominal() { return m_todayBuyNominal; }
				double get_today_sell_nominal() { return m_todaySellNominal; }


				int get_pur_red_OutcomeQty() { return m_purredOutcomeQty; }
				void set_pur_red_OutcomeQty(int qty) { m_purredOutcomeQty = qty; }

				int get_pur_red_IncomeQty() { return m_purredIncomeQty; }
				void set_pur_red_IncomeQty(int qty) { m_purredIncomeQty = qty; }

				double get_yesterday_price_local(){ return m_dYesterdayPriceLocal; }
				void set_yesterday_price_local(double value)
				{
					m_dYesterdayPriceLocal = value;
					update_yesterday_price();
				}

				double get_yesterday_price_external(){ return m_dYesterdayPriceExternal; }
				void set_yesterday_price_external(double value)
				{
					m_dYesterdayPriceExternal = value;
					update_yesterday_price();
				}
				int get_yesterday_position_local(){ return m_dYesterdayPositionLocal; }
				void set_yesterday_position_local(int value)
				{
					m_dYesterdayPositionLocal = value;
					update_yesterday_position();
				}

				int get_yesterday_position_external(){ return m_nYesterdayPositionExternal; }
				void set_yesterday_position_external(int value)
				{
					m_nYesterdayPositionExternal = value;
					update_yesterday_position();
				}

				int get_yesterday_position_manual(){ return m_nYesterdayPositionManual; }
				void set_yesterday_position_manual(int value)
				{
					m_nYesterdayPositionManual = value;
					update_yesterday_position();
				}

				bool get_use_manual_position(){ return m_bUseManualPosition; }
				void set_use_manual_position(bool value)
				{
					m_bUseManualPosition = value;
					update_yesterday_position();
				}
				double m_dYesterdayPriceTemp = 0;

				YesterdayPositionType::type get_yesterday_position_type(){ return m_YstPositionType; }
				void set_yesterday_position_type(YesterdayPositionType::type  value)
				{
					m_YstPositionType = value;
					update_yesterday_position();
				}

				YesterdayPriceType::type get_yesterday_price_type(){ return m_YstPriceType; }
				void set_yesterday_price_type(YesterdayPriceType::type  value)
				{
					m_YstPriceType = value;
					update_yesterday_price();
				}
				int m_nTodayRedPosition = 0;
				int m_TodayPurPosition = 0;
			private:
				double m_dYesterdayPriceLocal = 0;
				double m_dYesterdayPriceExternal = 0;
				int m_dYesterdayPositionLocal = 0;
				int m_nYesterdayPositionExternal = 0;
				int m_nYesterdayPositionManual = 0;
				//int m_iManualPosition;
				bool m_bUseManualPosition;
				YesterdayPositionType::type  m_YstPositionType;

				YesterdayPriceType::type m_YstPriceType;
	
				
				void update_yesterday_price();
				void update_yesterday_position();
				void set_yesterday_position(int p);
				void set_yesterday_price(double p) { m_yesterdayPrice = p; }

			protected:
				void compute_buy_position();
				void compute_sell_position();
				void compute_position();
				void compute_average_price();


			private:
				tradeitem* m_pInstrument;
				std::string m_portfolioName;

				int m_persistedPosition;

				int m_yesterdayPosition;
				double m_yesterdayPrice;

				int m_todayPosition;
				int m_totalPosition;

				int m_bufferPosition;
				int m_todayBuyPosition;
				double m_todayBuyNominal;
				double m_todayBuyPrice;

				int m_todaySellPosition;
				double m_todaySellNominal;
				double m_todaySellPrice;

				double m_todayAveragePrice;


				int m_purredOutcomeQty;
				int m_purredIncomeQty;
				//std::set<std::string> m_referencesSerialized;
			};
		}
	}
}
#endif // __POSITION_H__
