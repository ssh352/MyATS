#ifndef _PHASE_TASK_V2_H_
#define _PHASE_TASK_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
namespace terra
{
	namespace ats
	{
		class phase_task : public abstract_workflow_task<abstract_ats>
		{
		public:
			phase_task(abstract_ats * pAts, lwdur delay = std::chrono::milliseconds(500));
			~phase_task(){}
		public:
			virtual void execute();
		};
	}
}
#endif //_PHASE_TASK_V2_H_


