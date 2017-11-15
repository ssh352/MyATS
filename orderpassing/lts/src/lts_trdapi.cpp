#include "lts_trdapi.h"
#include "lts_connection.h"
#include "FastMemcpy.h"
#include "terra_logger.h"
using namespace terra::common;
//using namespace boost;


namespace lts
{
	//int lts_trdapi::process_inbound_input_cb()
	//{
	//	CSecurityFtdcInputOrderField* pInput = NULL;

	//	int i = 0;
	//	for (; i < 10 && m_inputQueue.m_queue.read_available()>0; ++i)
	//	{

	//		pInput = m_inputQueue.Pop();
	//		if (pInput != NULL)
	//		{

	//			OnRspOrderInsertAsync(pInput);
	//			delete pInput;
	//		}
	//		i++;
	//	}
	//	return i;
	//}

	// int lts_trdapi::process_inbound_input_action_cb()
	//{
	//	CSecurityFtdcInputOrderActionField* pInput = NULL;

	//	int i = 0;
	//	for (; i < 10 && m_inputActionQueue.m_queue.read_available()>0; ++i)
	//	{

	//		pInput = m_inputActionQueue.Pop();
	//		if (pInput != NULL)
	//		{
	//			OnRspOrderActionAsync(pInput);
	//			delete pInput;
	//		}
	//		i++;
	//	}
	//	return i;

	//}



	//int lts_trdapi::process_inbound_order_cb()
	//{
	//	CSecurityFtdcOrderField* pInput = NULL;

	//	int i = 0;
	//	for (; i < 10 && m_orderQueue.m_queue.read_available()>0; ++i)
	//	{

	//		pInput = m_orderQueue.Pop();
	//		if (pInput != NULL)
	//		{
	//			OnRtnOrderAsync(pInput);
	//			delete pInput;
	//		}
	//		i++;
	//	}
	//	return i;

	//}

	//int lts_trdapi::process_inbound_trade_cb()
	//{
	//	CSecurityFtdcTradeField* pTrade = NULL;

	//	int i = 0;
	//	for (; i < 10 && m_tradeQueue.m_queue.read_available()>0; ++i)
	//	{

	//		pTrade = m_tradeQueue.Pop();
	//		if (pTrade != NULL)
	//		{

	//			OnRtnTradeAsync(pTrade);
	//			delete pTrade;
	//		}
	//		i++;
	//	}
	//	return i;
	//}

	lts_trdapi::lts_trdapi()
	{
		m_isAlive = false;

		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;

	}

	lts_trdapi::lts_trdapi(lts_connection* pConnection)
	{
		m_pConnection = pConnection;
		//m_connectionStatus = false;

		m_isAlive = false;

		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;

		m_pTraderApi = CSecurityFtdcTraderApi::CreateFtdcTraderApi();

	}

	lts_trdapi::~lts_trdapi()
	{
		//delete m_pUserApi;
	}

	void lts_trdapi::init()
	{
		//m_pTraderApi = CSecurityFtdcTraderApi::CreateFtdcTraderApi();
		
		m_inputQueue.setHandler(boost::bind(&lts_trdapi::OnRspOrderInsertAsync, this,_1));
		m_orderQueue.setHandler(boost::bind(&lts_trdapi::OnRtnOrderAsync, this,_1));
		m_tradeQueue.setHandler(boost::bind(&lts_trdapi::OnRtnTradeAsync, this,_1));
		m_inputActionQueue.setHandler(boost::bind(&lts_trdapi::OnRspOrderActionAsync, this,_1));


		/*if (m_pConnection->m_blts_wrapper)
			return;*/
		m_pTraderApi->RegisterSpi(this);

		switch (m_pConnection->getResynchronizationMode())
		{
		case ResynchronizationMode::None:
			m_pTraderApi->SubscribePrivateTopic(SECURITY_TERT_QUICK);
			break;

		case ResynchronizationMode::Last:
			m_pTraderApi->SubscribePrivateTopic(SECURITY_TERT_RESUME);
			break;

		default:
		case ResynchronizationMode::Full:
			m_pTraderApi->SubscribePrivateTopic(SECURITY_TERT_RESTART);
			break;
		}

	}

	void lts_trdapi::release()
	{

		m_pTraderApi->Release();
	}

	//
	// RTThread
	//
	void lts_trdapi::Process()
	{
		//if (m_inputQueue.m_queue.read_available() > 0)
		//{
		//	for(auto &func:m_inputQueue.m_handler)
		//	{
		//		func();
		//	}
		//}

		//if (m_orderQueue.m_queue.read_available() > 0)
		//{
		//	for(auto &func:m_orderQueue.m_handler)
		//	{
		//		func();
		//	}
		//}

		//if (m_tradeQueue.m_queue.read_available() > 0)
		//{
		//	for(auto &func:m_tradeQueue.m_handler)
		//	{
		//		func();
		//	}
		//}

		//if (m_inputActionQueue.m_queue.read_available() > 0)
		//{
		//	for(auto &func:m_inputActionQueue.m_handler)
		//	{
		//		func();
		//	}
		//}
		//int i = 0;
		//while (m_inputQueue.Pop_Handle() && i < 10)
		//	++i;
		//i = 0;
		//while (m_orderQueue.Pop_Handle() && i < 10)
		//	++i;
		//i = 0;
		//while (m_tradeQueue.Pop_Handle() && i < 10)
		//	++i;
		//i = 0;
		//while (m_inputActionQueue.Pop_Handle() && i < 10)
		//	++i;
		m_inputQueue.Pops_Handle(0);
		m_orderQueue.Pops_Handle(0);
		m_tradeQueue.Pops_Handle(0);
		m_inputActionQueue.Pops_Handle(0);
	}

	bool lts_trdapi::connect()
	{
		//
		// For first connection, we are disconnected so we need to connect API first (RequestLogin will be done on API UP).
		// For later connections (disconnect / reconnect), API is already up so we just need to relogin.
		//
		/*if (m_pConnection->m_blts_wrapper == true)
			return true;*/

		m_bUserReqDiscon = false;
		if (m_isAlive == false)
		{
			char addr2[1024 + 1];
			snprintf(addr2, 1024, "%s:%s", m_pConnection->m_sHostnameTrd.c_str(), m_pConnection->m_sServiceTrd.c_str());
			m_pTraderApi->RegisterFront(addr2);
			m_pTraderApi->Init();
		}
		else
		{
			loggerv2::info("lts_trdapi::connect api is already alive, going to request auth");
			request_auth(); 
		}

		// no need ??
		//m_pUserApi->Join();

		return true;
	}

	bool lts_trdapi::disconnect()
	{
		m_bUserReqDiscon = true;
		CSecurityFtdcUserLogoutField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->m_sUsername.c_str());
		loggerv2::info("lts_trdapi::disconnect - requesting logout.");
		int res = m_pTraderApi->ReqUserLogout(&request, ++m_nRequestId);
		if (res != 0)
		{
			return false;
		}
		return true;
	}

	void lts_trdapi::request_login(CSecurityFtdcAuthRandCodeField *pAuthRandCode)
	{
		/*if (m_pConnection->m_blts_wrapper)
		{
			return;
		}*/
		loggerv2::info("lts_trdapi::request_login requesting login");
		CSecurityFtdcReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->m_sUsername.c_str());
		
		strcpy(request.RandCode, pAuthRandCode->RandCode);
		//strcpy(request.UserProductInfo, "LTS-Test");
		//strcpy(request.AuthCode, "N3EHKP4CYHZGM9VJ");
		strcpy(request.UserProductInfo, m_pConnection->m_sProductInfo.c_str());
		strcpy(request.AuthCode, m_pConnection->m_sAuthCode.c_str());


		strcpy(request.Password, m_pConnection->m_sPasswordTrd.c_str());

		int res = m_pTraderApi->ReqUserLogin(&request, ++m_nRequestId);
		if (res != 0)
		{
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "lts_trdapi - ReqUserLogin failed");
		}
	}

	void lts_trdapi::OnRspError(CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
		{

			loggerv2::error("lts_trdapi::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
		else
		{

			loggerv2::info("lts_trdapi::OnRspError - ok");
		}
	}

	
	void lts_trdapi::request_auth()
	{
		/*if (m_pConnection->m_blts_wrapper == true)
		{
			return;
		}*/
		loggerv2::info("lts_trdapi::request_auth.");
		CSecurityFtdcAuthRandCodeField pAuthRandCode;
		memset(&pAuthRandCode, 0, sizeof(pAuthRandCode));
		strcpy(pAuthRandCode.RandCode, "");
		int iResult2 = m_pTraderApi->ReqFetchAuthRandCode(&pAuthRandCode, ++m_nRequestId);
	}
	

	void lts_trdapi::OnFrontConnected()
	{
		/*if (m_pConnection->m_blts_wrapper)
			CSecurityFtdcTraderSpiBase::OnFrontConnected();*/
		if (!m_isAlive)
		{
			//request_login();
			request_auth();
		}
		else
		{
			loggerv2::info("lts_trdapi not asking for reconnect...");
		}
	}



	void lts_trdapi::OnFrontDisconnected(int nReason)
	{

		loggerv2::info("calling lts_trdapi::OnFrontDisconnected");

		//m_connectionStatus = false;

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

		set_status(false);
		
		//if (m_pConnection->get_status() != AtsType::ConnectionStatus::Disconnected)
		//	m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage);
	
		//resend auth
		if (!m_bUserReqDiscon)
			request_auth();
	}

	void lts_trdapi::OnHeartBeatWarning(int nTimeLapse)
	{

		loggerv2::info("lts_trdapi - heartbeat warning %d since last receive",nTimeLapse);
	}

	void lts_trdapi::OnRspUserLogin(CSecurityFtdcRspUserLoginField* pRspUserLogin, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{

		loggerv2::info("lts_trdapi::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

		if (pRspInfo->ErrorID == 0)
		{
			
			set_status(true);
			m_nCurrentOrderRef = atoi(pRspUserLogin->MaxOrderRef);

			if (m_pConnection->get_both_status() && m_pConnection->getStatus() != AtsType::ConnectionStatus::Connected)
			{
				m_pConnection->on_status_changed(AtsType::ConnectionStatus::Connected, "both api connected");
			}

		}
		else
		{
			
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pRspInfo->ErrorMsg);
		}
	}

	void lts_trdapi::OnRspUserLogout(CSecurityFtdcUserLogoutField* pUserLogout, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{

		loggerv2::info("calling lts_trdapi::OnRspUserLogout");
		m_bUserReqDiscon = true;
		if (pRspInfo->ErrorID == 0)
		{
			set_status(false);
			
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected,"lts_trdapi::OnRspUserLogout Receive Logout Msg Error Id 0");
		}
			
		else
			loggerv2::error("lts_trdapi::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);
	}




	
	void lts_trdapi::OnErrRtnOrderInsert(CSecurityFtdcInputOrderField* pInputOrder, CSecurityFtdcRspInfoField* pRspInfo)
	{
		loggerv2::info("lts_trdapi::OnErrRtnOrderInsert --> Error ID:%d,Error msg:%s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		loggerv2::info("lts_trdapi::OnErrRtnOrderInsert - "
			"BrokerID[%*.*s] "
			"InvestorID[%*.*s] "
			"InstrumentID[%*.*s] "
			"OrderRef[%*.*s]"
			"UserID[%*.*s] "
			"ExchangeID[%*.*s] "
			"OrderPriceType[%c] "
			"Direction[%c] "

			"CombOffsetFlag[%*.*s]"
			"CombHedgeFlag[%*.*s] "
			"LimitPrice[%*.*s]"

			"VolumeTotalOriginal[%d]"
			"TimeCondition[%c] "
			"GTDDate[%*.*s] "
			"VolumeCondition[%c] "

			"MinVolume[%d]"
			"ContingentCondition[%c]"
			"StopPrice[%f] "
			"ForceCloseReason[%c] "
			"IsAutoSuspend[%d]"
			"BusinessUnit[%*.*s]"
			"RequestID[%d]"
			"UserForceClose[%d]"
			,
			sizeof(pInputOrder->BrokerID), sizeof(pInputOrder->BrokerID), pInputOrder->BrokerID,
			sizeof(pInputOrder->InvestorID), sizeof(pInputOrder->InvestorID), pInputOrder->InvestorID,
			sizeof(pInputOrder->InstrumentID), sizeof(pInputOrder->InstrumentID), pInputOrder->InstrumentID,
			sizeof(pInputOrder->OrderRef), sizeof(pInputOrder->OrderRef), pInputOrder->OrderRef,
			sizeof(pInputOrder->UserID), sizeof(pInputOrder->UserID), pInputOrder->UserID,
			sizeof(pInputOrder->ExchangeID), sizeof(pInputOrder->ExchangeID), pInputOrder->ExchangeID,
			pInputOrder->OrderPriceType,
			pInputOrder->Direction,

			sizeof(pInputOrder->CombOffsetFlag), sizeof(pInputOrder->CombOffsetFlag), pInputOrder->CombOffsetFlag,
			sizeof(pInputOrder->CombHedgeFlag), sizeof(pInputOrder->CombHedgeFlag), pInputOrder->CombHedgeFlag,
			sizeof(pInputOrder->LimitPrice), sizeof(pInputOrder->LimitPrice), pInputOrder->LimitPrice,

			pInputOrder->VolumeTotalOriginal,
			pInputOrder->TimeCondition,
			sizeof(pInputOrder->GTDDate), sizeof(pInputOrder->GTDDate), pInputOrder->GTDDate,
			pInputOrder->VolumeCondition,


			pInputOrder->MinVolume,
			pInputOrder->ContingentCondition,
			pInputOrder->StopPrice,
			pInputOrder->ForceCloseReason,
			pInputOrder->IsAutoSuspend,
			sizeof(pInputOrder->BusinessUnit), sizeof(pInputOrder->BusinessUnit), pInputOrder->BusinessUnit,
			pInputOrder->RequestID,
			pInputOrder->UserForceClose

			);

	}

	void lts_trdapi::OnErrRtnOrderAction(CSecurityFtdcOrderActionField *pOrderAction, CSecurityFtdcRspInfoField *pRspInfo)
	{
		loggerv2::info("lts_trdapi::OnErrRtnOrderInsert --> Error ID:%d,Error msg:%s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		loggerv2::info("lts_trdapi::OnErrRtnOrderInsert - "
			"BrokerID[%*.*s] "
			"InvestorID[%*.*s] "
			"OrderActionRef[%d] "

			"OrderRef[%*.*s]"
			"RequestID[%d]"
			"FrontID[%d]"

			"SessionID[%d]"
			"ExchangeID[%*.*s] "
			"ActionFlag[%c]"

			"LimitPrice[%f]"
			"VolumeChange[%d] "
			"ActionDate[%*.*s]"
			"ActionTime[%*.*s] "
			"BranchPBU[%*.*s] "

			"InstallID[%d]"
			"OrderLocalID[%*.*s] "
			"ActionLocalID[%*.*s] "
			"ParticipantID[%*.*s] "

			"ClientID[%*.*s]"
			"BusinessUnit[%*.*s]"
			"OrderActionStatus[%c] "
			"UserID[%*.*s] "
			"BranchID[%*.*s] "

			"StatusMsg[%*.*s]"
			"InstrumentID[%*.*s]"
			"InstrumentType[%c] "
			,
			sizeof(pOrderAction->BrokerID), sizeof(pOrderAction->BrokerID), pOrderAction->BrokerID,
			sizeof(pOrderAction->InvestorID), sizeof(pOrderAction->InvestorID), pOrderAction->InvestorID,
			pOrderAction->OrderActionRef,

			sizeof(pOrderAction->OrderRef), sizeof(pOrderAction->OrderRef), pOrderAction->OrderRef,
			pOrderAction->RequestID,
			pOrderAction->FrontID,

			pOrderAction->SessionID,
			sizeof(pOrderAction->ExchangeID), sizeof(pOrderAction->ExchangeID), pOrderAction->ExchangeID,
			pOrderAction->ActionFlag,

			pOrderAction->LimitPrice,
			pOrderAction->VolumeChange,
			sizeof(pOrderAction->ActionDate), sizeof(pOrderAction->ActionDate), pOrderAction->ActionDate,
			sizeof(pOrderAction->ActionTime), sizeof(pOrderAction->ActionTime), pOrderAction->ActionTime,
			sizeof(pOrderAction->BranchPBU), sizeof(pOrderAction->BranchPBU), pOrderAction->BranchPBU,

			pOrderAction->InstallID,
			sizeof(pOrderAction->OrderLocalID), sizeof(pOrderAction->OrderLocalID), pOrderAction->OrderLocalID,
			sizeof(pOrderAction->ActionLocalID), sizeof(pOrderAction->ActionLocalID), pOrderAction->ActionLocalID,
			sizeof(pOrderAction->ParticipantID), sizeof(pOrderAction->ParticipantID), pOrderAction->ParticipantID,

			sizeof(pOrderAction->ClientID), sizeof(pOrderAction->ClientID), pOrderAction->ClientID,
			sizeof(pOrderAction->BusinessUnit), sizeof(pOrderAction->BusinessUnit), pOrderAction->BusinessUnit,
			pOrderAction->OrderActionStatus,
			sizeof(pOrderAction->UserID), sizeof(pOrderAction->UserID), pOrderAction->UserID,
			sizeof(pOrderAction->BranchID), sizeof(pOrderAction->BranchID), pOrderAction->BranchID,

			sizeof(pOrderAction->StatusMsg), sizeof(pOrderAction->StatusMsg), pOrderAction->StatusMsg,
			sizeof(pOrderAction->InstrumentID), sizeof(pOrderAction->InstrumentID), pOrderAction->InstrumentID,
			pOrderAction->InstrumentType

			);

	}

	void lts_trdapi::OnRspOrderInsert(CSecurityFtdcInputOrderField* pOrder, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		//loggerv2::info("calling lts_api::OnRspOrderInsert" ); 
		//CSecurityFtdcInputOrderField* i = new CSecurityFtdcInputOrderField;
		//memcpy_lw(i, pOrder, sizeof(CSecurityFtdcInputOrderField));

		//// use unused field to store errorId
		//i->IsAutoSuspend = pRspInfo->ErrorID;

		//m_inputQueue.Push(i);
		pOrder->IsAutoSuspend = pRspInfo->ErrorID;
		m_inputQueue.CopyPush(pOrder);
	}

	void lts_trdapi::OnRspOrderAction(CSecurityFtdcInputOrderActionField* pInputOrderAction, CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		//loggerv2::info("calling lts_api::OnRspOrderAction --> need to implement");

		if (!bIsLast)
		{
			return;
		}

		//CSecurityFtdcInputOrderActionField* i = new CSecurityFtdcInputOrderActionField;
		//memcpy_lw(i, pInputOrderAction, sizeof(CSecurityFtdcInputOrderActionField));
		//// use unused field to store errorId
		//i->OrderActionRef = pRspInfo->ErrorID;
		//m_inputActionQueue.Push(i);
		pInputOrderAction->OrderActionRef = pRspInfo->ErrorID;
		m_inputActionQueue.CopyPush(pInputOrderAction);

	}

	void lts_trdapi::OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pInputOrderAction)
	{
		//loggerv2::info("calling lts_api::OnRspOrderActionAsync");
		m_pConnection->OnRspOrderActionAsync(pInputOrderAction, pInputOrderAction->OrderActionRef);
	}



	void lts_trdapi::OnRtnOrder(CSecurityFtdcOrderField* pOrder)
	{
		//loggerv2::info("calling lts_api : OnRtnOrder");
		//CSecurityFtdcOrderField* o = new CSecurityFtdcOrderField;
		//memcpy_lw(o, pOrder, sizeof(CSecurityFtdcOrderField));
		//m_orderQueue.Push(o);
		m_orderQueue.CopyPush(pOrder);
	}

	void lts_trdapi::OnRtnTrade(CSecurityFtdcTradeField* pTrade)
	{
		//loggerv2::info("calling lts_api : OnRtnTrade");

		//CSecurityFtdcTradeField* t = new CSecurityFtdcTradeField;
		//memcpy_lw(t, pTrade, sizeof(CSecurityFtdcTradeField));
		//m_tradeQueue.Push(t);
		m_tradeQueue.CopyPush(pTrade);
	}


	//
	// callbacks
	//
	void lts_trdapi::OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pInput)
	{
		m_pConnection->OnRspOrderInsertAsync(pInput, pInput->IsAutoSuspend);
	}

	void lts_trdapi::OnRtnOrderAsync(CSecurityFtdcOrderField* pOrder)
	{
		//loggerv2::info("calling lts_api::OnRtnOrderAsync ");
		m_pConnection->OnRtnOrderAsync(pOrder);
	}

	void lts_trdapi::OnRtnTradeAsync(CSecurityFtdcTradeField* pTrade)
	{
		m_pConnection->OnRtnTradeAsync(pTrade);
	}


	//
	// order sending
	//
	bool lts_trdapi::ReqOrderInsert(CSecurityFtdcInputOrderField* pRequest)
	{
		//o->m_orderRef = ++m_nCurrentOrderRef;
		// loggerv2::info("calling lts_api::ReqOrderInsert");
		sprintf(pRequest->OrderRef, "%d", ++m_nCurrentOrderRef);


		int ret = m_pTraderApi->ReqOrderInsert(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}

	bool lts_trdapi::ReqOrderAction(CSecurityFtdcInputOrderActionField* pRequest)
	{
		//loggerv2::info("calling lts_api::ReqOrderAction");
		int ret = m_pTraderApi->ReqOrderAction(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}


	void lts_trdapi::OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		/*if (m_pConnection->m_blts_wrapper)
			CSecurityFtdcTraderSpiBase::OnRspFetchAuthRandCode(pAuthRandCode,pRspInfo,nRequestID,bIsLast);*/
		loggerv2::info("lts_trdapi::OnRspFetchAuthRandCode - sending login request");
		request_login(pAuthRandCode);
	
	}




}

