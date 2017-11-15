#include "femas_connection.h"

using namespace terra::common;
namespace feed
{
	namespace femas
	{
		femas_connection::femas_connection(femas_source* pSource)
		{
			m_pSource = pSource;

			m_pUserApi = NULL;

			m_nRequestId = 0;

			
		}

		femas_connection::~femas_connection()
		{
			this->cleanup();
		}

		void femas_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//

			m_pUserApi = CUstpFtdcMduserApi::CreateFtdcMduserApi();

			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());

			m_pUserApi->RegisterSpi(this);
			m_pUserApi->SubscribeMarketDataTopic(100, USTP_TERT_QUICK);
			m_pUserApi->RegisterFront(addr);
			m_pUserApi->Init();
		}

		void femas_connection::cleanup()
		{
			m_pUserApi->Release();
		}

		void femas_connection::Process()
		{
			m_pUserApi->Join();

			//loggerv2::info("end of thread");
		}


		bool femas_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			char* instruments[] =  { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->SubMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::info("femas_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("femas_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}

		bool femas_connection::unsubscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->UnSubMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::info("femas_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("femas_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}
		//
		// private
		//
		bool femas_connection::request_login()
		{
			// login
			CUstpFtdcReqUserLoginField request;
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
		void femas_connection::OnRspError(CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("femas_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("femas_connection::OnRspError - ok");
			}
		}

		void femas_connection::OnFrontConnected()
		{
			loggerv2::info("femas_connection is UP");

			if (!request_login())
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "femas_connection::request_login failed");
			}

			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void femas_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::info("femas_connection is DOWN");

			std::string pszMessage;
			switch (nReason)
			{
				// normal disconnection
			case 0:
				break;

				// error
			case 0x1001:
				pszMessage = "network read error";
				break;
			case 0x1002:
				pszMessage = "network write error";
				break;
			case 0x2001:
				pszMessage = "receive heartbeat timeout";
				break;
			case 0x2002:
				pszMessage = "send heartbeat timeout";
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

		void femas_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			loggerv2::info("femas_connection - heartbeat warning since last %d", nTimeLapse);
		}

		void femas_connection::OnRspUserLogin(CUstpFtdcRspUserLoginField* pRspUserLogin, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			//loggerv2::info("femas_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void femas_connection::OnRspSubMarketData(CUstpFtdcSpecificInstrumentField* pSpecificInstrument, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("femas_connection::subscribe_item %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void femas_connection::OnRspUnSubMarketData(CUstpFtdcSpecificInstrumentField* pSpecificInstrument, CUstpFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("femas_connection::unsubscribe_item %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void femas_connection::OnRtnDepthMarketData(CUstpFtdcDepthMarketDataField* pDepthMarketData)
		{
			m_pSource->publish_msg((void*)pDepthMarketData, sizeof(CUstpFtdcDepthMarketDataField), pDepthMarketData->InstrumentID);
			//
			// dynamic allocation for now
			// delete is done in femas_source (different thread).
			//
			//CUstpFtdcDepthMarketDataField* pData = new CUstpFtdcDepthMarketDataField;
			//memcpy(pData, pDepthMarketData, sizeof(CUstpFtdcDepthMarketDataField));

			//m_pSource->get_queue()->Push(pData);
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush(pDepthMarketData);
		}
	}

}