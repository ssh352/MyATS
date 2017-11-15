#include "futureclass.h"
#include "future.h"
#include "maturity.h"
namespace terra
{
	namespace instrument
	{

		futureclass::futureclass(underlying * pUnderlying, std::string & className, int pointValue, currency * pCurrency) :derivclassbase(pUnderlying, className, pointValue, pCurrency)
		{
		}


		futureclass::~futureclass()
		{
		}
		void futureclass::add(future * pFuture)
		{
			if (pFuture == nullptr)
				return;
			maturity * pMaturity = this->m_maturity_map.get_by_key(pFuture->get_maturity_str());
			if (pMaturity == nullptr)
			{
				pMaturity = new maturity(this, pFuture->get_maturity().date());
				this->m_maturity_map[pFuture->get_maturity_str()] = pMaturity;
				//loggerv2::info("futureclass::add TradingCodes Maturity:%s,className:%s", pFuture->get_maturity_str().c_str(), to_string().c_str());
			}
			if (m_refFutureContainer.contain_key(pFuture->get_maturity().date()) == false)
			{
				m_refFutureContainer.add(pFuture->get_maturity().date(), pFuture);
				this->m_abstract_derivative_map.add(pFuture->get_code(),pFuture);
				//loggerv2::info("futureclass::add futureName:%s,className:%s", pFuture->to_string().c_str(), to_string().c_str());
			}
		}
	}
}
