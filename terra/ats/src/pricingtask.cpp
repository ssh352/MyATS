#include "pricingtask.h"
namespace terra
{
	namespace ats
	{
		pricing_task::pricing_task(abstract_ats * pAts, lwdur delay) :abstract_workflow_task(pAts, "Pricing",delay)
		{
			is_running = false;

		}
		void pricing_task::execute()
		{
			if (is_running)
				return;
			is_running = true;
			if (m_pT != nullptr && m_pT->is_open_or_pre_open())
				m_pT->do_pricing();
			is_running = false;
		}
	}
}