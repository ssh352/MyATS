#include "forexclass.h"
#include "forex.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		forexclass::forexclass(std::string & className, int pointValue, currency * pCurrency) :instrumentclass(className, pointValue, pCurrency)
		{
		}


		forexclass::~forexclass()
		{
		}

		void forexclass::add(forex * pForex)
		{
			if (pForex != nullptr && this->m_forex_instrument_map.contain_key(pForex->get_code()) == false)
			{
				this->m_forex_instrument_map[pForex->get_code()] = pForex;
			}

		}

	}
}