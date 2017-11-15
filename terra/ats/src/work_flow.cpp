#include "work_flow.h"
#include "abstractworkflowtask.h"
#include <defaultdatetimepublisher.h>
#include "io_service_gh.h"
#include "atsconfig.h"

namespace terra
{
	namespace ats
	{
		work_flow_thread::work_flow_thread()
		{
			m_millis = 0;
			m_bActive = false;
			this->m_iMinDelay = 0;
		}


		work_flow_thread::~work_flow_thread()
		{			
			for (auto & v : m_TaskList)
			{				
				delete v;
			}
		}

		bool work_flow_thread::register_workflow(itask *pTask, int tasknum)
		{
			if (pTask == nullptr)
				return false;
			this->m_TaskList.push_back(pTask);

			if (tasknum >= 0)
				m_TaskbyNum[tasknum] = std::shared_ptr<itask>(pTask);
			return true;
		}		
		//void work_flow::run()
		//{
		//	if (this->m_iMinDelay == 0)
		//	{
		//		execute();
		//	}
		//	else
		//	{
		//		//ticks nowTick = date_time_publisher_gh::get_instance()->now();
		//		long delay = (date_time_publisher_gh::get_instance()->now()-m_last_run_time).total_milliseconds();
		//		if (delay > m_iMinDelay || delay < 0 || NewUpdate)
		//		{
		//			m_last_run_time = date_time_publisher_gh::get_instance()->now();
		//			NewUpdate = false;
		//			execute();
		//		}
		//	}
		//}
		
		void work_flow_thread::execute()
		{						
			//ptime now = microsec_clock::local_time();
			lwtp now = get_lwtp_now();
			for(auto &it: m_TaskList)
			{				
				it->execute_with_delay(now);
			}
		}

		void work_flow_thread::execute_by_num(int tasknum)
		{
			//ptime now = microsec_clock::local_time();
			lwtp now = get_lwtp_now();
			m_TaskbyNum[tasknum]->execute_with_delay(now);
		}

		void work_flow_thread::run()
		{
			if (this->m_iMinDelay == 0)
			{
				start();
			}
			else
			{
				start();
			}
		}

		void work_flow_thread::start()
		{
			m_bActive = true;
			/*RTThread::Create();*/
			if (m_thread.joinable())
				return;
			std::thread t(std::bind(&work_flow_thread::set_kernel_timer_thread, this));
			//m_thread.swap(t);
			//std::thread t(std::bind(&work_flow_thread::work_flow_interate, this));
			m_thread.swap(t);
		}

		void start_other_io_service()
		{
			io_service_gh::get_instance().start_other_io(ats_config::get_instance()->get_other_io_cpu_core());
		}

#ifdef Linux
		void work_flow_thread::timer_fd_process()
		{
			execute();
		}
#endif

		void work_flow_thread::set_kernel_timer_thread()
		{
#ifdef Linux
			timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
			if (timer_fd == -1)
			{
				printf("timefd error");
				exit(1);
			}

			//cout<<"tfd create:"<<timer_fd<<endl;

			io_service_gh::get_instance().add_fd_fun_map(io_service_type::other, timer_fd, std::bind(&work_flow_thread::timer_fd_process, this));
			struct itimerspec timer;

			timer.it_value.tv_nsec = 1;
			timer.it_value.tv_sec = 1;
			timer.it_interval.tv_nsec = 1000*1000 * (m_millis%1000);
			timer.it_interval.tv_sec = m_millis/1000;

			int res = timerfd_settime(timer_fd, 0, &timer, 0);
			if (res == -1)
			{
				printf("timerfd_settime fail");
				exit(1);
			}
#else
			boost::asio::high_resolution_timer *timer = new boost::asio::high_resolution_timer(*(io_service_gh::get_instance().get_io_service(io_service_type::other)), std::chrono::milliseconds(5));
			timer->async_wait(boost::bind(&work_flow_thread::process, this, boost::asio::placeholders::error, timer));
#endif
			std::thread td(start_other_io_service);
			td.detach();
		}

		void work_flow_thread::process(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		{
			while (m_bActive == true)
			{
				execute();

				t->expires_from_now(std::chrono::milliseconds(m_millis));
				t->async_wait(boost::bind(&work_flow_thread::process, this, boost::asio::placeholders::error, t));
				return;
			}
		}
	}
}
