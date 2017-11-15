#ifndef __ORDER2_H__
#define __ORDER2_H__


#include<string.h>
#include<list>
#include "tradeitem.h"
#include "orderdatadef.h"
#include "AtsType_types.h"
#include <thread>
#include <functional>
#include <array>

#define REASON_MAXLENGTH 128
using namespace AtsType;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class exec;
			class quote;
			class connection;
			class iorderobserver;
			class order
			{
			private:
				order(connection* pConnection);
				void update_bindingquote_status();
			public:


				virtual ~order() {}

				void add_observer(iorderobserver* pObserver);
				void rm_observer(iorderobserver* pObserver);
				iorderobserver* getObserver();

				int Create();
				int Modify();
				int Cancel();

				void dump_info();

				// get / set
				connection* get_connection() { return m_pConnection; }

				tradeitem* get_instrument() { return m_pInstrument; }
				void set_instrument(tradeitem *pInstrument) { m_pInstrument = pInstrument; }
				iorderobserver* get_observers() { return m_observers; }


				// char*
				//const std::string& get_account() const { return m_strAccount; }
				//void set_account(const std::string& account) { m_strAccount = account; }
				int get_account_num();
			
				const std::string& get_portfolio() const { return m_strPortfolio; }
				void set_portfolio(const std::string& portfolio) { m_strPortfolio = portfolio; }


				// active
				bool get_active() const { return m_bActive; }
				void set_active(bool b) { m_bActive = b; }


				// id
				int get_id() const { return m_nId; }
				void set_id(int id) { m_nId = id; }


				// misc
				OrderAction::type get_last_action() const { return m_Action; }
				void set_last_action(OrderAction::type action) { m_preAction = m_Action; m_Action = action; }

				OrderWay::type get_way() const { return m_Way; }
				void set_way(OrderWay::type way) { m_Way = way; }

				OrderPriceMode::type get_price_mode() const { return m_PriceMode; }
				void set_price_mode(OrderPriceMode::type priceMode) { m_PriceMode = priceMode; }

				OrderRestriction::type get_restriction() const { return m_Restriction; }
				void set_restriction(OrderRestriction::type restriction) { m_Restriction = restriction; }

				OrderStatus::type get_status() const { return m_Status; }
				void set_status(OrderStatus::type status);// { m_preStatus = m_Status; m_Status = status; }

				int get_quantity() const { return m_nQuantity; }
				void set_quantity(int quantity) { m_previousQuantity = m_nQuantity; m_nQuantity = quantity; }

				OrderHedge get_hedge() const { return m_Hedge; }
				void set_hedge(OrderHedge hedge) { m_Hedge = hedge; }


				double get_price() const { return m_dPrice; }
				void set_price(double price) { m_previousPrice = m_dPrice; m_dPrice = price; }

				int get_previous_quantity() const { return m_previousQuantity; }

				double get_previous_price() const { return m_previousPrice; }

				void set_previous_quantity(int value){ m_previousQuantity = value; }

				void set_previous_price(double value){ m_previousPrice = value; }

				int get_exec_quantity() const { return m_nExecQuantity; }
				void set_exec_quantity(int execQuantity) { m_nExecQuantity = execQuantity; }

				double get_exec_price() const { return m_dExecPrice; }
				void set_exec_price(double execPrice) { m_dExecPrice = execPrice; }

				int get_book_quantity() const { return m_nBookQuantity; }
				void set_book_quantity(int bookQuantity) { m_nBookQuantity = bookQuantity; }

				int get_nb_modif() const { return m_nModif; }
				void set_nb_modif(int i) { m_nModif = i; }

				int get_nb_cancel() const { return m_nCancel; }
				void set_nb_cancel(int i) { m_nCancel = i; }

				int get_trading_type() const { return m_TradingType; }
				void set_trading_type(int tradingType) { m_TradingType = tradingType; }

				OrderOpenClose::type get_open_close() const { return m_OpenClose; }
				void set_open_close(OrderOpenClose::type openClose) { m_OpenClose = openClose; }

				const lwtp & get_last_time() const { return m_lastTime; }
				void set_last_time(lwtp &value) { m_lastTime = value; }

				const char *get_lastreason() const { return m_lastReason; }
				void set_lastreason(const char* lastReason) { strncpy(m_lastReason, lastReason, sizeof(m_lastReason)); m_lastReason[sizeof(m_lastReason) - 1] = 0; }


				void set_bypass_crosscheck(bool bypass) { m_bBypass_crosscheck = bypass; }
				bool get_bypass_crosscheck() { return m_bBypass_crosscheck; }

			

				void rollback();
				void save_previous_values();

				int get_user_orderid() const { return m_nUser_ordid; }
				void set_user_orderid(int id) { m_nUser_ordid = id; }

			
				void set_unknown_order()
				{
					set_portfolio("UNKNOWN");
					set_trading_type(0);
				}

				void on_update_order();
				void on_add_order();
				void on_inactive_order();
				void on_add_exec(exec* e);
				std::string& get_exchange_id()
				{

					/*terra::instrument::financialinstrument *i = this->get_instrument()->getInstrument();
					if (i != nullptr)
					{*/
					//std::string  ex = i->m_strExchange;
					return get_instrument()->getInstrument()->get_exchange();
					/*}
					else
					{
					return "";
					}*/
				}
				//void set_locId(int value){ m_nlocalID = value; }
				//int get_locId(){ return m_nlocalID; }
				//int get_spdId() { return m_nSpdId; }
				//void set_spdId(int n) { m_nSpdId = n; }
				lwtp& get_rebuild_time() { return m_rebuildTime; }
				void set_rebuild_time(lwtp& v) { m_rebuildTime = v; }

				void set_binding_quote(quote* q){ m_bingding_quote = q; }
				quote* get_binding_quote(){ return m_bingding_quote; }
				std::array<int, 3> custome_ints;
				std::array<string, 3> custome_strings;
			protected:
				connection* m_pConnection;
				tradeitem* m_pInstrument;
				iorderobserver* m_observers;

				int m_nId;
				int m_nUser_ordid = 0;
				bool m_bActive;
				bool m_bBypass_crosscheck = false;

				OrderWay::type m_Way;
				OrderAction::type m_Action;
				OrderAction::type m_preAction;
				OrderPriceMode::type m_PriceMode;
				OrderRestriction::type m_Restriction;
				OrderStatus::type m_Status;
				OrderStatus::type m_preStatus;
				int m_TradingType;
				OrderOpenClose::type m_OpenClose;
				OrderHedge m_Hedge;

				int m_nQuantity;
				int m_previousQuantity;
				double m_dPrice;
				double m_previousPrice;
				int m_nBookQuantity;
				int m_nExecQuantity;
				double m_dExecPrice;
				int m_nModif;
				int m_nCancel;

				//std::string m_strAccount;
				std::string m_strPortfolio;
				char m_lastReason[REASON_MAXLENGTH + 1];
				//std::string m_strLastReason;
				std::string m_strLastTimestamp;

				lwtp m_lastTime;
				lwtp m_rebuildTime;
				quote* m_bingding_quote;
				
				/*飞创期货API专用*/
				//int m_nlocalID;
				//int m_nSpdId; //柜台委托号
				/**/

				//Api Specific field
				char m_log_str[256];
				friend class connection;
				friend class quote;
			};

			

			inline iorderobserver* order::getObserver()
			{
				return m_observers;
			}

		}
	}
}
#endif