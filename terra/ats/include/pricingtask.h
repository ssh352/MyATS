#ifndef _PRICING_TASK_V2_H_
#define _PRICING_TASK_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
namespace terra
{
	namespace ats
	{
		class pricing_task : public abstract_workflow_task<abstract_ats>
		{
		public:
			pricing_task(abstract_ats * pAts, lwdur delay = std::chrono::milliseconds(1));
			~pricing_task(){}
		public:
			virtual void execute();
		private: 
			atomic<bool> is_running;
		};
	}
}
#endif //_PRICING_TASK_V2_H_

