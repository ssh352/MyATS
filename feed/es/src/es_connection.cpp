#include "es_connection.h"
#include "referential.h"
using namespace terra::common;
using namespace terra::instrument;
namespace feed
{
	namespace es
	{
		es_connection::es_connection(es_source* pSource)
		{
			m_pSource    = pSource;
			m_pUserApi   = nullptr;
			m_nRequestId = 0;
		}
		es_connection::~es_connection()
		{

		}
		void es_connection::init()
		{	
			//
			if (m_pSource->is_sub() == true)
				return;
			//
			//
			this->cleanup();
			//
			TAPIINT32 iResult = TAPIERROR_SUCCEED;
			TapAPIApplicationInfo stAppInfo;
			memset(&stAppInfo, 0, sizeof(TapAPIApplicationInfo));
			//to do ...
			strncpy(stAppInfo.AuthCode, m_pSource->get_auth_code().c_str(),512);
			strcpy(stAppInfo.KeyOperationLogPath,"");
			m_pUserApi = CreateTapQuoteAPI(&stAppInfo, iResult);

			if (m_pUserApi != nullptr)
			{
				m_pUserApi->SetAPINotify(this);			
			}
			else
			{
				printf_ex("es_connection::init m_pUserApi is Null,auth:%d\n", strlen(stAppInfo.AuthCode));
				loggerv2::error("es_connection::init m_pUserApi is Null,auth:%d\n", strlen(stAppInfo.AuthCode));
				return;
			}
			//to do ...设定服务器IP、端口
			TAPIINT32 iErr = TAPIERROR_SUCCEED;			
			iErr = m_pUserApi->SetHostAddress(m_pSource->get_service_name().c_str(),atoi(m_pSource->get_port().c_str()));
			if (TAPIERROR_SUCCEED != iErr) {				
				printf_ex("es_connection::init SetHostAddress Error:%d\n",iErr);
				loggerv2::error("es_connection::init SetHostAddress Error:%d\n", iErr);
				return;
			}

			//to do ... 登录服务器
			TapAPIQuoteLoginAuth stLoginAuth;
			memset(&stLoginAuth, 0, sizeof(stLoginAuth));
			strcpy(stLoginAuth.UserNo,   m_pSource->get_user_name().c_str());
			strcpy(stLoginAuth.Password, m_pSource->get_passwd().c_str());
			stLoginAuth.ISModifyPassword = APIYNFLAG_NO;
			stLoginAuth.ISDDA = APIYNFLAG_NO;
			/*
			发起登录请求。API将先连接服务，建立链路，发起登录认证。
			在使用函数函数前用户需要完成服务器的设置SetHostAddress()，并且创建TapAPIQuoteLoginAuth类型的用户信息， 并且需要设置好回调接口。
			连接建立后的用户验证回馈信息通过回调OnLogin()返回给用户。
			登录成功后API会自动进行API的初始化，API向服务器请求基础数据，查询完以后会通过回调OnAPIReady() 指示用户API初始化完成，可以进行后续的操作了。
			*/
			iErr = m_pUserApi->Login(&stLoginAuth);
			if (TAPIERROR_SUCCEED != iErr) {
				cout << "Login Error:" << iErr << endl;
				loggerv2::error("es_connection::init Login Error:%d\n", iErr);
				return;
			}
		}
		void es_connection::cleanup()
		{
			if (m_pUserApi != nullptr)
			{
				m_pUserApi->Disconnect();
				FreeTapQuoteAPI(m_pUserApi);
				m_pUserApi = nullptr;
			}
		}	
		bool es_connection::subscribe_item(feed_item* item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			TAPIINT32 iErr         = TAPIERROR_SUCCEED;
			TapAPIContract stContract;
			memset(&stContract, 0, sizeof(stContract));		
			stContract.CallOrPutFlag1 = TAPI_CALLPUT_FLAG_NONE;
			stContract.CallOrPutFlag2 = TAPI_CALLPUT_FLAG_NONE;
			//exchangeNo
			financialinstrument* instrument = referential::get_instance()->get_instrument_map().get_by_key(item->get_code());
			if (instrument)
			{
				if (instrument->get_exchange().find("ZCE") != string::npos)
				{
					strcpy(stContract.Commodity.ExchangeNo, "ZCE");
				}
				else
				{
					strcpy(stContract.Commodity.ExchangeNo, instrument->get_exchange().c_str());
				}
			}
			switch (item->get_type())
			{			
			case  AtsType::InstrType::type::Future:
			case  AtsType::InstrType::type::Futurespread:
				stContract.Commodity.CommodityType = TAPI_COMMODITY_TYPE_FUTURES;
				break;			
			case  AtsType::InstrType::type::Call:
				stContract.Commodity.CommodityType = TAPI_COMMODITY_TYPE_OPTION;
				stContract.CallOrPutFlag1          = TAPI_CALLPUT_FLAG_CALL;				
				strcpy(stContract.StrikePrice1, get_Strike(item->get_feed_code()).c_str());
				break;
			case  AtsType::InstrType::type::Put:
				stContract.Commodity.CommodityType = TAPI_COMMODITY_TYPE_OPTION;
				stContract.CallOrPutFlag1          = TAPI_CALLPUT_FLAG_PUT;				
				strcpy(stContract.StrikePrice1, get_Strike(item->get_feed_code()).c_str());
				break;
			default:
				break;
			}										
			strcpy(stContract.Commodity.CommodityNo, get_CommodityNo(item->get_feed_code()).c_str());
			strcpy(stContract.ContractNo1, get_ContractNo(item->get_feed_code()).c_str());					
			m_nRequestId++;
			if (m_pUserApi)
			{
				iErr = m_pUserApi->SubscribeQuote(&m_nRequestId, &stContract);			
				if (TAPIERROR_SUCCEED != iErr)
				{
					loggerv2::error("es_connection::subscribe_item - %s:%s|%s|%c|%s :  Fail:%d !", item->get_feed_code().c_str(), stContract.Commodity.CommodityNo, stContract.ContractNo1, stContract.CallOrPutFlag1, stContract.StrikePrice1, iErr);
					return false;
				}				
			}
			loggerv2::info("es_connection::subscribe_item - %s :  OK !", item->get_feed_code().c_str());			
			return true;
		}

		bool es_connection::unsubscribe_item(feed_item * item)
		{
			if (item == nullptr || m_pUserApi == nullptr)
				return false;
			TAPIINT32 iErr = TAPIERROR_SUCCEED;
			TapAPIContract stContract;
			memset(&stContract, 0, sizeof(stContract));
			stContract.CallOrPutFlag1 = TAPI_CALLPUT_FLAG_NONE;
			stContract.CallOrPutFlag2 = TAPI_CALLPUT_FLAG_NONE;
			//exchangeNo
			financialinstrument* instrument = referential::get_instance()->get_instrument_map().get_by_key(item->get_code());
			if (instrument)
			{
				if (instrument->get_exchange().find("ZCE") != string::npos)
				{
					strcpy(stContract.Commodity.ExchangeNo, "ZCE");
				}
				else
				{
					strcpy(stContract.Commodity.ExchangeNo, instrument->get_exchange().c_str());
				}
			}
			switch (item->get_type())
			{
			case  AtsType::InstrType::type::Future:
			case  AtsType::InstrType::type::Futurespread:
				stContract.Commodity.CommodityType = TAPI_COMMODITY_TYPE_FUTURES;
				break;
			case  AtsType::InstrType::type::Call:
				stContract.Commodity.CommodityType = TAPI_COMMODITY_TYPE_OPTION;
				stContract.CallOrPutFlag1 = TAPI_CALLPUT_FLAG_CALL;
				strcpy(stContract.StrikePrice1, get_Strike(item->get_feed_code()).c_str());
				break;
			case  AtsType::InstrType::type::Put:
				stContract.Commodity.CommodityType = TAPI_COMMODITY_TYPE_OPTION;
				stContract.CallOrPutFlag1 = TAPI_CALLPUT_FLAG_PUT;
				strcpy(stContract.StrikePrice1, get_Strike(item->get_feed_code()).c_str());
				break;
			default:
				break;
			}
			strcpy(stContract.Commodity.CommodityNo, get_CommodityNo(item->get_feed_code()).c_str());
			strcpy(stContract.ContractNo1, get_ContractNo(item->get_feed_code()).c_str());
			m_nRequestId++;
			iErr = m_pUserApi->UnSubscribeQuote(&m_nRequestId, &stContract);
			if (TAPIERROR_SUCCEED != iErr) {
				loggerv2::error("es_connection::unsubscribe_item - %s:%s|%s|%c|%s :  Fail:%d !", item->get_feed_code().c_str(), stContract.Commodity.CommodityNo, stContract.ContractNo1, stContract.CallOrPutFlag1, stContract.StrikePrice1, iErr);
				return false;
			}
			loggerv2::info("es_connection::unsubscribe_item - %s :  OK !", item->get_feed_code().c_str());
			return true;
		}

		void TAP_CDECL es_connection::OnDisconnect(TAPIINT32 reasonCode)
		{
			loggerv2::error("es_connection is DOWN");
			std::string pszMessage;
			switch (reasonCode)
			{
			case 0:
				pszMessage.clear();
				break;
			case -1:
				pszMessage = "TAPIERROR_ConnectFail";
				break;
			case -2:
				pszMessage = "TAPIERROR_LinkAuthFail";
				break;
			case -3:
				pszMessage = "TAPIERROR_HostUnavailable";
				break;
			default:
				pszMessage = "unknown error";
				break;
			}
			m_pSource->update_state(AtsType::FeedSourceStatus::Down, pszMessage);
			m_bReconnected = true;
		}
		void TAP_CDECL es_connection::OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo *info)
		{
			loggerv2::error("OnRspLogin:%d",errorCode);
		}
		void TAP_CDECL es_connection::OnAPIReady()
		{
			m_pSource->update_state(AtsType::FeedSourceStatus::Up,"");

			if (m_bReconnected)
				m_pSource->resubscribe_all();
		}		
		void TAP_CDECL es_connection::OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole *info)
		{
			if (errorCode != 0)
			{
				loggerv2::error("es_connection::OnRspSubscribeQuote error:%d,%s%s%c%s", errorCode,info->Contract.Commodity.CommodityNo,info->Contract.ContractNo1,info->Contract.CallOrPutFlag1,info->Contract.StrikePrice1);
			}
			else
			{
				m_pSource->get_queue()->CopyPush((TapAPIQuoteWhole *)info);
			}			
		}
		void TAP_CDECL es_connection::OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract *info)
		{
			if (errorCode != 0)
			{
				loggerv2::error("es_connection::OnRspUnSubscribeQuote error:%d",errorCode);
			}
		}
		void TAP_CDECL es_connection::OnRtnQuote(const TapAPIQuoteWhole *info)
		{
			//
			std::string sFeedCode;
			switch (info->Contract.Commodity.CommodityType)
			{
			case TAPI_COMMODITY_TYPE_FUTURES:
				sFeedCode = string(info->Contract.Commodity.CommodityNo) + string(info->Contract.ContractNo1);
				break;
			case TAPI_COMMODITY_TYPE_OPTION:
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%s%s%c%s", info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1, info->Contract.CallOrPutFlag1, info->Contract.StrikePrice1);
				sFeedCode = buffer;
				break;
			default:
				break;
			}
			m_pSource->publish_msg((void*)info, sizeof(TapAPIQuoteWhole), sFeedCode);
			//
			if (m_pSource->get_strPub() != "pub")
				m_pSource->get_queue()->CopyPush((TapAPIQuoteWhole *)info);
		}
		void TAP_CDECL es_connection::OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo *info)
		{
			if (info)
			{
				printf_ex("es_connection::OnRspQryCommodity %s,%s\n", info->CommodityName, info->CommodityEngName);
			}
		}
		void TAP_CDECL es_connection::OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo *info)
		{
			if (info)
			{
				//printf_ex("es_connection::OnRspQryContract %s,%s,%s,%c\n", info->Contract.Commodity.ExchangeNo, info->Contract.Commodity.CommodityNo,info->Contract.ContractNo1,info->Contract.Commodity.CommodityType);
				loggerv2::info("es_connection::OnRspQryContract %s,%s,%s,%c\n", info->Contract.Commodity.ExchangeNo, info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1, info->Contract.Commodity.CommodityType);
			}			
		}
		string es_connection::get_CommodityNo(string code)
		{
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			strcat(buffer, code.c_str());
			string str;
			for (int i = 0; i < strlen(buffer); i++)
			{
				if (isdigit(buffer[i]) == 0)
				{
					str.push_back(buffer[i]);
				}
				else
					break;
			}			
			transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(std::toupper));
			//printf_ex("get_CommodityNo code:%s,CommodityNo:%s\n", code.c_str(), str.c_str());
			return str;
		}
		string es_connection::get_ContractNo(string code)
		{
			char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			strcat(buffer, code.c_str());
			string str;
			bool bFound = false;
			for (int i = 0; i < strlen(buffer); i++)
			{
				if (bFound == false)
				{
					if (isdigit(buffer[i]) != 0)//digit
					{
						bFound = true;
						str.push_back(buffer[i]);
					}					
				}
				else
				{
					if (isdigit(buffer[i]) != 0)//digit
					{						
						str.push_back(buffer[i]);
					}
					else
						break;					
				}
			}
			//printf_ex("get_ContractNo code:%s,ContractNo:%s\n", code.c_str(), str.c_str());
			return str;
		}
		string es_connection::get_Strike(string code)
		{
			string str=code.substr(code.length()-4,code.length()-1);
			//printf_ex("get_Strike code:%s,strike:%s\n", code.c_str(), str.c_str());
			return str;
		}
	}
}