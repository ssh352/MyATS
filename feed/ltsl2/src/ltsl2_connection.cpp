#include "ltsl2_connection.h"

using namespace terra::common;
namespace feed
{
	namespace ltsl2
	{
		ltsl2_connection::ltsl2_connection(ltsl2_source* pSource)
		{
			m_pSource = pSource;
			m_pUserApi = NULL;
			m_nRequestId = 0;
			/*RTThread::SetName("ltsl2_connection_v2");*/
		}

		ltsl2_connection::~ltsl2_connection()
		{

		}

		void ltsl2_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			if (m_pSource->isUdp())
			{
				loggerv2::info("ltsl2_connection using UDP connection");
				m_pUserApi = _LTS_::CSecurityFtdcL2MDUserApi::CreateFtdcL2MDUserApi(true);
			}

			else
			{
				loggerv2::info("ltsl2_connection using TCP connection");
				m_pUserApi = _LTS_::CSecurityFtdcL2MDUserApi::CreateFtdcL2MDUserApi(false);
			}
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
			printf_ex("ltsl2_connection::init addr:%s\n",addr);
			loggerv2::info("ltsl2_connection::init addr:%s\n", addr);
			m_pUserApi->RegisterSpi(this);
			m_pUserApi->RegisterFront(addr);
			m_pUserApi->Init();
		}

		void ltsl2_connection::cleanup()
		{
			m_pUserApi->Release();
		}

		void ltsl2_connection::Process()
		{
			//m_pUserApi->Join();
			//loggerv2::info("end of thread");
		}

		bool ltsl2_connection::subscribe_item(feed_item* item)
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
			int res = m_pUserApi->SubscribeL2MarketData(instruments, nbInstrument, pExchageID);
			if (res != 0)
			{
				loggerv2::info("ltsl2_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("ltsl2_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}


		bool ltsl2_connection::unsubscribe_item(feed_item * item)
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

			int res = m_pUserApi->UnSubscribeL2MarketData(instruments, nbInstrument, pExchageID);
			if (res != 0)
			{
				loggerv2::info("ltsl2_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("ltsl2_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}



		bool ltsl2_connection::request_login()
		{
			// login
			_LTS_::CSecurityFtdcUserLoginField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.BrokerID, m_pSource->get_broker().c_str());
			strcpy(request.UserID, m_pSource->get_user_name().c_str());
			strcpy(request.Password, m_pSource->get_passwd().c_str());
			int res = m_pUserApi->ReqUserLogin(&request, ++m_nRequestId);
			if (res != 0)
			{
				printf_ex("ltsl2_connection::request_login res:%d\n",res);
				loggerv2::error("ltsl2_connection::request_login res:%d\n", res);
				return false;
			}
			return true;
		}


		//
		// callbacks
		//
		void ltsl2_connection::OnRspError(CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("ltsl2_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("ltsl2_connection::OnRspError - ok");
			}
		}

		void ltsl2_connection::OnFrontConnected()
		{
			loggerv2::info("ltsl2_connection is Connected - Going to Login");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "ltsl2_connection::request_login failed");
		
			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void ltsl2_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::info("ltsl2_connection is DOWN,reason:%d",nReason);

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

		void ltsl2_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			//loggerv2::info("ltsl2_connection - heartbeat warning");
		}

		void ltsl2_connection::OnRspUserLogin(CSecurityFtdcUserLoginField* pRspUserLogin, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			loggerv2::info("ltsl2_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void ltsl2_connection::OnRspSubL2MarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("ltsl2_connection::subscribe_item  %s level 2 data failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void ltsl2_connection::OnRspUnSubL2MarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("ltsl2_connection::unsubscribe_item %s level 2 data failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void ltsl2_connection::OnRtnL2MarketData(CSecurityFtdcL2MarketDataField *pL2MarketData)
		{
			//
			std::string sFeedCode = std::string(pL2MarketData->InstrumentID) + "." + std::string(pL2MarketData->ExchangeID);
			m_pSource->publish_msg((void*)pL2MarketData, sizeof(CSecurityFtdcL2MarketDataField),sFeedCode);
			//
			/*loggerv2::info("ltsl2_connection::OnRtnL2MarketData");
			CSecurityFtdcL2MarketDataField* pData = new CSecurityFtdcL2MarketDataField;
			memcpy(pData, pL2MarketData, sizeof(CSecurityFtdcL2MarketDataField));
			m_pSource->get_queue()->Push(pData);*/
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush(pL2MarketData);
		}
	}
}
