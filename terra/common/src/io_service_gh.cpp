#include "io_service_gh.h"
//#include "atsconfig.h"


namespace terra
{
	namespace common
	{
#ifdef Linux

		void* FeedThreadProc(void *pvArgs)
		{
			if (pvArgs==nullptr)
			{
				return nullptr;
			}

			io_service_gh *pThread = (io_service_gh *)pvArgs;
			pThread->set_feed_io_thread();
			return nullptr;
		}

		void* TraderThreadProc(void *pvArgs)
		{
			if (pvArgs == nullptr)
			{
				return nullptr;
			}

			io_service_gh *pThread = (io_service_gh *)pvArgs;
			pThread->set_trader_io_thread();
			return nullptr;
		}

		void* OtherThreadProc(void *pvArgs)
		{
			if (pvArgs == nullptr)
			{
				return nullptr;
			}

			io_service_gh *pThread = (io_service_gh *)pvArgs;
			pThread->set_other_io_thread();
			return nullptr;
		}

		void io_service_gh::set_is_bind_feed_trader_core(bool b)
		{
			printf("set_is_bind_feed_trader_core,:%d\n",b);
			m_is_bind_feed_trader_core = b;
		}

#endif



		void io_service_gh::start_feed_io(int _feed_io_cpu_core)
		{
			feed_io_cpu_core=_feed_io_cpu_core;
#ifdef Linux
			pthread_create(&feed_id, NULL, FeedThreadProc, (void *)this);
			pthread_detach(feed_id);
#else
			if (!feed_thread.joinable())
			{
				std::thread t(std::bind(&io_service_gh::set_feed_io_thread, this));
				feed_thread.swap(t);
			}
#endif
		}

		void io_service_gh::start_trader_io(int _trader_io_cpu_core)
		{
			trader_io_cpu_core=_trader_io_cpu_core;
#ifdef Linux
			if(m_is_bind_feed_trader_core)
			{
				printf("trader io should stop when m_is_bind_feed_trader_core is true\n");
				return;
			}
			pthread_create(&trader_id, NULL, TraderThreadProc, (void *)this);
			pthread_detach(trader_id);
#else
			if (!trader_thread.joinable())
			{
				std::thread t(std::bind(&io_service_gh::set_trader_io_thread, this));
				trader_thread.swap(t);
			}
#endif
		}

		void io_service_gh::start_other_io(int _other_io_cpu_core)
		{
			other_io_cpu_core = _other_io_cpu_core;
			boost_write_lock lk(rw_mutex);
			if (is_ats_running == false)
			{
#ifdef Linux
				if(m_is_set_other_thread)
					return;
				pthread_create(&other_id, NULL, OtherThreadProc, (void *)this);
				pthread_detach(other_id);
				m_is_set_other_thread=true;
#else			
				if (!other_thread.joinable())
				{
					std::thread t(std::bind(&io_service_gh::set_other_io_thread, this));
					other_thread.swap(t);
				}
#endif
			}
		}

		void io_service_gh::init_other_io(int thread_num)
		{
			if (thread_num < 1 || thread_num>8)
				thread_num = 1;
			other_io = new boost::asio::io_service(thread_num);
		}

		void io_service_gh::set_feed_io_thread()
		{
#ifdef Linux
			//int feed_io_cpu_core = ats_config::get_instance()->get_feed_io_cpu_core();
			int epoll_time_t;
			if(feed_io_cpu_core>0)
			{
				epoll_time_t =0;
				cpu_set_t mask;
				cpu_set_t get;
				CPU_ZERO(&mask);
				CPU_SET(feed_io_cpu_core, &mask);
				int num = sysconf(_SC_NPROCESSORS_CONF);
				if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) //设置亲和性
				{
					fprintf(stderr, "set thread affinity failed\n");
				}
				CPU_ZERO(&get);
				if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
				{
					fprintf(stderr, "get thread affinity failed\n");

				}
				for (int j = 0; j < num; j++)
				{
					if (CPU_ISSET(j, &get))
					{
						printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
					}
				}
			}
			else
			{
				cout << "feed_io_cpu_core is " << feed_io_cpu_core << " fail to bind cpu core" << endl;
				epoll_time_t = 1;
			}

			efd_feed = epoll_create(MaxEPOLLSize);
			if (efd_feed == -1)
			{
				printf("feed epoll_create fail");
				exit(1);
			}

			for(auto &it:feed_fdlist)
			{
				struct epoll_event epe;
				epe.data.fd = it;
				epe.events = EPOLLIN | EPOLLET;
				int res = -1;
				res = epoll_ctl(efd_feed, EPOLL_CTL_ADD, it, &epe);
				if (res == -1)
				{
					printf("feed epoll ctl fail,exit");
					exit(1);
				}
			}

			epoll_proc(efd_feed, epoll_time_t,true);
#else
			cout << "start feed_io" << endl;
			feed_io.run();
#endif
		}

		void io_service_gh::set_trader_io_thread()
		{
#ifdef Linux
			//int trader_io_cpu_core = ats_config::get_instance()->get_trader_io_cpu_core();
			int epoll_time_t;
			if(trader_io_cpu_core>0)
			{
				epoll_time_t = 0;
				cpu_set_t mask;
				cpu_set_t get;
				CPU_ZERO(&mask);
				CPU_SET(trader_io_cpu_core, &mask);
				int num = sysconf(_SC_NPROCESSORS_CONF);
				if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) //设置亲和性
				{
					fprintf(stderr, "set thread affinity failed\n");
				}
				CPU_ZERO(&get);
				if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
				{
					fprintf(stderr, "get thread affinity failed\n");

				}
				for (int j = 0; j < num; j++)
				{
					if (CPU_ISSET(j, &get))
					{
						printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
					}
				}
			}
			else
			{
				cout << "trader_io_cpu_core is " << trader_io_cpu_core << " fail to bind cpu core" << endl;
				epoll_time_t = 1;
			}

			efd_trader = epoll_create(MaxEPOLLSize);
			if (efd_trader == -1)
			{
				printf("trader epoll_create fail");
				exit(1);
			}

			for (auto &it : trader_fdlist)
			{
				struct epoll_event epe;
				epe.data.fd = it;
				epe.events = EPOLLIN | EPOLLET;
				int res = -1;
				res = epoll_ctl(efd_trader, EPOLL_CTL_ADD, it, &epe);
				if (res == -1)
				{
					printf("trader epoll ctl fail,exit");
					exit(1);
				}
			}

			epoll_proc(efd_trader, epoll_time_t);
#else
			cout << "start trader_io" << endl;
			trader_io.run();
#endif
		}

		void io_service_gh::set_other_io_thread()
		{
#ifdef Linux
			//int other_io_cpu_core = ats_config::get_instance()->get_other_io_cpu_core();
			if(other_io_cpu_core>0)
			{
				cpu_set_t mask;
				cpu_set_t get;
				CPU_ZERO(&mask);
				CPU_SET(other_io_cpu_core, &mask);
				int num = sysconf(_SC_NPROCESSORS_CONF);
				if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) //设置亲和性
				{
					fprintf(stderr, "set thread affinity failed\n");
				}
				CPU_ZERO(&get);
				if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
				{
					fprintf(stderr, "get thread affinity failed\n");

				}
				for (int j = 0; j < num; j++)
				{
					if (CPU_ISSET(j, &get))
					{
						printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
					}
				}
			}
			else
			{
				cout << "other_io_cpu_core is " << other_io_cpu_core << " fail to bind cpu core" << endl;
			}

			efd_other = epoll_create(MaxEPOLLSize);
			if (efd_other == -1)
			{
				printf("other epoll_create fail");
				exit(1);
			}

			for (auto &it : other_fdlist)
			{
				struct epoll_event epe;
				epe.data.fd = it;
				epe.events = EPOLLIN | EPOLLET;
				int res = -1;
				res = epoll_ctl(efd_other, EPOLL_CTL_ADD, it, &epe);
				if (res == -1)
				{
					printf("other epoll ctl fail,exit");
					exit(1);
				}
				//cout<<"set fd "<<it<<" to other_epoll_fd"<<endl;
			}
			is_ats_running = true;
			epoll_proc(efd_other, 1);
			return;
#endif

//#else
			cout << "start other_io" << endl;
			if (is_ats_running == false)
			{
				is_ats_running = true;
				other_io->run();
			}
//#endif
		}
		
		boost::asio::io_service *io_service_gh::get_io_service(io_service_type _type)
		{
			switch (_type)
			{
			case io_service_type::feed:
				return &feed_io;
			case io_service_type::trader:
				return &trader_io;
			case io_service_type::other:
			default:
				return other_io;
			}
		}

#ifdef Linux
		void io_service_gh::epoll_proc(int efd,int timeout,bool is_feed)
		{
			struct epoll_event *events = new epoll_event[MaxEPOLLSize];
			while (1)
			{
				int n = epoll_wait(efd, events, MaxEPOLLSize, timeout);
				for (int i = 0; i<n; ++i)
				{
					if (events[i].events & EPOLLHUP||events[i].events & EPOLLERR)
					{
						printf("epoll eventfd has epoll hup or error.\n");
						exit(1);
					}
					else if (events[i].events & EPOLLIN)
					{
						int fd = events[i].data.fd;
						uint64_t buf;
						ssize_t readNum;

						while(1)
						{
							readNum = read(fd, &buf, sizeof(uint64_t));
							if(readNum == sizeof(uint64_t))
								break;
							else
							{
								if(errno == EAGAIN)// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读 在这里就当作是该次事件已处理处.
								{
									break;
								}
								else if (errno ==EINTR)// 被信号中断
								{
									printf("read eventfd:EINTR");
									continue;
								}
								else if (errno == EINVAL)
								{
									printf("read eventfd:EINVAL");
										exit(1);
								}
							}
						}
						//cout<<"fd:"<<fd<<" call fun"<<endl;
						auto it = ProcMap.find(fd);
						if(it!=ProcMap.end())
						{
							it->second();
						}
						else
						{
							loggerv2::error("can not find fd :%d int procMap",fd);
						}
					}

				}

				if (is_feed)
				{
					for (auto &it : nanomsg_rcv_list)
						it();
				}
			}
		}

		void io_service_gh::add_fd_fun_map(io_service_type _type, int fd, epoll_handler handler)
		{
			ProcMap.emplace(fd, handler);
			struct epoll_event epe;
			epe.data.fd = fd;
			epe.events = EPOLLIN | EPOLLET;
			int res = 0;
			switch (_type)
			{
			case io_service_type::feed:
				if(efd_feed>0)
					res = epoll_ctl(efd_feed, EPOLL_CTL_ADD, fd, &epe);
				else
					feed_fdlist.push_back(fd);
				break;
			case io_service_type::trader:
				if(m_is_bind_feed_trader_core==false)
				{
					if(efd_trader>0)
						res = epoll_ctl(efd_trader, EPOLL_CTL_ADD, fd, &epe);
					else
						trader_fdlist.push_back(fd);
				}
				else
				{
					printf("bind trade core to feed\n");
					if(efd_feed>0)
						res = epoll_ctl(efd_feed, EPOLL_CTL_ADD, fd, &epe);
					else
						feed_fdlist.push_back(fd);
				}
				break;
			case io_service_type::other:
			default:
				if(efd_other>0)
					res = epoll_ctl(efd_other, EPOLL_CTL_ADD, fd, &epe);
				else
					other_fdlist.push_back(fd);
				break;
			}

			if (res == -1)
			{
				printf("epoll ctl fail,exit");
				exit(1);
			}
		}


		void io_service_gh::add_nanomsg_rcv_handler(epoll_handler handler)
		{
			nanomsg_rcv_list.push_back(handler);
		}
#endif
	}
}
