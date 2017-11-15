#include "optionclass.h"
#include "terra_logger.h"
#include "option.h"
#include "maturity.h"
namespace terra
{
	namespace instrument
	{

		optionclass::optionclass(underlying * pUnderlying, std::string & className, int pointValue, currency * pCurrency) :derivclassbase(pUnderlying, className, pointValue, pCurrency)
		{
		}
		optionclass::~optionclass()
		{
		}
		void optionclass::add(option * pOption)
		{
			if (pOption == nullptr)
				return;			
			maturity * pMaturity = this->m_maturity_map.get_by_key(pOption->get_maturity_str());
			if (pMaturity == nullptr)
			{
				pMaturity = new maturity(this, pOption->get_maturity().date());
				this->m_maturity_map[pOption->get_maturity_str()] = pMaturity;
			}
			pMaturity->add(pOption);
			this->m_abstract_derivative_map.add(pOption->get_code(),pOption);
		}
	}
}
