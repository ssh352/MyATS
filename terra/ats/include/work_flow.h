#ifndef _WORK_FLOW_V2_H_
#define _WORK_FLOW_V2_H_
#include "common.h"
#include <vector>
#include "boost/asio.hpp"
#include <thread>
#include <unordered_map>
#include <memory>
#include "boost/asio/high_resolution_timer.hpp"
#ifdef Linux
#include <sys/timerfd.h>
#endif
using namespace terra::common;
#pragma once
namespace terra
{
	namespace ats
	{
		class itask;
		class work_flow_thread
		{
			/*	public:
					work_flow();
					~work_flow();*/
		protected:
			long m_millis;
			int  m_iMinDelay;
			bool m_bActive;
			std::vector<itask*> m_TaskList;
			std::unordered_map<int, std::shared_ptr<itask>> m_TaskbyNum;
			boost::posix_time::ptime m_last_run_time;
		public:
			bool NewUpdate;
		public:
			//virtual void run();
			void set_delay(int iDelay){ m_iMinDelay = iDelay; }
			void stop(){ m_bActive = false; }
			bool register_workflow(itask * pTask,int tasknum = -1);
			bool unregister_workflow(itask * pTask);
			void set_sleep_timer(int value){ m_millis = value; }
			void set_name(std::string name){}
			void execute_by_num(int tasknum);
		protected:
			virtual void execute();

		public:
			work_flow_thread();
			~work_flow_thread();
		public:
			void run();
			std::thread m_thread;
			//boost::asio::io_service io;
			void set_kernel_timer_thread();
			//void work_flow_interate();
		protected:
			void start();
			virtual void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
#ifdef Linux
			int timer_fd;
			void timer_fd_process();
#endif
		};
	}
}
#endif //_WORK_FLOW_V2_H_

