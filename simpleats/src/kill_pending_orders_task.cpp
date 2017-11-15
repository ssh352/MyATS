#include "kill_pending_orders_task.h"
namespace simpleats
{
	kill_pending_orders_task::kill_pending_orders_task(simple_ats * pAts, lwdur delay) :abstract_workflow_task(pAts, "KillPendingOrder", delay)
	{

	}
	void kill_pending_orders_task::execute()
	{
		if (m_pT != nullptr && m_pT->is_open_or_pre_open()==true)
		{
			m_pT->kill_pending_orders();
		}
	}
}
