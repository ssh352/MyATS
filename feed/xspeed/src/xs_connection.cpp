#include "xs_connection.h"

using namespace terra::common;
namespace feed
{
	namespace xs
	{
		xs_connection::xs_connection(xs_source* pSource)
		{
			m_pSource = pSource;
			m_pUserApi = NULL;
			m_nRequestId = 0;			
		}

		xs_connection::~xs_connection()
		{
		}

		void xs_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			m_pUserApi = DFITCSECMdApi::CreateDFITCMdApi(NULL);
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
			if (m_pUserApi->Init(addr, this) != 0)
			{
				loggerv2::error("xs_connection::init cannot create api.");
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "init cannot create api");
			}
		}

		void xs_connection::cleanup()
		{
			m_pUserApi->Release();
		}

		void xs_connection::Process()
		{
			//m_pUserApi->Join();

			//loggerv2::info("end of thread");
		}

		bool xs_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			//todo switch op/stock
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			switch (item->get_type())
			{
			case AtsType::InstrType::Call:
			case AtsType::InstrType::Put:
			{
							  int res = m_pUserApi->SubscribeSOPMarketData(instruments, nbInstrument, ++m_nRequestId);
							  if (res != 0)
							  {
								  loggerv2::error("xs_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
								  return false;
							  }
			}
				break;

			case AtsType::InstrType::ETF:
			case AtsType::InstrType::Stock:
			case AtsType::InstrType::Fund:
			{
							   int res = m_pUserApi->SubscribeStockMarketData(instruments, nbInstrument, ++m_nRequestId);
							   if (res != 0)
							   {
								   loggerv2::error("xs_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
								   return false;
							   }
			}
				break;
			default:
			{
				loggerv2::info("xs_connection::subscribe_item - %s : instrument type Not OK !", item->get_code().c_str());
					   return false;
			}
				break;
			}
			loggerv2::info("xs_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}

		bool xs_connection::unsubscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			switch (item->get_type())
			{
			case AtsType::InstrType::Call:
			case AtsType::InstrType::Put:
			{
							  int res = m_pUserApi->UnSubscribeSOPMarketData(instruments, nbInstrument, ++m_nRequestId);
							  if (res != 0)
							  {
								  loggerv2::error("xs_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
								  return false;
							  }
			}
				break;
			case AtsType::InstrType::ETF:
			case AtsType::InstrType::Stock:
			case AtsType::InstrType::Fund:
			{
							   int res = m_pUserApi->UnSubscribeStockMarketData(instruments, nbInstrument, ++m_nRequestId);
							   if (res != 0)
							   {
								   loggerv2::error("xs_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
								   return false;
							   }
			}
				break;
			default:
			{
				loggerv2::error("xs_connection::unsubscribe_item - %s : instrument tpye Not OK !", item->get_code().c_str());
					   return false;
			}
				break;
			}
			loggerv2::info("xs_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}



		//bool xs_connection::subscribe_item(const char* item)
		//{
		//	if (item == nullptr)
		//		return false;
		//	char* instruments[] = { const_cast<char*>(item) };
		//	int nbInstrument = 1;
		//	int res = m_pUserApi->SubscribeSOPMarketData(instruments, nbInstrument, ++m_nRequestId);
		//	if (res != 0)
		//	{
		//		loggerv2::error("xs_connection::subscribe_item - %s : Not OK !", item);
		//		return false;
		//	}
		//	//loggerv2::info("xs_connection::subscribe_item - %s : OK !", pItem->get_subject());
		//	return true;
		//}

		//bool xs_connection::unsubscribe_item(const char* item)
		//{
		//	if (item == nullptr)
		//		return false;
		//	char* instruments[] = { const_cast<char*>(item) };
		//	int nbInstrument = 1;
		//	int res = m_pUserApi->UnSubscribeSOPMarketData(instruments, nbInstrument, ++m_nRequestId);
		//	if (res != 0)
		//		return false;
		//	return true;
		//}


		//
		// private
		//
		bool xs_connection::request_op_login()
		{
			// login
			DFITCSECReqUserLoginField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.accountID, m_pSource->get_user_name().c_str());
			strcpy(request.password, m_pSource->get_passwd().c_str());
			request.requestID = ++m_nRequestId;
			int res = m_pUserApi->ReqSOPUserLogin(&request);
			if (res != 0)
				return false;
			return true;
		}

		bool xs_connection::request_stock_login()
		{
			// login
			DFITCSECReqUserLoginField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.accountID, m_pSource->get_user_name().c_str());
			strcpy(request.password, m_pSource->get_passwd().c_str());
			request.requestID = ++m_nRequestId;
			int res = m_pUserApi->ReqStockUserLogin(&request);
			if (res != 0)
				return false;
			return true;
		}


		//
		// callbacks
		//
		void xs_connection::OnRspError(struct DFITCSECRspInfoField *pRspInfo)
		{
			if (pRspInfo != NULL && pRspInfo->errorID != 0)
			{
				loggerv2::error("xs_connection::OnRspError - (%d, %s)", pRspInfo->errorID, pRspInfo->errorMsg);
			}
			else
			{
				loggerv2::info("xs_connection::OnRspError - ok");
			}
		}

		void xs_connection::OnFrontConnected()
		{
			loggerv2::info("xs_connection::OnFrontConnected - xs_connection is UP  going to request login");
			if (!request_op_login())
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "xs_connection::request_op_login failed");
			}
			if (!request_stock_login())
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "xs_connection::request_stock_login failed");
			}
			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void xs_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::error("xs_connection::OnFrontConnected - xs_connection is DOWN");

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

		void xs_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			loggerv2::info("xs_connection - heartbeat warning");
		}

		void xs_connection::OnRspSOPUserLogin(struct DFITCSECRspUserLoginField * pRspUserLogin, struct DFITCSECRspInfoField * pRspInfo)
		{
			if (pRspUserLogin)
			{
				loggerv2::info("xs_connection::OnRspSOPUserLogin - (%d, %s)", pRspUserLogin->result, pRspUserLogin->rtnMsg);
			}
			else
			{
				if (pRspInfo)
				{
					loggerv2::info("xs_connection::OnRspSOPUserLogin - (%s)", pRspInfo->errorMsg);
				}
				return;
			}

			if (pRspUserLogin->result == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspUserLogin->rtnMsg);
			}
		}

		void xs_connection::OnRspSOPSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo)
		{
			if (pRspInfo && pRspInfo->errorID != 0)
			{
				loggerv2::error("xs_connection::OnRspSOPSubMarketData failed (%d, %s)", pRspInfo->errorID, pRspInfo->errorMsg);
			}
		}

		void xs_connection::OnRspSOPUnSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo)
		{
			if (pRspInfo && pRspInfo->errorID != 0)
			{
				loggerv2::error("xs_connection::OnRspSOPUnSubMarketData failed (%d, %s)", pRspInfo->errorID, pRspInfo->errorMsg);
			}
		}


		void xs_connection::OnRspStockUserLogin(struct DFITCSECRspUserLoginField * pRspUserLogin, struct DFITCSECRspInfoField * pRspInfo)
		{
			if (pRspUserLogin)
			{
				loggerv2::info("xs_connection::OnRspStockUserLogin - (%d, %s)", pRspUserLogin->result, pRspUserLogin->rtnMsg);
			}
			else
			{
				if (pRspInfo)
				{
					loggerv2::info("xs_connection::OnRspStockUserLogin - (%s)", pRspInfo->errorMsg);
				}
				return;
			}
			if (pRspUserLogin->result == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspUserLogin->rtnMsg);
			}
		}


		void xs_connection::OnRspStockSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo)
		{
			if (pRspInfo && pRspInfo->errorID != 0)
			{
				loggerv2::error("xs_connection::OnRspStockSubMarketData failed (%d, %s)", pRspInfo->errorID, pRspInfo->errorMsg);
			}
		}

		void xs_connection::OnRspStockUnSubMarketData(struct DFITCSECSpecificInstrumentField * pSpecificInstrument, struct DFITCSECRspInfoField * pRspInfo)
		{
			if (pRspInfo && pRspInfo->errorID != 0)
			{
				loggerv2::error("xs_connection::OnRspStockUnSubMarketData failed (%d, %s)", pRspInfo->errorID, pRspInfo->errorMsg);
			}
		}
		void xs_connection::OnSOPMarketData(struct DFITCSOPDepthMarketDataField * pMarketDataField)
		{
			std::string sItemFeedcode = std::string(pMarketDataField->staticDataField.exchangeID) + std::string(pMarketDataField->staticDataField.securityID);
			m_pSource->publish_msg((void*)pMarketDataField, sizeof(DFITCSOPDepthMarketDataField), sItemFeedcode);
			//
			// dynamic allocation for now
			// delete is done in xs_source (different thread).
			//
			//DFITCSOPDepthMarketDataField* pData = new DFITCSOPDepthMarketDataField;
			//memcpy_lw(pData, pMarketDataField, sizeof(DFITCSOPDepthMarketDataField));
			//m_pSource->get_op_queue()->Push(pData);
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_op_queue()->CopyPush(pMarketDataField);
		}
		void xs_connection::OnStockMarketData(struct DFITCStockDepthMarketDataField * pMarketDataField)
		{
			std::string sItemFeedcode = std::string(pMarketDataField->staticDataField.exchangeID) + std::string(pMarketDataField->staticDataField.securityID);
			m_pSource->publish_msg((void*)pMarketDataField, sizeof(DFITCStockDepthMarketDataField),sItemFeedcode);
			//
			// dynamic allocation for now
			// delete is done in xs_source (different thread).
			//
			//DFITCStockDepthMarketDataField* pData = new DFITCStockDepthMarketDataField;
			//memcpy_lw(pData, pMarketDataField, sizeof(DFITCStockDepthMarketDataField));
			//m_pSource->get_stock_queue()->Push(pData);
			m_pSource->get_stock_queue()->CopyPush(pMarketDataField);
		}
	}
}
