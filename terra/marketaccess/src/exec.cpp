#include "exec.h"
#include "order.h"
#include "terra_logger.h"
#include <boost/format.hpp>

using namespace AtsType;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			exec::exec(order* o, int quantity, double price)
			{
				m_nOrderId = o->get_id();
				m_TradeItem = o->get_instrument();
				m_Way = o->get_way();
				m_TradingType = o->get_trading_type();
				m_strPortfolio = o->get_portfolio();

				m_nQuantity = quantity;
				m_dPrice = price;

				m_isPersisted = false;

				m_OpenClose = o->get_open_close();
				m_Hedge = o->get_hedge();

				//m_strAccount = o->get_account();
				n_account = o->get_account_num();

				
			}

			exec::exec(order* o, const string& pszReference, int quantity, double price, const char* pszExecTime)
			{
				m_nOrderId = o->get_id();
				m_TradeItem = o->get_instrument();
				m_Way = o->get_way();
				m_TradingType = o->get_trading_type();
				m_strPortfolio = o->get_portfolio();

				m_strReference = pszReference;
				switch (m_Way)
				{
				case AtsType::OrderWay::type::Buy:
				{
				m_strReference += "b";
				break;
				}
				case AtsType::OrderWay::type::Sell:
				{
				m_strReference += "s";
				break;
				}
				default:
					break;
				}
				m_nQuantity = quantity;
				m_dPrice = price;
				m_strExecTime = pszExecTime;

				m_OpenClose = o->get_open_close();
				m_Hedge = o->get_hedge();

				//m_strAccount = o->get_account();
				n_account = o->get_account_num();
				m_isPersisted = false;
			}

			exec::exec(int orderId, tradeitem* i, AtsType::OrderWay::type way, int tradingType, const char* pszPortfolio,
				string& pszReference, int quantity, double price, const char* pszExecTime, AtsType::OrderOpenClose::type oc, OrderHedge oh, int account)
			{
				m_nOrderId = orderId;
				m_TradeItem = i;
				m_Way = way;
				m_TradingType = tradingType;
				m_strPortfolio = pszPortfolio;

				m_strReference = pszReference;
				switch (m_Way)
				{
				case AtsType::OrderWay::type::Buy:
					{
					m_strReference += "b";
					break;
					}
				case AtsType::OrderWay::type::Sell:
					{
			        m_strReference += "s";
					break;
					}
				default:
					break;
				}
				m_nQuantity = quantity;
				m_dPrice = price;
				m_strExecTime = pszExecTime;

				m_OpenClose = oc;
				m_Hedge = oh;

				n_account = account;

				m_isPersisted = false;
			}

			bool exec::operator==(const exec& e) const
			{
				if (m_nOrderId != e.m_nOrderId)
					return false;

				if (m_TradeItem != e.m_TradeItem)
					return false;

				if (m_Way != e.m_Way)
					return false;

				if (m_dPrice != e.m_dPrice)
					return false;

				if (m_nQuantity != e.m_nQuantity)
					return false;

				if (m_TradingType != e.m_TradingType)
					return false;

				if (m_strPortfolio != e.m_strPortfolio)
					return false;

				if (m_strReference != e.m_strReference)
					return false;

				if (n_account != e.n_account)
					return false;

				return true;
			}

			
			void exec::to_string()
			{
				std::string str = (boost::format("exec_DumpInfo: EXEC (%06d) : way:[%s] quantity:[%d] code:[%s] price:[%f] openclose[%s] TradingType[%d] Portfolio[%s] Account[%d]")
					%m_nOrderId
					%_OrderWay_VALUES_TO_NAMES.at(static_cast<int>(m_Way))
					%m_nQuantity
					%m_TradeItem->getCode().c_str()
					%m_dPrice
					%_OrderOpenClose_VALUES_TO_NAMES.at((int)m_OpenClose)
					%m_TradingType
					%m_strPortfolio.c_str()
					%n_account).str();
				loggerv2::info("%s", str.c_str());
			}
		}
	}
}