#include "zd_connection.h"
#include "referential.h"
using namespace terra::common;
using namespace terra::instrument;
namespace feed
{
	namespace zd
	{
		zd_connection::zd_connection(zd_source* pSource)
		{
			m_pSource = pSource;

			m_pUserApi = NULL;

			m_nRequestId = 0;
						
		}

		zd_connection::~zd_connection()
		{
		}

		/*
		RegisterLoginFront 行情登陆，权限验证  RegisterFront订阅行情的地址
		*/
		void zd_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			/*
			apiHandle = CSHZdMarketApi::CreateSHZdMarketApi("..\\marketLog",false);
			apiHandle->RegisterSpi(new MarketSpi);
			apiHandle->Init();
			apiHandle->AuthonInfo("55822DC39D9316D5111D9EED00C1CED81B6F0DCEA8D97DDEBD350D939CF8A9D304E3C73A742CFB80");
			apiHandle->RegisterLoginFront("protocol://222.73.119.230:7003");
			*/
#ifndef Linux
			m_pUserApi = CSHZdMarketApi::CreateSHZdMarketApi("..\\marketLog");
#else
			m_pUserApi = CSHZdMarketApi::CreateSHZdMarketApi("marketLog");
#endif
			char addr[1024 + 1];
			memset(addr, 0, sizeof(addr));
			sprintf(addr,"%s:%s", m_pSource->get_auth_server_ip().c_str(), m_pSource->get_auth_port().c_str());
			m_pUserApi->RegisterSpi(this);
			m_pUserApi->Init();
//#if 0
//			m_pUserApi->AuthonInfo((char*)m_pSource->get_auth_code().c_str());
//#else
			char auth[128];
			memset(auth, 0, sizeof(auth));
			strncat(auth, (char*)m_pSource->get_auth_code().c_str(),80);			
			loggerv2::info("zd_connection::init auth:%s,len:%d,addr:%s\n",auth, strlen(auth),addr);
			m_pUserApi->AuthonInfo(auth);
//#endif
			m_pUserApi->RegisterLoginFront(addr);			
		}
		/*
		RegisterLoginFront 行情登陆，权限验证  RegisterFront订阅行情的地址
		*/
		void zd_connection::OnFrontLoginConnected()
		{
			if (m_pUserApi == nullptr)
				return;

			CTShZdReqUserLoginField field;
			memset(&field, 0, sizeof(CTShZdReqUserLoginField));
			memcpy(field.UserID, m_pSource->get_user_name().c_str(), 16);
			memcpy(field.Password, m_pSource->get_passwd().c_str(), 41);
			int ret = m_pUserApi->ReqUserLogin(&field, 1);	
//#if 0
//			char addr[1024 + 1];
//			memset(addr, 0, sizeof(addr));
//			sprintf(addr, "%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
//			m_pUserApi->RegisterFront(addr);
//#endif
		}
		void zd_connection::cleanup()
		{
			m_pUserApi->Release();
		}

		void zd_connection::Process()
		{
//#if 0
//			m_pUserApi->Join();
//#endif
			//loggerv2::info("end of thread");
		}
		/*
		char *ppInstrumentID[8];
		string temp1="HKEX,HHI1702;HKEX,HHI1703;HKEX,HHI1704;HKEX,HHI1705;HKEX,HHI1706";
		string temp2="HKEX,HHI1707;HKEX,HHI1708;HKEX,HHI1709;HKEX,HHI1710";
		string temp3="HKEX,HSI1702;HKEX,HSI1703;HKEX,HSI1704;HKEX,HSI1705;HKEX,HSI1706";
		string temp4="HKEX,HSI1707;HKEX,HSI1708;HKEX,HSI1709;HKEX,HSI1710";
		string temp5="ICE,BRN1704;ICE,BRN1705;ICE,BRN1706;ICE,BRN1707;ICE,BRN1708";
		string temp6="ICE,BRN1709;ICE,BRN1710;ICE,BRN1711;ICE,BRN1712";
		string temp7="LME,AA3M;LME,AH3M";
		string temp8="LME,CA3M";
		ppInstrumentID[0]=(char*)temp1.c_str();
		ppInstrumentID[1]=(char*)temp2.c_str();
		ppInstrumentID[2]=(char*)temp3.c_str();
		ppInstrumentID[3]=(char*)temp4.c_str();
		ppInstrumentID[4]=(char*)temp5.c_str();
		ppInstrumentID[5]=(char*)temp6.c_str();
		ppInstrumentID[6]=(char*)temp7.c_str();
		ppInstrumentID[7]=(char*)temp8.c_str();

		apiHandle->SubscribeMarketData(ppInstrumentID,8);
		*/
		bool zd_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi==nullptr)
				return false;
//#if 0
//			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
//#else
			financialinstrument* instrument = referential::get_instance()->get_instrument_map().get_by_key(item->get_code());
			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			if (instrument)
			{								
				sprintf(buffer,"%s,%s", instrument->get_exchange().c_str(), const_cast<char*>(item->get_feed_code().c_str()));				
			}
			char* instruments[] = { buffer };
//#endif
			int nbInstrument = 1;
			int res = m_pUserApi->SubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::error("zd_connection::subscribe_item - %s : Not OK !", buffer);
				return false;
			}
			loggerv2::info("zd_connection::subscribe_item - %s :  OK !", buffer);
				
			return true;
		}

		bool zd_connection::unsubscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			char* instruments[] = { const_cast<char*>(item->get_feed_code().c_str()) };
			int nbInstrument = 1;
			int res = m_pUserApi->UnSubscribeMarketData(instruments, nbInstrument);
			if (res != 0)
			{
				loggerv2::error("zd_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
			loggerv2::info("zd_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			
			return true;
		}
		//
		// private
		//
		bool zd_connection::request_login()
		{
			/*
			///直达用户登录请求
			struct CTShZdReqUserLoginField
			{
			///交易日
			TShZdDateType	TradingDay;
			///经纪公司代码
			TShZdBrokerIDType	BrokerID;
			///用户代码  直达必须填写
			TShZdUserIDType	UserID;
			///密码  直达必须填写
			TShZdPasswordType	Password;
			///用户端产品信息
			TShZdProductInfoType	UserProductInfo;
			///接口端产品信息
			TShZdProductInfoType	InterfaceProductInfo;
			///协议信息
			TShZdProtocolInfoType	ProtocolInfo;
			///Mac地址
			TShZdMacAddressType	MacAddress;
			///动态密码
			TShZdPasswordType	OneTimePassword;
			///终端IP地址  直达必须填写
			TShZdIPAddressType	ClientIPAddress;
			};
			*/
			// login
			CTShZdReqUserLoginField request;
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
		void zd_connection::OnRspError(CTShZdRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("zd_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("zd_connection::OnRspError - ok");
			}
		}

		void zd_connection::OnFrontConnected()
		{
			loggerv2::info("zd_connection is UP");

//#if 0
//			if (!request_login())
//				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "zd_connection::request_login failed");
//#else
			m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
//#endif

			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void zd_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::error("zd_connection::OnFrontDisconnected is DOWN");

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

		void zd_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			loggerv2::info("zd_connection - OnHeartBeatWarning warning");
		}

		void zd_connection::OnRspUserLogin(CTShZdRspUserLoginField *pRspUserLogin, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
		{
			loggerv2::info("zd_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				//m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
				//
				char addr[1024 + 1];
				memset(addr, 0, sizeof(addr));
				sprintf(addr, "%s:%s", m_pSource->get_service_name().c_str(), m_pSource->get_port().c_str());
				m_pUserApi->RegisterFront(addr);
				//
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void zd_connection::OnRspSubMarketData(CTShZdSpecificInstrumentField *pSpecificInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("zd_connection::OnRspSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void zd_connection::OnRspUnSubMarketData(CTShZdSpecificInstrumentField *pSpecificInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("zd_connection::OnRspUnSubMarketData %s failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void zd_connection::OnRtnDepthMarketData(CTShZdDepthMarketDataField* pDepthMarketData)
		{
			m_pSource->publish_msg((void*)pDepthMarketData, sizeof(CTShZdDepthMarketDataField), pDepthMarketData->InstrumentID);
			//
			// dynamic allocation for now
			// delete is done in cffex_source (different thread).
			//
			//CThostFtdcDepthMarketDataField* pData = new CThostFtdcDepthMarketDataField;
			//memcpy_lw(pData, pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
			//m_pSource->get_queue()->Push(pData);
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush(pDepthMarketData);
		}
	}
}
