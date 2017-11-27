#include "cffex_connection.h"

using namespace terra::common;
namespace feed
{
	namespace cffex
	{
		cffex_connection::cffex_connection(cffex_source* pSource)
		{
			m_pSource = pSource;

			m_pUserApi = NULL;

			m_nRequestId = 0;
						
		}

		cffex_connection::~cffex_connection()
		{
		}

		void cffex_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			m_pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
			m_pUserApi->RegisterSpi(this);
			m_pUserApi->RegisterFront(addr);
			m_pUserApi->Init();
		}

		void cffex_connection::cleanup()
		{
			if (m_pUserApi)
			{
				m_pUserApi->Release();
			}
		}

		void cffex_connection::Process()
		{
			if (m_pUserApi)
			{
				m_pUserApi->Join();
			}
			//loggerv2::info("end of thread");
		}

		bool cffex_connection::subscribe_item(feed_item* item)
		{
			//
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			//
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->SubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::error("cffex_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("cffex_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			if (m_pSource->FQR)
			{
				int res = m_pUserApi->SubscribeForQuoteRsp(instruments, nbInstrument);
				if (res != 0)
				{
					loggerv2::error("cffex_connection::subscribe_FQR_item - %s : Not OK !", item->get_code().c_str());
					return false;
				}
				loggerv2::info("cffex_connection::subscribe_FQR_item - %s :  OK !", item->get_code().c_str());
			}
			
			
			return true;
		}

		bool cffex_connection::unsubscribe_item(feed_item* item)
		{
			//
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			//
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->UnSubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::error("cffex_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("cffex_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			if (m_pSource->FQR)
			{
				int res = m_pUserApi->UnSubscribeForQuoteRsp(instruments, nbInstrument);
				if (res != 0)
				{
					loggerv2::error("cffex_connection::unsubscribe_FQR_item - %s : Not OK !", item->get_code().c_str());
					return false;
				}
				loggerv2::info("cffex_connection::unsubscribe_FQR_item - %s :  OK !", item->get_code().c_str());
			}
			
			return true;
		}
		//
		// private
		//
		bool cffex_connection::request_login()
		{
			//
			if (m_pUserApi == nullptr)
				return false;
			//
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
		void cffex_connection::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("cffex_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("cffex_connection::OnRspError - ok");
			}
		}

		void cffex_connection::OnFrontConnected()
		{
			loggerv2::info("cffex_connection is UP");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "cffex_connection::request_login failed");

			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void cffex_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::error("cffex_connection::OnFrontDisconnected is DOWN");

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
				pszMessage = std::move(std::string("ERROR MSG TO TRANSLATE"));
				break;


			default:
				//pszMessage = "unknown error [" + nReason + "];
				pszMessage = std::move(std::string("unknown error"));
				break;
			}

			m_pSource->update_state(AtsType::FeedSourceStatus::Down, pszMessage);
			m_bReconnected = true;
		}

		void cffex_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			loggerv2::info("cffex_connection - OnHeartBeatWarning warning");
		}

		void cffex_connection::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			//loggerv2::info("cffex_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void cffex_connection::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("cffex_connection::OnRspSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void cffex_connection::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("cffex_connection::OnRspUnSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void cffex_connection::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
		{
			//
			if (strcmp(pDepthMarketData->InstrumentID, "rb1801") == 0)
			{
				std::chrono::time_point<std::chrono::high_resolution_clock> tp = std::chrono::high_resolution_clock::now();
				long long timep = std::chrono::time_point_cast<lwdur>(tp).time_since_epoch().count();
				BOOST_LOG_TRIVIAL(info) << "ctp rb1801:" << pDepthMarketData->Turnover << " tp:" << timep;
			}

			//m_pSource->publish_msg((void*)pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField), pDepthMarketData->InstrumentID);
			//if (m_pSource->get_strPub() != "pub")
			std::string str = std::string(pDepthMarketData->InstrumentID);
			feed_item* afeed_item = m_pSource->get_feed_item(str);
			if (afeed_item != nullptr)
			{
				m_pSource->update_item(pDepthMarketData, afeed_item);
				m_pSource->get_queue()->CopyPush(&afeed_item);
			}
		}

		void cffex_connection::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
		{
			m_pSource->receive_FQR(pForQuoteRsp);
		}

		void cffex_connection::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != nullptr)
			{
				loggerv2::error("cffex_connection::OnRspSubForQuoteRsp %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void cffex_connection::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != nullptr)
			{
				loggerv2::error("cffex_connection::OnRspUnSubForQuoteRsp %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}



	}
}
