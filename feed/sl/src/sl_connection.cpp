#include "sl_connection.h"
using namespace terra::common;
namespace feed
{
	namespace sl
	{
		sl_connection::sl_connection(sl_source* pSource)
		{
			m_pSource = pSource;
			m_pUserApi = nullptr;
			m_nRequestId = 0;
		}
		sl_connection::~sl_connection()
		{

		}
		void sl_connection::init()
		{
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			if (m_pSource->isUdp())
			{
				loggerv2::info("sl_connection using UDP connection - no implement");
			}
			else
			{
				loggerv2::info("sl_connection using TCP connection");
				m_pUserApi = CreateEESQuoteApi();

				vector<EqsTcpInfo> tcp_info_vector;
				EqsTcpInfo tcp_info;
				memset(&tcp_info, 0, sizeof(EqsTcpInfo));
				strcpy(tcp_info.m_eqsIp, m_pSource->get_service_name().c_str());
				tcp_info.m_eqsPort = atoi(m_pSource->get_port().c_str());
				tcp_info_vector.push_back(tcp_info);
				m_pUserApi->ConnServer(tcp_info_vector, this);
			}
		}
		void sl_connection::cleanup()
		{
			if (m_pUserApi != nullptr)
			{
				m_pUserApi->DisConnServer();
				DestroyEESQuoteApi(m_pUserApi);
				m_pUserApi = nullptr;
			}
		}
		bool sl_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
#if 0
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
				loggerv2::info("sl_connection::subscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
#else
			/*
			struct InstrType {
			enum type {
			Undef = 0,
			Index = 1,
			Stock = 2,
			Option = 3,
			Call = 4,
			Put = 5,
			Future = 6,
			Futurespread = 7,
			ETF = 8,
			Repo = 9,
			Bond = 10,
			Fund = 11,
			Max = 12
			};
			};
			*/
			EesEqsIntrumentType eqs_type = EesEqsIntrumentType::EQS_INVALID_TYPE;
			/*
			/// \brief EES��������
			enum EesEqsIntrumentType
			{
			EQS_INVALID_TYPE = '0', ///< ��Ч����
			EQS_SH_STOCK,           ///< �Ϻ���Ʊ
			EQS_SZ_STOCK,           ///< ���ڹ�Ʊ
			EQS_STOCK_OPTION,       ///< ��Ʊ��Ȩ
			EQS_FUTURE_OPTION,      ///< �ڻ���Ȩ
			EQS_INDEX_OPTION,       ///< ��ָ��Ȩ
			EQS_FUTURE,             ///< �ڻ�
			};
			*/
			switch (item->get_type())
			{			
			case  AtsType::InstrType::type::Future:
			case  AtsType::InstrType::type::Futurespread:
				eqs_type = EesEqsIntrumentType::EQS_FUTURE;
				break;
			default:
				break;
			}
			if (m_pUserApi)
			{
				m_pUserApi->RegisterSymbol(eqs_type, item->get_feed_code().c_str());
			}
#endif			
			loggerv2::info("sl_connection::subscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}


		bool sl_connection::unsubscribe_item(feed_item * item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
#if 0
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
				loggerv2::info("sl_connection::unsubscribe_item - %s : Not OK !", item->get_code().c_str());
				return false;
			}
#else
			/*
			struct InstrType {
			enum type {
			Undef = 0,
			Index = 1,
			Stock = 2,
			Option = 3,
			Call = 4,
			Put = 5,
			Future = 6,
			Futurespread = 7,
			ETF = 8,
			Repo = 9,
			Bond = 10,
			Fund = 11,
			Max = 12
			};
			};
			*/
			EesEqsIntrumentType eqs_type = EesEqsIntrumentType::EQS_INVALID_TYPE;
			/*
			/// \brief EES��������
			enum EesEqsIntrumentType
			{
			EQS_INVALID_TYPE = '0', ///< ��Ч����
			EQS_SH_STOCK,           ///< �Ϻ���Ʊ
			EQS_SZ_STOCK,           ///< ���ڹ�Ʊ
			EQS_STOCK_OPTION,       ///< ��Ʊ��Ȩ
			EQS_FUTURE_OPTION,      ///< �ڻ���Ȩ
			EQS_INDEX_OPTION,       ///< ��ָ��Ȩ
			EQS_FUTURE,             ///< �ڻ�
			};
			*/
			switch (item->get_type())
			{
			case  AtsType::InstrType::type::Future:
			case  AtsType::InstrType::type::Futurespread:
				eqs_type = EesEqsIntrumentType::EQS_FUTURE;
				break;
			default:
				break;
			}
			if (m_pUserApi)
			{
				m_pUserApi->UnregisterSymbol(eqs_type, item->get_feed_code().c_str());
			}
#endif			
			loggerv2::info("sl_connection::unsubscribe_item - %s :  OK !", item->get_code().c_str());
			return true;
		}


#if 0
		bool sl_connection::request_login()
		{
			// login
			_LTS_::CSecurityFtdcUserLoginField request;
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
		void sl_connection::OnRspError(CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
			{
				loggerv2::error("sl_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
			else
			{
				loggerv2::info("sl_connection::OnRspError - ok");
			}
		}

		void sl_connection::OnFrontConnected()
		{
			loggerv2::info("sl_connection is Connected - Going to Login");

			if (!request_login())
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, "sl_connection::request_login failed");
		
			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}

		void sl_connection::OnFrontDisconnected(int nReason)
		{
			loggerv2::info("sl_connection is DOWN");

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

		void sl_connection::OnHeartBeatWarning(int nTimeLapse)
		{
			//loggerv2::info("sl_connection - heartbeat warning");
		}

		void sl_connection::OnRspUserLogin(CSecurityFtdcUserLoginField* pRspUserLogin, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			loggerv2::info("sl_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

			if (pRspInfo->ErrorID == 0)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, "");
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pRspInfo->ErrorMsg);
			}
		}

		void sl_connection::OnRspSubL2MarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("sl_connection::subscribe_item  %s level 2 data failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void sl_connection::OnRspUnSubL2MarketData(CSecurityFtdcSpecificInstrumentField* pSpecificInstrument, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
		{
			if (pRspInfo->ErrorID != 0)
			{
				loggerv2::error("sl_connection::unsubscribe_item %s level 2 data failed (%d, %s)", pSpecificInstrument->InstrumentID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			}
		}

		void sl_connection::OnRtnL2MarketData(CSecurityFtdcL2MarketDataField *pL2MarketData)
		{
			/*loggerv2::info("sl_connection::OnRtnL2MarketData");
			CSecurityFtdcL2MarketDataField* pData = new CSecurityFtdcL2MarketDataField;
			memcpy(pData, pL2MarketData, sizeof(CSecurityFtdcL2MarketDataField));
			m_pSource->get_queue()->Push(pData);*/
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush(pL2MarketData);
		}
	}
#else
		/// \brief �����������ӳɹ�����¼ǰ����, ������鲥ģʽ���ᷢ��, ֻ���ж�InitMulticast����ֵ����
		void sl_connection::OnEqsConnected()
		{
			loggerv2::info("sl_connection is Connected - Going to Login");
			EqsLoginParam param;
			memset(&param, 0, sizeof(EqsLoginParam));
			strcpy(param.m_loginId, m_pSource->get_user_name().c_str());
			strcpy(param.m_password, m_pSource->get_passwd().c_str());
			this->m_pUserApi->LoginToEqs(param);
		}
		/// \brief ���������������ӳɹ������Ͽ�ʱ���ã��鲥ģʽ���ᷢ�����¼�
		void sl_connection::OnEqsDisconnected()
		{
			m_pSource->update_state(AtsType::FeedSourceStatus::Down, "");
		}
		/// \brief ����¼�ɹ�����ʧ��ʱ���ã��鲥ģʽ���ᷢ��
		/// \param bSuccess ��½�Ƿ�ɹ���־  
		/// \param pReason  ��½ʧ��ԭ��  
		void sl_connection::OnLoginResponse(bool bSuccess, const char* pReason)
		{
			if (bSuccess == true)
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Up, pReason);
			}
			else
			{
				m_pSource->update_state(AtsType::FeedSourceStatus::Down, pReason);
			}
		}
		/// \brief �յ�����ʱ����,�����ʽ����instrument_type��ͬ����ͬ
		/// \param chInstrumentType  EES��������
		/// \param pDepthQuoteData   EESͳһ����ָ��  
		void sl_connection::OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* pDepthQuoteData)
		{
			//pDepthQuoteData->UpdateMillisec = microsec_clock::local_time().time_of_day().total_milliseconds();

			m_pSource->publish_msg((void*)pDepthQuoteData, sizeof(EESMarketDepthQuoteData), pDepthQuoteData->InstrumentID);
			if (m_pSource->get_strPub() != "pub")
			{
				m_pSource->get_queue()->CopyPush(pDepthQuoteData);
			}
		}
		/// \brief ��־�ӿڣ���ʹ���߰���д��־��
		/// \param nlevel    ��־����
		/// \param pLogText  ��־����
		/// \param nLogLen   ��־����
		void sl_connection::OnWriteTextLog(EesEqsLogLevel nlevel, const char* pLogText, int nLogLen)
		{
		}
		/// \brief ע��symbol��Ӧ��Ϣ��ʱ���ã��鲥ģʽ��֧������ע��
		/// \param chInstrumentType  EES��������
		/// \param pSymbol           ��Լ����
		/// \param bSuccess          ע���Ƿ�ɹ���־
		void sl_connection::OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess)
		{
		}
		/// \brief  ע��symbol��Ӧ��Ϣ��ʱ���ã��鲥ģʽ��֧������ע��
		/// \param chInstrumentType  EES��������
		/// \param pSymbol           ��Լ����
		/// \param bSuccess          ע���Ƿ�ɹ���־
		void sl_connection::OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess)
		{
		}
		/// \brief ��ѯsymbol�б���Ӧ��Ϣ��ʱ���ã��鲥ģʽ��֧�ֺ�Լ�б��ѯ
		/// \param chInstrumentType  EES��������
		/// \param pSymbol           ��Լ����
		/// \param bLast             ���һ����ѯ��Լ�б���Ϣ�ı�ʶ
		/// \remark ��ѯ��Լ�б���Ӧ, last = trueʱ��������������Ч���ݡ�
		void sl_connection::OnSymbolListResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bLast)
		{
		}
#endif
	}
}