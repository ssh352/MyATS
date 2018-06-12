#pragma once
#include "order.h"

#include "connection.h"

#include "iorderobserver.h"

#include "terra_logger.h"
#include "exec.h"

using namespace AtsType;

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			order::order(connection* pConnection)
			{
				m_pConnection = pConnection;
				m_pInstrument = NULL;
				m_observers = nullptr;

				m_Action = AtsType::OrderAction::Undef;
				m_preAction = AtsType::OrderAction::Undef;

				m_bActive = false;
				m_nId = 0;
				m_nUser_ordid = 0;

				m_nQuantity = 0;
				m_dPrice = 0;
				m_nBookQuantity = 0;
				m_nExecQuantity = 0;
				m_dExecPrice = 0;

				m_nModif = 0;
				m_nCancel = 0;

				m_Way = AtsType::OrderWay::Undef;
				m_Status = AtsType::OrderStatus::Undef;
				m_preStatus = AtsType::OrderStatus::Undef;

				m_PriceMode = AtsType::OrderPriceMode::Limit;
				m_Restriction = AtsType::OrderRestriction::None;
				m_TradingType = 0;

				m_OpenClose = AtsType::OrderOpenClose::Undef;
				m_Hedge = Undef;
                
				m_lastReason[0] = ' ';
				m_lastReason[1] = '\0';

				m_previousQuantity = 0;
				m_previousPrice = 0;
				//m_nSpdId = 0;
				m_observers = nullptr;

				m_bingding_quote = nullptr;
				memset(m_log_str, 0, 256);
			}
			int order::get_account_num()
			{
				return m_pConnection->getAccountNum();
			}
			int order::Create()
			{
				//char szReason[REASON_MAXLENGTH + 1];
				int res = m_pConnection->create(this, m_lastReason);
				if (res == 0)
					loggerv2::error("%s", m_lastReason);
				return res;
			}

			int order::Modify()
			{
				//char szReason[REASON_MAXLENGTH + 1];
				int res = m_pConnection->modify(this, m_lastReason);
				if (res == 0)
					loggerv2::error("%s", m_lastReason);
				return res;
			}

			int order::Cancel()
			{
				//char szReason[REASON_MAXLENGTH + 1];
				int res = m_pConnection->cancel(this, m_lastReason);
				if (res == 0)
					loggerv2::error("%s", m_lastReason);
				return res;
			}

			void order::save_previous_values()
			{
				m_preAction = m_Action;
				m_preStatus = m_Status;
				m_previousQuantity = m_nQuantity;
				m_previousPrice = m_dPrice;
			}


			void order::rollback()
			{
				if (m_preStatus == OrderStatus::Undef)
				{
					return;
				}
				m_Action = m_preAction;
				m_Status = m_preStatus;
				if (m_previousQuantity != 0)
				{
					m_nQuantity = m_previousQuantity;
				}
				if (math2::not_zero(m_previousPrice))
				{
					m_dPrice = m_previousPrice;
				}


				m_nBookQuantity = m_nQuantity - m_nExecQuantity;
				update_bindingquote_status();
			}

			void order::add_observer(iorderobserver* pObserver)
			{
				m_observers = pObserver;
			}

			void order::rm_observer(iorderobserver* pObserver)
			{

			}

			void order::on_update_order()
			{
				if (getObserver() != nullptr)
					getObserver()->update_order_cb(this);
				if (m_bingding_quote != nullptr)
				{
					m_bingding_quote->on_update_quote();
				}
			}

			void order::on_add_order()
			{
				if (getObserver() != nullptr)
				{
					getObserver()->add_order_cb(this);
				}
				quote *q = get_binding_quote();
				if (q != nullptr)
				{
					q->on_add_quote();
				}
			}

			void order::on_inactive_order()
			{
				if (getObserver() != nullptr)
					getObserver()->inactive_order_cb(this);

				quote *q = get_binding_quote();
				if (q != nullptr)
					q->on_inactive_quote();
			}

			void order::on_add_exec(exec* e)
			{
				if (getObserver() != nullptr)
					getObserver()->add_exec_cb(e);

				quote *q = get_binding_quote();
				if (q != nullptr)
					q->on_add_exec(e);
			}

			void order::dump_info()
			{
				snprintf(m_log_str, 256, "order_DumpInfo: %s ORD (%06d) : %s %d %s at %f ex[%d] open[%s] source[%d] ptf[%s] acc[%d] restrict[%s] status[%s]\n",
					_OrderAction_VALUES_TO_NAMES.at((int)m_Action),
					m_nId,
					_OrderWay_VALUES_TO_NAMES.at((int(m_Way))),
					m_nQuantity,
					m_pInstrument->getCode().c_str(),
					m_dPrice, m_nExecQuantity,
					_OrderOpenClose_VALUES_TO_NAMES.at((int)m_OpenClose),
					m_TradingType, m_strPortfolio.c_str(), get_account_num(), _OrderRestriction_VALUES_TO_NAMES.at((int)m_Restriction), _OrderStatus_VALUES_TO_NAMES.at((int)m_Status));

				BOOST_LOG_TRIVIAL(info) << m_log_str;
			}

			void order::set_status(OrderStatus::type status)
			{
				if(status==m_Status)
					return;
				if(m_preStatus==OrderStatus::Cancel||m_preStatus==OrderStatus::Exec)
					return;
				
				m_preStatus = m_Status;
				m_Status = status;
				update_bindingquote_status();
				
				if(m_Status!=OrderStatus::WaitServer)
					on_update_order();
			}

			void order::update_bindingquote_status()
			{
				quote *q = get_binding_quote();
				if (q != nullptr)
				{
					order *another = (this == q->get_ask_order()) ? q->get_bid_order() : q->get_ask_order();
					if (another != nullptr)
					{
						auto status = another->get_status();
						if (status == get_status())
						{
							q->m_Status = status;
							return;
						}
						switch (get_status())
						{
						case OrderStatus::Cancel:
							if (status == OrderStatus::Exec || status == OrderStatus::Cancel || status == OrderStatus::Reject)
								//q->set_status(OrderStatus::Cancel);
								q->m_Status = OrderStatus::Cancel;
							break;

						case OrderStatus::Exec:
							if (status == OrderStatus::Exec || status == OrderStatus::Reject)
								q->m_Status = OrderStatus::Exec;
							else if (status == OrderStatus::Cancel)
								q->m_Status = OrderStatus::Cancel;
							break;

						case OrderStatus::Reject:
							q->m_Status = OrderStatus::Reject;
							break;

						default:
							break;
						}
					}
					else
					{
						if (q->get_status() == OrderStatus::Undef)
						{
							q->set_status(m_Status);
						}
					}
				}
			}
		}
	}
}

