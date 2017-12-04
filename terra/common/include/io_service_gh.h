#ifndef __IO_SERVICE_GH_H__
#define __IO_SERVICE_GH_H__

#include "boost/asio/io_service.hpp"
#include "boost/asio/high_resolution_timer.hpp"
#include <thread>
#include <functional>
#include "tbb/concurrent_unordered_map.h"
#include "singleton.hpp"
#include "common.h"
#ifdef __linux__
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#define MaxEPOLLSize 1000
#endif

namespace terra
{
	namespace common
	{
		enum io_service_type
		{
			feed = 0,
			trader = 1,
			other
		};

		typedef std::function<void()> feed_trader_handler;

		class io_service_gh :public SingletonBase<io_service_gh>
		{
		public:
			void start_feed_io(int _feed_io_cpu_core);
			void start_trader_io(int _trader_io_cpu_core);
			void start_other_io(int _other_io_cpu_core);

			void set_feed_io_thread();
			void set_trader_io_thread();
			void set_other_io_thread();
			
			boost::asio::io_service feed_io;
			boost::asio::io_service trader_io;
			boost::asio::io_service *other_io;
			boost::asio::io_service *get_io_service(io_service_type);
			void init_other_io(int thread_num = 1);

#ifdef __linux__
			void epoll_proc(int efd,int timeout,io_service_type _type);
			void add_fd_fun_map(io_service_type _type,int fd,feed_trader_handler);
			void set_is_bind_feed_trader_core(bool b);
			void add_feed_update_handler(feed_trader_handler);
			void add_trader_update_handler(feed_trader_handler);

			pthread_t feed_id;
			pthread_t trader_id;
			pthread_t other_id;

			int feed_io_cpu_core;
			int trader_io_cpu_core;
			int other_io_cpu_core;

			std::list<feed_trader_handler> feed_update_list;
			std::list<feed_trader_handler> trader_update_list;

			int efd_feed = -1;
			int efd_trader = -1;
			int efd_other = -1;

			tbb::concurrent_unordered_map<int, feed_trader_handler> ProcMap;
			std::list<int>  feed_fdlist;
			std::list<int>  trader_fdlist;
			std::list<int>  other_fdlist;
			bool m_is_set_other_thread=false;
			bool m_is_bind_feed_trader_core = true;
#else
			std::thread feed_thread;
			std::thread trader_thread;
			std::thread other_thread;
#endif
			
		private:
			bool is_ats_running = false;
			boost::shared_mutex rw_mutex;
		};
	}
}

#endif 
