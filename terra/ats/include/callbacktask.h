#ifndef _CALLBACK_TASK_V2_H_
#define _CALLBACK_TASK_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
namespace terra
{
	namespace ats
	{
		class callback_task : public abstract_workflow_task<abstract_ats>
		{
		public:
			callback_task(abstract_ats * pAts, lwdur delay=std::chrono::milliseconds(500));
			~callback_task(){}
		public:
			virtual void execute();
		};
	}
}
#endif //_CALLBACK_TASK_V2_H_

