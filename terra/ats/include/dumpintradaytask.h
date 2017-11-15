#ifndef _DUMP_INTRADAY_TASK_V2_H_
#define _DUMP_INTRADAY_TASK_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
namespace terra
{
	namespace ats
	{
		class dump_intraday_task : public abstract_workflow_task<abstract_ats>
		{
		public:
			dump_intraday_task(abstract_ats * pAts, lwdur delay = std::chrono::milliseconds(500));
			~dump_intraday_task(){}
		public:
			virtual void execute();
		};
	}
}
#endif //_DUMP_INTRADAY_TASK_V2_H_

