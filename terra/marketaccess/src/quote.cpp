#include "quote.h"
#include "order.h"
#include "iorderobserver.h"
#include "connection.h"
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			quote::quote(connection* pConnection)
			{
				m_pConnection = pConnection;
				bid_order = nullptr;
				ask_order = nullptr;
				m_pInstrument = nullptr;

				m_Action = AtsType::OrderAction::Undef;
				m_preAction = AtsType::OrderAction::Undef;
				m_Status = AtsType::OrderStatus::Undef;
				m_preStatus = AtsType::OrderStatus::Undef;
				m_lastReason[0] = 0;
				FQR_ID = "";
				m_nCancel = 0;

				m_observers= nullptr;
				m_nId = 0;
				m_TradingType = 0;
				m_lastTime = get_lwtp_now();
				
			}


			quote::~quote()
			{
			}

			void quote::set_status(OrderStatus::type status)
			{
				m_Status = status;
				auto b_status = OrderStatus::Undef;
				if (bid_order)
				{
					b_status = bid_order->get_status();
				}
				auto a_status = OrderStatus::Undef; 
				if (ask_order)
				{
					a_status = ask_order->get_status();
				}
				bool _b = (b_status == OrderStatus::Exec || b_status == OrderStatus::Cancel || b_status == OrderStatus::Reject);// && status == OrderStatus::WaitServer;
				bool _a = (a_status == OrderStatus::Exec || a_status == OrderStatus::Cancel || b_status == OrderStatus::Reject);// && status == OrderStatus::WaitServer;
				if (!_b && bid_order)
					bid_order->m_Status = status;
				if (!_a && ask_order)
					ask_order->m_Status = status;
			}
			int quote::Create()
			{
				//char szReason[REASON_MAXLENGTH + 1];
				int res = m_pConnection->create(this, m_lastReason);
				if (res == 0)
					loggerv2::error("%s", m_lastReason);
				return res;
			}


			int quote::Cancel()
			{
				//char szReason[REASON_MAXLENGTH + 1];
				int res = m_pConnection->cancel(this, m_lastReason);
				if (res == 0)
					loggerv2::error("%s", m_lastReason);
				return res;
			}


			void quote::add_observer(iquoteobserver* pObserver)
			{
				m_observers = pObserver;
			}

			void quote::rm_observer(iquoteobserver* pObserver)
			{

			}

			void quote::on_update_quote()
			{
				if (getObserver() != nullptr)
					getObserver()->update_quote_cb(this);
			}

			void quote::on_add_quote()
			{
				if (getObserver() != nullptr)
					getObserver()->add_quote_cb(this);
			}

			void quote::on_inactive_quote()
			{
				if (getObserver() != nullptr)
					getObserver()->inactive_quote_cb(this);
			}

			void quote::on_add_exec(exec* e)
			{
				if (getObserver() != nullptr)
					getObserver()->add_exec_cb(e);
			}

			int quote::get_account_num()
			{
				return m_pConnection->getAccountNum();
			}

		}
	}
}