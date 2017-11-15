#include "ltscffexudp_source.h"

//#include <stdio.h>

#include <nn.h>
#include <pubsub.h>
#ifndef WIN32
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
using namespace terra::common;
namespace feed
{
	namespace ltscffexudp
	{
		ltscffexudp_source::ltscffexudp_source(const string & sourceName, const string & hostname, const string & service, const string & db, string pub, string url,string req_url)
			: feed_source("LTSCFFEXUDP", sourceName, hostname, service, "", "", "", "", pub, url,req_url)
		{

		}

		ltscffexudp_source::~ltscffexudp_source()
		{
			release_source();
		}


		void ltscffexudp_source::init_source()
		{
			//
			feed_source::init_source();
			//

			get_queue()->setHandler(boost::bind(&ltscffexudp_source::process_depthMarketDataField, this, _1));
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
			//	std::string sCmd = "select Code, FeedCodes from " + s + " where FeedCodes like '%@LTSCFFEXUDP%' ";
			//	std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());

			//	for (std::vector<boost::property_tree::ptree>::iterator it = res->begin(); it != res->end(); ++it)
			//	{
			//		std::string temp = abstract_database::get_item((*it).get("FeedCodes", ""), "LTSCFFEXUDP");
			//		std::string code = (*it).get("Code", "");
			//		m_feedCode2CodeMap.insert(make_pair(temp, code));
			//		m_code2FeedCodeMap.insert(make_pair(code, temp));
			//	}
			//	delete res;
			//}

			//db->close_databse();
			//loggerv2::info("ltscffexudp_source - %d instruments loaded from database", m_feedCode2CodeMap.size());

			//
			/*RTThread::Create();*/

			//std::thread t(std::bind(&ltscffexudp_source::process, this));
			//t.detach();

			if (setUDPSockect(get_service_name().c_str(), atoi(get_port().c_str())))
			{
				update_state(AtsType::FeedSourceStatus::Up, "");
				//process();
				m_tListhen2Udp = std::thread(&ltscffexudp_source::listhen2udp, this);
			}


		}


		void ltscffexudp_source::listhen2udp()
		{
			loggerv2::info("ltscffexudp_source::process");
			sockaddr_in		SrcAddr;
#ifdef WIN32
			int				nAddrLen;
#else
			socklen_t		nAddrLen;
#endif
			int				nCount;
			char			RcvBuff[4 * 1024];
			TFTDCHeader                header;
			TFieldHeader               fieldHeader;
			int         nHeadLen = sizeof(TFTDCHeader);
			while (is_alive())
			{
				nAddrLen = sizeof(sockaddr_in);
				nCount = recvfrom(m_UDPSockID, RcvBuff, 2048, 0, (sockaddr *)&SrcAddr, &nAddrLen);
				if (nCount > nHeadLen)
				{
					//1.get the TFTDCHeader
					memset(&header, 0, sizeof(TFTDCHeader));
					memcpy_lw(&header, RcvBuff, sizeof(TFTDCHeader));

					///校验一下包是否正确
					if (header.Chain != 'L' && header.Chain != 'C')
					{
						//loggerv2::error("ltscffexudp_source::process--Invalid Package!,Invalid Header[%c]\n", header.Chain);
						continue;
					}
					if (header.FTDCContentLength != (nCount - sizeof(TFTDCHeader)))
					{
						loggerv2::error("ltscffexudp_source::process--Invalid Package Length! Content Len[%d],Expected Len[%d]\n", nCount - sizeof(TFTDCHeader), header.FTDCContentLength);
						continue;
					}
					if (header.TransactionId != 0x0000F103 && header.TransactionId != 0x0000F101)
					{
						loggerv2::error("ltscffexudp_source::process--Not a Market Package!,Invalid TransactionId[%X]\n", header.TransactionId);
						continue;
					}
					char* pz = RcvBuff + sizeof(TFTDCHeader);
					if (header.FieldCount <= 0)
					{
						loggerv2::warn("ltscffexudp_source::process--header.FieldCount <= 0");
						continue;
					}
					//3.fill the cffexpdu,the message format is TLV
					for (int i = 0; i < header.FieldCount; i++)
					{
						memcpy_lw(&fieldHeader, pz, sizeof(TFieldHeader));
						pz += sizeof(TFieldHeader);//TL part
						switch (fieldHeader.FieldID)
						{
						case 0x2439:
						{
							pz += fieldHeader.Size;//value part												
						}
						break;
						case 0x2434:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2432:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2433:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2435:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2436:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2437:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2438:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x2431:
						{
							pz += fieldHeader.Size;
						}
						break;
						case 0x12:
						{
							CDepthMarketDataField *pDepthField = NULL;
							pDepthField = new CDepthMarketDataField();
							if (pDepthField != NULL)
							{
								memcpy_lw(pDepthField, pz, fieldHeader.Size);
								//
								this->publish_msg((void*)pDepthField, sizeof(CDepthMarketDataField), pDepthField->InstrumentID);
								//
								m_queue.Push(pDepthField);
							}

							pz += fieldHeader.Size;
						}
						break;
						default:
						{
							pz += fieldHeader.Size;
						}
						break;
						}
					}
				}
			}
		}

		void ltscffexudp_source::start_receiver()
		{
			CDepthMarketDataField msg;
			int rc = -1;
			//while (true)
			{
				rc = nn_recv(m_sub_handle, &msg, sizeof(msg), 0);
				if (rc == sizeof(msg))
				{
					//int t = microsec_clock::local_time().time_of_day().total_milliseconds();
					//printf_ex("sl_source::start_receiver,%d-%d,%d\n", t, msg.UpdateMillisec, t - msg.UpdateMillisec);
					//loggerv2::info("sl_source::start_receiver,%d-%d,%d", t, msg.UpdateMillisec, t - msg.UpdateMillisec);							
					CDepthMarketDataField *pDepthField = new CDepthMarketDataField();
					memcpy_lw(pDepthField, &msg, sizeof(msg));
					get_queue()->Push(&msg);
				}
			}
		}

		bool ltscffexudp_source::setUDPSockect(const char * pBroadcastIP, int nBroadcastPort)
		{
			//
			if (this->is_sub() == true)
				return false;
			//

			int				nFlag = 1;
			unsigned long   uFlag = 1;
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




		void ltscffexudp_source::release_source()
		{
			stop_process();
			m_tListhen2Udp.join();
		}


		void ltscffexudp_source::process_depthMarketDataField(CDepthMarketDataField* pMsg)
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
				//loggerv2::info("lts_source: instrument %s not found", pMsg->InstrumentID);
				return;
			}
			process_depthMarketDataField(pMsg, afeed_item);
			return post(afeed_item);
		}
		

		void ltscffexudp_source::process()
		{

			get_queue()->Pops_Handle();

		}
		//int ltscffexudp_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		CDepthMarketDataField* msg = get_queue()->Pop();
		//		this->process_depthMarketDataField(msg);
		//	}
		//	return i;
		//}
		void ltscffexudp_source::process_depthMarketDataField(CDepthMarketDataField* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;

			process_depth(0, pMsg->BidVolume1, pMsg->BidPrice1 != NO_PRICE ? pMsg->BidPrice1 : 0., pMsg->AskPrice1 != NO_PRICE ? pMsg->AskPrice1 : 0., pMsg->AskVolume1, feed_item);
			process_depth(1, pMsg->BidVolume2, pMsg->BidPrice2 != NO_PRICE ? pMsg->BidPrice2 : 0., pMsg->AskPrice2 != NO_PRICE ? pMsg->AskPrice2 : 0., pMsg->AskVolume2, feed_item);
			process_depth(2, pMsg->BidVolume3, pMsg->BidPrice3 != NO_PRICE ? pMsg->BidPrice3 : 0., pMsg->AskPrice3 != NO_PRICE ? pMsg->AskPrice3 : 0., pMsg->AskVolume3, feed_item);
			process_depth(3, pMsg->BidVolume4, pMsg->BidPrice4 != NO_PRICE ? pMsg->BidPrice4 : 0., pMsg->AskPrice4 != NO_PRICE ? pMsg->AskPrice4 : 0., pMsg->AskVolume4, feed_item);
			process_depth(4, pMsg->BidVolume5, pMsg->BidPrice5 != NO_PRICE ? pMsg->BidPrice5 : 0., pMsg->AskPrice5 != NO_PRICE ? pMsg->AskPrice5 : 0., pMsg->AskVolume5, feed_item);

			double closePrice = (!(pMsg->ClosePrice == NO_PRICE || pMsg->ClosePrice == 0)) ? pMsg->ClosePrice : (pMsg->PreClosePrice != NO_PRICE ? pMsg->PreClosePrice : 0);
			feed_item->set_close_price(closePrice);

			double lastPrice = pMsg->LastPrice != NO_PRICE ? pMsg->LastPrice : 0;
			feed_item->set_last_price(lastPrice);

			int dailyVolume = pMsg->Volume;
			feed_item->set_daily_volume(dailyVolume);
			feed_item->set_turnover(pMsg->Turnover);
			//double hightest = pMsg->HighestPrice;

			//double lowest = pMsg->LowestPrice;

			double upperlmt = pMsg->UpperLimitPrice;
			feed_item->set_upper_limit(upperlmt);

			double lowerlmt = pMsg->LowerLimitPrice;
			feed_item->set_lower_limit(lowerlmt);

			double selltement = pMsg->SettlementPrice;
			feed_item->set_settlement(selltement);

			//double presettle = pMsg->PreSettlementPrice;

			//double preopeninterest = pMsg->PreOpenInterest;
		}
	}
}
