#ifndef __QUOTE_H__
#define __QUOTE_H__
#include "AtsType_types.h"
#include "tradeitem.h"
#include "order.h"
#include <boost/format.hpp>
using namespace AtsType;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class connection;
			class iquoteobserver;
			class exec;
			class quote
			{
			private:
				quote(connection* pConnection);
			public:
				~quote();

				OrderStatus::type get_status() const { return m_Status; }
				void set_status(OrderStatus::type status);

				OrderAction::type get_last_action() const { return m_Action; }
				void set_last_action(OrderAction::type action)
				{
					m_preAction = m_Action;
					m_Action = action;
					if (bid_order != nullptr)
						bid_order->set_last_action(action);
					if (ask_order != nullptr)
						ask_order->set_last_action(action);
				}

				const char *get_lastreason() const { return m_lastReason; }
				void set_lastreason(const char* lastReason) { strncpy(m_lastReason, lastReason, sizeof(m_lastReason)); m_lastReason[sizeof(m_lastReason) - 1] = 0; }
				order* get_bid_order(){ return bid_order; }
				order* get_ask_order(){ return ask_order; }

				void set_bid_order(order* o)
				{
					bid_order = o;
					o->set_binding_quote(this);
				}
				void set_ask_order(order* o)
				{
					ask_order = o;
					o->set_binding_quote(this);
				}

				tradeitem* get_instrument() { return m_pInstrument; }
				void set_instrument(tradeitem *pInstrument) { m_pInstrument = pInstrument; }
				int Create();

				int Cancel();
				int get_id() const { return m_nId; }
				void set_id(int id) { m_nId = id; }

				void add_observer(iquoteobserver* pObserver);
				void rm_observer(iquoteobserver* pObserver);
				iquoteobserver* getObserver();

				int get_account_num();

				const std::string& get_portfolio() const { return m_strPortfolio; }
				void set_portfolio(const std::string& portfolio)
				{
					m_strPortfolio = portfolio;
					if (bid_order != nullptr)
						bid_order->set_portfolio(portfolio);
					if (ask_order != nullptr)
						ask_order->set_portfolio(portfolio);
				}

				void on_update_quote();
				void on_add_quote();
				void on_inactive_quote();
				void on_add_exec(exec* e);

				int get_trading_type() const { return m_TradingType; }
				void set_trading_type(int tradingType)
				{
					m_TradingType = tradingType;
					if (bid_order != nullptr)
						bid_order->set_trading_type(tradingType);
					if (ask_order != nullptr)
						ask_order->set_trading_type(tradingType);


				}
				void set_FQR_ID(const string& fqr_id){ FQR_ID = fqr_id; }
				string& get_FQR_ID(){ return FQR_ID; }

				void rollback()
				{
					if (m_preStatus == OrderStatus::Undef)
					{
						return;
					}
					m_Action = m_preAction;
					m_Status = m_preStatus;
					if (bid_order != nullptr)
						bid_order->rollback();
					if (ask_order != nullptr)
						ask_order->rollback();
				}
				std::string& get_exchange_id()
				{


					return get_instrument()->getInstrument()->get_exchange();

				}

				void set_unknown_quote()
				{
					set_portfolio("UNKNOWN");
					set_trading_type(0);
					if (bid_order != nullptr)
						bid_order->set_unknown_order();
					if (ask_order != nullptr)
						ask_order->set_unknown_order();
				}

				void dump_info()
				{
					std::string str = (boost::format("DumpInfo: QUOTE (%06d) : code:[%s] TradingType[%d] Portfolio[%s] Account[%d] status[%s]")
						% m_nId
						%m_pInstrument->getCode().c_str()
						% m_TradingType
						%m_strPortfolio.c_str()
						%get_account_num()
						% _OrderStatus_VALUES_TO_NAMES.at((int)m_Status)).str();
					loggerv2::info("%s", str.c_str());
					if (bid_order != nullptr)
						bid_order->dump_info();
					if (ask_order != nullptr)
						ask_order->dump_info();
				}
				std::array<int, 3> custome_ints;
				std::array<string, 3> custome_strings;
				const lwtp & get_last_time() const { return m_lastTime; }
				void set_last_time(lwtp &value) { m_lastTime = value; }

			private:
				std::string m_strPortfolio;
				iquoteobserver* m_observers;
				int m_nId;
				order* bid_order;
				order* ask_order;
				connection* m_pConnection;
				friend class connection;
				OrderAction::type m_Action;
				OrderStatus::type m_Status;
				OrderStatus::type m_preStatus;
				OrderAction::type m_preAction;
				char m_lastReason[128 + 1];
				tradeitem* m_pInstrument;
				lwtp m_lastTime;
				int m_TradingType;
				string FQR_ID;
				int m_nCancel;
				friend class order;
			};
			inline iquoteobserver* quote::getObserver()
			{
				return m_observers;
			}
		}

	}
}
#endif 