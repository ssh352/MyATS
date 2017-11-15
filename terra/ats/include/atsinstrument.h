#ifndef _ATS_INSTRUMENT_V2_H_
#define _ATS_INSTRUMENT_V2_H_
#include "terra_safe_map.h"
#include "feedcommon.h"
#include "financialinstrument.h"
#include "feeditem.h"
#include "position.h"
#include "AtsType_types.h"
#include "tradeitem.h"
#include "iorderobserver.h"
#include "order.h"
#include "exec.h"

using namespace terra::instrument;
using namespace terra::feedcommon;
using namespace terra::marketaccess::orderpassing;
using namespace AtsType;
using namespace terra::common;
#pragma once
namespace terra
{
	namespace ats
	{		
		//const int MaxTradingType = 15;
		class AtsTradingType
		{
		public:
			static const int Unkown = 0;
			static const int Manual = 1;
			static const int Hitter = 2;
			static const int Contrib = 3;
		};
		class ats_instrument;
		typedef std::function<void(ats_instrument * instr)> SubscribedEventHandler;
		typedef std::function<void(ats_instrument * instr)> UnSubscribedEventHandler;

		class price_order_container;				
		typedef terra_safe_map<int, order*>  order_map;
		typedef vector<order_map*>  order_map_array;
		class ats_instrument : public iorderobserver,public iquoteobserver
		{
		public:
			ats_instrument(financialinstrument * pInstrument, string portfolioName, std::vector<string> & feedsources, std::vector<string> & connections,int max_trading_type);
			virtual ~ats_instrument(){}
		public:
			double UnitBuyFees;
			double UnitSellFees;
			double SellFees;
			double BuyFees;		
			double TodayPnlLast;
			double YesterdayPnlLast;
			double TodayPnlMid;
			double YesterdayPnlMid;						
			int    size_to_send;
			price_order_container * market_maker_item_bid;
			price_order_container * market_maker_item_ask;
			int max_trading_type;
			string& get_connection_name(){ return m_strConnectionName; }
		protected:
			financialinstrument * m_pInstrument;
			feed_item * m_pFeedItem;
			string     m_strPortfolioName;
			string     m_strFeedSourceName;
			string     m_strConnectionName;
			//int        m_Precision;
			//int        m_SizeMultiplier;
			//int        m_PrizeMultiplier;
			position * m_pPosition;
			tradeitem* m_pTradeItem;
			order_map_array m_order_map_array;			
			///Profit and Loss=¾»ÖµÇúÏß?
			double     m_today_pnl_bary_center;
			double     m_yesterday_phl_bary_center;
			double     m_broker_fees;
			double     m_exchange_fees;
			int        m_subscribed;			
		protected:
			void       on_subscribed();
			void       on_unsubscribed();
		public:
			int        size_sent_buy;
			int        size_sent_sell;
			int        size_exec_buy;
			int        size_exec_sell;
			bool       fix_feed_by_exec;
			std::list<SubscribedEventHandler> subscribed_event_handler;
			std::list<UnSubscribedEventHandler> unsubscribed_event_handler;
		public:			
			//int  get_precision(){ return m_Precision; }
			//int  get_size_multiplier(){ return m_SizeMultiplier; }
			//int  get_prize_multiplier(){ return m_PrizeMultiplier; }
			//void  set_precision(int value){ m_Precision = value; }
			//void  set_size_multiplier(int value){ m_SizeMultiplier = value; }
			//void  set_prize_multiplier(int value){ m_PrizeMultiplier = value; }
			int  get_size_sent_buy(){ return size_sent_buy; }
			int  get_size_sent_sell(){ return size_sent_sell; }			
			int  get_size_exec_buy(){ return size_exec_buy; }
			int  get_size_exec_sell(){ return size_exec_sell; }
			bool get_fix_feed_by_exec(){ return fix_feed_by_exec; }

			price_order_container * get_market_maker_item_bid(){ return market_maker_item_bid; }
			price_order_container * get_market_maker_item_ask(){ return market_maker_item_ask; }
			tradeitem* get_trade_item(){ return m_pTradeItem; }
			financialinstrument * get_instrument(){ return m_pInstrument; }
			position * get_position(){ return m_pPosition; }
			virtual string match_feed_source(financialinstrument * pInstrument, std::vector<string> & feedsources);
			string match_connection(financialinstrument * pInstrument, std::vector<string> & conns);
			double get_nominal();
			double get_fees(){ return m_broker_fees + m_exchange_fees; }
			double get_total_pnl(){ return m_yesterday_phl_bary_center + m_today_pnl_bary_center - get_fees(); }
			double get_yesterday_phl_bary_center(){return m_yesterday_phl_bary_center; }
			double get_today_phl_bary_center(){ return m_today_pnl_bary_center; }
			double get_yesterday_pnl_mid(){ return YesterdayPnlMid; }
			double get_today_pnl_mid(){ return TodayPnlMid; }
			double get_yesterday_pnl_last(){ return YesterdayPnlLast; }
			double get_today_pnl_last(){ return TodayPnlLast; }
			double get_broker_fees(){ return m_broker_fees;}
			double get_exchange_fees(){ return m_exchange_fees; }

			bool create_order(double price, OrderWay::type way, int quantity, iorderobserver * observer, int tradingType, OrderRestriction::type restriction = OrderRestriction::None, OrderOpenClose::type openClose = OrderOpenClose::Undef, OrderPriceMode::type priceMode = OrderPriceMode::Limit);
			bool create_quote(double bidprice, int bidquantity, double askprice, int askquantity, iquoteobserver * observer, int tradingType, OrderOpenClose::type bidopenClose = OrderOpenClose::Undef, OrderOpenClose::type askopenClose = OrderOpenClose::Undef, const string& fqr_id="");
			int get_nb_open_orders();
			int get_manual_orders_qty(OrderWay::type way);

			order_map_array & get_order_map_array(){ return m_order_map_array; }

			virtual void update_instrument(string code){}
			virtual void stop_feed();
			virtual bool start_feed();
			virtual feed_item * get_feed_item(){ return m_pFeedItem; }
			void    update_feed_item(feed_item * item){ m_pFeedItem = item; }
			void update_portfolio(string portfolioName);
			virtual void compute_fees();
			virtual void compute_pnl();
			virtual double compute_trade_fees(double price, OrderWay::type way);

			
			virtual void add_order_cb(order *order);
			virtual void update_order_cb(order *order);
			virtual void inactive_order_cb(order *order);

			virtual void add_exec_cb(exec *exec);

			virtual void clear_orders();
			virtual void clear_execs();

			virtual void add_quote_cb(quote *quote);

			virtual void update_quote_cb(quote *quote);

			virtual void inactive_quote_cb(quote *quote);

			virtual void clear_quotes();

		};
	}
}
#endif //_ATS_INSTRUMENT_V2_H_

