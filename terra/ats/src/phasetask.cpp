#include "phasetask.h"
namespace terra
{
	namespace ats
	{
		phase_task::phase_task(abstract_ats * pAts,lwdur delay) :abstract_workflow_task(pAts, "Phase",delay)
		{
		}
		void phase_task::execute()
		{
			if (m_pT != nullptr)
				m_pT->compute_trading_phase();
		}
	}
}