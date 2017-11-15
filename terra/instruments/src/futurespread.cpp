#include "futurespread.h"
#include "derivclassbase.h"
namespace terra
{
	namespace instrument
	{

		futurespread::futurespread(std::string & code) :abstractderivative(code)
		{
			set_type(AtsType::InstrType::Futurespread);
		}


		futurespread::~futurespread()
		{
		}

		void futurespread::on_maturity_time_changed()
		{
			if (m_pRefClass != nullptr)
			{
				//derivclassbase * pClass = (derivclassbase*)m_pRefClass;
				//m_date_time += pClass->get_maturity_time().get_date();
			}
		}

	}
}
