#ifndef _PNL_TASK_V2_H_
#define _PNL_TASK_V2_H_
#include "abstractworkflowtask.h"
#include "abstractats.h"
#pragma once
namespace terra
{
	namespace ats
	{
		class pnl_task : public abstract_workflow_task<abstract_ats>
		{
		public:
			pnl_task(abstract_ats * pAts, lwdur delay = std::chrono::milliseconds(500));
			~pnl_task(){}
		public:
			virtual void execute();
		};
	}
}
#endif //_PNL_TASK_V2_H_


