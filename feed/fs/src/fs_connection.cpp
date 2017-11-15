#include "fs_connection.h"
#include "terra_logger.h"
using namespace terra::common;
namespace feed
{
	namespace fs
	{
		fs_connection::fs_connection(fs_source* pSource)
		{
			m_pSource = pSource;

			m_pUserApi = NULL;

			m_nRequestId = 0;
						
		}

		fs_connection::~fs_connection()
		{
		}

		void fs_connection::init()
		{
			m_pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
			m_pUserApi->RegisterSpi(this);
			m_pUserApi->RegisterFront(addr);
			m_pUserApi->Init();
		}

		void fs_connection::cleanup()
		{
			m_pUserApi->Release();
		}

		void fs_connection::Process()
		{
			m_pUserApi->Join();

			//loggerv2::info("end of thread");
		}

		bool fs_connection::subscribe_item(feed_item* item)
		{
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->SubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::error("fs_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("fs_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}

		bool fs_connection::unsubscribe_item(feed_item* item)
		{

			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->UnSubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::error("fs_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("fs_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}
		//
		// private
		//
		bool fs_connection::request_login()
		{
			// login
			CThostFtdcReqUserLoginField request;
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
		void fs_connection::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("fs_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("fs_connection::OnRspError - ok");
			}
		}

		void fs_connection::OnFrontConnected()
		{
			loggerv2::info("fs_connection is UP");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "fs_connection::request_login failed");
		
			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void fs_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::error("fs_connection::OnFrontDisconnected is DOWN");

			std::string pszMessage;
			switch (nReason)
			{
				// normal disconnection
			case 0:
				break;

				// error
			case 1:
			case 2:
			case 3:
			case 4:
				//pszMessage = "ERROR MSG TO TRANSLATE [" + nReason + "];
				pszMessage = "ERROR MSG TO TRANSLATE";
				break;


			default:
				//pszMessage = "unknown error [" + nReason + "];
				pszMessage = "unknown error";
				break;
			}

			m_pSource->update_state(AtsType::FeedSourceStatus::Down, pszMessage);
			m_bReconnected = true;
		}

		void fs_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			loggerv2::info("fs_connection - OnHeartBeatWarning warning");
		}

		void fs_connection::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			//loggerv2::info("fs_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void fs_connection::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("fs_connection::OnRspSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void fs_connection::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("fs_connection::OnRspUnSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void fs_connection::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
		{
			//
			// dynamic allocation for now
			// delete is done in fs_source (different thread).
			//
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush(pDepthMarketData);
		}
	}
}
