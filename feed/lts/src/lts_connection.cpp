#include "lts_connection.h"
#include "terra_logger.h"
using namespace terra::common;
namespace feed
{
	namespace lts
	{
		lts_connection::lts_connection(lts_source* pSource)
		{
			m_pSource = pSource;

			m_pUserApi = NULL;

			m_nRequestId = 0;
					
		}

		lts_connection::~lts_connection()
		{
			this->cleanup();
		}

		void lts_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//

			m_pUserApi = CSecurityFtdcMdApi::CreateFtdcMdApi("");

			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());

			m_pUserApi->RegisterSpi(this);
			m_pUserApi->RegisterFront(addr);
			m_pUserApi->Init();
		}

		void lts_connection::cleanup()
		{
			m_pUserApi->Release();
		}

		void lts_connection::Process()
		{
			m_pUserApi->Join();
		}

		bool lts_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			char instrumentsOrigin[16];
			memset(instrumentsOrigin, '0', 16);
			strcpy(instrumentsOrigin, item->get_feed_code().c_str());
			instrumentsOrigin[15] = '\0';

			char* instruments[1];
			char* pExchageID;
			char* pch = strtok(instrumentsOrigin, ".");
			instruments[0] = pch;
			pch = strtok(NULL, ".");
			pExchageID = pch;

			int nbInstrument = 1;
			int res = m_pUserApi->SubscribeMarketData(instruments, nbInstrument, pExchageID);
			if (res != 0)
			{
				loggerv2::error("lts_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("lts_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}


		bool lts_connection::unsubscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			char instrumentsOrigin[16];
			memset(instrumentsOrigin, '0', 16);
			strcpy(instrumentsOrigin, item->get_feed_code().c_str());
			instrumentsOrigin[15] = '\0';

			char* instruments[1];
			char* pExchageID;
			char* pch = strtok(instrumentsOrigin, ".");
			instruments[0] = pch;
			pch = strtok(NULL, ".");
			pExchageID = pch;

			int nbInstrument = 1;

			int res = m_pUserApi->UnSubscribeMarketData(instruments, nbInstrument, pExchageID);
			if (res != 0)
			{
				loggerv2::error("lts_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("lts_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}
		//
		// private
		//
		bool lts_connection::request_login()
		{
			// login
			CSecurityFtdcReqUserLoginField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.BrokerID, m_pSource->get_broker().c_str());
			strcpy(request.UserID, m_pSource->get_user_name().c_str());
			strcpy(request.Password, m_pSource->get_passwd().c_str());
			int res = m_pUserApi->ReqUserLogin(&request, ++m_nRequestId);
			if (res != 0)
				return false;
			return true;
		}
		//
		// callbacks
		//
		void lts_connection::OnRspError(CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("lts_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("lts_connection::OnRspError - ok");
			}
		}

		void lts_connection::OnFrontConnected()
		{
			loggerv2::info("lts_connection is connected, going to request login.");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "lts_connection::request_login failed");

			loggerv2::info("lts_connection is waiting for login relpy");

			if (m_bReconnected)
				m_pSource->resubscribe_all();

		}

		void lts_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::error("lts_connection::OnFrontDisconnected is disconnected, error=%d", nReason);

			std::string pszMessage;
			switch (nReason)
			{
			case 0:
				pszMessage = "normal disconnection";
				break;
				// error
			case 0x1001:
				pszMessage = "network write failed";
				break;
			case 0x1002:
				pszMessage = "network read failed";
				break;
			case 0x2001:
				pszMessage = "receive heartbeat timeout";
				break;
			case 0x2002:
				pszMessage = "send heartbeat failed";
				break;
			case 0x2003:
				pszMessage = "receive wrong message";
				break;


			default:
				//pszMessage = "unknown error [" + nReason + "];
				pszMessage = "unknown error";
				break;
			}

			m_pSource->update_state(AtsType::FeedSourceStatus::Down, pszMessage);
			m_bReconnected = true;
		}

		void lts_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			//loggerv2::info("lts_connection - heartbeat warning");
		}

		void lts_connection::OnRspUserLogin(CSecurityFtdcRspUserLoginField* pRspUserLogin, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			loggerv2::info("lts_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				//loggerv2::info("lts_connection OnRspUserLogin : login failed - error %s",pRspInfo->ErrorID);
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void lts_connection::OnRspSubMarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("lts_connection::OnRspSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void lts_connection::OnRspUnSubMarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("lts_connection::OnRspUnSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void lts_connection::OnRtnDepthMarketData(CSecurityFtdcDepthMarketDataField* pDepthMarketData)
		{
			std::string sFeedCode = std::string(pDepthMarketData->InstrumentID) + "." + std::string(pDepthMarketData->ExchangeID);
			m_pSource->publish_msg((void*)pDepthMarketData, sizeof(CSecurityFtdcDepthMarketDataField),sFeedCode);
			//
			// dynamic allocation for now
			// delete is done in lts_source (different thread).
			//
			//CSecurityFtdcDepthMarketDataField* pData = new CSecurityFtdcDepthMarketDataField;
			//memcpy_lw(pData, pDepthMarketData, sizeof(CSecurityFtdcDepthMarketDataField));

			//m_pSource->get_queue()->Push(pData);
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush(pDepthMarketData);
		}
	}
}
