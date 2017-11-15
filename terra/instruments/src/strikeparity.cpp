#include "strikeparity.h"
#include "option.h"
#include "maturity.h"
namespace terra
{
	namespace instrument
	{
		strikeparity::strikeparity(maturity * pMaturity, double strike)
		{
			this->m_pRefMaturity = pMaturity;
			m_dStrike = strike;
			this->m_pRefCallOption = nullptr;
			this->m_pRefPutOption = nullptr;
		}


		strikeparity::~strikeparity()
		{
		}

		void strikeparity::add(option * pOption)
		{
			if (pOption == nullptr)
				return;
			if (pOption->get_type() == AtsType::InstrType::Call)
			{
				this->m_pRefCallOption = pOption;
			}
			if (pOption->get_type() == AtsType::InstrType::Put)
			{
				this->m_pRefPutOption = pOption;
			}
			pOption->set(this);
		}
		void strikeparity::show()
		{
			loggerv2::info("strikeparity::show enter----------");
			loggerv2::info("strikeparity::show strike:%f", this->m_dStrike);
			if (this->m_pRefMaturity != nullptr)
			{
				loggerv2::info("strikeparity::show maturity:%s", this->m_pRefMaturity->get_day_string().data());
			}
			else
			{
				loggerv2::info("strikeparity::show maturity:null");
			}
			if (this->m_pRefCallOption != nullptr)
			{
				loggerv2::info("strikeparity::show callOption:%s", m_pRefCallOption->get_code().c_str());
			}
			else
			{
				loggerv2::info("strikeparity::show callOption:null");
			}
			if (this->m_pRefPutOption != nullptr)
			{
				loggerv2::info("strikeparity::show putOption:%s", m_pRefPutOption->get_code().c_str());
			}
			else
			{
				loggerv2::info("strikeparity::show putOption:null");
			}
			loggerv2::info("strikeparity::show end----------");
		}
	}
}
