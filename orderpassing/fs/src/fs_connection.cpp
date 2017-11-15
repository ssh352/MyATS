#include "fs_connection.h"
#include "string_tokenizer.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>
#include "tradeItem_gh.h"
#include "terra_logger.h"
#include "SgitFtdcUserApiStruct.h"


using namespace terra::common;
using namespace fstech;
namespace fs
{

	//
	// fs_connection
	//
	fs_connection::fs_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "fs_connection";



#if 0
		m_pfsApi = new fs_api(this);
#else
		m_connectionStatus = false;
		m_isAlive = true;
		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
#endif


		m_bKey_with_exchange = false;

	}
	//
	//	fs_connection::~fs_connection()
	//	{
	//#if 0
	//		delete m_pfsApi;
	//#else
	//
	//#endif
	//
	//	}


	bool fs_connection::init_config(const string &name, const std::string &strConfigFile)
	{

		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;

		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 4)
			m_bTsession = true;
		else
			m_bTsession = false;
		return true;
	}

#if 0
	void fs_connection::request_instruments()
	{
		m_pfsApi->request_instruments();
	}
#endif



	void fs_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{

		fstech::CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		strcpy(pRequest.InstrumentID, i->get_trading_code());
#if 0
		m_pfsApi->ReqQryInvestorPosition(&pRequest);
#else
		this->ReqQryInvestorPosition(&pRequest);
#endif


		if (m_debug)
			loggerv2::info("fs_connection::request_investor_position requesting investor position for tradeitem %s", i->getCode().data());


		return;
	}


	void fs_connection::request_investor_full_positions()
	{

		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
#if 0
		m_pfsApi->ReqQryInvestorPosition(&pRequest);
#else
		this->ReqQryInvestorPosition(&pRequest);
#endif

		if (m_debug)
			loggerv2::info("fs_connection:: calling OnRspQryInvestorPosition ");

		return;
	}



	void fs_connection::request_trading_account()
	{
		if (m_debug)
			loggerv2::info("fs_connection:: calling ReqQryTradingAccount ");

		fstech::CThostFtdcQryTradingAccountField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		//strcpy(pRequest.InstrumentID, i->get_trading_code());
#if 0
		m_pfsApi->ReqQryTradingAccount(&pRequest);
#else
		this->ReqQryTradingAccount(&pRequest);
#endif

		return;

	}

	void fs_connection::init_connection()
	{

		//m_outboundQueue.setHandler(boost::bind(&fs_connection::process_outbound_msg_cb, this));
		loggerv2::info("fs_connection::init_connection create trader api..");
#if 0	
		m_pfsApi->init();
#else
		this->init_api();
#endif

#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
		//init_process(io_service_type::trader, 10);
		//std::thread th(boost::bind(&fs_connection::set_kernel_timer_thread, this));
		//m_thread.swap(th);
	}

#ifdef Linux
	void  fs_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&fs_connection::process, this));
		m_inputQueue.set_fd(efd);
		m_orderQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_inputActionQueue.set_fd(efd);
		m_outboundQueue.set_fd(efd);
	}
#endif

	void fs_connection::release()
	{

		ctpbase_connection::release();

#if 0		
		m_pfsApi->release();
#else
		this->release_api();
#endif


	}

	void fs_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("fs_connection::connect connecting to fs...");

			on_status_changed(AtsType::ConnectionStatus::WaitConnect);

#if 0
			m_pfsApi->connect();
#else
			this->connect_api();
#endif 
		}
	}

	void fs_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
#if 0
			if (m_pfsApi->disconnect() == false)
#else
			if (this->disconnect_api() == false)
#endif
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "fs_connection - ReqUserLogout failed");
			}
		}
	}




	void fs_connection::process()
	{


		m_outboundQueue.Pops_Handle_Keep(10);
#if 0
		m_pfsApi->Process();
#else
		this->Process_api();
#endif


	}

	int fs_connection::market_create_order_async(order* o, char* pszReason)
	{
		


		CThostFtdcInputOrderField *request = xs_create_pool.get_mem();
		memset(request, 0, sizeof(CThostFtdcInputOrderField));

		strcpy(request->BrokerID, m_sBrokerId.c_str());
		strcpy(request->InvestorID, m_sUsername.c_str());

		strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());


		if (o->get_way() == AtsType::OrderWay::Buy)
			request->Direction = THOST_FTDC_D_Buy;
		else if (o->get_way() == AtsType::OrderWay::Sell)
			request->Direction = THOST_FTDC_D_Sell;

		request->LimitPrice = o->get_price();
		request->VolumeTotalOriginal = o->get_quantity();

		if (o->get_restriction() == AtsType::OrderRestriction::None)
			request->TimeCondition = THOST_FTDC_TC_GFS; // or GFS ???
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)
			request->TimeCondition = THOST_FTDC_TC_IOC;
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			xs_create_pool.free_mem(request);
			return 0;
		}

		TThostFtdcOffsetFlagType oc = THOST_FTDC_OF_Open;
		if (o->get_open_close() == OrderOpenClose::Undef)
		{
			o->set_open_close(compute_open_close(o, m_bCloseToday));
		}

		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			break;

		case AtsType::OrderOpenClose::Close:
			oc = THOST_FTDC_OF_Close;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			oc = THOST_FTDC_OF_CloseToday;
			break;

		default:

			break;
		}

		request->CombOffsetFlag[0] = oc;
		if (m_debug)
			loggerv2::info("fs_connection::market_create_order CombOffsetFlag is %c", oc);





#if 0
		if (!compute_userId(o, request->UserID, sizeof(request->UserID)))
		{
#else
		if (!get_userId(m_sUsername.c_str(), request->UserID, sizeof(request->UserID)))
		{
#endif
			xs_create_pool.free_mem(request);
			return 0;
		}


		switch (o->get_price_mode())
		{
		case AtsType::OrderPriceMode::Limit:
		{
			request->OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			//strcpy(request->LimitPrice, std::to_string(o->get_price()).c_str());
			request->LimitPrice = o->get_price();
		}
		break;
		case AtsType::OrderPriceMode::Market:
		{
			request->OrderPriceType = THOST_FTDC_OPT_BestPrice;
		}

		break;
		default:
		{
			snprintf(pszReason, REASON_MAXLENGTH, "undefined price mode.\n");
			o->set_status(AtsType::OrderStatus::Reject);
			xs_create_pool.free_mem(request);
			return 0;
		}
		}


		request->CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;

		//request->TimeCondition = THOST_FTDC_TC_GFD;

		if (o->get_restriction() == AtsType::OrderRestriction::None)
		{
			request->TimeCondition = THOST_FTDC_TC_GFD; // or GFS ???
			strcpy(request->GTDDate, "");
		}
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)
			request->TimeCondition = THOST_FTDC_TC_IOC;
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			xs_create_pool.free_mem(request);
			return 0;
		}


		request->VolumeCondition = THOST_FTDC_VC_AV;
		request->ContingentCondition = THOST_FTDC_CC_Immediately;
		request->ForceCloseReason = THOST_FTDC_FCC_NotForceClose;

		request->MinVolume = 1;
		request->IsAutoSuspend = 0;
		request->UserForceClose = 0;

#if 0
		if (!m_pfsApi->ReqOrderInsert(request))
#else
		if (!this->ReqOrderInsert(request))
#endif
		{
			this->create_user_info(o, atoi(request->OrderRef), request->UserID);
			snprintf(pszReason, REASON_MAXLENGTH, "fs api reject!\n");
			xs_create_pool.free_mem(request);
			return 0;
		}
		this->create_user_info(o, atoi(request->OrderRef), request->UserID);
		xs_create_pool.free_mem(request);
		return 1;
		}


	int fs_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		//#endif

		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());

		//fs_order* o = dynamic_cast<fs_order*>(ord);
		//if (o == NULL)
		//{
		//	snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to fs_order*...\n");
		//	o->set_status(AtsType::OrderStatus::Nack);
		//	o->rollback();
		//	return 0;
		//}


		CThostFtdcInputOrderActionField *request = xs_cancel_pool.get_mem();
		memset(request, 0, sizeof(CThostFtdcInputOrderActionField));

		//strcpy(request->BrokerID, m_sBrokerId.c_str());
		strcpy(request->InvestorID, m_sUsername.c_str());

		strcpy(request->ExchangeID, o->get_exchange_id().c_str());
		strcpy(request->OrderSysID, fs_order_aux::get_order_sys_id(o).c_str());

#if 0
		if (!compute_userId(o, request->UserID, sizeof(request->UserID)))
#else
		if (!get_userId(m_sUsername.c_str(), request->UserID, sizeof(request->UserID)))
#endif
		{
			xs_cancel_pool.free_mem(request);
			return -1;
		}


		loggerv2::info("reqest.UserId %s", request->UserID);

		request->ActionFlag = THOST_FTDC_AF_Delete;

		//to do ...
		sprintf(request->OrderRef, "%012d", fs_order_aux::get_ord_ref(o));
		sprintf(request->InstrumentID, "%s", o->get_instrument()->getCode().c_str());
		//end
#if 0
		if (!m_pfsApi->ReqOrderAction(request))
#else
		if (!this->ReqOrderAction(request))
#endif
		{
			xs_cancel_pool.free_mem(request);
			return 0;
		}

		//
		xs_cancel_pool.free_mem(request);
		return 1;
		//return ord->get_id();
	}


	//
	// fs callbacks
	//
	void fs_connection::OnRspOrderInsertAsync(CThostFtdcInputOrderField* pOrder)
	{
		//
		// used only for rejects.
		//


		//fs_order* o = NULL;

		// 0 - log
		if (m_debug)
			loggerv2::info("fs_connection::OnRspOrderInsertAsync - orderRef[%s] userId[%s] RequestID[%d] errorId[%d]", pOrder->OrderRef, pOrder->UserID, pOrder->RequestID, pOrder->IsAutoSuspend);

		// 1 - retrieve order
#if 0
		int orderId = get_order_id(pOrder->UserID);
#else
		int orderId = get_order_id(pOrder->OrderRef);
#endif
		if (orderId == -1)
		{
			loggerv2::warn("fs_connection::OnRspOrderInsertAsync - cannot extract orderId from UserId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return;
			//o->set_portfolio("UNKNOWN");
			orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
		}

		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<fs_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<fs_order*>(ord);
			loggerv2::warn("fs_connection::OnRspOrderInsertAsync - message received on dead order[%d]...", orderId);
			break;

		case 2:

			o = fs_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("fs_connection::OnRspOrderInsertAsync cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;

		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("fs_connection::OnRspOrderInsertAsync - order recovered NULL");
			return;
		}


		// 2 - treat message
		if (pOrder->IsAutoSuspend != 0)
		{
			char szErrorMsg[32 + 1];
			snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", pOrder->IsAutoSuspend);

			on_nack_from_market_cb(o, szErrorMsg);
			//bug fix. should not call update_instr_on_nack_from_market_cb, because we haven't ack the order before!
			//
			//update_instr_on_nack_from_market_cb(o);
		}
		else
		{
			loggerv2::error("OnRspOrderInsertAsync - order[%d] errorId[0] ???", orderId);
		}
	}

	void fs_connection::OnRspOrderActionAsync(int* nOrdId)
	{
		if (m_debug)
			loggerv2::info("fs_connection::OnRspOrderActionAsync");
		//std::map<int, fs_order*>::iterator it = m_cancelOrdMap.find(nOrdRef);
		//
		//if (it != m_cancelOrdMap.end())
		//{
		// fs_order* o = reinterpret_cast<fs_order*>(it->second); // or dynamic_cast??
		// if (o != nullptr)
		// {
		//  on_nack_from_market_cb(o,NULL);
		// }

		// m_cancelOrdMap.erase(it);
		//}
		//fs_order* o = NULL;

		int ret;
		order* o = get_order_from_map(*nOrdId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<fs_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<fs_order*>(ord);
			loggerv2::warn("fs_connection::OnRspOrderActionAsync - message received on dead order[%d]...", nOrdId);
			break;

		case 2:
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("fs_connection::OnRspOrderActionAsync - order recovered NULL");
			return;
		}

		on_nack_from_market_cb(o, NULL);
		//update_instr_on_nack_from_market_cb(o);
	}



	void fs_connection::OnRtnOrderAsync(CThostFtdcOrderField* pOrder)
	{

		// 0 - log
		if (m_debug)
			loggerv2::info("fs_connection::OnRtnOrderAsync - "
			"BrokerID[%*.*s] "
			"InvestorID[%*.*s] "
			"InstrumentID[%*.*s] "
			"OrderRef[%*.*s] "
			"UserID[%*.*s] "

			"OrderPriceType[%c] "
			"Direction[%c] "
			"CombOffsetFlag[%*.*s] "
			"CombHedgeFlag[%*.*s] "
			"LimitPrice[%f] "
			"VolumeTotalOriginal[%d] "
			"TimeCondition[%c] "

			"GTDDate[%*.*s] "

			"VolumeCondition[%c] "
			"MinVolume[%d] "
			"ContingentCondition[%c] "
			"StopPrice[%f] "
			"ForceCloseReason[%c] "
			"IsAutoSuspend[%d] "

			"BusinessUnit[%*.*s] "
			"RequestID[%d] "
			"OrderLocalID[%*.*s] "
			"ExchangeID[%*.*s] "
			"ParticipantID[%*.*s] "
			"ClientID[%*.*s] "
			"ExchangeInstID[%*.*s] "
			"TraderID[%*.*s] "

			"InstallID[%d] "
			"OrderSubmitStatus*[%c] "
			"NotifySequence[%d] "
			"TradingDay[%*.*s] "
			"SettlementID[%d] "
			"OrderSysID[%*.*s] "
			"OrderSource[%c] "
			"OrderStatus*[%c] "
			"OrderType[%c] "
			"VolumeTraded[%d] "
			"VolumeTotal[%d] "

			"InsertDate[%*.*s] "
			"InsertTime[%*.*s] "
			"ActiveTime[%*.*s] "
			"SuspendTime[%*.*s] "
			"UpdateTime[%*.*s] "
			"CancelTime[%*.*s] "

			"ActiveTraderID[%*.*s] "
			"ClearingPartID[%*.*s] "

			"SequenceNo[%d] "
			"FrontID[%d] "
			"SessionID[%d] "
			"UserProductInfo[%*.*s] "
			"StatusMsg[%*.*s] "
			"UserForceClose[%d] "
			"ActiveUserID[%*.*s] "
			"BrokerOrderSeq[%d] "
			"RelativeOrderSysID[%*.*s] "
			,

			sizeof(pOrder->BrokerID), sizeof(pOrder->BrokerID), pOrder->BrokerID,
			sizeof(pOrder->InvestorID), sizeof(pOrder->InvestorID), pOrder->InvestorID,
			sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID,
			sizeof(pOrder->OrderRef), sizeof(pOrder->OrderRef), pOrder->OrderRef,
			sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID,

			pOrder->OrderPriceType,
			pOrder->Direction,
			sizeof(pOrder->CombOffsetFlag), sizeof(pOrder->CombOffsetFlag), pOrder->CombOffsetFlag,
			sizeof(pOrder->CombHedgeFlag), sizeof(pOrder->CombHedgeFlag), pOrder->CombHedgeFlag,
			pOrder->LimitPrice,
			pOrder->VolumeTotalOriginal,
			pOrder->TimeCondition,

			sizeof(pOrder->GTDDate), sizeof(pOrder->GTDDate), pOrder->GTDDate,
			pOrder->VolumeCondition,
			pOrder->MinVolume,
			pOrder->ContingentCondition,
			pOrder->StopPrice,
			pOrder->ForceCloseReason,
			pOrder->IsAutoSuspend,

			sizeof(pOrder->BusinessUnit), sizeof(pOrder->BusinessUnit), pOrder->BusinessUnit,
			pOrder->RequestID,
			sizeof(pOrder->OrderLocalID), sizeof(pOrder->OrderLocalID), pOrder->OrderLocalID,
			sizeof(pOrder->ExchangeID), sizeof(pOrder->ExchangeID), pOrder->ExchangeID,
			sizeof(pOrder->ParticipantID), sizeof(pOrder->ParticipantID), pOrder->ParticipantID,
			sizeof(pOrder->ClientID), sizeof(pOrder->ClientID), pOrder->ClientID,
			sizeof(pOrder->ExchangeInstID), sizeof(pOrder->ExchangeInstID), pOrder->ExchangeInstID,
			sizeof(pOrder->TraderID), sizeof(pOrder->TraderID), pOrder->TraderID,

			pOrder->InstallID,
			pOrder->OrderSubmitStatus,
			pOrder->NotifySequence,
			sizeof(pOrder->TradingDay), sizeof(pOrder->TradingDay), pOrder->TradingDay,
			pOrder->SettlementID,
			sizeof(pOrder->OrderSysID), sizeof(pOrder->OrderSysID), pOrder->OrderSysID,
			pOrder->OrderSource,
			pOrder->OrderStatus,
			pOrder->OrderType,
			pOrder->VolumeTraded,
			pOrder->VolumeTotal,

			sizeof(pOrder->InsertDate), sizeof(pOrder->InsertDate), pOrder->InsertDate,
			sizeof(pOrder->InsertTime), sizeof(pOrder->InsertTime), pOrder->InsertTime,
			sizeof(pOrder->ActiveTime), sizeof(pOrder->ActiveTime), pOrder->ActiveTime,
			sizeof(pOrder->SuspendTime), sizeof(pOrder->SuspendTime), pOrder->SuspendTime,
			sizeof(pOrder->UpdateTime), sizeof(pOrder->UpdateTime), pOrder->UpdateTime,
			sizeof(pOrder->CancelTime), sizeof(pOrder->CancelTime), pOrder->CancelTime,

			sizeof(pOrder->ActiveTraderID), sizeof(pOrder->ActiveTraderID), pOrder->ActiveTraderID,
			sizeof(pOrder->ClearingPartID), sizeof(pOrder->ClearingPartID), pOrder->ClearingPartID,

			pOrder->SequenceNo,
			pOrder->FrontID,
			pOrder->SessionID,
			sizeof(pOrder->UserProductInfo), sizeof(pOrder->UserProductInfo), pOrder->UserProductInfo,
			sizeof(pOrder->StatusMsg), sizeof(pOrder->StatusMsg), pOrder->StatusMsg,
			pOrder->UserForceClose,
			sizeof(pOrder->ActiveUserID), sizeof(pOrder->ActiveUserID), pOrder->ActiveUserID,
			pOrder->BrokerOrderSeq,
			sizeof(pOrder->RelativeOrderSysID), sizeof(pOrder->RelativeOrderSysID), pOrder->RelativeOrderSysID
			);

		//fs_order* o = NULL;

		// 1 - retrieve order


		//std::string insertDate = pOrder->InsertDate;
		//if (insertDate != m_sCurrentBizDate)
		//{
		//	/*loggerv2::warn("fs_connection::OnRtnOrderAsync - order insert date [%s] doesn't match current business date[%s].", insertDate.c_str(), m_sCurrentBizDate.c_str());
		//	return;*/
		//	loggerv2::warn("fs_connection::OnRtnOrderAsync - order insert date [%s] doesn't match current business date[%s]. shoule be from T+1 session", insertDate.c_str(), m_sCurrentBizDate.c_str());
		//	//return;
		//}



		int orderId = get_order_id(pOrder->OrderRef);
		if (orderId == -1)
		{
			loggerv2::warn("fs_connection::OnRtnOrderAsync - cannot extract orderId from USerId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return;
			orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
		}

		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<fs_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<fs_order*>(ord);
			loggerv2::warn("fs_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;

		case 2:

			o = fs_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("fs_connection::OnSOPEntrustOrderRtnAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;
		}



		//if (m_activeOrders.find(orderId)!=m_activeOrders.end())
		//{
		//	o = reinterpret_cast<fs_order*>(m_activeOrders[orderId]); // or dynamic_cast??
		//	//o = dynamic_cast<fs_order*>(it->second);

		//}
		//else
		//{
		//	if (m_deadOrders.find(orderId) != m_deadOrders.end())
		//	{
		//		o = reinterpret_cast<fs_order*>(m_deadOrders[orderId]); // or dynamic_cast??
		//		//o = dynamic_cast<fs_order*>(it->second);
		//		loggerv2::warn("fs_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
		//	}
		//	else
		//	{
		//		o = fs_order::anchor(this, pOrder);
		//		if (o == NULL)
		//			return;

		//		add_pending_order(o);
		//	}
		//}
		if (o == NULL) // should not happen
		{
			loggerv2::error("fs_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}


		// fs BUG:
		// on a cancel, first message has OrderSubmitStatus = THOST_FTDC_OSS_Accepted instead of THOST_FTDC_OSS_CancelSubmitted
		// -> trash
		//if (o->get_last_action() == AtsType::OrderAction::Cancelled && pOrder->OrderSubmitStatus == THOST_FTDC_OSS_Accepted && pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
		//{
		//   loggerv2::info("fs_connection::OnRtnOrder - fs bug, TRASH");
		//   return;
		//}


		// 2 - treat message
		//o->set_exchange_id(pOrder->ExchangeID);
		fs_order_aux::set_order_sys_id(o, pOrder->OrderSysID);
		if (o->get_quantity() != pOrder->VolumeTotalOriginal)
		{
			if (m_debug)
				loggerv2::debug("fs_connection::OnRtnOrderAsync resetting order quantity to %d", pOrder->VolumeTotalOriginal);
			o->set_quantity(pOrder->VolumeTotalOriginal);
		}


		if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
		{
			if (m_debug)
				loggerv2::debug("fs_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
			o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
		}

		// cancel 1: 3-3
		// cancel 2: 3-5 : sometimes 3-5 is not coming

		//printf("fs_connection::OnRtnOrderAsync orderId:%d,orderRef:%s,OrderSubmitStatus:%c(%d),OrderStatus:%c(%d),orderSysID:%s,status:%d,last_action:%d,ret:%d\n", orderId, pOrder->OrderRef, pOrder->OrderSubmitStatus, pOrder->OrderSubmitStatus, pOrder->OrderStatus, pOrder->OrderStatus,pOrder->OrderSysID,o->get_status(),o->get_last_action(),ret);
#if 0
		switch (pOrder->OrderStatus)
		{
		case THOST_FTDC_OST_AllTraded:
			printf("pOrder->OrderStatus THOST_FTDC_OST_AllTraded(全部成交)\n");
			break;
		case THOST_FTDC_OST_PartTradedQueueing:
			printf("pOrder->OrderStatus THOST_FTDC_OST_PartTradedQueueing(部分成交还在队列中)\n");
			break;
		case THOST_FTDC_OST_PartTradedNotQueueing:
			printf("pOrder->OrderStatus THOST_FTDC_OST_PartTradedNotQueueing(部分成交不在队列中)\n");
			break;
		case THOST_FTDC_OST_NoTradeQueueing:
			printf("pOrder->OrderStatus THOST_FTDC_OST_NoTradeQueueing(未成交还在队列中)\n");
			break;
		case THOST_FTDC_OST_NoTradeNotQueueing:
			printf("pOrder->OrderStatus THOST_FTDC_OST_NoTradeNotQueueing(未成交不在队列中)\n");
			break;
		case THOST_FTDC_OST_Canceled:
			printf("pOrder->OrderStatus THOST_FTDC_OST_Canceled(撤单)\n");
			break;
		case THOST_FTDC_OST_Unknown:
			printf("pOrder->OrderStatus THOST_FTDC_OST_Unknown(未知)\n");
			break;
		case THOST_FTDC_OST_NotTouched:
			printf("pOrder->OrderStatus THOST_FTDC_OST_NotTouched(尚未触发)\n");
			break;
		case THOST_FTDC_OST_Touched:
			printf("pOrder->OrderStatus THOST_FTDC_OST_Touched(已触发)\n");
			break;
		default:
			break;
		}
#endif
		switch (pOrder->OrderSubmitStatus)
		{
		case THOST_FTDC_OSS_InsertSubmitted:
		{
			//printf("THOST_FTDC_OSS_InsertSubmitted 已经提交\n");
			//special case, we could receive status 0-5 for cancellation from another application.
			if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
			{
				//o->set_last_action(AtsType::OrderAction::Cancelled);

				update_instr_on_ack_from_market_cb(o);
				on_cancel_from_market_cb(o);
			}

			else if (o->get_last_action() == AtsType::OrderAction::Created || o->get_last_action() == AtsType::OrderAction::Cancelled)
			{
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{

					update_instr_on_ack_from_market_cb(o);
					if (o->get_last_action() == AtsType::OrderAction::Cancelled)
						on_cancel_from_market_cb(o);
					else
						on_ack_from_market_cb(o);
				}
				if (o->get_status() == AtsType::OrderStatus::Exec)
				{
					loggerv2::info("fs_connection::OnRtnOrderAsync receive order ack while order status is exec, this is possibly during the resynchro process.");
					//we recvieve the exec earlier than ack , so order is already created.

					update_instr_on_ack_from_market_cb(o);
					on_ack_from_market_cb(o);
				}

			}

		}
		break;
		case THOST_FTDC_OSS_CancelSubmitted:
		{
			//printf("THOST_FTDC_OSS_CancelSubmitted 撤单已经提交\n");
			if (o->get_last_action() != AtsType::OrderAction::Cancelled)
			{
				o->set_last_action(AtsType::OrderAction::Cancelled);
			}
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{
				on_cancel_from_market_cb(o);
				//update_instr_on_ack_from_market_cb(o);
				//don't need to update because order will be updated on the second cancel confirmation.
			}
		}
		break;
		case THOST_FTDC_OSS_ModifySubmitted:
		{
			//printf("THOST_FTDC_OSS_ModifySubmitted 修改已经提交\n");
			if (o->get_last_action() != AtsType::OrderAction::Modified)
			{
				o->set_last_action(AtsType::OrderAction::Modified);
			}
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{

				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}
		}
		break;
		case THOST_FTDC_OSS_Accepted:
		{
			//printf("THOST_FTDC_OSS_Accepted 已经接受\n");
			// FIX bug on resynchro

			//ToTest
			//if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
			//{
			//	//3. order is being cancelled, but not acked yet.
			//	o->set_last_action(AtsType::OrderAction::Cancelled);
			//	if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			//	{

			//		update_instr_on_nack_from_market_cb(o);
			//		on_nack_from_market_cb(o,"waiting for cancel confirmation");
			//	}
			//
			//}
			//End of Test

			if (o->get_last_action() == AtsType::OrderAction::Created || o->get_last_action() == AtsType::OrderAction::Cancelled)
			{
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{

					update_instr_on_ack_from_market_cb(o);
					if (o->get_last_action() == AtsType::OrderAction::Cancelled)
						on_cancel_from_market_cb(o);
					else
						on_ack_from_market_cb(o);
				}
				if (o->get_status() == AtsType::OrderStatus::Exec)
				{
					loggerv2::warn("fs_connection::OnRtnOrderAsync receive order ack while order status is exec, this is possibly during the resynchro process.");
					//we recvieve the exec earlier than ack , so order is already created.

					update_instr_on_ack_from_market_cb(o);
					on_ack_from_market_cb(o);
				}

			}

			if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled || pOrder->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing || pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing)
				// '5','2','4'
			{
				o->set_last_action(AtsType::OrderAction::Cancelled);
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{

					update_instr_on_ack_from_market_cb(o);
					on_cancel_from_market_cb(o);
				}
				else if (o->get_status() == AtsType::OrderStatus::Ack || o->get_status() == AtsType::OrderStatus::Nack)
				{
					loggerv2::error("Calling on_ack_from_market_cb.. this is possibly the resynchro process..");

					/*on_ack_from_market_cb(o);
					*///update_instr_on_ack_from_market_cb(o);


					update_instr_on_cancel_from_market_cb(o);
					on_cancel_from_market_cb(o);

					//m_statistics.incr_can();
					loggerv2::error("fs_connection::OnRtnOrderAsync Current cancel number is %d", m_statistics.get_can());
				}

				//earse the pointer from m_cancelOrdMap;
				//std::map<int, fs_order*>::iterator it = m_cancelOrdMap.find(atoi(pOrder->OrderRef));
				//if (it != m_cancelOrdMap.end())
				//	m_cancelOrdMap.erase(it);
				//update_instr_on_ack_from_market_cb(o);
			}
		}
		break;
		case THOST_FTDC_OSS_InsertRejected:
		{
			//printf("THOST_FTDC_OSS_InsertRejected 报单已经被拒绝\n");
			update_instr_on_nack_from_market_cb(o);
			on_nack_from_market_cb(o, pOrder->StatusMsg);
			break;
		}
		case THOST_FTDC_OSS_CancelRejected:
		{
			//printf("THOST_FTDC_OSS_CancelRejected 撤单已经被拒绝\n");
			on_nack_from_market_cb(o, pOrder->StatusMsg);
			if (o->get_status() == AtsType::OrderStatus::Nack) //this is nack of a cancel and we rolled back to order with book qty!=0
				update_instr_on_nack_from_market_cb(o);
		}
		break;
		case THOST_FTDC_OSS_ModifyRejected:
		{
			//printf("THOST_FTDC_OSS_ModifyRejected 改单已经被拒绝\n");
			on_nack_from_market_cb(o, pOrder->StatusMsg);
			//update_instr_on_nack_from_market_cb(o);
			break;
		}
		default:
			break;
		}
	}

	void fs_connection::OnRtnTradeAsync(CThostFtdcTradeField* pTrade)
	{
		// 0 - log
		//loggerv2::info();

		//fs_order* o = NULL;

		// 1 - retrieve order
#if 0
		int orderId = get_order_id(pTrade->UserID);
#else
		int orderId = get_order_id(pTrade->OrderRef);
#endif
		if (orderId == -1)
		{
			loggerv2::warn("fs_connection::OnRtnTradeAsync - cannot extract orderId from USerId[%*.*s]...", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			//return;
			//o->set_portfolio("UNKNOWN");
			orderId = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}

		//std::string tradeDate = pTrade->TradeDate;
		//if (tradeDate != m_sCurrentBizDate)
		//{
		//	//loggerv2::warn("fs_connection::OnRtnTradeAsync - trading date [%s] doesn't match current business date[%s].", tradeDate.c_str(), m_sCurrentBizDate.c_str());
		//	//return;
		//	loggerv2::warn("fs_connection::OnRtnTradeAsync - trading date [%s] doesn't match current business date[%s]. should be from T+1 session", tradeDate.c_str(), m_sCurrentBizDate.c_str());
		//	//return;
		//}

		int ret;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<fs_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<fs_order*>(ord);
			loggerv2::warn("fs_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;

		case 2:

			o = fs_order_aux::anchor(this, pTrade);
			if (o == NULL)
			{
				loggerv2::error("fs_connection::OnRtnTradeAsync cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("fs_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}

		//printf("fs_connection::OnRtnTradeAsync orderId:%d,pTrade->OrderRef:%s,ret:%d,trandeId:%s\n",orderId,pTrade->OrderRef,ret,pTrade->TradeID);

		// 2 - treat message
		int execQty = pTrade->Volume;
		double execPrc = pTrade->Price;
		const char* pszExecRef = pTrade->TradeID;
		const char* pszTime = pTrade->TradeTime;


		exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);

		on_exec_from_market_cb(o, e);

		//date_time tradeTime;
		//tradeTime.set_date(pTrade->TradeDate, date_time::FN2);
		//tradeTime.set_time(pszTime);
		lwtp tp;
#if 0
		//ptime tradeTime(from_undelimited_string(pTrade->TradeDate), duration_from_string(pTrade->TradeTime));
#else
		if (strlen(pTrade->TradeDate) > 0)
		{
			//tradeTime = ptime(from_undelimited_string(pTrade->TradeDate), duration_from_string(pTrade->TradeTime));
			tp = string_to_lwtp(from_undelimited_string(pTrade->TradeDate), (pTrade->TradeTime));
		}
		else
		{
			if (strlen(pTrade->TradingDay) > 0)
			{
				//tradeTime = ptime(from_undelimited_string(pTrade->TradingDay), duration_from_string(pTrade->TradeTime));
				tp = string_to_lwtp(from_undelimited_string(pTrade->TradingDay), (pTrade->TradeTime));
			}
		}

#endif
		bool onlyUpdatePending = false;
		//if (o->get_instrument()->get_last_sychro_time() > tradeTime)

		int hour = get_hour_from_lwtp(tp);
		tp = tp + std::chrono::seconds(2);


		if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			onlyUpdatePending = true;
		if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;

		if (onlyUpdatePending)
		{
			loggerv2::info("fs_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time. tradeTime %s", pTrade->TradeTime);
		}

		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);


	}
	int fs_connection::get_order_id(const char*pszOrderRef)
	{
		if (pszOrderRef != nullptr)
		{
			if (m_user_info_map.find(atoi(pszOrderRef)) != m_user_info_map.end())
			{
				user_info * info = m_user_info_map[atoi(pszOrderRef)];
#if 0
				return info->InternalRef;
#else

				return ctpbase_connection::get_order_id(info->UserID.c_str());
#endif
			}
		}
		return 0;
	}
	/*根据资金账号生成userID*/
	bool fs_connection::get_userId(const char* pszUserName, char* userID, int n)
	{
		memset(userID, 0, n);
		strcpy(userID, pszUserName);
		return true;
	}
	/*根据order.internalRef简历internalRef<->orderRef的映射关系*/
	void fs_connection::create_user_info(order * o, int orderRef, string userID)
	{
		if (o == nullptr)
			return;
		//
		fs_order_aux::set_ord_ref(o, orderRef);
		//
		//
		//if (m_order_id_map.find(o->get_id()) == m_order_id_map.end())
		//{
		//m_order_id_map.emplace(o->get_id(), orderRef);
		//}
		//
		if (m_user_info_map.find(orderRef) == m_user_info_map.end())
		{
			user_info * info = new user_info();
			info->OrderRef = orderRef;
#if 0
			info->UserID = userID;
			info->InternalRef = o->get_id();
			info->AccountNum = getAccountNum(o->get_account().c_str());
			info->UserOrderID = o->get_user_orderid();
			info->TradeType = getPortfolioNum(o->get_portfolio().c_str());
			info->Portfolio = o->get_trading_type();
#else
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			ctpbase_connection::compute_userId(o, buffer, sizeof(buffer));
			info->UserID = buffer;
#endif
			m_user_info_map.emplace(orderRef, info);

			//to do ... append the file every day
			this->append(info);

			//printf("fs_connection::create_user_info orderRef:%d,size:%d\n", orderRef, m_user_info_map.size());
		}
		else
		{
			printf("warn:fs_connection::create_user_info already include the userid:%s,orderRef:%d\n", userID.c_str(), orderRef);
		}
	}
	void fs_connection::get_user_info(const char* pszOrderRef, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType)
	{
		if (pszOrderRef != nullptr)
		{
			if (m_user_info_map.find(atoi(pszOrderRef)) != m_user_info_map.end())
			{
				user_info * info = m_user_info_map[atoi(pszOrderRef)];
#if 0
				internalRef = info->InternalRef;
				nAccount = info->AccountNum;
				userOrderId = info->UserOrderID;
				nPortfolio = info->Portfolio;
				nTradeType = info->TradeType;
#else
				ctpbase_connection::get_user_info(info->UserID.c_str(), nAccount, userOrderId, internalRef, nPortfolio, nTradeType);
#endif
				loggerv2::info("fs_connection::get_user_info internalRef:%d,orderRef:%d,size:%d\n", internalRef, atoi(pszOrderRef), m_user_info_map.size());
			}
			else
			{
				loggerv2::warn("fs_connection::get_user_info didn't find the orderRef:%s\n", pszOrderRef);
			}
		}
		else
		{
			internalRef = -1;
		}
	}
	void fs_connection::init_user_info(char * user_info_file)
	{
		if (user_info_file == nullptr)
			return;
		boost::filesystem::path p;
		p.clear();
		p.append(user_info_file);
		p.append("user_info.csv");
		m_user_info_file_name = p.string();
		printf("fs_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
		if (!boost::filesystem::exists(p))
			return;
		boost::filesystem::ifstream stream;
		stream.open(m_user_info_file_name.c_str());
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		std::string line;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0 || line[0] == '#')
				continue;
			tokenizer.break_line(line.c_str(), szSeparators);
			/*
			class user_info
			{
			public:
			int    OrderRef;
			string UserID;
			#if 0
			int    InternalRef;
			int    AccountNum;
			int    UserOrderID;
			int    TradeType;
			int    Portfolio;
			#endif
			};
			*/
			user_info * info = new user_info();
			info->OrderRef = atoi(tokenizer[0]);
			info->UserID = tokenizer[1];
#if 0
			info->InternalRef = atoi(tokenizer[2]);
			info->AccountNum = atoi(tokenizer[3]);
			info->UserOrderID = atoi(tokenizer[4]);
			info->TradeType = atoi(tokenizer[5]);
			info->Portfolio = atoi(tokenizer[6]);
#endif
			m_user_info_map.emplace(info->OrderRef, info);

			//if (m_order_id_map.find(info->InternalRef) == m_order_id_map.end())
			//{
			//m_order_id_map.emplace(info->InternalRef,info->OrderRef);
			//}
		}
		stream.close();
	}

	void fs_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
#if 0
		sprintf(buffer, "%s,%d,%d,%d,%d,%d,%d\n", info->UserID.c_str(), info->OrderRef, info->InternalRef, info->AccountNum, info->UserOrderID, info->TradeType, info->Portfolio);
#else
		sprintf(buffer, "%d,%s\n", info->OrderRef, info->UserID.c_str());
#endif
		stream << buffer;
		stream.close();
	}

	//begin add on 20160929,from fstech::CThostFtdcTraderSpi
	//int fs_connection::process_inbound_input_cb()
	//{
	//	CThostFtdcInputOrderField* pInput = NULL;
	//
	//	int i = 0;
	//	for (; i < 10 && m_inputQueue.m_queue.read_available()>0; ++i)
	//	{
	//
	//		pInput = m_inputQueue.Pop();
	//		if (pInput != NULL)
	//		{
	//
	//			OnRspOrderInsertAsync(pInput, pInput->IsAutoSuspend);
	//			delete pInput;
	//		}
	//		i++;
	//	}
	//	return i;
	//}
	//
	//int fs_connection::process_inbound_input_action_cb()
	//{
	//	int* pInput = NULL;
	//
	//	int i = 0;
	//	for (; i < 10 && m_inputActionQueue.m_queue.read_available()>0; ++i)
	//	{
	//
	//		pInput = m_inputActionQueue.Pop();
	//		if (pInput != NULL)
	//		{
	//			OnRspOrderActionAsync(*pInput);
	//			delete pInput;
	//		}
	//		i++;
	//	}
	//	return i;
	//
	//}
	//
	//int fs_connection::process_inbound_order_cb()
	//{
	//	CThostFtdcOrderField* pInput = NULL;
	//
	//	int i = 0;
	//	for (; i < 10 && m_orderQueue.m_queue.read_available()>0; ++i)
	//	{
	//
	//		pInput = m_orderQueue.Pop();
	//		if (pInput != NULL)
	//		{
	//			OnRtnOrderAsync(pInput);
	//			delete pInput;
	//		}
	//		i++;
	//	}
	//	return i;
	//
	//}
	//
	//int fs_connection::process_inbound_trade_cb()
	//{
	//	CThostFtdcTradeField* pTrade = NULL;
	//
	//	int i = 0;
	//	for (; i < 10 && m_tradeQueue.m_queue.read_available()>0; ++i)
	//	{
	//
	//		pTrade = m_tradeQueue.Pop();
	//		if (pTrade != NULL)
	//		{
	//
	//			OnRtnTradeAsync(pTrade);
	//			delete pTrade;
	//		}
	//		i++;
	//	}
	//	return i;
	//}


#if 0
	fs_connection::fs_api(fs_connection* pConnection)
	{
		m_pConnection = pConnection;
		m_connectionStatus = false;

		m_isAlive = true;

		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;

	}

	fs_connection::~fs_api()
	{
		//delete m_pUserApi;
	}
#endif

	void fs_connection::init_api()
	{
		m_pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();

		//pListener->AddFDToReadList(m_inputQueue.GetFD(), process_inbound_input_cb, this);
		//pListener->AddFDToReadList(m_orderQueue.GetFD(), process_inbound_order_cb, this);
		//pListener->AddFDToReadList(m_tradeQueue.GetFD(), process_inbound_trade_cb, this);
		//pListener->AddFDToReadList(m_inputActionQueue.GetFD(), process_inbound_input_action_cb, this);

		m_inputQueue.setHandler(boost::bind(&fs_connection::OnRspOrderInsertAsync, this, _1));
		m_orderQueue.setHandler(boost::bind(&fs_connection::OnRtnOrderAsync, this, _1));
		m_tradeQueue.setHandler(boost::bind(&fs_connection::OnRtnTradeAsync, this, _1));
		m_inputActionQueue.setHandler(boost::bind(&fs_connection::OnRspOrderActionAsync, this, _1));
	}

	void fs_connection::release_api()
	{
		// removeFDFromList...

		m_pUserApi->Release();
	}

	//
	// RTThread
	//
	void fs_connection::Process_api()
	{
		//if (m_inputQueue.m_queue.read_available() > 0)
		//{
		//	for (auto &func : m_inputQueue.m_handler)
		//	{
		//		func();
		//	}
		//}

		//if (m_orderQueue.m_queue.read_available() > 0)
		//{
		//	for (auto &func : m_orderQueue.m_handler)
		//	{
		//		func();
		//	}
		//}

		//if (m_tradeQueue.m_queue.read_available() > 0)
		//{
		//	for (auto &func : m_tradeQueue.m_handler)
		//	{
		//		func();
		//	}
		//}

		//if (m_inputActionQueue.m_queue.read_available() > 0)
		//{
		//	for (auto &func : m_inputActionQueue.m_handler)
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
#if 0
	int fs_connection::get_order_id_from_order_sys_id(int orderSysID)
	{
		auto search = m_ordInputActiondRefMap.find(orderSysID);
		if (search != m_ordInputActiondRefMap.end()) {
			int i = search->second;
			loggerv2::info("get_order_id_from_order_sys_id %d->%d", orderSysID, search->second);
			m_ordInputActiondRefMap.erase(search);
			return i;
		}
		else
		{
			return 0;
		}
	}
#endif
	bool fs_connection::connect_api()
	{
		//loggerv2::info("calling fs_connection::connect");
		// For first connection, we are disconnected so we need to connect API first (RequestLogin will be done on API UP).
		// For later connections (disconnect / reconnect), API is already up so we just need to relogin.
		//
		if (m_connectionStatus == false)
		{
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", this->m_sHostname.c_str(), this->m_sService.c_str());

			m_pUserApi->RegisterSpi(this);

			m_pUserApi->RegisterFront(addr);
			loggerv2::info("fs_connection::connect connecting to %s", addr);
			switch (this->getResynchronizationMode())
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

			loggerv2::info("fs_connection::connect initializing api");
			m_pUserApi->Init();
			loggerv2::info("fs_connection::connect api intialized");
		}
		else
		{
			request_login();
		}

		// no need ??
		//m_pUserApi->Join();

		return true;
	}

	bool fs_connection::disconnect_api()
	{
		CThostFtdcUserLogoutField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, this->m_sBrokerId.c_str());
		strcpy(request.UserID, this->m_sUsername.c_str());

		int res = m_pUserApi->ReqUserLogout(&request, ++m_nRequestId);
		if (res != 0)
		{
			return false;
		}
		return true;
	}

	void fs_connection::request_login()
	{
		loggerv2::info("fs_connection::request_login");

		CThostFtdcReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.BrokerID, this->m_sBrokerId.c_str());
		strcpy(request.UserID, this->m_sUsername.c_str());
		strcpy(request.Password, this->m_sPassword.c_str());

		int res = m_pUserApi->ReqUserLogin(&request, ++m_nRequestId);
		if (res != 0)
		{
			this->on_status_changed(AtsType::ConnectionStatus::Disconnected, "fs_api - ReqUserLogin failed");
		}
	}

	void fs_connection::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo != NULL && pRspInfo->ErrorID != 0)
		{
			loggerv2::error("fs_connection::OnRspError - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		}
		else
		{
			loggerv2::info("fs_connection::OnRspError - ok");
		}
	}

	void fs_connection::OnFrontConnected()
	{
		loggerv2::info("fs_api is UP");

		m_connectionStatus = true;

		if (this->getStatus() == AtsType::ConnectionStatus::WaitConnect)
		{
			request_login();
		}
		else
		{
			loggerv2::info("fs_api not asking for reconnect...");
		}
	}

	bool fs_connection::ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition)
	{

		//loggerv2::info("calling fs_connection::ReqQryInvestorPosition");
		int ret = m_pUserApi->ReqQryInvestorPosition(pQryInvestorPosition, ++m_nRequestId);
		loggerv2::info("fs_connection::ReqQryInvestorPosition instr %s , requestId %d", pQryInvestorPosition->InstrumentID, m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}


	bool fs_connection::ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount)
	{
		int ret = m_pUserApi->ReqQryTradingAccount(pQryTradingAccount, ++m_nRequestId);
		loggerv2::info("fs_connection::ReqQryTradingAccount brokerid %s , requestId %d", pQryTradingAccount->BrokerID, m_nRequestId);
		if (ret != 0)
		{
			return false;
		}

		return true;
	}

	void fs_connection::OnFrontDisconnected(int nReason)
	{
		//loggerv2::info("fs_api is DOWN");

		m_connectionStatus = false;

		char* pszMessage;
		switch (nReason)
		{
			// normal disconnection
		case 0:
			pszMessage = "";
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

		this->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage);
	}

	void fs_connection::OnHeartBeatWarning(int nTimeLapse)
	{
		loggerv2::info("fs_api - heartbeat warning");
	}

	void fs_connection::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::info("fs_connection::OnRspUserLogin - (%d, %s)", pRspInfo->ErrorID, pRspInfo->ErrorMsg);

		if (pRspInfo->ErrorID == 0)
		{
			//frontId = pRspUserLogin->FrontID;
			//sessionId = pRspUserLogin->SessionID;
			m_nCurrentOrderRef = atoi(pRspUserLogin->MaxOrderRef);
			//this->on_status_changed(AtsType::ConnectionStatus::WaitConnect, "fs_api - OnRspUserLogin with error id =0, requesting settlement Info Confirm. ");


			//loggerv2::info("fs_connection::OnRspUserLogin - login OK. Requesting SettlementInfoConfirm.");

			CThostFtdcSettlementInfoConfirmField req;
			memset(&req, 0, sizeof(req));
			strcpy(req.BrokerID, this->m_sBrokerId.c_str());
			strcpy(req.InvestorID, this->m_sUsername.c_str());

			int res = m_pUserApi->ReqSettlementInfoConfirm(&req, ++m_nRequestId);
			if (res != 0)
			{
				this->on_status_changed(AtsType::ConnectionStatus::Disconnected, "fs_api - ReqSettlementInfoConfirm failed");
			}
		}
		else
		{
			this->on_status_changed(AtsType::ConnectionStatus::Disconnected, pRspInfo->ErrorMsg);
		}
	}

	void fs_connection::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo->ErrorID == 0)
			this->on_status_changed(AtsType::ConnectionStatus::Disconnected, "fs_connection::OnRspUserLogout Receive Logout Msg Error Id 0");
		else
			loggerv2::error("fs_connection::OnRspUserLogout logout failed ErrId[%d]", pRspInfo->ErrorID);


	}

	void fs_connection::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		if (pRspInfo->ErrorID == 0)
		{
			this->on_status_changed(AtsType::ConnectionStatus::Connected, "fs_connection::Settlement Info Confirmed.");
			this->request_investor_full_positions();
			//this->request_trading_account();
		}

		else
			this->on_status_changed(AtsType::ConnectionStatus::Disconnected, "fs_connection::Settlement Info Confirmation failed.");

	}



	void fs_connection::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{

		//loggerv2::info("calling OnRspQryInvestorPosition");
		if (pInvestorPosition != NULL)
			loggerv2::info("fs_connection::OnRspQryInvestorPosition InstrumentID=%s "
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
			pInvestorPosition->MarginRateByVolume
			);
		/*else
		loggerv2::warn("fs_connection::OnRspQryInvestorPosition receives error, pInvestorPosition is null");*/


		//if (pRspInfo != NULL)
		// loggerv2::info("fs_connection::OnRspQryInvestorPosition receives error %d msg: %s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		//else
		// loggerv2::warn("fs_connection::OnRspQryInvestorPosition receives error, pRspInfo is null.");



		//loggerv2::info("nRequestId is %d , bIsLast %s", nRequestID, bIsLast?"True":"False");


		//date_time t = date_time();
		//t.set_date();
		////i->set_last_sychro_time(t);
		//loggerv2::info("fs_connection::OnRspQryInvestorPosition setting tradeitem last synchro time to %s", t.get_string());

		if (pInvestorPosition->InstrumentID)
		{

			std::string sInstrCode = std::string(pInvestorPosition->InstrumentID) + "@" + this->getName();

			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str()/* pInvestorPosition->InstrumentID*/);
			if (i)
			{
				std::string str = i->getMarket();

				if (pInvestorPosition->PositionDate == '1')
				{
					if (pInvestorPosition->PosiDirection == '2')//long
					{
						i->set_long_used_margin(pInvestorPosition->UseMargin);
						//i->set_tot_long_position(pInvestorPosition->Position);
						if (str != "SHFE")
						{
							loggerv2::info("tradeitem Code=%s  exchange=%s updateYsterday by difference of Positoin and TodayPosition", i->getCode().c_str(), str.c_str());
							i->set_yst_long_position(pInvestorPosition->Position - pInvestorPosition->TodayPosition);
						}
						/*if (pInvestorPosition->Position - pInvestorPosition->TodayPosition != pInvestorPosition->YdPosition)
						i->set_yst_long_position(pInvestorPosition->Position - pInvestorPosition->TodayPosition);
						else if (pInvestorPosition->YdPosition!=0)
						i->set_yst_long_position(pInvestorPosition->YdPosition);*/
						i->set_today_long_position(pInvestorPosition->TodayPosition);
						//i->set_pending_short_close_qty(pInvestorPosition->ShortFrozen);
					}
					else if (pInvestorPosition->PosiDirection == '3')//short
					{
						i->set_short_used_margin(pInvestorPosition->UseMargin);
						//i->set_tot_short_position(pInvestorPosition->Position);
						if (str != "SHFE")
						{
							loggerv2::info("tradeitem Code=%s  exchange=%s updateYsterday by difference of Positoin and TodayPosition", i->getCode().c_str(), str.c_str());
							i->set_yst_short_position(pInvestorPosition->Position - pInvestorPosition->TodayPosition);
						}
						/*if (pInvestorPosition->Position - pInvestorPosition->TodayPosition != pInvestorPosition->YdPosition)
						i->set_yst_short_position(pInvestorPosition->Position - pInvestorPosition->TodayPosition);
						else if (pInvestorPosition->YdPosition != 0)
						i->set_yst_short_position(pInvestorPosition->YdPosition);*/
						i->set_today_short_position(pInvestorPosition->TodayPosition);
						//i->set_pending_long_close_qty(pInvestorPosition->LongFrozen);
					}
				}
				if (pInvestorPosition->PositionDate == '2')
				{
					if (pInvestorPosition->PosiDirection == '2')//long
					{
						i->set_long_used_margin(pInvestorPosition->UseMargin);
						loggerv2::info("tradeitem Code=%s  exchange=%s updateYsterday by Positoin", i->getCode().c_str(), str.c_str());
						i->set_yst_long_position(pInvestorPosition->Position);
						//i->set_pending_short_close_qty(pInvestorPosition->ShortFrozen);
					}
					else if (pInvestorPosition->PosiDirection == '3')//short
					{
						i->set_short_used_margin(pInvestorPosition->UseMargin);
						loggerv2::info("tradeitem Code=%s  exchange=%s updateYsterday by Positoin", i->getCode().c_str(), str.c_str());
						i->set_yst_short_position(pInvestorPosition->Position);
						//i->set_pending_long_close_qty(pInvestorPosition->LongFrozen);
					}
				}
				i->set_tot_long_position(i->get_today_long_position() + i->get_yst_long_position());
				i->set_tot_short_position(i->get_today_short_position() + i->get_yst_short_position());
				/*if (this->m_debug)
				i->dumpinfo();
				*/
				////recompute pending qty
				//int totPendingLong = 0;
				//int totPendingShort = 0;
				//for (auto& it : this->m_activeOrders)
				//{
				// if ( strcmp( i->get_key() ,it.second->get_instrument()->get_key())==0 && it.second->get_open_close() == AtsType::OrderOpenClose::Close)
				// {

				//  if (it.second->get_way() == AtsType::OrderWay::Buy)
				//   totPendingLong += it.second->get_book_quantity();
				//  else if (it.second->get_way() == AtsType::OrderWay::Sell)
				//   totPendingShort += it.second->get_book_quantity();
				// }
				//}

				//i->set_pending_long_close_qty(totPendingLong);
				//i->set_pending_short_close_qty(totPendingShort);



				/*terra::common::date_time t = terra::common::date_time(time(NULL));*/
				//t.set_date();
				auto tp = get_lwtp_now();
				i->set_last_sychro_timepoint(tp);
				//loggerv2::info("fs_connection::OnRspQryInvestorPosition setting tradeitem last synchro time to %s", t.get_string(date_time::date_format::ISO));

				//if (this->m_debug)
				//{
				// loggerv2::info("fs_connection::OnRspQryInvestorPosition pending long close is %d, pending short close is %d", totPendingLong,totPendingShort);
				//}

			}
			else
			{
				loggerv2::info("fs_connection::OnRspQryInvestorPosition cannot find tradeitem %s by second key", std::string(pInvestorPosition->InstrumentID).c_str());
			}
		}

		else

		{
			loggerv2::warn("fs_connection::OnRspQryInvestorPosition could not get tradeitem %s.", pInvestorPosition->InstrumentID);
		}

	}


	void fs_connection::OnRspOrderInsert(CThostFtdcInputOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		//CThostFtdcInputOrderField* i = new CThostFtdcInputOrderField;
		//memcpy_lw(i, pOrder, sizeof(CThostFtdcInputOrderField));

		//// use unused field to store errorId
		//i->IsAutoSuspend = pRspInfo->ErrorID;

		//m_inputQueue.Push(i);
		pOrder->IsAutoSuspend = pRspInfo->ErrorID;
		m_inputQueue.CopyPush(pOrder);
	}


	void fs_connection::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
	{
		loggerv2::info("fs_connection::OnErrRtnOrderInsert --> need to implement");




	}

	void fs_connection::OnErrRtnOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo)
	{
		loggerv2::info("fs_connection::OnErrRtnOrderAction --> need to implement");
		loggerv2::info("fs_connection::OnErrRtnOrderAction orderref %s , userid %s", pInputOrderAction->OrderRef, pInputOrderAction->UserID);

	}


	void fs_connection::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	{
		//loggerv2::info("calling fs_connection::OnRspOrderAction . request id=[%d] --> need to implement",nRequestID);

		if (!bIsLast)
		{
			printf("fs_connection::OnRspOrderAction bIsLast:%d\n", bIsLast);
			return;
		}


		loggerv2::error("fs_connection::OnRspOrderAction pInputOrderAction - "
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

		loggerv2::error("fs_connection::OnRspOrderActionpRspInfo - "
			"ErrorID[%d]"
			"ErrorMsg[%s]",
			pRspInfo->ErrorID,
			pRspInfo->ErrorMsg
			);



#if 0
		int orderID = get_order_id_from_order_sys_id(atoi(pInputOrderAction->OrderSysID));
		if (!orderID)
		{
			loggerv2::error("fs_connection::OnRspOrderAction:could not find associated order id for OrderSysID %d", atoi(pInputOrderAction->OrderSysID));
			return;
		}
#else
		int orderID = this->get_order_id(pInputOrderAction->OrderRef);
		if (!orderID)
		{
			loggerv2::error("fs_connection::OnRspOrderAction:could not find associated order id for OrderRef: %d", atoi(pInputOrderAction->OrderRef));
			return;
		}
#endif
		int* i = new int(orderID);
		m_inputActionQueue.Push(i);


	}

	void fs_connection::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::info("fs_connection::OnRspQryTradingAccount"

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


		//tradingaccount* ta = new tradingaccount(lexical_cast<double>(pTradingAccount->PreMortgage),
		// lexical_cast<double>(pTradingAccount->PreCredit),
		// lexical_cast<double>(pTradingAccount->PreDeposit),
		// lexical_cast<double>(pTradingAccount->PreBalance),
		// lexical_cast<double>(pTradingAccount->PreMargin),
		// lexical_cast<double>(pTradingAccount->Interest),
		// lexical_cast<double>(pTradingAccount->Deposit),
		// lexical_cast<double>(pTradingAccount->Withdraw),
		// lexical_cast<double>(pTradingAccount->FrozenMargin),
		// lexical_cast<double>(pTradingAccount->FrozenCash),
		// lexical_cast<double>(pTradingAccount->FrozenCommission),
		// lexical_cast<double>(pTradingAccount->CurrMargin),
		// lexical_cast<double>(pTradingAccount->CashIn),
		// lexical_cast<double>(pTradingAccount->Commission),
		// lexical_cast<double>(pTradingAccount->Balance),
		// lexical_cast<double>(pTradingAccount->Available),
		// lexical_cast<double>(pTradingAccount->WithdrawQuota),
		// lexical_cast<double>(pTradingAccount->Reserve),
		// lexical_cast<double>(pTradingAccount->Credit),
		// lexical_cast<double>(pTradingAccount->Mortgage),
		// lexical_cast<double>(pTradingAccount->ExchangeMargin));

		tradingaccount* ta = new tradingaccount(
			this->getName(),
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



		this->on_trading_account_cb(ta);

		delete ta;

	}



#if 0
	void fs_connection::OnRspOrderActionAsync(int orderID)
	{
		//loggerv2::info("calling fs_connection::OnRspOrderActionAsync");
		this->OnRspOrderActionAsync(orderID);
	}
#endif

	void fs_connection::OnRtnOrder(CThostFtdcOrderField* pOrder)
	{
		//CThostFtdcOrderField* o = new CThostFtdcOrderField;
		//memcpy_lw(o, pOrder, sizeof(CThostFtdcOrderField));

		//m_orderQueue.Push(o);
		m_orderQueue.CopyPush(pOrder);
	}

	void fs_connection::OnRtnTrade(CThostFtdcTradeField* pTrade)
	{
		//CThostFtdcTradeField* t = new CThostFtdcTradeField;
		//memcpy_lw(t, pTrade, sizeof(CThostFtdcTradeField));

		//m_tradeQueue.Push(t);
		m_tradeQueue.CopyPush(pTrade);
	}

#if 0
	//void fs_connection::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
	//{
	// loggerv2::info("calling fs_connection::OnRspOrderAction --> need to implement");
	//}


	//
	// callbacks
	//
	void fs_connection::OnRspOrderInsertAsync(CThostFtdcInputOrderField* pInput)
	{
		this->OnRspOrderInsertAsync(pInput, pInput->IsAutoSuspend);
	}

	void fs_connection::OnRtnOrderAsync(CThostFtdcOrderField* pOrder)
	{
		this->OnRtnOrderAsync(pOrder);
	}

	void fs_connection::OnRtnTradeAsync(CThostFtdcTradeField* pTrade)
	{
		this->OnRtnTradeAsync(pTrade);
	}
#endif

	//
	// prder sending
	//
	bool fs_connection::ReqOrderInsert(CThostFtdcInputOrderField* pRequest)
	{
		//o->m_ordRef = ++m_nCurrentOrderRef;
		sprintf(pRequest->OrderRef, "%012d", ++m_nCurrentOrderRef);

		printf("fs_connection::ReqOrderInsert pRequest->OrderRef:%s\n", pRequest->OrderRef);

		int ret = m_pUserApi->ReqOrderInsert(pRequest, ++m_nRequestId);
		if (ret != 0)
		{
			return false;
		}
		return true;
	}

	bool fs_connection::ReqOrderAction(CThostFtdcInputOrderActionField* pRequest)
	{

		int ret = m_pUserApi->ReqOrderAction(pRequest, ++m_nRequestId);
		//loggerv2::info("m_nRequestId %d", m_nRequestId);

		if (ret != 0)
		{
			printf("error:fs_connection::ReqOrderAction ret:%d\n", ret);
			return false;
		}
#if 0
#if 0
		int nOrderId = get_order_id(pRequest->UserID);
#else
		int nOrderId = get_order_id(pRequest->OrderRef);
#endif
		m_ordInputActiondRefMap.insert(std::pair<int, int>(atoi(pRequest->OrderSysID), nOrderId));
		loggerv2::error("fs_connection::ReqOrderAction insert <%d,%d> to m_ordInputActiondRefMap,orderRef:%s", atoi(pRequest->OrderSysID), nOrderId, pRequest->OrderRef);
#endif
		return true;
	}

#if 0
	int fs_connection::get_order_id(const char* psz)
	{
		if (m_pConnection != nullptr)
		{
			return this->get_order_id(psz);
		}
		return 0;
	}
#endif

	void fs_connection::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{
		loggerv2::info("OnRspQryInstrument");
		if (pInstrument == nullptr)
		{
			loggerv2::error("OnRspQryInstrument error");
		}
		else
		{
			FILE *fp = fopen("test.txt", "wb+");
			/*loggerv2::info("fs_connection::OnRspQryInstrument InstrumentID[%s] ExchangeID[%s] InstrumentName[%s] ExchangeInstID[%s] ProductID[%s] ProductClass[%s]"
			"DeliveryYear[%d]"
			"DeliveryMonth[%d]"
			"MaxMarketOrderVolume[%d]"
			"MinMarketOrderVolume[%d]"
			"MaxLimitOrderVolume[%d]"
			"MinLimitOrderVolume[%d]"
			"VolumeMultiple[%d]"
			"PriceTick[%lf]"
			"CreateDate[%s]"
			"OpenDate[%f]"
			"ExpireDate[%s]"
			"StartDelivDate[%s]"
			"EndDelivDate[%s]"
			"InstLifePhase[%s]"
			"IsTrading[%d]"
			"PositionType[%s]"
			"PositionDateType[%s]"
			"LongMarginRatio[%lf]"
			"ShortMarginRatio[%lf]"
			"MaxMarginSideAlgorithm[%s]",
			pInstrument->InstrumentID,
			pInstrument->ExchangeID,
			pInstrument->InstrumentName,
			pInstrument->ExchangeInstID,
			pInstrument->ProductID,
			pInstrument->ProductClass
			/*pInstrument->DeliveryYear,
			pInstrument->DeliveryMonth,
			pInstrument->MaxMarketOrderVolume,
			pInstrument->MinMarketOrderVolume,
			pInstrument->MaxLimitOrderVolume,
			pInstrument->MinLimitOrderVolume,
			pInstrument->VolumeMultiple,
			pInstrument->PriceTick,
			pInstrument->CreateDate,
			pInstrument->OpenDate,
			pInstrument->ExpireDate,
			pInstrument->StartDelivDate,
			pInstrument->EndDelivDate,
			pInstrument->InstLifePhase,
			pInstrument->IsTrading,
			pInstrument->PositionType,
			pInstrument->PositionDateType,
			pInstrument->LongMarginRatio,
			pInstrument->ShortMarginRatio,
			pInstrument->MaxMarginSideAlgorithm
			);*/

			std::string sInstr = std::string(pInstrument->ExchangeInstID);
#if 0
			trim(sInstr);
#endif
			std::string sSearch = "select * from Futures where Code like '" + std::string(pInstrument->InstrumentID) + "%'";
			const char* data = "Callback function called";
			char *zErrMsg = 0;

			std::string sUnderlying = "A"; //
			std::string sCP = "C";  //"CallPut"
			std::string sInstClass = "D";

			std::string sCmd = "";

			std::vector<boost::property_tree::ptree>* pTree = this->m_database->get_table(sSearch.c_str());



			// if (pTree->size() == 0) //tradeitem doesn't exist
			// {
			//  //loggerv2::info("Could not find tradeitem %s", std::string(pInstrument->InstrumentID).c_str());
			//  sqlite3_free(zErrMsg);

			//  sCmd = "INSERT INTO Futures VALUES (";
			//  sCmd += "'" + sInstr + "',";
			//  sCmd += "'" + std::string(pInstrument->MarketID) + "',";
			//  sCmd += "'" + sInstr + "',";
			//  sCmd += "' ',";
			//  sCmd += "'" + std::string(pInstrument->InstrumentID) + "@LTSUDP|" + std::string(pInstrument->InstrumentID) + "." + std::string(pInstrument->MarketID) + "@LTS|" + std::string(pInstrument->InstrumentID) + ".SH" + "@TDF|SH" + std::string(pInstrument->InstrumentID) + "@XS',";
			//  sCmd += "'" + std::string(pInstrument->InstrumentID) + "." + std::string(pInstrument->MarketID) + "@LTS|" + std::string(pInstrument->InstrumentID) + "." + std::string(pInstrument->MarketID) + "@XS" + "',";
			//  sCmd += "'" + sUnderlying + "',";
			//  sCmd += "'" + sMat + "',";
			//  sCmd += "'" + std::to_string(pInstrument->ExecPrice) + "',";
			//  sCmd += "'" + std::to_string(pInstrument->VolumeMultiple) + "',";
			//  sCmd += "'" + sCP + "',";
			//  sCmd += "'" + sInstClass + "')";

			//  int rc = this->m_database->executeNonQuery(sCmd.c_str());

			//  if (rc == 0)
			//  {
			//   //loggerv2::info("failed to insert into database, ret is %d",rc);
			//   sqlite3_free(zErrMsg);
			//  }

			// }

			// else //exists
			// {
			//  //loggerv2::info("tradeitem %s exist already in the database", std::string(pInstrument->InstrumentID).c_str());
			//  std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@LTS";
			//  sCmd = "UPDATE Options SET ";
			//  sCmd += "Code = '" + sInstr + "',";
			//  sCmd += "ISIN = '" + sInstr + "',";
			//  sCmd += "Maturity = '" + sMat + "',";
			//  sCmd += "Strike = '" + std::to_string(pInstrument->ExecPrice) + "',";
			//  sCmd += "PointValue ='" + std::to_string(pInstrument->VolumeMultiple) + "'";
			//  sCmd += " where Code='" + sConnectionCodes + "';";

			//  int rc = this->m_database->executeNonQuery(sCmd.c_str());

			//  if (rc == 0)
			//  {
			//   //loggerv2::info("failed to update the database,error is %d",rc);
			//   sqlite3_free(zErrMsg);
			//  }
			// }
			//}
			//if (bIsLast)
			//{
			// this->m_database->close_databse();
			// this->m_bIsDicoRdy = true;

		}

	}

	void fs_connection::request_instruments()
	{
		fstech::CThostFtdcQryInstrumentField request;
		memset(&request, 0, sizeof(request));
		m_pUserApi->ReqQryInstrument(&request, ++m_nRequestId);
	}
	//end add on 20160929
	/*void fs_connection::cancel_num_warning(tradeitem* i)
	{

	//}

	//void fs_connection::cancel_num_ban(tradeitem* i)
	//{

	}*/

	}

