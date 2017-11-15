#include "abstract_processor.h"

namespace terra
{
	namespace common
	{

		abstract_processor::abstract_processor()
		{

		}

		abstract_processor::~abstract_processor()
		{
			delete timer;
		}

		void abstract_processor::init_process(io_service_type _type, int micro, int poll_num)
		{
			is_alive(true);
			//std::thread t(std::bind(&abstract_processor::set_kernel_timer_thread, this));
			//m_thread.swap(t);
			//set_kernel_timer();
			timer = new boost::asio::high_resolution_timer(*(io_service_gh::get_instance().get_io_service(_type)), std::chrono::milliseconds(5));
			timer->async_wait(boost::bind(&abstract_processor::process_loop, this, boost::asio::placeholders::error, timer));
			m_pollnum = poll_num;
			interval = std::chrono::microseconds(micro);
		}


#ifdef Linux
		void abstract_processor::add_fd_fun_to_io_service(io_service_type _type, int fd, epoll_handler handler)
		{
			io_service_gh::get_instance().add_fd_fun_map(_type, fd, handler);
		}
#endif


		void abstract_processor::process_loop(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		{
			int i = 0;

			while (is_alive())
			{
				process();
				++i;
				if (i > m_pollnum)
				{
					//t->expires_at(t->expires_at() + std::chrono::microseconds(20));
					t->expires_from_now(interval);
					t->async_wait(boost::bind(&abstract_processor::process_loop, this, boost::asio::placeholders::error, t));
					return;
				}

			}
		}

		void abstract_processor::stop_process()
		{
			m_isAlive = false;
			//m_thread.join();
		}
	}
}