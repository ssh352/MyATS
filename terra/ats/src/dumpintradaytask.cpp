#include "dumpintradaytask.h"
namespace terra
{
	namespace ats
	{
		dump_intraday_task::dump_intraday_task(abstract_ats * pAts, lwdur delay) :abstract_workflow_task(pAts, "Dump Intraday", delay)
		{
		}
		void dump_intraday_task::execute()
		{
			if (m_pT != nullptr)
				m_pT->dump_intra_day();
		}
	}
}