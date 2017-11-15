#include "abstractderivative.h"
#include <derivclassbase.h>

namespace terra
{
	namespace instrument
	{

		abstractderivative::abstractderivative(std::string & code) :financialinstrument(code)
		{
		}


		abstractderivative::~abstractderivative()
		{
		}

		void abstractderivative::on_maturity_time_changed()
		{
			if (m_pRefClass != nullptr)
			{
				derivclassbase * pClass = (derivclassbase*)m_pRefClass;

				m_date_time = m_date_time - m_date_time.time_of_day() + pClass->get_maturity_time();
				//m_date_time += (int)(pClass->get_maturity_time().total_seconds());
			}
		}

		void abstractderivative::show()
		{
			loggerv2::info("abstractderivative::show enter");
			financialinstrument::show();
			loggerv2::info("abstractderivative::show maturityString:%s", this->m_strMaturity.c_str());
			loggerv2::info("abstractderivative::show end");
		}
	}
}
