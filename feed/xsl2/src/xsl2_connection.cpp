#include "xsl2_connection.h"

using namespace terra::common;
namespace feed
{
	namespace xsl2
	{
		xsl2_connection::xsl2_connection(xsl2_source* pSource)
		{
			m_pSource = pSource;
			m_pUserApi = NULL;
			m_nRequestId = 0;			
		}

		xsl2_connection::~xsl2_connection()
		{

		}

		void xsl2_connection::init()
		{
			m_pUserApi = NEW_CONNECTOR();
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr, "%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
			printf_ex("xsl2_connection::init addr:%s\n", addr);
			loggerv2::info("xsl2_connection::init addr:%s\n", addr);
			int ret = -1;
			if (m_pSource->isUdp())
			{
				loggerv2::info("xsl2_connection using UDP connection");
				ret = m_pUserApi->Connect(addr, this, 0);
			}
			else
			{
				loggerv2::info("xsl2_connection using TCP connection");
				ret = m_pUserApi->Connect(addr, this, 1);
			}
			if (ret != 0)
			{
				loggerv2::error("xsl2_connection:%d",ret);
			}
		}

		void xsl2_connection::cleanup()
		{			
			DELETE_CONNECTOR(m_pUserApi);
			m_pUserApi = nullptr;
		}

		void xsl2_connection::Process()
		{
			
		}

		bool xsl2_connection::subscribe_item(feed_item* item)
		{
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
			int res = m_pUserApi->SubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::info("xsl2_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("xsl2_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			//
			//m_pUserApi->SubscribeAll();
			//
			return true;
		}


		bool xsl2_connection::unsubscribe_item(feed_item * item)
		{
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

			int res = m_pUserApi->UnSubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::info("xsl2_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("xsl2_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}



		bool xsl2_connection::request_login()
		{
			// login
			struct DFITCUserLoginField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.accountID, m_pSource->get_user_name().c_str());
			strcpy(request.passwd, m_pSource->get_passwd().c_str());			
			int res = m_pUserApi->ReqUserLogin(&request);
			if (res != 0)
			{
				printf_ex("xsl2_connection::request_login res:%d\n",res);
				loggerv2::error("xsl2_connection::request_login res:%d\n", res);
				return false;
			}
			return true;
		}

		void xsl2_connection::OnConnected()
		{
			loggerv2::info("xsl2_connection is Connected - Going to Login");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "xsl2_connection::request_login failed");

			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void xsl2_connection::OnDisconnected(int pReason)
		{
			loggerv2::error("xsl2_connection is DOWN,reason:%d", pReason);
			char* pszMessage;
			switch (pReason)
			{
			case 0:
				pszMessage = "";
				break;
			case 1:
			case 2:
			case 3:
			case 4:				
				pszMessage = "ERROR MSG TO TRANSLATE";
				break;
			default:				
				pszMessage = "unknown error";
				break;
			}
			m_pSource->update_state(AtsType::FeedSourceStatus::Down, pszMessage);
			m_bReconnected = true;
		}
		void xsl2_connection::OnRspUserLogin(struct ErrorRtnField * pErrorField)
		{
			loggerv2::info("xsl2_connection::OnRspUserLogin - (%d, %s)", pErrorField->ErrorID, pErrorField->ErrorMsg);

			if (pErrorField->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pErrorField->ErrorMsg);
			}
		}
		void xsl2_connection::OnRspUserLogout(struct ErrorRtnField * pErrorField)
		{

		}
		void xsl2_connection::OnRspSubscribeMarketData(struct ErrorRtnField * pErrorField)
		{
			if (pErrorField->ErrorID != 0)
			{
				loggerv2::error("xsl2_connection::OnRspSubscribeMarketData failed (%d, %s)",pErrorField->ErrorID, pErrorField->ErrorMsg);
			}
		}
		void xsl2_connection::OnRspUnSubscribeMarketData(ErrorRtnField * pErrorField)
		{
			if (pErrorField->ErrorID != 0)
			{
				loggerv2::error("xsl2_connection::OnRspUnSubscribeMarketData failed (%d, %s)", pErrorField->ErrorID, pErrorField->ErrorMsg);
			}
		}
		void xsl2_connection::OnRspSubscribeAll(struct ErrorRtnField * pErrorField)
		{

		}
		void xsl2_connection::OnRspUnSubscribeAll(struct ErrorRtnField * pErrorField)
		{

		}
		void xsl2_connection::OnBestAndDeep(MDBestAndDeep * const pQuote)
		{
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_5_queue()->CopyPush(pQuote);
		}
		void xsl2_connection::OnTenEntrust(MDTenEntrust * const pQuote)
		{
			//m_pSource->get_10_queue()->CopyPush(pQuote);
		}
		void xsl2_connection::OnHeartBeatLost()
		{

		}
	}
}
