#include "lts_file_reqapi.h"
#include "lts_file_connection.h"
#include "tradeItem_gh.h"


#include "sqliteclient.h"
#include <boost/algorithm/string.hpp>

#include "FastMemcpy.h"
#include "terra_logger.h"
using namespace terra::common;
using namespace boost::algorithm;
//using namespace boost;
namespace lts_file
{
	lts_file_reqapi::lts_file_reqapi(lts_file_connection* pConnection)
	{
		m_pConnection = pConnection;
		m_isAlive = false;
		m_nRequestId = 0;
		if (m_pConnection->m_blts_wrapper == false)
			m_pQueryApi = CSecurityFtdcQueryApi::CreateFtdcQueryApi("");
		else
		{
			m_pQueryApi = CLTSAgent::CreateQueryApi(m_pConnection->m_sUsername.c_str(), m_pConnection->m_sPasswordQry.c_str(), m_pConnection->m_str_ltsSrv.c_str(),this);

		}
	}

	lts_file_reqapi::~lts_file_reqapi()
	{

	}


	void lts_file_reqapi::Process()
	{
	
	}
	
	void lts_file_reqapi::request_login(CSecurityFtdcAuthRandCodeField *pAuthRandCode)
	{
		if (m_pConnection->m_blts_wrapper)
			return;
		loggerv2::info("lts_file_reqapi::request_login");
		
		CSecurityFtdcReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->m_sUsername.c_str());

		strcpy(request.RandCode, pAuthRandCode->RandCode);
		//strcpy(request.UserProductInfo, "LTS-Test");
		//strcpy(request.AuthCode, "N3EHKP4CYHZGM9VJ");
		strcpy(request.UserProductInfo, m_pConnection->m_sProductInfo.c_str());
		strcpy(request.AuthCode, m_pConnection->m_sAuthCode.c_str());


		strcpy(request.Password, m_pConnection->m_sPasswordQry.c_str());
		loggerv2::info("lts_file_reqapi::request_login broker %s, user %s, ranCode %s, password %s", m_pConnection->m_sBrokerId.c_str(), m_pConnection->m_sUsername.c_str(), pAuthRandCode->RandCode, m_pConnection->m_sPasswordQry.c_str());
		int res = m_pQueryApi->ReqUserLogin(&request, ++m_nRequestId);
		//if (res != 0)
		//{
		//	m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "lts_api - ReqUserLogin failed");
		//}
	}
	
	void lts_file_reqapi::request_auth()
	{
		if (m_pConnection->m_blts_wrapper)
		{
			return;
		}
		loggerv2::info("lts_file_reqapi::request_auth.");
		CSecurityFtdcAuthRandCodeField pAuthRandCode;
		memset(&pAuthRandCode, 0, sizeof(pAuthRandCode));
		strcpy(pAuthRandCode.RandCode, "");
		int iResult2 = m_pQueryApi->ReqFetchAuthRandCode(&pAuthRandCode, ++m_nRequestId);
	}

	void lts_file_reqapi::OnRspQryInstrument(CSecurityFtdcInstrumentField *pInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pInstrument == nullptr)
			return;
		if (pInstrument->ProductClass == '2') //Options
			 {
			loggerv2::info("lts_file_reqapi::OnRspQryInstrument"
				 " CreateDate[%s]"
				 "DeliveryYear[%d]"
				 "DeliveryMonth[%d]"
				 "EndDelivDate[%s]"
				 "ExpireDate[%s]"
				 "InstLifePhase[%c]"
				 "InstrumentID[%s]"
				 "InstrumentName[%s]"
				 "VolumeMultiple[%d]"
				 "MarketID[%s]"
				 "ProductClass[%c]"
				 "RightModelID[%s]"
				 "ExchangeID[%s]"
				 "ExchangeInstID[%s]"
				 "ProductID[%s]"
				 "PositionType[%c]"
				 "ExecPrice[%f]"
				 "InstrumentType[%c]"
				 "PriceTick[%f]",
				pInstrument->CreateDate,
				pInstrument->DeliveryYear,
				pInstrument->DeliveryMonth,
				pInstrument->EndDelivDate,
				pInstrument->ExpireDate,
				pInstrument->InstLifePhase,
				pInstrument->InstrumentID,
				pInstrument->InstrumentName,
				pInstrument->VolumeMultiple,
				pInstrument->MarketID,
				pInstrument->ProductClass,
				pInstrument->RightModelID,
				pInstrument->ExchangeID,
				pInstrument->ExchangeInstID,
				pInstrument->ProductID,
				pInstrument->PositionType,
				pInstrument->ExecPrice,
				pInstrument->InstrumentType,
				pInstrument->PriceTick
				 );
						//TODO here.
				
				
			std::string sInstr = std::string(pInstrument->ExchangeInstID);
			trim(sInstr);
			
			std::string sSearch = "select * from Options where ConnectionCodes like '" + std::string(pInstrument->InstrumentID) + "%'";
			const char* data = "Callback function called";
			char *zErrMsg = 0;
			
			std::string sUnderlying = "A"; //
			std::string sCP = "C";  //"CallPut"
			std::string sInstClass = "D";
			
			getOptionsFeatures(pInstrument->ExchangeInstID, sUnderlying, sCP, sInstClass);
			std::string sMat = getMaturity(pInstrument->ExpireDate); //pInstrument->ExpireDate;
			
			std::string sCmd = "";
			
			std::vector<boost::property_tree::ptree>* pTree = m_pConnection->m_database->get_table(sSearch.c_str());
			
				
				
			if (pTree->size() == 0) //instrument doesn't exist
			{
								//loggerv2::info("Could not find instrument %s", std::string(pInstrument->InstrumentID).c_str());
					sqlite3_free(zErrMsg);
				
					sCmd = "INSERT INTO Options VALUES (";
				sCmd += "'" + sInstr + "',";
				sCmd += "'" + std::string(pInstrument->MarketID) + "',";
				sCmd += "'" + sInstr + "',";
				sCmd += "' ',";
				sCmd += "'" + std::string(pInstrument->InstrumentID) + "@LTSUDP|" + std::string(pInstrument->InstrumentID) + "." + std::string(pInstrument->MarketID) + "@LTS|" + std::string(pInstrument->InstrumentID) + ".SH" + "@TDF|SH" + std::string(pInstrument->InstrumentID) + "@XS',";
				sCmd += "'" + std::string(pInstrument->InstrumentID) + "." + std::string(pInstrument->MarketID) + "@LTS|" + std::string(pInstrument->InstrumentID) + "." + std::string(pInstrument->MarketID) + "@XS" + "',";
				sCmd += "'" + sUnderlying + "',";
				sCmd += "'" + sMat + "',";
				sCmd += "'" + std::to_string(pInstrument->ExecPrice) + "',";
				sCmd += "'" + std::to_string(pInstrument->VolumeMultiple) + "',";
				sCmd += "'" + sCP + "',";
				sCmd += "'" + sInstClass + "')";
				
					int rc = m_pConnection->m_database->executeNonQuery(sCmd.c_str());
				
					if (rc == 0)
					 {
										//loggerv2::info("failed to insert into database, ret is %d",rc);
						sqlite3_free(zErrMsg);
					}
				
			}

			else //exists
			{
								//loggerv2::info("instrument %s exist already in the database", std::string(pInstrument->InstrumentID).c_str());
				std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@LTS";
				sCmd = "UPDATE Options SET ";
				sCmd += "Code = '" + sInstr + "',";
				sCmd += "ISIN = '" + sInstr + "',";
				sCmd += "Maturity = '" + sMat + "',";
				sCmd += "Strike = '" + std::to_string(pInstrument->ExecPrice) + "',";
				sCmd += "PointValue ='" + std::to_string(pInstrument->VolumeMultiple) + "'";
				sCmd += " where Code='" + sConnectionCodes + "';";
				
					int rc = m_pConnection->m_database->executeNonQuery(sCmd.c_str());
				
					if (rc == 0)
					 {
										//loggerv2::info("failed to update the database,error is %d",rc);
						sqlite3_free(zErrMsg);
					}
			}
		}
		if (bIsLast)
		{
			m_pConnection->m_database->close_databse();
			m_pConnection->m_bIsDicoRdy = true;
			
		}

	}


	std::string lts_file_reqapi::getMaturity(std::string sMat)
	{
		std::string newMat;
		newMat = sMat.substr(0, 4);
		newMat += "-";
		newMat += sMat.substr(4, 2);
		newMat += "-";
		newMat += sMat.substr(6, 2);
		return newMat.c_str();
	}
	
	void lts_file_reqapi::getOptionsFeatures(const char* Code, std::string& underlying, std::string& CallPut, std::string& instrumentCalss)
	{
		char sUnder[6 + 1];
		char sCallPut[1 + 1];
		std::string sInstrumentClass = "O_";
		
		memcpy_lw(sUnder, Code, 6);
		sUnder[6] = '\0';
		underlying = std::string(sUnder);
		
		memcpy_lw(sCallPut, Code + 6, 1);
		sCallPut[1] = '\0';
		CallPut = std::string(sCallPut);
		sInstrumentClass += underlying.c_str();
		instrumentCalss = std::string(sInstrumentClass);
		return;
	}



	void lts_file_reqapi::OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pInvestorPosition, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pInvestorPosition != NULL)
			loggerv2::info("lts_file_reqapi::OnRspQryInvestorPosition InstrumentID[%s]"
			"ExchangeID[%s]"
			"PosiDirection[%c]"
			"YdPosition[%d]"
			"Position[%d] "
			"LongFrozen[%d]"
			"ShortFrozen[%d]"
			"LongFrozenAmount[%f]"
			"ShortFrozenAmount[%f]"

			"LockFrozenPosition[%d]"
			"LockPosition[%d]"
			"UnlockFrozenPosition[%d]"


			"OpenVolume[%d]"
			"CloseVolume[%d]"
			"OpenAmount[%f]"
			"CloseAmount[%f]"
			"PositionCost[%f]"
			"FrozenCash[%f]"
			"CashIn[%f]"
			"Commission[%f]"
			"ExchangeMargin[%f]"
			"TodayPosition[%d]",

			pInvestorPosition->InstrumentID,
			pInvestorPosition->ExchangeID,
			pInvestorPosition->PosiDirection,
			pInvestorPosition->YdPosition,
			pInvestorPosition->Position,
			pInvestorPosition->LongFrozen,
			pInvestorPosition->ShortFrozen,
			pInvestorPosition->LongFrozenAmount,
			pInvestorPosition->ShortFrozenAmount,

			pInvestorPosition->LockFrozenPosition,
			pInvestorPosition->LockPosition,
			pInvestorPosition->UnlockFrozenPosition,

			pInvestorPosition->OpenVolume,
			pInvestorPosition->CloseVolume,
			pInvestorPosition->OpenAmount,
			pInvestorPosition->CloseAmount,
			pInvestorPosition->PositionCost,
			pInvestorPosition->FrozenCash,
			pInvestorPosition->CashIn,
			pInvestorPosition->Commission,
			pInvestorPosition->ExchangeMargin,
			pInvestorPosition->TodayPosition
			);

		if (pInvestorPosition->InstrumentID)
		{

			//std::string instr = std::string(pInvestorPosition->InstrumentID) + "." + std::string(pInvestorPosition->ExchangeID);
			std::string instr = std::string(pInvestorPosition->InstrumentID) + "@" + m_pConnection->getName();

			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(instr.c_str());
			if (i)
			{
				//loggerv2::info("lts_api::OnRspQryInvestorPosition found instrument %s", pInvestorPosition->InstrumentID);
				if (pInvestorPosition->PosiDirection == '2')//long
				{
					i->set_tot_long_position(int(pInvestorPosition->Position));
					i->set_today_long_position(int(pInvestorPosition->TodayPosition));
					i->set_pending_short_close_qty(int(pInvestorPosition->ShortFrozen));

				}

				else if (pInvestorPosition->PosiDirection == '1') // net
				{
					i->set_tot_long_position(int(pInvestorPosition->Position));
					i->set_today_long_position(int(pInvestorPosition->TodayPosition));
					i->set_pending_short_close_qty(int(pInvestorPosition->ShortFrozen));
					i->set_frozen_long_position(int(pInvestorPosition->LockPosition));
					i->set_yst_long_position(int(pInvestorPosition->YdPosition));
					i->set_open_position(int(pInvestorPosition->OpenVolume));
					i->set_closed_position(int(pInvestorPosition->CloseVolume));
				}



				//3+4 = total short position in options
				else if (pInvestorPosition->PosiDirection == '3')//short
				{
					i->set_tot_short_position((int)(pInvestorPosition->Position));
					i->set_today_short_position((int)(pInvestorPosition->TodayPosition));
					i->set_pending_long_close_qty((int)(pInvestorPosition->LongFrozen));
				}

				else if (pInvestorPosition->PosiDirection == '4') // covered sell
				{
					i->set_covered_sell_open_position((int)(pInvestorPosition->Position));
					i->set_pending_covered_sell_close_qty((int)(pInvestorPosition->LongFrozen));
					i->set_pending_covered_sell_open_qty((int)(pInvestorPosition->ShortFrozen));
				}

				i->set_last_sychro_timepoint(get_lwtp_now());

				if (m_pConnection->m_debug)
					i->dumpinfo();

			}
			else
				loggerv2::warn("lts_file_reqapi::OnRspQryInvestorPosition cannot find instrument %s", pInvestorPosition->InstrumentID);
		}



	}
	void lts_file_reqapi::OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pTradingAccount, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::info("lts_file_reqapi::OnRspQryTradingAccount"

			"AccountID[%s]"
			"AccountType[%c]"
			"PreMortgage[%f]"
			"PreCredit[%f]"
			"PreDeposit[%f]"
			"PreBalance[%f]"
			"PreMargin[%f]"
			"Interest[%f]"
			"Deposit[%f]"
			"Withdraw[%f]"
			"FrozenMargin[%f]"
			"FrozenCash[%f]"
			"FrozenCommission[%f]"
			"CurrMargin[%f]"
			"CashIn[%f]"
			"Commission[%f]"
			"Balance[%f]"
			"Available[%f]"
			"WithdrawQuota[%f]"
			"Reserve[%f]"
			"Credit[%f]"
			"Mortgage[%f]"
			"ExchangeMargin[%f]",


			pTradingAccount->AccountID,
			pTradingAccount->AccountType,
			pTradingAccount->PreMortgage,
			pTradingAccount->PreCredit,
			pTradingAccount->PreDeposit,
			pTradingAccount->PreBalance,
			pTradingAccount->PreMargin,
			pTradingAccount->Interest,
			pTradingAccount->Deposit,
			pTradingAccount->Withdraw,
			pTradingAccount->FrozenMargin,
			pTradingAccount->FrozenCash,
			pTradingAccount->FrozenCommission,
			pTradingAccount->CurrMargin,
			pTradingAccount->CashIn,
			pTradingAccount->Commission,
			pTradingAccount->Balance,
			pTradingAccount->Available,
			pTradingAccount->WithdrawQuota,
			pTradingAccount->Reserve,
			pTradingAccount->Credit,
			pTradingAccount->Mortgage,
			pTradingAccount->ExchangeMargin
			);

		tradingaccount* ta = new tradingaccount(
			m_pConnection->getName(),
			pTradingAccount->AccountID,
			pTradingAccount->PreMortgage,
			pTradingAccount->PreCredit,
			pTradingAccount->PreDeposit,
			pTradingAccount->PreBalance,
			pTradingAccount->PreMargin,
			pTradingAccount->Interest,
			pTradingAccount->Deposit,
			pTradingAccount->Withdraw,
			pTradingAccount->FrozenMargin,
			pTradingAccount->FrozenCash,
			pTradingAccount->FrozenCommission,
			pTradingAccount->CurrMargin,
			pTradingAccount->CashIn,
			pTradingAccount->Commission,
			pTradingAccount->Balance,
			pTradingAccount->Available,
			pTradingAccount->WithdrawQuota,
			pTradingAccount->Reserve,
			pTradingAccount->Credit,
			pTradingAccount->Mortgage,
			pTradingAccount->ExchangeMargin);



		m_pConnection->on_trading_account_cb(ta);

		delete ta;

	}
	void lts_file_reqapi::request_instruments()
	{
		CSecurityFtdcQryInstrumentField request;
		memset(&request, 0, sizeof(request));
		m_pQueryApi->ReqQryInstrument(&request, ++m_nRequestId);
	}

	bool lts_file_reqapi::ReqQryTradingAccount(CSecurityFtdcQryTradingAccountField *pQryTradingAccount)
	{
		int ret = m_pQueryApi->ReqQryTradingAccount(pQryTradingAccount, ++m_nRequestId);
		loggerv2::info("lts_file_reqapi::ReqQryTradingAccount brokerid %s , requestId %d", pQryTradingAccount->BrokerID, m_nRequestId);
		if (ret != 0)
		{
			return false;
		}

		return true;
	}
	
	bool lts_file_reqapi::ReqQryInvestorPosition(CSecurityFtdcQryInvestorPositionField *pQryInvestorPosition)
	{

		//loggerv2::info("calling cffex_api::ReqQryInvestorPosition");
		int ret = m_pQueryApi->ReqQryInvestorPosition(pQryInvestorPosition, ++m_nRequestId);
		//loggerv2::info("lts_api::ReqQryInvestorPosition instr %s , requestId %d", pQryInvestorPosition->InstrumentID, m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}

	void lts_file_reqapi::OnFrontConnected()
	{
		if (m_pConnection->m_blts_wrapper)
			CSecurityFtdcQuerySpiBase::OnFrontConnected();
		m_isAlive = true;
		request_auth();
	}
	
	void lts_file_reqapi::OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (m_pConnection->m_blts_wrapper)
			CSecurityFtdcQuerySpiBase::OnRspFetchAuthRandCode(pAuthRandCode, pRspInfo, nRequestID, bIsLast);
		loggerv2::info("lts_file_reqapi::OnRspFetchAuthRandCode - sending login request");
		request_login(pAuthRandCode);
	}
	
	void lts_file_reqapi::OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{

		loggerv2::info("lts_file_reqapi::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		if (pRspInfo->ErrorID == 0)
		{
			set_status(true);
			loggerv2::info("lts_file_reqapi status set to true");

			if (m_pConnection->get_both_status() && m_pConnection->getStatus() != AtsType::ConnectionStatus::Connected)
			{
				m_pConnection->on_status_changed(AtsType::ConnectionStatus::Connected,"both api connected");

				if (m_pConnection->m_quant_proxy->connect() == true)
				{
					m_pConnection->on_status_changed(AtsType::ConnectionStatus::Connected);
				}
				else
				{
					m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected);
				}
			}

			if (m_pConnection->m_bRequestPosition)
			m_pConnection->request_investor_full_positions();
		}
		else
		{
			//m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pRspInfo->ErrorMsg);
			set_status(false);
		}
	}

	void lts_file_reqapi::OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
	{
		loggerv2::info("calling lts_file_reqapi::OnRspUserLogout");
		m_bUserReqDiscon = true;
		if (pRspInfo->ErrorID == 0)
		{
			set_status(false);
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "lts_file_reqapi::OnRspUserLogout Receive Logout Msg Error Id 0");
		}

		else
			loggerv2::error("lts_file_reqapi::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);



	}


	void lts_file_reqapi::OnFrontDisconnected(int nReason)
	{
		loggerv2::info("calling lts_file_reqapi::OnFrontDisconnected");

		

		char* pszMessage;
		switch (nReason)
		{
			// normal disconnection
		case 0:
			pszMessage = "normal disconnection";
			break;

			// error
		case 0x1001:
			pszMessage = "network write failed";
			break;
		case 0x1002:
			pszMessage = "network read failed";
			break;
		case 0x2001:
			pszMessage = "receive heartbeat timeout";
			break;
		case 0x2002:
			pszMessage = "send heartbeat failed";
			break;
		case 0x2003:
			pszMessage = "receive wrong message";
			break;


		default:
			//pszMessage = "unknown error [" + nReason + "];
			pszMessage = "unknown error";
			break;
		}

		//if (m_pConnection->get_status() != AtsType::ConnectionStatus::Disconnected)
		//	m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage);
		
		set_status(false);
		if (!m_bUserReqDiscon)
		request_auth();
	}

	bool lts_file_reqapi::disconnect()
	{
		m_bUserReqDiscon = true;

		CSecurityFtdcUserLogoutField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->m_sUsername.c_str());
		
		loggerv2::info("lts_file_reqapi::disconnect - requesting logout.");
		int res = m_pQueryApi->ReqUserLogout(&request, ++m_nRequestId);
		if (res != 0)
		{
			return false;
		}
		return true;
	}

	bool lts_file_reqapi::connect()
	{
		//loggerv2::info("lts_file_reqapi connecting...");
		m_bUserReqDiscon = false;
		
		if (m_isAlive == false && m_pConnection->m_blts_wrapper==false)
		{
			char addr1[1024 + 1];
			snprintf(addr1, 1024, "%s:%s", m_pConnection->m_sHostnameQry.c_str(), m_pConnection->m_sServiceQry.c_str());
			loggerv2::info("lts_file_reqapi connecting to %s:%s", m_pConnection->m_sHostnameQry.c_str(), m_pConnection->m_sServiceQry.c_str());
			m_pQueryApi->RegisterFront(addr1);
			m_pQueryApi->Init();

			//request_auth();
		}
		else
		{
			loggerv2::info("lts_file_reqapi::connect api is already alive, going to request auth");
			request_auth();
		}
		return true;
	}

	void lts_file_reqapi::init()
	{
		//m_pQueryApi = CSecurityFtdcQueryApi::CreateFtdcQueryApi("");
		if (m_pConnection->m_blts_wrapper == false)
			m_pQueryApi->RegisterSpi(this);
	
	}

	void lts_file_reqapi::release()
	{
	
		m_pQueryApi->Release();
	}

}