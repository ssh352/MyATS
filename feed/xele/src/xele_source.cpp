#include "xele_source.h"
#include "nn.h"
#include "pubsub.h"
namespace feed
{
	namespace xele
	{		

		void* job_recv_market_data(void* arg)
		{
			xele_source *ptr = (xele_source*)arg;
			CXeleMdApi* api = ptr->m_Api;

			int cpu_core = ptr->get_code();
			if (cpu_core>0)
			{
#ifdef __linux__
				cpu_set_t mask;
				cpu_set_t get;
				CPU_ZERO(&mask);
				CPU_SET(cpu_core, &mask);
				int num = sysconf(_SC_NPROCESSORS_CONF);
				if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) //ÉèÖÃÇ×ºÍÐÔ
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
				cout << "xele cpu_core is" << cpu_core << " fail to bind cpu core" << endl;
#endif
			}
		
			int handle = api->GetHandle();
			CXeleShfeMarketDataUnion mdtick;
			char  *sFeedCode = nullptr;
			while (1)
			{
				if (RecvShfeMarketDataTick(handle, &mdtick))
				{
					//g_xele_source->publish_msg((void*)(&mdtick.data), sizeof(CXeleShfeMarketDataUnion), mdtick.data.InstrumentID);
					
					if (mdtick.md_type[0] == 'M') 
					{
						if (mdtick.type_high.LastPrice == 0)
							continue;

						sFeedCode = mdtick.type_high.Instrument;

						if (strcmp(sFeedCode,"rb1801")==0)
						{
							std::chrono::time_point<std::chrono::high_resolution_clock> tp = std::chrono::high_resolution_clock::now();
							long long timep = std::chrono::time_point_cast<lwdur>(tp).time_since_epoch().count();
							BOOST_LOG_TRIVIAL(info) << "xele rb1801:" << mdtick.type_high.Turnover<<" tp:"<<timep;
						}
						//cout<<"code:"<<sFeedCode<<" Lastprice:"<<mdtick.type_high.LastPrice<<endl;
					}
                    else if (mdtick.md_type[0] == 'S') 
			        {
						continue;
						sFeedCode = mdtick.type_low.Instrument;
						//cout<<"code:"<<sFeedCode<<" Lastprice:"<<mdtick.type_low.ClosePrice<<endl;
                    }
                    else if (mdtick.md_type[0] == 'Q') 
					{
						continue;
						sFeedCode = mdtick.type_depth.Instrument;
						//cout<<"code:"<<sFeedCode<<" Lastprice:"<<mdtick.type_depth.Price1<<endl;
                    }
					std::string str = std::move(sFeedCode);
					feed_item* afeed_item = ptr->get_feed_item(str);
					if (afeed_item != nullptr)
					{
						if (ptr->get_strPub() != "pub")
							ptr->get_queue()->CopyPush((CXeleShfeMarketDataUnion *)&mdtick);
					}
				}
			}
		}

		xele_source::xele_source(const string & sourceName, const string & frontAddr, const string & BCAddr, string &ip, string &port, const string & UserID, const string & Passwd,
			const string & ethName, string &db, int mcore, string pub, string url, string req_url)
			: feed_source("XELE", sourceName, ip, port, "", "", "", db, pub, url,req_url)
		{
			m_Api = nullptr;
			m_strFrontAddr = frontAddr;
			m_strBCAddr = BCAddr;
			m_strEthName = ethName;
			m_strUserID = UserID;
			m_strPasswd = Passwd;

			m_core = mcore;
			
		}

		xele_source::~xele_source()
		{
			release_source();
		}

		void xele_source::init_source()
		{
			feed_source::init_source();
			get_queue()->setHandler(boost::bind(&xele_source::process_msg, this,_1));			
#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif
			load_database();
			
			if (Connect_and_Login())
			{
				update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				std::cout<<"xele login error"<<endl;
				exit(1);
			}
			
		}		

#ifdef Linux
		void  xele_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "xele efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&xele_source::process, this));
			m_queue.set_fd(efd);
		}
#endif

		bool xele_source::Connect_and_Login()
		{
			m_Spi.ptr = this;
			m_Api = CXeleMdApi::CreateMdApi(&m_Spi);

			CXeleMdFtdcReqUserLoginField login_info;
			memset((void *)&login_info, 0, sizeof(login_info));
			strcpy(login_info.UserID, m_strUserID.data());
			strcpy(login_info.Password, m_strPasswd.data());
			strcpy(login_info.ProtocolInfo, "protocol");

			int status = m_Api->LoginInit(m_strFrontAddr.data(), m_strBCAddr.data(), m_strEthName.data(), &login_info);

			switch (status)
			{
			case XELEAPI_SUCCESS:
				cout << "LoginInit:\"XELEAPI_SUCCESS\"" << endl;
				break;
			case XELEAPI_BAD_USERID_OR_PASSWORD:
				m_Api->Release();
				cout << "LoginInit:\"Bad UserID or Password\"" << endl;
				return false;
			case XELEAPI_CHECKSUM_FAIL:
				m_Api->Release();
				cout << "LoginInit:\"API Checksum Fail\"" << endl;
				return false;
			}
#ifdef __linux__
			pthread_create(&xele_id, NULL, job_recv_market_data, (void *)this);
			pthread_detach(xele_id);
#endif

			return true;
		}

		void xele_source::release_source()
		{
			feed_source::release_source();
			if (m_Api!=nullptr)
			{
				m_Api->Release();
			}

		}

		void xele_source::start_receiver()
		{
			CXeleShfeMarketDataUnion msg;
			int rc = -1;
			//while (true)
			{
				rc = nn_recv(m_sub_handle, &msg, sizeof(msg), 0);
				if (rc == sizeof(msg))
				{
					get_queue()->CopyPush(&msg);
				}
			}
		}

		void xele_source::process_msg(CXeleShfeMarketDataUnion* pMsg)
		{			
			std::string  sFeedCode;
			if (pMsg->md_type[0] == 'M')
			{
				sFeedCode = pMsg->type_high.Instrument;
			}
			else if (pMsg->md_type[0] == 'S')
			{
				sFeedCode = pMsg->type_low.Instrument;
			}
			else if (pMsg->md_type[0] == 'Q')
			{
				sFeedCode = pMsg->type_depth.Instrument;
			}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				return;
			}

			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}

		void xele_source::process()
		{
			get_queue()->Pops_Handle();
		}	

		void xele_source::process_msg(CXeleShfeMarketDataUnion* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;
			if (pMsg->md_type[0] != 'M')
				return;
			//pMsg->type_high.

			
			if (math2::not_zero(pMsg->type_high.BidPrice) && math2::not_zero(pMsg->type_high.AskPrice))
			{
				process_depth(0, pMsg->type_high.BidVolume, pMsg->type_high.BidPrice != NO_PRICE ? pMsg->type_high.BidPrice : 0., pMsg->type_high.AskPrice != NO_PRICE ? 
				pMsg->type_high.AskPrice : 0., pMsg->type_high.AskVolume, feed_item);
			}
				
			double lastPrice = pMsg->type_high.LastPrice != NO_PRICE ? pMsg->type_high.LastPrice : 0;
			if (math2::not_zero(lastPrice))
				feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->type_high.Volume;
			if(dailyVolume>0)
				feed_item->set_daily_volume(dailyVolume);
			if (pMsg->type_high.Turnover > 0)
				feed_item->set_turnover(pMsg->type_high.Turnover);
		}

		/*bool xele_source::subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || get_status() == AtsType::FeedSourceStatus::Down)
			{
				loggerv2::error("subscribe feed fail,feed_itme invailable,or feedsourceStatus:%s is %d", Name.c_str(), get_status());
				return false;
			}
			if (feed_item->is_subsribed() == true)
				return true;
			add_feed_item(feed_item);
			feed_item->subscribe();
			return true;
		}*/	

		void FeedXeleSpi::OnFrontDisconnected(int nReason)
		{
			(cout) << "<" << __FUNCTION__ << ">" << " Errcode:" << nReason << endl;
			if (ptr != nullptr)
				ptr->update_state(AtsType::FeedSourceStatus::Down, "");
		}
	}
}
