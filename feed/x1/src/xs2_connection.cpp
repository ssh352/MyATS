#include "xs2_connection.h"

using namespace terra::common;


namespace feed
{
	namespace xs2
	{
		xs2_connection::xs2_connection(xs2_source* pSource)
		{
			m_pSource = pSource;
			m_pUserApi = NULL;
			m_nRequestId = 0;			
		}

		xs2_connection::~xs2_connection()
		{
		}

		void xs2_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			m_pUserApi = DFITCMdApi::CreateDFITCMdApi();
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());

			if (m_pUserApi->Init(addr, this) != 0)
			{
				loggerv2::error("xs2_connection::init cannot create api.");
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "init cannot create api");
			}
		}

		void xs2_connection::cleanup()
		{
			if (m_pUserApi)
			{
				m_pUserApi->Release();
			}
		}

		void xs2_connection::Process()
		{
			//m_pUserApi->Join();
		}

		bool xs2_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;

			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };

			int nbInstrument = 1;
			int res = m_pUserApi->SubscribeMarketData(instruments, nbInstrument, ++m_nRequestId);
			if (res != 0)
			{
				loggerv2::error("xs2_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}

			loggerv2::info("xs2_connection::subscribe_item - %s : OK !", item->get_code().c_str());
			return true;
		}

		bool xs2_connection::unsubscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;

			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;

			int res = m_pUserApi->UnSubscribeMarketData(instruments, nbInstrument, ++m_nRequestId);
			if (res != 0)
			{
				loggerv2::error("xs2_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}

			return true;
		}

		bool xs2_connection::request_login()
		{
			struct DFITCUserLoginField data;
			memset(&data, 0, sizeof(data));
			data.lRequestID = ++m_nRequestId;			
			strcpy(data.accountID, m_pSource->get_user_name().c_str());
			strcpy(data.passwd, m_pSource->get_passwd().c_str());
			int res = m_pUserApi->ReqUserLogin(&data);
			if (res != 0)
				return false;
			return true;
		}

		//
		// callbacks
		//
		void xs2_connection::OnRspError(struct DFITCErrorRtnField *pRspInfo)
		{
			if (pRspInfo != NULL && pRspInfo->nErrorID != 0)
			{
				loggerv2::error("xs2_connection::OnRspError - (%d, %s)", pRspInfo->nErrorID, pRspInfo->errorMsg);
			}
			else
			{
				loggerv2::info("xs2_connection::OnRspError - ok");
			}
		}

		void xs2_connection::OnFrontConnected()
		{
			loggerv2::info("xs2_connection::OnFrontConnected - xs2_connection is UP  going to request login");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "xs2_connection::request_login failed");

		}

		void xs2_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::error("xs2_connection::OnFrontConnected - xs2_connection is DOWN,nReason:%d", nReason);

			char* pszMessage;
			switch (nReason)
			{
				// normal disconnection
			case 0:
				pszMessage = "";
				break;
			default:
				//pszMessage = "unknown error [" + nReason + "];
				pszMessage = "unknown error";
				break;
			}

			m_pSource->update_state(AtsType::FeedSourceStatus::Down, pszMessage);
		}

		void xs2_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			loggerv2::info("xs2_connection::OnHeartBeatWarning,nTimeLapse:%d", nTimeLapse);
		}

		void xs2_connection::OnRspUserLogin(struct DFITCUserLoginInfoRtnField *pRspUserLogin, struct DFITCErrorRtnField *pRspInfo)
		{
			if (pRspInfo)
			{
				loggerv2::error("xs2_connection::OnRspUserLogin fail - (%d, %s)", pRspInfo->nErrorID, pRspInfo->errorMsg);
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->errorMsg);
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up,"");
			}
		}

		void xs2_connection::OnRspSubMarketData(struct DFITCSpecificInstrumentField *pSpecificInstrument, struct DFITCErrorRtnField *pRspInfo)
		{
			if (pRspInfo)
			{
				loggerv2::error("xs2_connection::OnRspSubMarketData failed (%d, %s)", pRspInfo->nErrorID, pRspInfo->errorMsg);
			}
		}

		void xs2_connection::OnRspUnSubMarketData(struct DFITCSpecificInstrumentField *pSpecificInstrument, struct DFITCErrorRtnField *pRspInfo)
		{
			if (pRspInfo)
			{
				loggerv2::error("xs2_connection::OnRspUnSubMarketData failed (%d, %s)", pRspInfo->nErrorID, pRspInfo->errorMsg);
			}
		}


		void xs2_connection::OnMarketData(struct DFITCDepthMarketDataField *pMarketDataField)
		{
			m_pSource->publish_msg((void*)pMarketDataField, sizeof(DFITCDepthMarketDataField));
			//
			// dynamic allocation for now
			// delete is done in xs2_source (different thread).
			//
			/*auto tb = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now());
			std::string str = pMarketDataField->instrumentID;
			if (str=="i1609")
				loggerv2::info("time_test: rev timepoint is %lld,instr:%s", tb.time_since_epoch().count(), pMarketDataField->instrumentID);*/

			//DFITCDepthMarketDataField* pData = new DFITCDepthMarketDataField;
			//memcpy_lw(pData, pMarketDataField, sizeof(DFITCDepthMarketDataField));
			//m_pSource->get_queue()->Push(pData);
			m_pSource->get_queue()->CopyPush(pMarketDataField);
		}
	}
}

