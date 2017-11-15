#include "stockclass.h"
#include "instrumentcommon.h"
#include "stock.h"
namespace terra
{
	namespace instrument
	{
		stockclass::stockclass(std::string & className, int pointValue, currency * pCurrency) :instrumentclass(className, pointValue, pCurrency)
		{
		}
		stockclass::~stockclass()
		{
		}
		void stockclass::add(stock * pStock)
		{
			if (pStock != nullptr && this->m_stock_instrument_map.contain_key(pStock->get_code()) == false)
			{
				this->m_stock_instrument_map[pStock->get_code()] = pStock;
			}
		}
		stock * stockclass::get_stock(string strCode)
		{			
			return m_stock_instrument_map.get_by_key(strCode);			
		}
		void stockclass::show()
		{
			loggerv2::info("stockclass::show enter");
			instrumentclass::show();
			loggerv2::info("stockclass::show stockList,size:$d", this->m_stock_instrument_map.size());
			
			for(auto &it:m_stock_instrument_map)
			{				
				loggerv2::info("stockclass::show code:%s\n", it.second->get_code().c_str());
			}
			loggerv2::info("stockclass::show end");
		}
	}
}