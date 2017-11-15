#include "sl2_source.h"
#include "nn.h"
#include "pubsub.h"
namespace feed
{
	namespace sl2
	{		
		sl2_source * g_sl2_source = nullptr;

		void  callback_efh_quote(struct sl_efh_quote* p, const struct guava_udp_normal* p_quote)
		{
#if 0
			printf("recv data req:\t%s:%d\t%d\t%s\t%s\t%d\t%f\n",
				p->m_mc_ip, p->m_mc_port,
				p_quote->m_sequence,
				p_quote->m_symbol,
				p_quote->m_update_time,
				p_quote->m_quote_flag,
				p_quote->m_last_px
				);
#endif
			if (g_sl2_source)
			{
				//
				g_sl2_source->publish_msg((void*)p_quote, sizeof(guava_udp_normal), p_quote->m_symbol);
				//
				std::string  sFeedCode  = std::string(p_quote->m_symbol);
				feed_item* afeed_item   = g_sl2_source->get_feed_item(sFeedCode);
				if (afeed_item != nullptr)
				{
					if (g_sl2_source->get_strPub() != "pub")
						g_sl2_source->get_queue()->CopyPush((guava_udp_normal *)p_quote);
			}
		}
		}

		sl2_source::sl2_source(const string & sourceName, const string & hostname, const string & service, const string & db, const string & ethName, string pub, string url, string req_url)
			: feed_source("SL2", sourceName, hostname, service, "", "", "", db, pub, url,req_url)
		{
			m_pSlEfh     = nullptr;
			m_strEthName = ethName;
			g_sl2_source = this;
		}
		sl2_source::~sl2_source()
		{
			release_source();
		}
		void sl2_source::init_source()
		{			
			//
			feed_source::init_source();
			//

			get_queue()->setHandler(boost::bind(&sl2_source::process_msg, this,_1));			
#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif
			load_database();
			
			if (setUDPSockect(get_service_name().c_str(), atoi(get_port().c_str())))
			{
				update_state(AtsType::FeedSourceStatus::Up, "");
			}
		}		

#ifdef Linux
		void  sl2_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "sl2 efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&sl2_source::process, this));
			m_queue.set_fd(efd);
		}
#endif
		bool sl2_source::setUDPSockect(const char * pBroadcastIP, int nBroadcastPort)
		{
			//
			if (this->is_sub() == true)
				return false;
			//
			m_pSlEfh = (struct sl_efh_quote*) sl_create_efh_sf_api(m_strEthName.c_str(),pBroadcastIP,nBroadcastPort);

			int ret = sl_start_etf_quote(m_pSlEfh, callback_efh_quote);
			
			printf_ex("sl2_source::setUDPSockect ret:%d\n",ret);

			return (SL_EFH_NO_ERROR==ret);
		}		
		void sl2_source::release_source()
		{
			feed_source::release_source();
			if (m_pSlEfh)
			{
				sl_stop_efh_quote(m_pSlEfh);
				m_pSlEfh = nullptr;
			}
			g_sl2_source = nullptr;
		}
		//
		void sl2_source::start_receiver()
		{
			guava_udp_normal msg;
			int rc = -1;
			//while (true)
			{
				rc = nn_recv(m_sub_handle, &msg, sizeof(msg), 0);
				if (rc == sizeof(msg))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					get_queue()->CopyPush(&msg);
				}
			}
		}
		//
		void sl2_source::process_msg(guava_udp_normal* pMsg)
		{			
			std::string sFeedCode = std::string(pMsg->m_symbol);
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("lts_source:process_msg instrument %s not found", pMsg->m_symbol);
				return;
			}
			//
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		void sl2_source::process()
		{
			get_queue()->Pops_Handle();
		}		
		void sl2_source::process_msg(guava_udp_normal* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;

			if (math2::not_zero(pMsg->m_bid_px) && math2::not_zero(pMsg->m_ask_px))
			{
			process_depth(0, pMsg->m_bid_share, pMsg->m_bid_px != NO_PRICE ? pMsg->m_bid_px : 0., pMsg->m_ask_px != NO_PRICE ? pMsg->m_ask_px : 0., pMsg->m_ask_share, feed_item);
			}
				
			double lastPrice = pMsg->m_last_px != NO_PRICE ? pMsg->m_last_px : 0;
			if (math2::not_zero(lastPrice))
				feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->m_last_share;
			if(dailyVolume>0)
				feed_item->set_daily_volume(dailyVolume);
			if (pMsg->m_total_value > 0)
				feed_item->set_turnover(pMsg->m_total_value);
		}
		bool sl2_source::subscribe(feed_item * feed_item)
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
		}		
	}
}
