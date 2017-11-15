#include "future.h"
#include "derivclassbase.h"
namespace terra
{
	namespace instrument
	{

		future::future(std::string & code) :abstractderivative(code)
		{
			set_type(AtsType::InstrType::Future);
		}


		future::~future()
		{
		}

		void future::compute_actu()
		{
			double Rate = ((derivclassbase*)m_pRefClass)->getUnderLying()->get_rate();
			//time_t m_time = get_maturity() - date_time_publisher_gh::get_instance()->now();
			//date_time_ex ti;
			//ti.set_date(m_time);
			//int day = ti.get_day();
			double day = (m_date_time - second_clock::local_time()).total_seconds() / 60.0 / 60.24;


			Actu = 1 / (1 + Rate * day / 360);
		}

		//void future::on_maturity_time_changed()
		//{
		//	if (m_pRefClass != nullptr)
		//	{
		//		derivclassbase * pClass = (derivclassbase*)m_pRefClass;

		//		m_date_time = m_date_time-m_date_time.time_of_day() + pClass->get_maturity_time();
		//		//m_date_time += (int)(pClass->get_maturity_time().total_seconds());
		//	}
		//}
	}
}
