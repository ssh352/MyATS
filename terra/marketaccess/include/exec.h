#ifndef __EXEC_H2__
#define __EXEC_H2__

#include <string>
#include "tradeitem.h"
#include "AtsType_types.h"
#include "order.h"
#include "orderdatadef.h"
#include <thread>

using namespace AtsType;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class exec
			{
			public:
				exec(){};
				exec(order* o, int quantity, double price);
				exec(order* o, const string& pszReference, int quantity, double price, const char* pszExecTime);
				exec(int orderId, tradeitem* i, AtsType::OrderWay::type way, int tradingType, const char* pszPortfolio, 
					string& pszReference, int quantity, double price, const char* pszExecTime, AtsType::OrderOpenClose::type oc, OrderHedge oh, int account);

				~exec() {};

				bool operator==(const exec& e) const;
				bool operator!=(const exec& e) const
				{
					return !(*this == e);
				}
				void dump_info();
				void to_string();

				tradeitem *getTradeItem(){ return m_TradeItem; }
				void setTradeItem(tradeitem *value){ m_TradeItem = value; }

				int getOrderId(){ return m_nOrderId; }
				void setOrderId(int value){ m_nOrderId = value; }

				OrderWay::type getWay(){ return m_Way; }
				void setWay(OrderWay::type value){ m_Way = value; }

				int getQuantity(){ return m_nQuantity; }
				void setQuantity(int value){ m_nQuantity = value; }

				double getPrice(){ return m_dPrice; }
				void setPrice(double value){ m_dPrice = value; }

				std::string& getReference(){ return m_strReference; }
				void setReference(std::string &value){ m_strReference = value; }

				std::string& getTime(){ return m_strExecTime; }
				void setTime(std::string &value){ m_strExecTime = value; }

				int getTradingType(){ return m_TradingType; }
				void setTradingType(int value){ m_TradingType = value; }


				std::string& getPortfolioName(){ return m_strPortfolio; }
				void setPortfolioName(std::string &value){ m_strPortfolio = value; }

				OrderOpenClose::type getOpenClose(){ return m_OpenClose; }
				void setOpenClose(OrderOpenClose::type value){ m_OpenClose = value; }

				bool is_persisted() { return m_isPersisted; }
				void is_persisted(bool b) { m_isPersisted = b; }

				OrderHedge get_hedge() { return m_Hedge; }
				void       set_hedge(OrderHedge hedge){ m_Hedge = hedge; }

				//std::string get_account() { return m_strAccount; }
				int get_account_num(){ return n_account; }
				void set_account_num(int account){ n_account = account; }

				const std::string& get_time() const { return m_strExecTime; }
				void set_time(const std::string& psz) { m_strExecTime = psz; }

			private:
				tradeitem *m_TradeItem;
				int m_nOrderId;

				OrderWay::type m_Way;
				int m_nQuantity;
				double m_dPrice;
				std::string m_strReference;
				std::string m_strTime;

				int m_TradingType;
				std::string m_strPortfolio;
				bool m_isPersisted;
				OrderOpenClose::type m_OpenClose;
				OrderHedge m_Hedge;

				//std::string m_strAccount;
				int n_account;
				std::string m_strExecTime;
				
			};

			inline void exec::dump_info()
			{
				std::thread t(std::bind(&exec::to_string, this));
				t.detach();
			}

		}
	}
}


#endif

