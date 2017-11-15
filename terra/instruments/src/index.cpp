#include "index.h"
#include "stock.h"
#include "underlying.h"
#include "AtsType_types.h"
namespace terra
{
	namespace instrument
	{
		index::index(string& code) :financialinstrument(code)
		{
			set_type(AtsType::InstrType::Index);
			
			m_dDivisor = 0.0;
		}


		index::~index()
		{
		}

		void index::add(stock * pStock, double nbShare)
		{
			if (pStock == nullptr)
				return;
			if (this->m_refNbShares.contain_key(pStock) == false)
			{
				this->m_refNbShares[pStock] = nbShare;
			}
			else
			{
				loggerv2::info("index::add already add the stock:%s", pStock->get_code().c_str());
			}
		}
		void index::show()
		{
			loggerv2::info("index::show enter");
			loggerv2::info("index::show divisor:%f", m_dDivisor);
			if (this->m_pRefUnderLying == nullptr)
			{
				loggerv2::info("index::show m_pRefUnderLying is null");
			}
			else
			{
				loggerv2::info("index::show underLyingName:%s", this->m_pRefUnderLying->get_name().c_str());
			}
			
			for(auto &it: m_refNbShares)
			{
				stock * pStock = it.first;
				loggerv2::info("index::show code:%s,share:%f", pStock->get_code().c_str(), it.second);
			}
			loggerv2::info("index::show end");
		}
	}
}
