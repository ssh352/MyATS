#include "callbacktask.h"

namespace terra
{
	namespace ats
	{
		callback_task::callback_task(abstract_ats * pAts, lwdur delay) :abstract_workflow_task(pAts, "Callbacks",delay)
		{
		}
		void callback_task::execute()
		{
			if (m_pT != nullptr)
				m_pT->check_callbacks();
		}
	}
}
