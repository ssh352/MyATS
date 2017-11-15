#include "maturity.h"
#include "futureclass.h"
#include "optionclass.h"
#include "option.h"
#include "strikeparity.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		maturity::maturity(optionclass * poptionclass, date maturityDay) :m_MaturityDay(maturityDay), m_pDerivClassBase(poptionclass)
		{
			init();
		}

		maturity::maturity(futureclass * pfutureclass, date maturityDay) : m_MaturityDay(maturityDay), m_pDerivClassBase(pfutureclass)
		{
			init();

		}

		maturity::~maturity()
		{
			for (auto &it : m_strikeparitylist)
			{
				delete it.second;
			}
		}
		void maturity::init()
		{
			m_iOffDays = 0;
			m_dOpenDays = 0.0;
			m_dTimeToMaturity = 0.0;
			m_dActu = 0.0;
			m_dFirstStrike = 0.0;
			m_dLastStrike = 0.0;
			compute_days_off();
			compute_actu();
			compute_time_to_maturity();
		}
		void maturity::compute_days_off()
		{
			if (this->m_pDerivClassBase != nullptr && this->m_pDerivClassBase->getUnderLying() != nullptr)
			{
				this->m_iOffDays = this->m_pDerivClassBase->getUnderLying()->compute_days_off(this->m_MaturityDay.date());
			}
		}
		void maturity::compute_time_to_maturity()
		{
			/*time_t toDay;
			time(&toDay);*/
			/*date_time_ex  dtTodayDateTime;
			dtTodayDateTime.set_date(toDay);*/
			m_dOpenDays = (m_MaturityDay + m_pDerivClassBase->get_maturity_time() - second_clock::local_time()).total_seconds() / 60.0 / 60 / 24 - m_iOffDays;
			m_dTimeToMaturity = m_dOpenDays / 252;
		}
		void maturity::compute_actu()
		{
			/*time_t toDay;
			time(&toDay);
			date_time_ex  dtTodayDateTime;
			dtTodayDateTime.set_date(toDay);*/
			if (this->m_pDerivClassBase != nullptr && this->m_pDerivClassBase->getUnderLying() != nullptr)
			{
				//m_dActu = 1 / (1 + this->m_pDerivClassBase->getUnderLying()->get_rate() * m_MaturityDay.get_days_between(dtTodayDateTime) / 360);
				m_dActu = 1 / (1 + this->m_pDerivClassBase->getUnderLying()->get_rate() * (m_MaturityDay + m_pDerivClassBase->get_maturity_time() - second_clock::local_time()).total_seconds() / 60.0 / 60 / 24 / 360);
			}
		}
		strikeparity * maturity::get_strike(double strike)
		{
			strikeparity * pstrikeparity = nullptr;

			for (auto &it : m_strikeparitylist)
			{
				pstrikeparity = it.second;
				if (pstrikeparity && math2::eq(strike, pstrikeparity->get_strike()) == true)
				{
					return pstrikeparity;
				}
			}
			return nullptr;
		}
		void maturity::add(option * pOption)
		{
			if (pOption == nullptr)
				return;
			strikeparity * pstrikeparity = this->get_strike(pOption->get_strike());
			if (pstrikeparity == nullptr)
			{
				pstrikeparity = new strikeparity(this, pOption->get_strike());
				this->m_strikeparitylist.add(pOption->get_strike(), pstrikeparity);
			}
			else
			{
				//loggerv2::info("maturity::add same strike:%f,option:%s", pOption->get_strike(), pOption->get_code().c_str());
			}
			pstrikeparity->add(pOption);

			compute_first_last_strikes();


		}
		void maturity::compute_first_last_strikes()
		{
			m_dFirstStrike = 0.0;
			m_dLastStrike = 0.0;
			if (this->m_strikeparitylist.size() > 0)
			{
				map_ex<double, strikeparity*>::iterator it;
				it = m_strikeparitylist.begin();
				if (it != m_strikeparitylist.end())
				{
					strikeparity * pstrikeparity = it->second;
					if (pstrikeparity != nullptr)
					{
						m_dFirstStrike = pstrikeparity->get_strike();
					}
				}

				it = m_strikeparitylist.end();
				it--;
				strikeparity * pStrikteParity = it->second;
				if (pStrikteParity != nullptr)
				{
					m_dLastStrike = pStrikteParity->get_strike();
				}
			}
		}
		void maturity::show()
		{
			loggerv2::info("maturity::show enter----------");
			loggerv2::info("maturity::show MaturityDay:%s", to_iso_extended_string(this->m_MaturityDay).data());
			loggerv2::info("maturity::show m_iOffDays:%d", m_iOffDays);
			loggerv2::info("maturity::show m_dOpenDays:%d", m_dOpenDays);
			loggerv2::info("maturity::show m_dActu:%d", m_dActu);
			loggerv2::info("maturity::show m_dFirstStrike:%d", m_dFirstStrike);
			loggerv2::info("maturity::show m_dLastStrike:%d", m_dLastStrike);
			if (this->m_pDerivClassBase != nullptr)
			{
				loggerv2::info("maturity::show className:%s", m_pDerivClassBase->get_name().c_str());
			}
			else
			{
				loggerv2::info("maturity::show className:null");
			}
			loggerv2::info("maturity::show m_strikeparityList.size:%d", m_strikeparitylist.size());

			for (auto &it : m_strikeparitylist)
			{
				strikeparity * pstrikeparity = it.second;
				if (pstrikeparity != nullptr)
					pstrikeparity->show();
			}
			loggerv2::info("maturity::show end");
		}
	}
}
