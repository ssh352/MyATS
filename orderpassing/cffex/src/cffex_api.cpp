#include "cffex_api.h"
#include "ctp_connection.h"
#include "tradeItem_gh.h"
#include <boost/algorithm/string.hpp>
#include "terra_logger.h"
#include "FastMemcpy.h"
#include <thread>
#include "order_reference_provider.h"

using namespace terra::common;
namespace cffex
{

	cffex_api::cffex_api(cffex_connection* pConnection)
	{
		m_pConnection = pConnection;
		m_connectionStatus = false;

		m_isAlive = true;

		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		m_begin_Id = order_reference_provider::get_instance().get_current_int();

	}

	cffex_api::~cffex_api()
	{
		//delete m_pUserApi;
	}

	void cffex_api::init()
	{
		m_pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();

		m_inputQueue.setHandler(boost::bind(&cffex_connection::OnRspOrderInsertAsync, m_pConnection, _1));
		m_inputQuoteQueue.setHandler(boost::bind(&cffex_connection::OnRspQuoteInsertAsync, m_pConnection, _1));

		m_orderQueue.setHandler(boost::bind(&cffex_connection::OnRtnOrderAsync, m_pConnection, _1));
		m_quoteQueue.setHandler(boost::bind(&cffex_connection::OnRtnQuoteAsync, m_pConnection, _1));
		
		m_tradeQueue.setHandler(boost::bind(&cffex_connection::OnRtnTradeAsync, m_pConnection, _1));

		m_inputActionQueue.setHandler(boost::bind(&cffex_connection::OnRspOrderActionAsync, m_pConnection, _1));
		m_inputActionQuoteQueue.setHandler(boost::bind(&cffex_connection::OnRspQuoteActionAsync, m_pConnection, _1));

	}

	void cffex_api::release()
	{
		// removeFDFromList...

		m_pUserApi->Release();
	}

	void cffex_api::Process()
	{
		
		m_inputQueue.Pops_Handle(0);
		m_inputQuoteQueue.Pops_Handle(0);

		m_orderQueue.Pops_Handle(0);
		m_quoteQueue.Pops_Handle(0);

		m_tradeQueue.Pops_Handle(0);
		m_inputActionQueue.Pops_Handle(0);
		m_inputActionQuoteQueue.Pops_Handle(0);
	
	}

	int cffex_api::get_ord_ref_from_reqid(int nReqId)
	{
		auto search = m_ordInputActiondRefMap.find(nReqId);
		if (search != m_ordInputActiondRefMap.end()) {
			int i = search->second;
			m_ordInputActiondRefMap.erase(search);
			return i;
		}
		else
		{
			return 0;
		}
	}

	bool cffex_api::connect()
	{

		if (m_connectionStatus == false)
		{
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", m_pConnection->m_sHostname.c_str(), m_pConnection->m_sService.c_str());

			m_pUserApi->RegisterSpi(this);

			m_pUserApi->RegisterFront(addr);
			loggerv2::info("cffex_api::connect connecting to %s", addr);
			switch (m_pConnection->getResynchronizationMode())
			{
			case ResynchronizationMode::None:
				m_pUserApi->SubscribePrivateTopic(THOST_TERT_QUICK);
				break;

			case ResynchronizationMode::Last:
				m_pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);
				break;

			default:
			case ResynchronizationMode::Full:
				m_pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);
				break;
			}
			loggerv2::info("cffex_api::connect initializing api");
			m_pUserApi->Init();
			loggerv2::info("cffex_api::connect api intialized");
		}
		else
		{
			request_login();
		}

		return true;
	}

	bool cffex_api::disconnect()
	{
		CThostFtdcUserLogoutField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->m_sUsername.c_str());

		int res = m_pUserApi->ReqUserLogout(&request, ++m_nRequestId);
		if (res != 0)
		{
			return false;
		}
		return true;
	}

	void cffex_api::request_login()
	{
		loggerv2::info("cffex_api::request_login");

		CThostFtdcReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.UserID, m_pConnection->m_sUsername.c_str());
		strcpy(request.Password, m_pConnection->m_sPassword.c_str());

		int res = m_pUserApi->ReqUserLogin(&request, ++m_nRequestId);
		if (res != 0)
		{
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "cffex_api - ReqUserLogin failed");
		}
		else if (m_pConnection->getRequestInstruments() == true)
		{
			request_instruments();
		}
	}

	void cffex_api::request_ack_order()
	{
		loggerv2::info("cffex_api::request_ack_order");

		CThostFtdcQryOrderField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, m_pConnection->m_sBrokerId.c_str());
		strcpy(request.InvestorID, m_pConnection->m_sUsername.c_str());
		strcpy(request.InsertTimeStart,"0:00:00");
		strcpy(request.InsertTimeEnd, "23:59:59");
		//strcpy(request.Password, m_pConnection->m_sPassword.c_str());

		int res = m_pUserApi->ReqQryOrder(&request, ++m_nRequestId);
		if (res != 0)
		{
			cout << "ReqQryOrder error,errorID:" << res << endl;
		}
	}

	void  cffex_api::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
	
		if (pRspInfo != nullptr)
		{
			cout << pRspInfo->ErrorMsg;
		}
		else
		{
			if (pOrder->OrderStatus != THOST_FTDC_OST_AllTraded&&pOrder->OrderStatus != THOST_FTDC_OST_Canceled)
			{
				loggerv2::info("OnRspQryOrder:UserID:[%s] InsertTime:[%s],status:[%c],LimitPrice:[%f],VolumeTotalOriginal:[%d],VolumeTraded:[%d],VolumeTotal:[%d],UpdateTime:[%s],CancelTime:[%s]",
					pOrder->UserID, pOrder->InsertTime, pOrder->OrderStatus, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal, pOrder->UpdateTime, pOrder->CancelTime);
			}
		}
	}

	void cffex_api::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
		{
			loggerv2::error("cffex_api::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
		else
		{
			loggerv2::info("cffex_api::OnRspError - ok");
		}
	}

	void cffex_api::OnFrontConnected()
	{
		loggerv2::info("cffex_api is UP");

		m_connectionStatus = true;

		if (m_pConnection->getStatus() == AtsType::ConnectionStatus::WaitConnect)
		{
			request_login();
		}
		else
		{
			loggerv2::info("cffex_api not asking for reconnect...");
		}
	}

	bool cffex_api::ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition)
	{

		//loggerv2::info("calling cffex_api::ReqQryInvestorPosition");
		int ret = m_pUserApi->ReqQryInvestorPosition(pQryInvestorPosition, ++m_nRequestId);
		loggerv2::info("cffex_api::ReqQryInvestorPosition instr %s , requestId %d,ret:%d", pQryInvestorPosition->InstrumentID, m_nRequestId,ret);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}


	bool cffex_api::ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount)
	{
		int ret = m_pUserApi->ReqQryTradingAccount(pQryTradingAccount, ++m_nRequestId);
		loggerv2::info("cffex_api::ReqQryTradingAccount brokerid %s , requestId %d,ret:%d", pQryTradingAccount->BrokerID, m_nRequestId,ret);
		if (ret != 0)
		{
			return false;
		}

		return true;
	}

	void cffex_api::OnFrontDisconnected(int nReason)
	{
		//loggerv2::info("cffex_api is DOWN");

		m_connectionStatus = false;

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

		m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage.data());
	}

	void cffex_api::OnHeartBeatWarning(int nTimeLapse)
	{
		loggerv2::info("cffex_api - heartbeat warning");
	}

	void cffex_api::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::info("cffex_api::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

		if (pRspInfo->ErrorID == 0)
		{
			//request_ack_order();

		/*	ptime Now = microsec_clock::local_time();
			std::string czce_time = pRspUserLogin->CZCETime;
			std::string dce_time = pRspUserLogin->DCETime;
			std::string cffex_time = pRspUserLogin->FFEXTime;
			std::string shfe_time = pRspUserLogin->SHFETime;

			if (czce_time.size() > 0)
			{
				ptime CZCETime(day_clock::local_day(), duration_from_string(czce_time));
				CZCE_Time_dur = CZCETime - Now;
			}

			if (dce_time.size() > 0)
			{
				ptime DCETime(day_clock::local_day(), duration_from_string(dce_time));
				DCE_Time_dur = DCETime - Now;
			}

			if (cffex_time.size() > 0)
			{
				ptime CFFEXTime(day_clock::local_day(), duration_from_string(cffex_time));
				CFFEX_Time_dur = CFFEXTime - Now;
			}

			if (shfe_time.size() > 0)
			{
				ptime SHFETime(day_clock::local_day(), duration_from_string(shfe_time));
				SHFE_Time_dur = SHFETime - Now;
			}*/

			m_nCurrentOrderRef = atoi(pRspUserLogin->MaxOrderRef);

			CThostFtdcSettlementInfoConfirmField req;
			memset(&req, 0, sizeof(req));
			strcpy(req.BrokerID, m_pConnection->m_sBrokerId.c_str());
			strcpy(req.InvestorID, m_pConnection->m_sUsername.c_str());

			int res = m_pUserApi->ReqSettlementInfoConfirm(&req, ++m_nRequestId);
			if (res != 0)
			{
				m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "cffex_api - ReqSettlementInfoConfirm failed");
			}
		}
		else
		{
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, pRspInfo->ErrorMsg);
		}
	}

	void cffex_api::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo->ErrorID == 0)
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "cffex_api::OnRspUserLogout Receive Logout Msg Error Id 0");
		else
			loggerv2::error("cffex_api::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);


	}

	void cffex_api::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo->ErrorID == 0)
		{
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Connected, "cffex_api::Settlement Info Confirmed.");
			m_pConnection->request_investor_full_positions();
			//m_pConnection->request_trading_account();
		}

		else
			m_pConnection->on_status_changed(AtsType::ConnectionStatus::Disconnected, "cffex_api::Settlement Info Confirmation failed.");

	}

	void log_CThostFtdcInvestorPositionField(CThostFtdcInvestorPositionField *pInvestorPosition)
	{
		if (pInvestorPosition != NULL)
			loggerv2::info("cffex_api::OnRspQryInvestorPosition InstrumentID=%s "
			"PosiDirection=%c "
			"PositionDate=%c "
			"YdPosition=%d "
			"TodayPosition=%d "
			"Position=%d "
			"TradingDay=%s "
			"LongFrozen=%d "
			"ShortFrozen=%d "
			"LongFrozenAmount=%f "
			"ShortFrozenAmount=%f "
			"OpenVolume=%d "
			"CloseVolume=%d "
			"OpenAmount=%f "
			"CloseAmount=%f "
			"PositionCost=%f "
			"PreMargin=%f "
			"UseMargin=%f "
			"FrozenMargin=%f "
			"FrozenCash=%f "
			"FrozenCommission=%f "
			"CashIn=%f "
			"Commission=%f "
			"ExchangeMargin=%f "
			"MarginRateByMoney=%f "
			"MarginRateByVolume=%f "
			"CombPosition=%d "
			"CombLongFrozen=%d "
			"CombShortFrozen=%d "
			,

			pInvestorPosition->InstrumentID,
			pInvestorPosition->PosiDirection,
			pInvestorPosition->PositionDate,
			pInvestorPosition->YdPosition,
			pInvestorPosition->TodayPosition,
			pInvestorPosition->Position,
			pInvestorPosition->TradingDay,
			pInvestorPosition->LongFrozen,
			pInvestorPosition->ShortFrozen,
			pInvestorPosition->LongFrozenAmount,
			pInvestorPosition->ShortFrozenAmount,
			pInvestorPosition->OpenVolume,
			pInvestorPosition->CloseVolume,
			pInvestorPosition->OpenAmount,
			pInvestorPosition->CloseAmount,
			pInvestorPosition->PositionCost,
			pInvestorPosition->PreMargin,
			pInvestorPosition->UseMargin,
			pInvestorPosition->FrozenMargin,
			pInvestorPosition->FrozenCash,
			pInvestorPosition->FrozenCommission,
			pInvestorPosition->CashIn,
			pInvestorPosition->Commission,
			pInvestorPosition->ExchangeMargin,
			pInvestorPosition->MarginRateByMoney,
			pInvestorPosition->MarginRateByVolume,
			pInvestorPosition->CombPosition,
			pInvestorPosition->CombLongFrozen,
			pInvestorPosition->CombShortFrozen
			);
	}


	void cffex_api::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (bIsLast == true)
		{
			if (m_pConnection->getRequestInstruments() == true)
			{
				sleep_by_milliseconds(2000);
				this->request_instruments();	
				m_pConnection->setRequestInstruments(false);
			}
			else
			{
				sleep_by_milliseconds(2000);
				m_pConnection->request_trading_account();
			}
		}

		/*if (pInvestorPosition != NULL)
			loggerv2::info("cffex_api::OnRspQryInvestorPosition InstrumentID=%s "
			"PosiDirection=%c "
			"PositionDate=%c "
			"YdPosition=%d "
			"TodayPosition=%d "
			"Position=%d "
			"TradingDay=%s "
			"LongFrozen=%d "
			"ShortFrozen=%d "
			"LongFrozenAmount=%f "
			"ShortFrozenAmount=%f "
			"OpenVolume=%d "
			"CloseVolume=%d "
			"OpenAmount=%f "
			"CloseAmount=%f "
			"PositionCost=%f "
			"PreMargin=%f "
			"UseMargin=%f "
			"FrozenMargin=%f "
			"FrozenCash=%f "
			"FrozenCommission=%f "
			"CashIn=%f "
			"Commission=%f "
			"ExchangeMargin=%f "
			"MarginRateByMoney=%f "
			"MarginRateByVolume=%f "
			"CombPosition=%d "
			"CombLongFrozen=%d "
			"CombShortFrozen=%d "
			,

			pInvestorPosition->InstrumentID,
			pInvestorPosition->PosiDirection,
			pInvestorPosition->PositionDate,
			pInvestorPosition->YdPosition,
			pInvestorPosition->TodayPosition,
			pInvestorPosition->Position,
			pInvestorPosition->TradingDay,
			pInvestorPosition->LongFrozen,
			pInvestorPosition->ShortFrozen,
			pInvestorPosition->LongFrozenAmount,
			pInvestorPosition->ShortFrozenAmount,
			pInvestorPosition->OpenVolume,
			pInvestorPosition->CloseVolume,
			pInvestorPosition->OpenAmount,
			pInvestorPosition->CloseAmount,
			pInvestorPosition->PositionCost,
			pInvestorPosition->PreMargin,
			pInvestorPosition->UseMargin,
			pInvestorPosition->FrozenMargin,
			pInvestorPosition->FrozenCash,
			pInvestorPosition->FrozenCommission,
			pInvestorPosition->CashIn,
			pInvestorPosition->Commission,
			pInvestorPosition->ExchangeMargin,
			pInvestorPosition->MarginRateByMoney,
			pInvestorPosition->MarginRateByVolume,
			pInvestorPosition->CombPosition,
			pInvestorPosition->CombLongFrozen,
			pInvestorPosition->CombShortFrozen
			);
*/
		bool islog = false;
		if (pInvestorPosition->InstrumentID)
		{

			std::string sInstrCode = std::string(pInvestorPosition->InstrumentID) + "@" + m_pConnection->getName();

			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str()/* pInvestorPosition->InstrumentID*/);
			if (i)
			{
				std::string str = i->getMarket();

				if (pInvestorPosition->PositionDate == '1')
				{
					if (pInvestorPosition->PosiDirection == '2')//long
					{
						i->set_long_used_margin(pInvestorPosition->UseMargin);

						if (str != "SHFE")
						{
							if (i->get_yst_long_position() != pInvestorPosition->Position - pInvestorPosition->TodayPosition)
							{
								loggerv2::info("cffex_api::OnRspQryInvestorPosition:tradeitem Code=%s  exchange=%s updateYsterday by difference of Positoin and TodayPosition", i->getCode().c_str(), str.c_str());
								i->dumpinfo();
								i->set_yst_long_position(pInvestorPosition->Position - pInvestorPosition->TodayPosition);
								log_CThostFtdcInvestorPositionField(pInvestorPosition);
								i->dumpinfo();
								islog = true;
							}
							i->set_pending_short_close_qty(pInvestorPosition->ShortFrozen);


						}
						else
						{
							i->set_pending_short_close_today_qty(pInvestorPosition->ShortFrozen);

							
						}

						i->set_today_long_position(pInvestorPosition->TodayPosition);
						if (i->get_yst_comb_long() != pInvestorPosition->CombPosition)
						{
							loggerv2::info("cffex_api::OnRspQryInvestorPosition:tradeitem Code=%s  exchange=%s updateYst Comb Long Position", i->getCode().c_str(), str.c_str());
							i->dumpinfo();
							i->set_yst_comb_long(pInvestorPosition->CombPosition);
							log_CThostFtdcInvestorPositionField(pInvestorPosition);
							i->dumpinfo();
							islog = true;
						}

					}
					else if (pInvestorPosition->PosiDirection == '3')//short
					{
						i->set_short_used_margin(pInvestorPosition->UseMargin);
						//i->set_tot_short_position(pInvestorPosition->Position);
						if (str != "SHFE")
						{
							if (i->get_yst_short_position() != pInvestorPosition->Position - pInvestorPosition->TodayPosition)
							{
								loggerv2::info("cffex_api::OnRspQryInvestorPosition:tradeitem Code=%s  exchange=%s updateYsterday by difference of Positoin and TodayPosition", i->getCode().c_str(), str.c_str());
								i->dumpinfo();
								i->set_yst_short_position(pInvestorPosition->Position - pInvestorPosition->TodayPosition);
								log_CThostFtdcInvestorPositionField(pInvestorPosition);
								i->dumpinfo();
								islog = true;
							}
							i->set_pending_long_close_qty(pInvestorPosition->LongFrozen);


						}
						else
						{
							i->set_pending_long_close_today_qty(pInvestorPosition->LongFrozen);
						}

						i->set_today_short_position(pInvestorPosition->TodayPosition);


						if (i->get_yst_comb_short() != pInvestorPosition->CombPosition)
						{
							loggerv2::info("cffex_api::OnRspQryInvestorPosition:tradeitem Code=%s  exchange=%s updateYst Comb Short Position", i->getCode().c_str(), str.c_str());
							i->dumpinfo();
							i->set_yst_comb_short(pInvestorPosition->CombPosition);
							log_CThostFtdcInvestorPositionField(pInvestorPosition);
							i->dumpinfo();
							islog = true;
						}

					}
				}
				if (pInvestorPosition->PositionDate == '2')//历史仓位，只有上期所才会推送这个类型的报文
				{
					if (pInvestorPosition->PosiDirection == '2')//long
					{
						i->set_long_used_margin(pInvestorPosition->UseMargin);

						if (i->get_yst_long_position() != pInvestorPosition->Position)
						{
							loggerv2::info("cffex_api::OnRspQryInvestorPosition:tradeitem Code=%s  exchange=%s updateYsterday by Positoin", i->getCode().c_str(), str.c_str());
							i->dumpinfo();
							i->set_yst_long_position(pInvestorPosition->Position);
							log_CThostFtdcInvestorPositionField(pInvestorPosition);
							i->dumpinfo();

							islog = true;
						}
						i->set_pending_short_close_qty(pInvestorPosition->ShortFrozen);

					}
					else if (pInvestorPosition->PosiDirection == '3')//short
					{
						i->set_short_used_margin(pInvestorPosition->UseMargin);
						if (i->get_yst_short_position() != pInvestorPosition->Position)
						{
							loggerv2::info("cffex_api::OnRspQryInvestorPosition:tradeitem Code=%s  exchange=%s updateYsterday by Positoin", i->getCode().c_str(), str.c_str());
							i->dumpinfo();
							i->set_yst_short_position(pInvestorPosition->Position);
							log_CThostFtdcInvestorPositionField(pInvestorPosition);
							i->dumpinfo();
							islog = true;
						}
						i->set_pending_long_close_qty(pInvestorPosition->LongFrozen);
					}
				}
				

				if (i->get_tot_long_position() != i->get_today_long_position() + i->get_yst_long_position())
				{
					loggerv2::info("cffex_api::OnRspQryInvestorPosition:position change,new state:");
					i->dumpinfo();
					i->set_tot_long_position(i->get_today_long_position() + i->get_yst_long_position());
					if (islog == false)
						log_CThostFtdcInvestorPositionField(pInvestorPosition);
					i->dumpinfo();
					islog = true;

				}


				if (i->get_tot_short_position() != i->get_today_short_position() + i->get_yst_short_position())
				{
					loggerv2::info("cffex_api::OnRspQryInvestorPosition:position change,new state:");
					i->dumpinfo();
					i->set_tot_short_position(i->get_today_short_position() + i->get_yst_short_position());
					if (islog == false)
						log_CThostFtdcInvestorPositionField(pInvestorPosition);
					i->dumpinfo();
					islog = true;

				}

				if (islog)
					i->set_last_sychro_timepoint(get_lwtp_now());
			}
			else
			{
				if (unknown_pos.find(pInvestorPosition->InstrumentID)==unknown_pos.end())
				{
					loggerv2::info("cffex_api::OnRspQryInvestorPosition cannot find tradeitem %s by second key", std::string(pInvestorPosition->InstrumentID).c_str());
					log_CThostFtdcInvestorPositionField(pInvestorPosition);
					unknown_pos.emplace(pInvestorPosition->InstrumentID, pInvestorPosition->InstrumentID);
				}
			}
		}

		else

		{
			//loggerv2::warn("cffex_api::OnRspQryInvestorPosition could not get tradeitem %s.", pInvestorPosition->InstrumentID);
		}

	}


	void cffex_api::OnRspOrderInsert(CThostFtdcInputOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		//CThostFtdcInputOrderField* i = new CThostFtdcInputOrderField;
		//memcpy_lw(i, pOrder, sizeof(CThostFtdcInputOrderField));

		//// use unused field to store errorId
		//i->IsAutoSuspend = pRspInfo->ErrorID;
		pOrder->IsAutoSuspend = pRspInfo->ErrorID;
		//m_inputQueue.Push(i);
		m_inputQueue.CopyPush(pOrder);
	}


	void cffex_api::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
	{
		loggerv2::info("cffex_api::OnErrRtnOrderInsert --> need to implement");




	}

	void cffex_api::OnErrRtnOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo)
	{
		loggerv2::info("cffex_api::OnErrRtnOrderAction --> need to implement");
		loggerv2::info("cffex_api::OnErrRtnOrderAction orderref %s , userid %s", pInputOrderAction->OrderRef, pInputOrderAction->UserID);

	}


	void cffex_api::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		//loggerv2::info("calling cffex_api::OnRspOrderAction . request id=[%d] --> need to implement",nRequestID);

		if (!bIsLast)
		{
			return;
		}


		loggerv2::info("cffex_api::OnRspOrderAction pInputOrderAction - "
			"OrderActionRef[%d]"
			"OrderRef[%s]"
			"ExchangeID[%s]"
			"OrdSysID[%s]"
			"ActionFlag[%c]"
			"LimitPrice[%f]"
			"VolumeChange[%d]"
			"UserID[%s]"
			"InstrumentID[%s]",

			pInputOrderAction->OrderActionRef,
			pInputOrderAction->OrderRef,
			pInputOrderAction->ExchangeID,
			pInputOrderAction->OrderSysID,
			pInputOrderAction->ActionFlag,
			pInputOrderAction->LimitPrice,
			pInputOrderAction->VolumeChange,
			pInputOrderAction->UserID,
			pInputOrderAction->InstrumentID

			);

		loggerv2::error("cffex_api::OnRspOrderActionpRspInfo - "
			"ErrorID[%d]"
			"ErrorMsg[%s]",
			pRspInfo->ErrorID,
			pRspInfo->ErrorMsg
			);




		int orderRef = get_ord_ref_from_reqid(nRequestID);
		if (!orderRef)
		{
			loggerv2::error("could not find associated order for nRequestId %d", nRequestID);
			return;
		}

		int* i = new int(orderRef);
		m_inputActionQueue.Push(i);


	}

	void cffex_api::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pTradingAccount==nullptr)
		{
			return;
		}
		
		double ratio = 0;
		
		if (pTradingAccount->Balance != 0)
		{
			ratio = pTradingAccount->CurrMargin / (pTradingAccount->Balance);
		}

		loggerv2::info("cffex_api::OnRspQryTradingAccount,"

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
			"ExchangeMargin[%f]"
			"ratio[%f]",

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
			pTradingAccount->ExchangeMargin,
			ratio
			);

		//
		m_pConnection->set_RiskDegree(ratio);
		//


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




	//void cffex_api::OnRspOrderActionAsync(int* nRequestID)
	//{
	//	//loggerv2::info("calling cffex_api::OnRspOrderActionAsync");
	//	m_pConnection->OnRspOrderActionAsync(*nRequestID);
	//}


	void cffex_api::OnRtnOrder(CThostFtdcOrderField* pOrder)
	{
		//CThostFtdcOrderField* o = new CThostFtdcOrderField;
		//memcpy_lw(o, pOrder, sizeof(CThostFtdcOrderField));

		//m_orderQueue.Push(o);
		m_orderQueue.CopyPush(pOrder);
	}

	void cffex_api::OnRtnQuote(CThostFtdcQuoteField* pQuote)
	{
		m_quoteQueue.CopyPush(pQuote);
	}


	void cffex_api::OnRtnTrade(CThostFtdcTradeField* pTrade)
	{
		//CThostFtdcTradeField* t = new CThostFtdcTradeField;
		//memcpy_lw(t, pTrade, sizeof(CThostFtdcTradeField));

		//m_tradeQueue.Push(t);
		m_tradeQueue.CopyPush(pTrade);
	}


	//
	// callbacks
	//
	//void cffex_api::OnRspOrderInsertAsync(CThostFtdcInputOrderField* pInput)
	//{
	//	m_pConnection->OnRspOrderInsertAsync(pInput, pInput->IsAutoSuspend);
	//}

	//void cffex_api::OnRtnOrderAsync(CThostFtdcOrderField* pOrder)
	//{
	//	m_pConnection->OnRtnOrderAsync(pOrder);
	//}

	//void cffex_api::OnRtnTradeAsync(CThostFtdcTradeField* pTrade)
	//{
	//	m_pConnection->OnRtnTradeAsync(pTrade);
	//}


	//
	// prder sending
	//
	bool cffex_api::ReqOrderInsert(CThostFtdcInputOrderField* pRequest)
	{
		//o->m_ordRef = ++m_nCurrentOrderRef;
		sprintf(pRequest->OrderRef, "%d", ++m_nCurrentOrderRef);

		int ret = m_pUserApi->ReqOrderInsert(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}

	bool cffex_api::ReqQuoteInsert(CThostFtdcInputQuoteField * pRequest)
	{
		sprintf(pRequest->QuoteRef, "%d", ++m_nCurrentOrderRef);
		sprintf(pRequest->AskOrderRef, "%d", ++m_nCurrentOrderRef);
		sprintf(pRequest->BidOrderRef, "%d", ++m_nCurrentOrderRef);

		int ret = m_pUserApi->ReqQuoteInsert(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}

	bool cffex_api::ReqExecOrderInsert(CThostFtdcInputExecOrderField* pRequest)
	{
		//o->m_ordRef = ++m_nCurrentOrderRef;
		sprintf(pRequest->ExecOrderRef, "%d", ++m_nCurrentOrderRef);

		int ret = m_pUserApi->ReqExecOrderInsert(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}


	bool cffex_api::ReqOrderAction(CThostFtdcInputOrderActionField* pRequest)
	{

		int ret = m_pUserApi->ReqOrderAction(pRequest, ++m_nRequestId);
		//loggerv2::info("m_nRequestId %d", m_nRequestId);

		if (ret != 0)
		{
			return false;
		}

		int nOrderId = m_pConnection->get_order_id(pRequest->UserID);
		m_ordInputActiondRefMap.insert(std::pair<int, int>(m_nRequestId, nOrderId));
		//loggerv2::info("cffex_api::ReqOrderAction insert <%d,%d> to m_ordInputActiondRefMap", m_nRequestId, nOrderId);

		return true;
	}


	bool cffex_api::ReqQuoteAction(CThostFtdcInputQuoteActionField * pRequest)
	{
		int ret = m_pUserApi->ReqQuoteAction(pRequest, ++m_nRequestId);
		//loggerv2::info("m_nRequestId %d", m_nRequestId);

		if (ret != 0)
		{
			return false;
		}

		int nOrderId = m_pConnection->get_order_id(pRequest->UserID, OrderWay::Buy);
		m_ordInputActiondRefMap.insert(std::pair<int, int>(m_nRequestId, nOrderId));
		//loggerv2::info("cffex_api::ReqOrderAction insert <%d,%d> to m_ordInputActiondRefMap", m_nRequestId, nOrderId);

		return true;
	}



	bool cffex_api::ReqExecOrderAction(CThostFtdcInputExecOrderActionField *pRequest)
	{

		int ret = m_pUserApi->ReqExecOrderAction(pRequest, ++m_nRequestId);
		//loggerv2::info("m_nRequestId %d", m_nRequestId);

		if (ret != 0)
		{
			return false;
		}

		int nOrderId = m_pConnection->get_order_id(pRequest->UserID);
		m_ordInputActiondRefMap.insert(std::pair<int, int>(m_nRequestId, nOrderId));
		//loggerv2::info("cffex_api::ReqOrderAction insert <%d,%d> to m_ordInputActiondRefMap", m_nRequestId, nOrderId);

		return true;
	}
	

	void cffex_api::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		if (m_pConnection != nullptr)
		{
			m_pConnection->OnRspQryInstrument(pInstrument, pRspInfo, nRequestID, bIsLast);
		}
	}

	void cffex_api::request_instruments()
	{
		CThostFtdcQryInstrumentField request;
		memset(&request, 0, sizeof(request));
		int ret = m_pUserApi->ReqQryInstrument(&request, ++m_nRequestId);
		printf_ex("cffex_api::request_instruments ret:%d\n", ret);
	}

	void cffex_api::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		m_pConnection->OnRspExecOrderInsert(pInputExecOrder, pRspInfo->ErrorID);
	}

	void cffex_api::OnRtnExecOrder(CThostFtdcExecOrderField *pOrder)
	{
		m_pConnection->OnRtnExecOrder(pOrder);
		
	}

	void cffex_api::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		pInputQuote->RequestID = pRspInfo->ErrorID;
		//m_inputQueue.Push(i);
		m_inputQuoteQueue.CopyPush(pInputQuote);
	}

	void cffex_api::OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		//loggerv2::info("calling cffex_api::OnRspOrderAction . request id=[%d] --> need to implement",nRequestID);

		if (!bIsLast)
		{
			return;
		}


		loggerv2::info("cffex_api::OnRspQuoteAction pInputQuoteAction - "
			"QuoteActionRef[%d]"
			"QuoteRef[%s]"
			"ExchangeID[%s]"
			"QuoteSysID[%s]"
			"ActionFlag[%c]"
			"UserID[%s]"
			"InstrumentID[%s]",

			pInputQuoteAction->QuoteActionRef,
			pInputQuoteAction->QuoteRef,
			pInputQuoteAction->ExchangeID,
			pInputQuoteAction->QuoteSysID,
			pInputQuoteAction->ActionFlag,
			pInputQuoteAction->UserID,
			pInputQuoteAction->InstrumentID

			);

		loggerv2::info("cffex_api::OnRspOrderActionpRspInfo - "
			"ErrorID[%d]"
			"ErrorMsg[%s]",
			pRspInfo->ErrorID,
			pRspInfo->ErrorMsg
			);




		int orderRef = get_ord_ref_from_reqid(nRequestID);
		if (!orderRef)
		{
			loggerv2::error("could not find associated order for nRequestId %d", nRequestID);
			return;
		}

		int* i = new int(orderRef);
		m_inputActionQuoteQueue.Push(i);
	}

	


}

