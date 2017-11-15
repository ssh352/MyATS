#include "ltsudp_source.h"
#include "fastmd2.h"
#include <nn.h>
#include <pubsub.h>
using namespace terra::common;
namespace feed
{
	namespace ltsudp
	{		
		ltsudp_source::ltsudp_source(const string & sourceName, const string & hostname, const string & service, const string & db, string pub, string url, string req_url)
			: feed_source("LTSUDP", sourceName, hostname, service, "", "", "", db, pub, url,req_url)
		{
	
		}

		ltsudp_source::~ltsudp_source()
		{
			release_source();
		}


		void ltsudp_source::init_source()
		{
			//
			feed_source::init_source();
			//
			get_queue()->setHandler(boost::bind(&ltsudp_source::process_msg, this,_1));

			get_l2_queue()->setHandler(boost::bind(&ltsudp_source::process_l2_msg, this, _1));
			
			//std::thread t(std::bind(&ltsudp_source::set_kernel_timer_thread, this));
			//m_thread.swap(t);
			init_process(io_service_type::feed);
			load_database();
			//abstract_database* db = database_factory::create("sqlite", get_database_name().c_str());

			//if (db->open_database())
			//{
			//	loggerv2::info("connect to database %s", get_database_name().c_str());
			//}

			//std::array<std::string, 4> sInst = { "Futures", "Stocks", "Options", "ETFs" };

			//for (auto s : sInst)
			//{
			//	std::string sCmd = "select Code, FeedCodes from " + s + " where FeedCodes like '%@LTSUDP%' ";
			//	std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());

			//	for (std::vector<boost::property_tree::ptree>::iterator it = res->begin(); it != res->end(); ++it)
			//	{
			//		std::string temp = abstract_database::get_item((*it).get("FeedCodes", ""), "LTSUDP");
			//		std::string code = (*it).get("Code", "");
			//		m_feedCode2CodeMap.insert(make_pair(temp, code));
			//		m_code2FeedCodeMap.insert(make_pair(code, temp));
			//	}
			//	delete res;
			//}

			//db->close_databse();
			//loggerv2::info("ltsudp_source - %d instruments loaded from database", m_feedCode2CodeMap.size());

			//
			/*RTThread::Create();*/



			if (setUDPSockect(get_service_name().c_str(), atoi(get_port().c_str())))
			{
				update_state(AtsType::FeedSourceStatus::Up, "");
				//process();
				m_tListhen2Udp = std::thread(&ltsudp_source::Listhen2Udp, this);
			}


		}


		void ltsudp_source::Listhen2Udp()
		{
			loggerv2::info("ltsudp_source process");

			sockaddr_in		SrcAddr;
#ifdef WIN32
			int				nAddrLen;
#else
			socklen_t		nAddrLen;
#endif
			int				nCount;
			char			RcvBuff[2048];
			//CFAST_MD *		pFastMD;
			int 			nOffset;
			int				nHeadLen;
			unsigned short	nMDLen;

			nHeadLen = sizeof(nMDLen);
			while (is_alive())
			{
				nAddrLen = sizeof(sockaddr_in);
				nCount = recvfrom(m_UDPSockID, RcvBuff, 2048, 0, (sockaddr *)&SrcAddr, &nAddrLen);
				if (nCount > nHeadLen)
				{
					memcpy_lw(&nMDLen, RcvBuff, nHeadLen);
#if 0
					if ((nCount - nHeadLen) % nMDLen != 0)
					{
						printf("Invalid Fast Market Data\n");
						continue;
					}
#endif
					//printf("Receive[%d] From [%s][%d]\n\n", nCount, inet_ntoa(SrcAddr.sin_addr), ntohs(SrcAddr.sin_port));
					nOffset = nHeadLen;
					bool level2 = false;
					bool skip   = false;
					if (strncmp(RcvBuff + nOffset, "MD", 2) == 0)
					{
						level2 = true;
						nOffset += 2;
					}
					else if (strncmp(RcvBuff + nOffset, "ID", 2) == 0 || strncmp(RcvBuff + nOffset, "OD", 2) == 0 || strncmp(RcvBuff + nOffset, "TD", 2) == 0)
					{
						level2 = true;
						nOffset += 2;
						skip = true;
					}					
					while (skip==false && nOffset < nCount)
					{	
						if (level2 == false)
						{
							CFAST_MD* nFastMd = new CFAST_MD();
							memcpy_lw(nFastMd, RcvBuff + nOffset, sizeof(CFAST_MD));
							//
							this->publish_msg((void*)nFastMd, sizeof(CFAST_MD), nFastMd->InstrumentID);
							//
							//printf_ex("level1:Instrument[%s] Buy[%f] Sell[%f] \n", nFastMd->InstrumentID, nFastMd->BuyPrice1, nFastMd->SellPrice1); //todo comment this line							
							m_queue.Push(nFastMd);
						}
						else
						{
							CL2FAST_MD* nFastMd = new CL2FAST_MD();
							memcpy_lw(nFastMd, RcvBuff + nOffset, sizeof(CL2FAST_MD));
							//
							this->publish_msg((void*)nFastMd, sizeof(CL2FAST_MD), nFastMd->InstrumentID);
							//
							//printf_ex("level2:Instrument[%s] Bid[%f] Offer[%f] \n", nFastMd->InstrumentID,nFastMd->BidPrice1,nFastMd->OfferPrice1); //todo comment this line
							m_l2_queue.Push(nFastMd);
						}
						nOffset += nMDLen;
					}
				}
			}
		}



		bool ltsudp_source::setUDPSockect(const char * pBroadcastIP, int nBroadcastPort)
		{
			//
			if (this->is_sub() == true)
				return false;
			//
			int				nFlag = 1;
#ifdef WIN32
			unsigned long   uFlag = 1;
#endif
			sockaddr_in		ListenAddr;

#ifdef WIN32
			WSADATA			InitData = { 0 };
			//unsigned long	uFlag = 1;

			if (0 != WSAStartup(MAKEWORD(2, 2), &InitData)) {
				loggerv2::error("Failed to Init Network[%d]\n", WSAGetLastError());
				return false;
			}
#endif

			m_UDPSockID = socket(AF_INET, SOCK_DGRAM, 0);
			if (0 > m_UDPSockID) {
				loggerv2::error("Failed to Create Socket\n");
				return false;
			}

			if (0 != setsockopt(m_UDPSockID, SOL_SOCKET, SO_REUSEADDR, (char *)&nFlag, sizeof(nFlag))) {
				loggerv2::error("Failed to Reuse Address\n");
				return false;
			}
#ifdef WIN32
			if (ioctlsocket(m_UDPSockID, FIONBIO, &uFlag) < 0) {
#else
			if (ioctl(m_UDPSockID, FIONBIO, &nFlag) < 0) {
#endif
				loggerv2::error("Failed to Set FIONBIO\n");
				return false;
			}

			nFlag = 1024 * 1024;
			if (0 != setsockopt(m_UDPSockID, SOL_SOCKET, SO_RCVBUF, (const char *)&nFlag, sizeof(nFlag))) {
				loggerv2::error("Failed to Set Receive Buffer Size\n");
				return false;
			}

			memset(&ListenAddr, 0x00, sizeof(ListenAddr));
			ListenAddr.sin_family = AF_INET;
			ListenAddr.sin_port = htons(nBroadcastPort);
#ifdef WIN32
			ListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
#else
			ListenAddr.sin_addr.s_addr = inet_addr(pBroadcastIP);
#endif

			if (0 != ::bind(m_UDPSockID, (sockaddr *)&ListenAddr, sizeof(ListenAddr))) {
				loggerv2::error("Failed to Bind Socket\n");
				return false;
			}

#ifdef WIN32
			char	Broad = 1;
#else
			int		Broad = 1;
#endif
			if (0 != setsockopt(m_UDPSockID, SOL_SOCKET, SO_BROADCAST, &Broad, sizeof(Broad))) {
				loggerv2::error("Failed to Set Broadcast\n");
				return false;
			}

			return true;
		}




		void ltsudp_source::release_source()
		{
			feed_source::release_source();
			m_tListhen2Udp.join();
		}

		void ltsudp_source::start_receiver()
		{
			char buffer[1024];
			int rc = -1;
			//while (true)
			{
				memset(buffer, 0, sizeof(buffer));
				rc = nn_recv(m_sub_handle, buffer, sizeof(buffer), 0);
				if (rc == sizeof(CFAST_MD))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);	
					CFAST_MD* nFastMd = new CFAST_MD();
					memcpy_lw(nFastMd, buffer, sizeof(CFAST_MD));
					m_queue.Push((CFAST_MD*)nFastMd);
				}
				else if (rc == sizeof(CL2FAST_MD))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);		
					CL2FAST_MD* nFastMd = new CL2FAST_MD();
					memcpy_lw(nFastMd, buffer, sizeof(CL2FAST_MD));
					m_l2_queue.Push((CL2FAST_MD*)nFastMd);
				}
			}
		}
		void ltsudp_source::process_msg(CFAST_MD* pMsg)
		{			
			std::string sFeedCode = std::string(pMsg->InstrumentID);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::info("lts_cource cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("lts_source:process_msg instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}

		void ltsudp_source::process()
		{
			get_queue()->Pops_Handle();
			get_l2_queue()->Pops_Handle();
		}

		//void  ltsudp_source::set_kernel_timer_thread()
		//{
		//	boost::asio::high_resolution_timer t(io, std::chrono::microseconds(20));
		//	t.async_wait(boost::bind(&ltsudp_source::process, this, boost::asio::placeholders::error, &t));
		//	io.run();
		//}

		//void ltsudp_source::process(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		//{
		//	//int i = 0;
		//	while (true)
		//	{
		//		/*if (get_queue()->m_queue.read_available() > 0)
		//		{
		//			for(auto &func: get_queue()->m_handler)
		//			{
		//				func();
		//			}
		//		}*/
		//		//while (get_queue()->Pop_Handle())
		//		//++i;
		//		get_queue()->Pops_Handle();
		//		//if (i >= 10)
		//		{
		//			//i = 0;
		//			t->expires_at(t->expires_at() + std::chrono::microseconds(20));
		//			t->async_wait(boost::bind(&ltsudp_source::process, this, boost::asio::placeholders::error, t));
		//			return;
		//		}
		//	}
		//}
		//int ltsudp_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		CFAST_MD* msg = get_queue()->Pop();
		//		this->process_msg(msg);
		//	}
		//	return i;
		//}
		void ltsudp_source::process_msg(CFAST_MD* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;
			process_depth(0, pMsg->BuyVolume1, pMsg->BuyPrice1 != NO_PRICE ? pMsg->BuyPrice1 : 0., pMsg->SellPrice1 != NO_PRICE ? pMsg->SellPrice1 : 0., pMsg->SellVolume1, feed_item);
			process_depth(1, pMsg->BuyVolume2, pMsg->BuyPrice2 != NO_PRICE ? pMsg->BuyPrice2 : 0., pMsg->SellPrice2 != NO_PRICE ? pMsg->SellPrice2 : 0., pMsg->SellVolume2, feed_item);
			process_depth(2, pMsg->BuyVolume3, pMsg->BuyPrice3 != NO_PRICE ? pMsg->BuyPrice3 : 0., pMsg->SellPrice3 != NO_PRICE ? pMsg->SellPrice3 : 0., pMsg->SellVolume3, feed_item);
			process_depth(3, pMsg->BuyVolume4, pMsg->BuyPrice4 != NO_PRICE ? pMsg->BuyPrice4 : 0., pMsg->SellPrice4 != NO_PRICE ? pMsg->SellPrice4 : 0., pMsg->SellVolume4, feed_item);
			process_depth(4, pMsg->BuyVolume5, pMsg->BuyPrice5 != NO_PRICE ? pMsg->BuyPrice5 : 0., pMsg->SellPrice5 != NO_PRICE ? pMsg->SellPrice5 : 0., pMsg->SellVolume5, feed_item);

			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || math2::is_zero(pMsg->ClosePrice))) ? pMsg->ClosePrice : 0;
			if (math2::not_zero(closePrice))
				feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->TradeVolume;
			feed_item->set_daily_volume(dailyVolume);
			feed_item->set_turnover(pMsg->TradeValue);
			feed_item->set_theoretical_open_price(pMsg->AuctionPrice);
			//double hightest = pMsg->HighPrice;

			//double lowest = pMsg->LowPrice;
		}
		bool ltsudp_source::subscribe(feed_item * feed_item)
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
		void ltsudp_source::process_l2_msg(CL2FAST_MD* pMsg)
		{
			std::string sFeedCode = std::string(pMsg->InstrumentID);
			//std::string instrName = get_code_by_feedcode(sFeedCode);
			//if (instrName.empty())
			//{
			//	loggerv2::info("lts_cource cannot find Code for FeedCode %s.", sFeedCode.c_str());
			//	return;
			//}
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == NULL)
			{
				//loggerv2::info("lts_source:process_l2_msg instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_l2_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		void ltsudp_source::process_l2_msg(CL2FAST_MD* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;
			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->OfferPrice1 != NO_PRICE ? pMsg->OfferPrice1 : 0., pMsg->OfferVolume1, feed_item);
			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->OfferPrice2 != NO_PRICE ? pMsg->OfferPrice2 : 0., pMsg->OfferVolume2, feed_item);
			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->OfferPrice3 != NO_PRICE ? pMsg->OfferPrice3 : 0., pMsg->OfferVolume3, feed_item);
			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->OfferPrice4 != NO_PRICE ? pMsg->OfferPrice4 : 0., pMsg->OfferVolume4, feed_item);
			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->OfferPrice5 != NO_PRICE ? pMsg->OfferPrice5 : 0., pMsg->OfferVolume5, feed_item);

			process_depth(5, pMsg->BidVolume6, pMsg->BidPrice6 != NO_PRICE ? pMsg->BidPrice6 : 0., pMsg->OfferPrice6 != NO_PRICE ? pMsg->OfferPrice6 : 0., pMsg->OfferVolume6, feed_item);
			process_depth(6, pMsg->BidVolume7, pMsg->BidPrice7 != NO_PRICE ? pMsg->BidPrice7 : 0., pMsg->OfferPrice7 != NO_PRICE ? pMsg->OfferPrice7 : 0., pMsg->OfferVolume7, feed_item);
			process_depth(7, pMsg->BidVolume8, pMsg->BidPrice8 != NO_PRICE ? pMsg->BidPrice8 : 0., pMsg->OfferPrice8 != NO_PRICE ? pMsg->OfferPrice8 : 0., pMsg->OfferVolume8, feed_item);
			process_depth(8, pMsg->BidVolume9, pMsg->BidPrice9 != NO_PRICE ? pMsg->BidPrice9 : 0., pMsg->OfferPrice9 != NO_PRICE ? pMsg->OfferPrice9 : 0., pMsg->OfferVolume9, feed_item);
			process_depth(9, pMsg->BidVolumeA, pMsg->BidPriceA != NO_PRICE ? pMsg->BidPriceA : 0., pMsg->OfferPriceA != NO_PRICE ? pMsg->OfferPriceA : 0., pMsg->OfferVolumeA, feed_item);

			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || math2::is_zero(pMsg->ClosePrice))) ? pMsg->ClosePrice : 0;
			if (math2::not_zero(closePrice))
				feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->TotalTradeVolume;
			feed_item->set_daily_volume(dailyVolume);

			//double hightest = pMsg->HighPrice;

			//double lowest = pMsg->LowPrice;
		}
	}

}