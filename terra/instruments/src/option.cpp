#include "option.h"
#include "optionclass.h"
namespace terra
{
	namespace instrument
	{

		option::option(std::string & code) :abstractderivative(code)
		{
			set_type(AtsType::InstrType::Option);
			this->m_strike_parity = nullptr;
		}

		option::~option()
		{
		}
		/*void option::on_maturity_time_changed()
		{
			if (m_pRefClass != nullptr)
			{
				derivclassbase * pClass = (derivclassbase*)m_pRefClass;
				m_date_time += (int)(pClass->get_maturity_time().total_seconds());
			}
		}*/
	}
}
