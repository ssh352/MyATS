#include "heartbeatcheck.h"
#include "defaultdatetimepublisher.h"
#include "order_gh.h"
//#include "orderqueue.h"
using namespace terra::marketaccess::orderpassing;
using namespace terra::common;
namespace terra
{
	namespace ats
	{
		heart_beat_check::heart_beat_check(abstract_ats * pAts,lwdur delay) :abstract_workflow_task(pAts, "heart_beat_check",delay)
		{
			m_disconnecTimeSpan = time_duration(0,2,0);
			m_lastDisconnectDateTime = date_time_publisher_gh::get_instance()->now();
		}
		void heart_beat_check::execute()
		{
			//auto span = date_time_publisher_gh::get_instance()->now() - ats_workflow::LastHeartBeatTime;
			//date_time dt(time_v);
			//int hh = 0, mi = 0, ss = 0;
			//dt.get_time(&hh, &mi, &ss);
			//time_duration span(hh, mi, ss);
			//m_pT->current_task = "heartbeat_task";

			if (date_time_publisher_gh::get_instance()->now() - LastHeartBeatTime < m_disconnecTimeSpan)
				return;

			//span = date_time_publisher_gh::get_instance()->now() - m_lastDisconnectDateTime;
			/*	date_time dt2(time_v);
				dt2.get_time(&hh, &mi, &ss);
				time_duration span2(hh, mi, ss);*/

			if (date_time_publisher_gh::get_instance()->now() - m_lastDisconnectDateTime < m_disconnecTimeSpan)
				return;

		    m_lastDisconnectDateTime = date_time_publisher_gh::get_instance()->now();

			if (m_pT != nullptr && m_pT->is_open_or_pre_open_or_pre_close() && m_pT->get_started() == true)
			{
				loggerv2::error("Stop Ats %s,stop_ats:HeartBeat overtime",m_pT->get_name().c_str());
				m_pT->stop_automaton(false);
			}

			//loggerv2::error("Kill ALL");
			//order_queue::get_instance()->clean_order();

			loggerv2::error("Kill ALL");
			order_gh::get_instance().GetBook()->killall();

		}

		boost::posix_time::ptime heart_beat_check::LastHeartBeatTime = date_time_publisher_gh::get_instance()->now();

	}
}