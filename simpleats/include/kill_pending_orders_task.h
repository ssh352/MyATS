#ifndef _KILL_PENDING_ORDERS_TASK_H_
#define _KILL_PENDING_ORDERS_TASK_H_
#include "abstractworkflowtask.h"
#include "abstractats.h"
#include "simpleats.h"
#pragma once
using namespace terra::ats;
namespace simpleats
{
	class kill_pending_orders_task : public abstract_workflow_task<simple_ats>
	{
	public:
		kill_pending_orders_task(simple_ats * pAtsk, lwdur delay = std::chrono::milliseconds(500));
		~kill_pending_orders_task(){}
	public:
		virtual void execute();
	};
}
#endif //_KILL_PENDING_ORDERS_TASK_H_

