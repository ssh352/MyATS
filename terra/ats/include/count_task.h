#ifndef _COUNT_TASK_V2_H_
#define _COUNT_TASK_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
namespace terra
{
	namespace ats
	{
		class count_task : public abstract_workflow_task<abstract_ats>
		{
		public:
			count_task(abstract_ats * pAts, lwdur delay = std::chrono::milliseconds(500));
			~count_task(){}
		public:
			virtual void execute();
			int total_ms = 0;
			int last_num = 0;
			int num_workflow = 0;
		};
	}
}
#endif //_COUNT_TASK_V2_H_

