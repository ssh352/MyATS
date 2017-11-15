#include "pnltask.h"
namespace terra
{
	namespace ats
	{
		pnl_task::pnl_task(abstract_ats * pAts,lwdur delay) :abstract_workflow_task(pAts, "Pnl",delay)
		{

		}
		void pnl_task::execute()
		{
			if (m_pT != nullptr && m_pT->is_open_or_pre_open())
				m_pT->compute_pnl();
		}
	}
}