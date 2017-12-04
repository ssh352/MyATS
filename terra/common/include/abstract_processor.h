#ifndef _ABSTRACT_PROCESSOR_H_
#define _ABSTRACT_PROCESSOR_H_
#include "common.h"
#include "boost/asio/io_service.hpp"
#include "boost/asio/high_resolution_timer.hpp"
#include "boost/asio.hpp"
#include "io_service_gh.h"
#include <chrono>
#pragma once
namespace terra
{
	namespace common
	{
		class abstract_processor
		{
		public:
			abstract_processor();
			~abstract_processor();
			void init_process(io_service_type _type = io_service_type::feed, int micro = 15, int poll_num = 0);
			inline bool is_alive() { return m_isAlive; }
			inline void is_alive(bool b) { m_isAlive = b; }
			void stop_process();
#ifdef Linux
			void add_fd_fun_to_io_service(io_service_type _type, int fd, feed_trader_handler handler);
#endif
		protected:
			virtual void process() = 0;
		private:
			bool m_isAlive;
			//boost::asio::io_service io;
			void process_loop(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			//void set_kernel_timer();
			boost::asio::high_resolution_timer *timer;
			int m_pollnum;
			std::chrono::microseconds interval;
		};

	}
}

#endif