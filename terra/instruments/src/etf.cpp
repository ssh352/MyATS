#include "etf.h"
#include "stock.h"
namespace terra
{
	namespace instrument
	{
		etfcomponent::etfcomponent(stock * pRefStock, int iShares, int iMustCashReplace, double dCashReplaceAmt)
		{
			this->m_pRefStock = pRefStock;
			this->m_iShares = iShares;
			this->m_iMustCashReplace = iMustCashReplace;
			this->m_dCashReplaceAmt = dCashReplaceAmt;
		}
		void etfcomponent::show()
		{
			loggerv2::info("etfcomponent::show enter--");
			loggerv2::info("etfcomponent::show iShares:%d", this->m_iShares);
			loggerv2::info("etfcomponent::show m_iMustCashReplace:%d", this->m_iMustCashReplace);
			loggerv2::info("etfcomponent::show m_dCashReplaceAmt:%f", this->m_dCashReplaceAmt);
			if (m_pRefStock != nullptr)
			{
				loggerv2::info("etfcomponent::show stock:%s", this->m_pRefStock->get_code().c_str());
			}
			else
			{
				loggerv2::error("etfcomponent::show stock:null");
			}
			loggerv2::info("etfcomponent::show end--");
		}
		string etfcomponent::get_code()
		{
			if (m_pRefStock != nullptr)
				return m_pRefStock->get_code();
			return "";
		}
		etf::etf(std::string & code) :financialinstrument(code)
		{
			m_iUnitSize = 0;
			m_dCashDiffPerUnit = 0.0;
			set_type(AtsType::InstrType::ETF);
		}

		etf::~etf()
		{			
			for(auto &it :m_etf_component_map)
			{				
				delete it.second;			
			}
		}

		void etf::add(etfcomponent * pETFComponent)
		{
			if (pETFComponent && pETFComponent->get_stock())
				this->m_etf_component_map.add(pETFComponent->get_stock()->get_code(),pETFComponent);
		}
		void etf::show()
		{
			loggerv2::info("etf::show enter");
			financialinstrument::show();
			loggerv2::info("etf::show m_iUnitSize:%d", this->m_iUnitSize);
			loggerv2::info("etf::show m_dCashDiffPerUnit:%f", this->m_dCashDiffPerUnit);
			loggerv2::info("etf::show m_etf_component_map,size:%d", this->m_etf_component_map.size());
			for (auto &it : m_etf_component_map)
			{				
				it.second->show();			
			}
			loggerv2::info("etf::show end");
		}
	}
}