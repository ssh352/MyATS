#ifndef _HEART_BEAT_CHECK_V2_H_
#define _HEART_BEAT_CHECK_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
namespace terra
{
	namespace ats
	{
		class heart_beat_check : public abstract_workflow_task<abstract_ats>
		{
		public:
			heart_beat_check(abstract_ats * pAts, lwdur delay = std::chrono::milliseconds(500));
			~heart_beat_check(){}
		private:
			time_duration m_disconnecTimeSpan;
			ptime m_lastDisconnectDateTime;
		public:
			virtual void execute();
			static ptime LastHeartBeatTime;
		};
	}
}
#endif //_HEART_BEAT_CHECK_V2_H_


