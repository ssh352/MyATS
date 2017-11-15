#ifndef _ABSTRACT_WORKFLOW_V2_H_
#define _ABSTRACT_WORKFLOW_V2_H_
#pragma once
#include "feedcommon.h"
#include <chrono>
namespace terra
{
	namespace ats
	{
		class itask
		{

		public:
			itask(lwdur &delay)
			{
				last_execute_time = get_lwtp_now();
				min_delay = delay;
			}

			virtual ~itask(){};
			
			void execute_with_delay(lwtp& now)
			{
				if (now - last_execute_time > min_delay)
				{
					last_execute_time = now;
					execute();
				}
			}
			lwtp last_execute_time; 
			lwdur min_delay;
		protected:
			virtual void execute() = 0;
		};
		template<class T>
		class abstract_workflow_task : public itask
		{
			//refer to the Terra.ATS.ats_workflow design
		public:
			abstract_workflow_task(T * t, string strName, lwdur delay) :itask(delay)

			{ 
				m_pT = t;
				m_strName = strName;
			}
			~abstract_workflow_task(){}
		protected:
			T * m_pT;
			string m_strName;
		};
	}
}
#endif //_ABSTRACT_WORKFLOW_V2_H_


