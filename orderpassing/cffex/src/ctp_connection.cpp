#include "ctp_connection.h"
#include "ThostFtdcUserApiStruct.h"
#include <string>


using namespace terra::common;
namespace cffex
{

	cffex_connection::cffex_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "cffex_connection";


		m_pCffexApi = new cffex_api(this);


	}

	cffex_connection::~cffex_connection()
	{
		delete m_pCffexApi;
	}


	bool cffex_connection::init_config(const string &name, const std::string &strConfigFile)
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

	void cffex_connection::request_instruments()
	{
		m_pCffexApi->request_instruments();
	}



	void cffex_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{

		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pCffexApi->ReqQryInvestorPosition(&pRequest);

		if (m_debug)
			loggerv2::info("cffex_connection::request_investor_position requesting investor position for tradeitem %s", i->getCode().c_str());

	}



	void cffex_connection::request_investor_full_positions()
	{

		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());

		m_pCffexApi->ReqQryInvestorPosition(&pRequest);

		if (m_debug)
			loggerv2::info("cffex_connection:: calling OnRspQryInvestorPosition ");

	}
	//
	void cffex_connection::req_RiskDegree()
	{
		//request_trading_account();
	}
	//
	void cffex_connection::request_trading_account()
	{
		if (m_debug)
			loggerv2::info("cffex_connection:: calling ReqQryTradingAccount ");

		CThostFtdcQryTradingAccountField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		//strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pCffexApi->ReqQryTradingAccount(&pRequest);


	}

	void cffex_connection::init_connection()
	{
		loggerv2::info("cffex_connection::init_connection create trader api..");
		m_pCffexApi->init();

#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
		//std::thread th(boost::bind(&cffex_connection::set_kernel_timer_thread, this));
		//m_thread.swap(th);
		m_bKey_with_exchange = false;

	}

#ifdef Linux
	void  cffex_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&cffex_connection::process, this));

		m_pCffexApi->get_input_queue() ->set_fd(efd);
		m_pCffexApi->get_input_quote_queue()->set_fd(efd);

		m_pCffexApi->get_order_queue()->set_fd(efd);
		m_pCffexApi->get_order_quote_queue()->set_fd(efd);

		m_pCffexApi->get_trade_queue()->set_fd(efd);
		m_pCffexApi->get_input_action_queue()->set_fd(efd);
		m_pCffexApi->get_input_action_quote_queue()->set_fd(efd);

		m_outboundQueue.set_fd(efd);
		m_outquoteboundQueue.set_fd(efd);
	}
#endif

	void cffex_connection::release()
	{

		//is_alive(false);
		//m_thread.join();
		ctpbase_connection::release();

		m_pCffexApi->release();

	}

	void cffex_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("cffex_connection::connect connecting to cffex...");

			on_status_changed(AtsType::ConnectionStatus::WaitConnect);

			m_pCffexApi->connect();
		}
	}

	void cffex_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			if (m_pCffexApi->disconnect() == false)
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "cffex_connection - ReqUserLogout failed");
			}
		}
	}




	void cffex_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_outquoteboundQueue.Pops_Handle_Keep(10);
		m_pCffexApi->Process();
	}
	int cffex_connection::market_create_order_async(order* o, char* pszReason)
	{
		//#endif

		//cffex_order* o = dynamic_cast<cffex_order*>(ord);
		//if (o == NULL)
		//{
		//	snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to cffex_order*...\n");
		//	ord->set_status(AtsType::OrderStatus::Reject);
		//	return 0;
		//}
		if (o->get_way() != AtsType::OrderWay::Exercise)
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
			else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			{
				request->TimeCondition   = THOST_FTDC_TC_IOC;
				request->VolumeCondition = THOST_FTDC_VC_AV;
			}
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
				loggerv2::info("cffex_connection::market_create_order CombOffsetFlag is %c", oc);



			if (!compute_userId(o, request->UserID, sizeof(request->UserID)))
			{
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
			request->VolumeCondition = THOST_FTDC_VC_AV;
			if (o->get_restriction() == AtsType::OrderRestriction::None)
			{
				request->TimeCondition = THOST_FTDC_TC_GFD; // or GFS ???
				strcpy(request->GTDDate, "");
			}
			else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
			{
				request->TimeCondition   = THOST_FTDC_TC_IOC;
				request->VolumeCondition = THOST_FTDC_VC_CV;
			}
			else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			{
				request->TimeCondition   = THOST_FTDC_TC_IOC;
				request->VolumeCondition = THOST_FTDC_VC_AV;
			}
			else
			{
				snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
				xs_create_pool.free_mem(request);
				return 0;
			}


			//request->VolumeCondition = THOST_FTDC_VC_AV;
			request->ContingentCondition = THOST_FTDC_CC_Immediately;
			request->ForceCloseReason = THOST_FTDC_FCC_NotForceClose;

			request->MinVolume = 1;
			request->IsAutoSuspend = 0;
			request->UserForceClose = 0;


			if (!m_pCffexApi->ReqOrderInsert(request))
			{


				snprintf(pszReason, REASON_MAXLENGTH, "cffex api reject!\n");
				xs_create_pool.free_mem(request);
				return 0;
			}
			xs_create_pool.free_mem(request);
		}
		else
		{
			CThostFtdcInputExecOrderField exc;
			memset(&exc, 0, sizeof(exc));
			strcpy(exc.BrokerID, m_sBrokerId.c_str());
			strcpy(exc.InvestorID, m_sUsername.c_str());
			strcpy(exc.InstrumentID, o->get_instrument()->get_trading_code());

			//use _CloseToday/_CloseYesterday for SHFE exc.OffsetFlag = THOST_FTDC_OF_Close;  exc.HedgeFlag = THOST_FTDC_HF_Speculation; 
			// to exercise or to abandon exc.ActionType = THOST_FTDC_ACTP_Exec; 
			// long or short position to hold after exercising exc.PositionDirection = THOST_FTDC_PD_Long; 
			//use _UnReserve for CFFEX, use _Reserve for DCE/CZCE, both are available for SHFE exc.ReservePositionFlag = THOST_FTDC_EOPF_Unreserce; 
			//use _AutoClose for CFFEX, use _NotToClose for DCE/CZCE, both are available for SHFE exc.CloseFlag = THOST_FTDC_EOCF_AutoClose; 
			if (!compute_userId(o, exc.UserID, sizeof(exc.UserID)))
			{
				return 0;
			}
			TThostFtdcOffsetFlagType oc = THOST_FTDC_OF_Close;
			if (o->get_open_close() == OrderOpenClose::Undef)
			{
				o->set_open_close(compute_open_close(o, m_bCloseToday));
			}
			switch (o->get_open_close())
			{
			case AtsType::OrderOpenClose::Open:
			case AtsType::OrderOpenClose::Close:
				oc = THOST_FTDC_OF_Close;
				break;
			case AtsType::OrderOpenClose::CloseToday:
				oc = THOST_FTDC_OF_CloseToday;
				break;

			default:

				break;
			}
			exc.OffsetFlag = oc;
			exc.HedgeFlag = THOST_FTDC_HF_Speculation;
			exc.ActionType = THOST_FTDC_ACTP_Exec;
			if (o->get_instrument()->getMarket() == "DCE" || o->get_instrument()->getMarket() == "CZCE")
			{
				exc.ReservePositionFlag = THOST_FTDC_EOPF_Reserve;

			}
			else
			{
				exc.ReservePositionFlag = THOST_FTDC_EOPF_UnReserve;

			}
			exc.PosiDirection = THOST_FTDC_PD_Long;
			exc.CloseFlag = THOST_FTDC_EOCF_NotToClose;
			exc.Volume = o->get_quantity();
			m_pCffexApi->ReqExecOrderInsert(&exc);
		}
		return 1;
	}



	int cffex_connection::market_cancel_order_async(order* o, char* pszReason)
	{

		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());

		//cffex_order* o = dynamic_cast<cffex_order*>(ord);
		//if (o == NULL)
		//{
		//	snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to cffex_order*...\n");
		//	o->set_status(AtsType::OrderStatus::Nack);
		//	o->rollback();
		//	return 0;
		//}

		if (o->get_way() != AtsType::OrderWay::Exercise)
		{
			CThostFtdcInputOrderActionField *request = xs_cancel_pool.get_mem();
			memset(request, 0, sizeof(CThostFtdcInputOrderActionField));

			strcpy(request->BrokerID, m_sBrokerId.c_str());
			strcpy(request->InvestorID, m_sUsername.c_str());

			strcpy(request->ExchangeID, o->get_exchange_id().c_str());
			if (o->custome_strings[0].size() == 0)
			{
				xs_cancel_pool.free_mem(request);
				return -1;
			}

			strcpy(request->OrderSysID, cffex_order_aux::get_order_sys_id(o).c_str());

			if (!compute_userId(o, request->UserID, sizeof(request->UserID)))
			{
				xs_cancel_pool.free_mem(request);
				return -1;
			}


			loggerv2::info("reqest.UserId %s", request->UserID);

			request->ActionFlag = THOST_FTDC_AF_Delete;

			if (!m_pCffexApi->ReqOrderAction(request))
			{
				xs_cancel_pool.free_mem(request);
				return 0;
			}

			//
			xs_cancel_pool.free_mem(request);
			return 1;
		}
		else
		{
			CThostFtdcInputExecOrderActionField request;
			memset(&request, 0, sizeof(request));

			strcpy(request.BrokerID, m_sBrokerId.c_str());
			strcpy(request.InvestorID, m_sUsername.c_str());

			strcpy(request.ExchangeID, o->get_exchange_id().c_str());
			strcpy(request.ExecOrderSysID, cffex_order_aux::get_order_sys_id(o).c_str());

			if (!compute_userId(o, request.UserID, sizeof(request.UserID)))
			{
				return -1;
			}


			loggerv2::info("reqest.UserId %s", request.UserID);

			request.ActionFlag = THOST_FTDC_AF_Delete;

			if (!m_pCffexApi->ReqExecOrderAction(&request))
			{

				return 0;
			}

			return 1;



		}
	}

	int cffex_connection::market_create_quote_async(quote* q, char* pszReason)
	{
		CThostFtdcInputQuoteField *request = quote_create_pool.get_mem();
		memset(request, 0, sizeof(CThostFtdcInputQuoteField));

		strcpy(request->BrokerID, m_sBrokerId.c_str());
		strcpy(request->InvestorID, m_sUsername.c_str());

		strcpy(request->InstrumentID, q->get_instrument()->get_trading_code());

		request->AskPrice = q->get_ask_order()->get_price();
		request->BidPrice = q->get_bid_order()->get_price();

		request->AskVolume = q->get_ask_order()->get_quantity();
		request->BidVolume = q->get_bid_order()->get_quantity();


		if (q->get_bid_order()->get_open_close() == OrderOpenClose::Undef)
		{
			q->get_bid_order()->set_open_close(compute_open_close(q->get_bid_order(), m_bCloseToday));
		}

		switch (q->get_bid_order()->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			request->BidOffsetFlag = THOST_FTDC_OF_Open;
			break;

		case AtsType::OrderOpenClose::Close:
			request->BidOffsetFlag = THOST_FTDC_OF_Close;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->BidOffsetFlag = THOST_FTDC_OF_CloseToday;
			break;

		default:

			break;
		}

		if (q->get_ask_order()->get_open_close() == OrderOpenClose::Undef)
		{
			q->get_ask_order()->set_open_close(compute_open_close(q->get_ask_order(), m_bCloseToday));
		}

		switch (q->get_ask_order()->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			request->AskOffsetFlag = THOST_FTDC_OF_Open;
			break;

		case AtsType::OrderOpenClose::Close:
			request->AskOffsetFlag = THOST_FTDC_OF_Close;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->AskOffsetFlag = THOST_FTDC_OF_CloseToday;
			break;

		default:

			break;
		}

		request->AskHedgeFlag = THOST_FTDC_HF_Speculation;
		request->BidHedgeFlag = THOST_FTDC_HF_Speculation;

		if (!compute_userId(q, request->UserID, sizeof(request->UserID)))
		{
			quote_create_pool.free_mem(request);
			return 0;
		}
		strcpy(request->ForQuoteSysID, q->get_FQR_ID().c_str()); 

		if (!m_pCffexApi->ReqQuoteInsert(request))
		{


			snprintf(pszReason, REASON_MAXLENGTH, "cffex api reject!\n");
			quote_create_pool.free_mem(request);
			return 0;
		}
		quote_create_pool.free_mem(request);

		return 1;
	}

	int cffex_connection::market_cancel_quote_async(quote* q, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_quote_async : %d", q->get_id());


		CThostFtdcInputQuoteActionField *request = quote_cancel_pool.get_mem();
		memset(request, 0, sizeof(CThostFtdcInputQuoteActionField));

		strcpy(request->BrokerID, m_sBrokerId.c_str());
		strcpy(request->InvestorID, m_sUsername.c_str());

		strcpy(request->ExchangeID, q->get_exchange_id().c_str());
		strcpy(request->QuoteSysID, cffex_order_aux::get_quote_sys_id(q).c_str());

		if (!compute_userId(q, request->UserID, sizeof(request->UserID)))
		{
			quote_cancel_pool.free_mem(request);
			return -1;
		}


		loggerv2::info("reqest.UserId %s", request->UserID);

		request->ActionFlag = THOST_FTDC_AF_Delete;

		if (!m_pCffexApi->ReqQuoteAction(request))
		{
			quote_cancel_pool.free_mem(request);
			return 0;
		}

		//
		quote_cancel_pool.free_mem(request);
		return 1;
	}




//
// cffex callbacks
//
void cffex_connection::OnRspOrderInsertAsync(CThostFtdcInputOrderField* pOrder)
{
	//
	// used only for rejects.
	//
	int errorId = pOrder->IsAutoSuspend;

	//cffex_order* o = NULL;

	// 0 - log
	if (m_debug)
		loggerv2::info("cffex_connection::OnRspOrderInsert - orderRef[%s] userId[%s] RequestID[%d] errorId[%d]", pOrder->OrderRef, pOrder->UserID, pOrder->RequestID, errorId);

	// 1 - retrieve order
	OrderWay::type way = pOrder->Direction == THOST_FTDC_D_Buy ? OrderWay::Buy : OrderWay::Sell;

	int orderId = get_order_id(pOrder->UserID, way);
	
	loggerv2::info("cffex_connection::OnRspOrderInsertAsync,orderID:%d", orderId);
	if (orderId == -1)
	{
		loggerv2::warn("cffex_connection::OnRspOrderInsert - cannot extract orderId from UserId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
		//return;
		//o->set_portfolio("UNKNOWN");
		orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
	}

	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRspOrderInsert - message received on dead order[%d]...", orderId);
		break;

	case 2:

		o = cffex_order_aux::anchor(this, pOrder);
		if (o == NULL)
		{
			loggerv2::error("cffex_connection::OnSOPEntrustOrderRtnAsyn cannot anchor order");
			return;
		}

		add_pending_order(o);
		break;
	default:
		break;

	}


	if (o == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRspOrderInsert - order recovered NULL");
		return;
	}


	// 2 - treat message
	if (errorId != 0)
	{
		char szErrorMsg[32 + 1];
		snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", errorId);

		on_nack_from_market_cb(o, szErrorMsg);
		//bug fix. should not call update_instr_on_nack_from_market_cb, because we haven't ack the order before!
		//
		//update_instr_on_nack_from_market_cb(o);
	}
	else
	{
		loggerv2::error("OnRspOrderInsert - order[%d] errorId[0] ???", orderId);
	}
}



void cffex_connection::OnRspOrderActionAsync(int* nOrdId)
{
	if (m_debug)
		loggerv2::info("cffex_connection::OnRspOrderActionAsync");

	//cffex_order* o = NULL;

	int ret;
	order *o = get_order_from_map(*nOrdId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRspOrderActionAsync - message received on dead order[%d]...", *nOrdId);
		break;

	case 2:
	default:
		break;
	}

	if (o == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRspOrderActionAsync - order recovered NULL");
		return;
	}

	on_nack_from_market_cb(o, NULL);
	//update_instr_on_nack_from_market_cb(o);
}



void cffex_connection::OnRspQuoteActionAsync(int* nOrdId)
{
	if (m_debug)
		loggerv2::info("cffex_connection::OnRspQuoteActionAsync");

	//cffex_order* o = NULL;

	int ret;
	quote *q = get_quote_from_map(*nOrdId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRspQuoteActionAsync - message received on dead quote[%d]...", *nOrdId);
		break;

	case 2:
	default:
		break;
	}

	if (q == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRspQuoteActionAsync - quote recovered NULL");
		return;
	}

	on_nack_quote_from_market_cb(q, NULL);
	//update_instr_on_nack_from_market_cb(o);
}


void cffex_connection::OnRtnOrderAsync(CThostFtdcOrderField* pOrder)
{

	// 0 - log
	if (m_debug)
		loggerv2::info("cffex_connection::OnRtnOrderAsync - "
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

	//cffex_order* o = NULL;

	// 1 - retrieve order
	OrderWay::type way = pOrder->Direction == THOST_FTDC_D_Buy ? OrderWay::Buy : OrderWay::Sell;

	
	int account, bidId, askId, portfolioId, ntradingType;
	get_user_info(pOrder->UserID, account, bidId, askId, portfolioId, ntradingType);

	int orderId = (way==OrderWay::Buy && bidId > 0) ? bidId : askId;

	
	if (orderId == -1)
	{
		loggerv2::warn("cffex_connection::OnRtnOrderAsync - cannot extract orderId from USerId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
		//return;
		orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
	}
	loggerv2::info("cffex_connection::OnRtnOrderAsync,orderID:%d", orderId);

	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
		break;

	case 2:

		o = cffex_order_aux::anchor(this, pOrder);
		if (o == NULL)
		{
			loggerv2::error("cffex_connection::OnSOPEntrustOrderRtnAsyn cannot anchor order");
			return;
		}

		add_pending_order(o);
		break;
	default:
		break;
	}


	if (o == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRtnOrderAsync - order recovered NULL");
		return;
	}


	cffex_order_aux::set_order_sys_id(o, pOrder->OrderSysID);
	if (o->get_quantity() != pOrder->VolumeTotalOriginal)
	{
		if (m_debug)
			loggerv2::debug("cffex_connection::OnRtnOrderAsync resetting order quantity to %d", pOrder->VolumeTotalOriginal);
		o->set_quantity(pOrder->VolumeTotalOriginal);
	}

	if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
	{
		if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
		{
			if (m_debug)
				loggerv2::debug("cffex_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
			o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
		}
	}

	// cancel 1: 3-3
	// cancel 2: 3-5 : sometimes 3-5 is not coming

	if (bidId > 0 && o->get_binding_quote()==nullptr)
	{
		int ret1;
		quote* q = get_quote_from_map(bidId, ret1);
		switch (ret1)
		{
		case 0:
			if (way == OrderWay::Buy)
			{
				q->set_bid_order(o);
			}
			if (way == OrderWay::Sell)
			{
				q->set_ask_order(o);
			}
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			if (way == OrderWay::Buy)
			{
				q->set_bid_order(o);
			}
			if (way == OrderWay::Sell)
			{
				q->set_ask_order(o);
			}
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("cffex_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;

		case 2:
			{
			q = get_quote_from_pool();
			if (way==OrderWay::Buy)
			{
				q->set_bid_order(o);
			}
			if (way == OrderWay::Sell)
			{
				q->set_ask_order(o);
			}
			q->set_instrument(o->get_instrument());
			q->set_id(bidId);
			//to do ...			
			auto ltime = o->get_last_time();
			q->set_last_time(ltime);
			
			q->set_portfolio(getPortfolioName(portfolioId).c_str());
			q->set_trading_type(ntradingType);
			add_pending_quote(q);
			break;
			}
		default:
			break;
		}
		
		//o->set_binding_quote(q);
		
		////to do ... update the order book
		//if (ret == 2 && ret1 != 2)
		//{
		//	if (way == OrderWay::Buy)
		//	{
		//		q->set_bid_order(o);
		//		if (q->get_bid_order() != nullptr)
		//		{
		//			q->get_instrument()->get_order_book()->add_order(abs(q->get_bid_order()->get_id()), q->get_bid_order()->get_price(), /*o->get_book_quantity(), */q->get_bid_order()->get_way());
		//		}
		//	}
		//	if (way == OrderWay::Sell)
		//	{
		//		q->set_ask_order(o);
		//		if (q->get_ask_order() != nullptr)
		//		{
		//			q->get_instrument()->get_order_book()->add_order(abs(q->get_ask_order()->get_id()), q->get_ask_order()->get_price(), /*o->get_book_quantity(), */q->get_ask_order()->get_way());
		//		}
		//	}
		//}
	}
	switch (pOrder->OrderSubmitStatus)
	{
	case THOST_FTDC_OSS_InsertSubmitted:
		//if (o->get_last_action() != AtsType::OrderAction::Created)
		//{
		//   o->set_last_action(AtsType::OrderAction::Created);
		//}
	{
		//special case, we could receive status 0-5 for cancellation from another application.
		if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
		{
			o->set_last_action(AtsType::OrderAction::Cancelled);

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
				loggerv2::info("cffex_connection::OnRtnOrderAsync receive order ack while order status is exec, this is possibly during the resynchro process.");
				//we recvieve the exec earlier than ack , so order is already created.

				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}

		}

	}



	break;

	case THOST_FTDC_OSS_CancelSubmitted:
	{
		if (o->get_last_action() != AtsType::OrderAction::Cancelled)
		{
			o->set_last_action(AtsType::OrderAction::Cancelled);
		}
		if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
		{
			//on_ack_from_market_cb(o);
			on_cancel_from_market_cb(o);
			//update_instr_on_ack_from_market_cb(o);
			//don't need to update because order will be updated on the second cancel confirmation.
		}
	}
	break;

	case THOST_FTDC_OSS_ModifySubmitted:
	{
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
				loggerv2::info("Calling on_ack_from_market_cb.. this is possibly the resynchro process..");

				/*on_ack_from_market_cb(o);
				*///update_instr_on_ack_from_market_cb(o);

				on_cancel_from_market_cb(o);
				update_instr_on_cancel_from_market_cb(o);
				//on_cancel_from_market_cb(o);

				//m_statistics.incr_can();
				loggerv2::info("cffex_connection::OnRtnOrderAsync Current cancel number is %d", m_statistics.get_can());
			}

			//earse the pointer from m_cancelOrdMap;
			//std::map<int, cffex_order*>::iterator it = m_cancelOrdMap.find(atoi(pOrder->OrderRef));
			//if (it != m_cancelOrdMap.end())
			//	m_cancelOrdMap.erase(it);
			//update_instr_on_ack_from_market_cb(o);
		}
	}
	break;

	case THOST_FTDC_OSS_InsertRejected:

		update_instr_on_nack_from_market_cb(o);
		on_nack_from_market_cb(o, pOrder->StatusMsg);
		break;

	case THOST_FTDC_OSS_CancelRejected:

	{
		on_nack_from_market_cb(o, pOrder->StatusMsg);
		if (o->get_status() == AtsType::OrderStatus::Nack) //this is nack of a cancel and we rolled back to order with book qty!=0
			update_instr_on_nack_from_market_cb(o);
	}
	break;

	case THOST_FTDC_OSS_ModifyRejected:
		on_nack_from_market_cb(o, pOrder->StatusMsg);
		//update_instr_on_nack_from_market_cb(o);
		break;


	default:
		break;
	}
}

void cffex_connection::OnRtnTradeAsync(CThostFtdcTradeField* pTrade)
{
	// 0 - log
	//loggerv2::info();
	if (m_debug)
		loggerv2::info("cffex_connection::OnRtnTradeAsync - "
		"BrokerID[%*.*s] "
		"InvestorID[%*.*s] "
		"InstrumentID[%*.*s] "
		"OrderRef[%*.*s] "
		"UserID[%*.*s] "

		"OffsetFlag[%c] "
		"HedgeFlag[%c] "
		"Price[%f] "

		"Volume[%d] "
		"TradeTime[%*.*s] "
		"TradeType[%c] "
		,

		sizeof(pTrade->BrokerID), sizeof(pTrade->BrokerID), pTrade->BrokerID,
		sizeof(pTrade->InvestorID), sizeof(pTrade->InvestorID), pTrade->InvestorID,
		sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID,
		sizeof(pTrade->OrderRef), sizeof(pTrade->OrderRef), pTrade->OrderRef,
		sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID,

		pTrade->OffsetFlag,
		pTrade->HedgeFlag,
		pTrade->Price,

		pTrade->Volume,
		sizeof(pTrade->TradeTime), sizeof(pTrade->TradeTime), pTrade->TradeTime,
		pTrade->TradeType
		);

	OrderWay::type way = pTrade->Direction == THOST_FTDC_D_Buy ? OrderWay::Buy : OrderWay::Sell;
	bool onlyUpdatePending = false;

	int account, bidId, askId, portfolioId, ntradingType;
	get_user_info(pTrade->UserID, account, bidId, askId, portfolioId, ntradingType);

	int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;

	loggerv2::info("cffex_connection::OnRtnTradeAsync,orderID:%d", orderId);
	if (orderId == -1)
	{
		loggerv2::warn("cffex_connection::OnRtnTradeAsync - cannot extract orderId from USerId[%*.*s]...", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
		orderId = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
	}

	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
		break;

	case 2:

		o = cffex_order_aux::anchor(this, pTrade);
		if (o == NULL)
		{
			loggerv2::error("cffex_connection::OnRtnTradeAsync cannot anchor order");
			return;
		}

		add_pending_order(o);
		break;
	default:
		break;
	}

	if (o == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRtnTradeAsync - order recovered NULL");
		return;
	}

	// 2 - treat message
	int execQty = pTrade->Volume;
	double execPrc = pTrade->Price;
	const char* pszExecRef = pTrade->TradeID;
	const char* pszTime = pTrade->TradeTime;

	exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);
	on_exec_from_market_cb(o, e);

	if (account == m_account_num)//这个回报对应的account是对的，按orderID来判断这个回报是否为历史回报
	{
		if (orderId > m_pCffexApi->m_begin_Id)//当前id大于beginID，这个回报不是历史回包
			onlyUpdatePending = false;
		else
			onlyUpdatePending = true;
	}
	else
	{
		/*ptime tradeTime(from_undelimited_string(pTrade->TradeDate), duration_from_string(pTrade->TradeTime));

		int hour = tradeTime.time_of_day().hours();*/

		//if (o->get_instrument()->getMarket() == "CZCE")
		//{
		//	tradeTime -= m_pCffexApi->CZCE_Time_dur;
		//}
		//else if (o->get_instrument()->getMarket() == "DCE")
		//{
		//	tradeTime -= m_pCffexApi->DCE_Time_dur;
		//}
		//else if (o->get_instrument()->getMarket() == "CFFEX")
		//{
		//	tradeTime -= m_pCffexApi->CFFEX_Time_dur;
		//}
		//else if (o->get_instrument()->getMarket() == "SHFE")
		//{
		//	tradeTime -= m_pCffexApi->SHFE_Time_dur;
		//}

		//tradeTime += boost::posix_time::time_duration(boost::posix_time::milliseconds(888));//允许50ms的网络抖动
		lwtp tp = string_to_lwtp(from_undelimited_string(pTrade->TradeDate), pTrade->TradeTime);
		int hour = get_hour_from_lwtp(tp);
		tp = tp + std::chrono::seconds(2);//允许2s的误差

		if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			onlyUpdatePending = true;
		if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;

		if (onlyUpdatePending)
		{
			loggerv2::info("cffex_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time. tradeTime %s", pTrade->TradeTime);
		}
		
	}

	update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
}
//void cffex_connection::cancel_num_warning(tradeitem* i)
//{
//	loggerv2::warn("tradeitem:%s cancel num is more than warning lev", i->getCode().c_str());
//	this->setTradingAllowed(false);
//}
//
//void cffex_connection::cancel_num_ban(tradeitem* i)
//{
//	if (i->get_instr_type() == InstrType::Future)
//	{
//		loggerv2::warn("tradeitem:%s cancel num is more than forbid lev", i->getCode().c_str());
//		this->setTradingAllowed(false);
//		i->set_cancel_forbid(true);
//	}
//}

std::string cffex_connection::getMaturity(std::string& sMat)
{
	std::string newMat;
	newMat = sMat.substr(0, 4);
	newMat += "-";
	newMat += sMat.substr(4, 2);
	newMat += "-";
	newMat += sMat.substr(6, 2);
	return newMat.c_str();
}

string get_instrument_class(string code)
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
		return str;
	}

void cffex_connection::OnRspQryInstrument_Future(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{		
		std::string sInstr = std::string(pInstrument->ExchangeInstID);
		boost::trim(sInstr);			
		std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";			
		char *zErrMsg = 0;
		std::string sUnderlying = pInstrument->UnderlyingInstrID; //
		if (sUnderlying == "")
		{
			printf_ex("cffex_connection::OnRspQryInstrument_Future sInstr：%s,sUnderlyiny:%s,instrument_class:%s\n", sInstr.c_str(), sUnderlying.c_str(), get_instrument_class(sInstr).c_str());
			sUnderlying = get_instrument_class(sInstr);
		}
		std::string sInstClass = "F_" + get_instrument_class(sInstr);
		std::string strExecDate = pInstrument->ExpireDate;
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = pInstrument->ExchangeID;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Futures VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("cffex_connection::OnRspQryInstrument_Future:failed to insert into database, ret is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("cffex_connection::OnRspQryInstrument_Future cmd:%s\n", sCmd.c_str());
				printf_ex("cffex_connection::OnRspQryInstrument_Future cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@" + get_type();
			sCmd = "UPDATE Futures SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "',";			
			sCmd += "ConnectionCodes='" + sConnectionCodes + "',";			
			sCmd += "Underlying='" + sUnderlying + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("cffex_connection::OnRspQryInstrument_Future:failed to update the database,error is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("cffex_connection::OnRspQryInstrument_Future update to the cmd:%s\n", sCmd.c_str());
				printf_ex("cffex_connection::OnRspQryInstrument_Future update to the cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		m_database->close_databse();	
		if (bIsLast && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
	}

void cffex_connection::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if (pInstrument == nullptr)
	{
		loggerv2::error("OnRspQryInstrument error");
	}
	else
	{
		std::string sInstr = std::string(pInstrument->ExchangeInstID);
		boost::trim(sInstr);
		printf_ex("cffex_connection::OnRspQryInstrument sInstr：%s,bIsLast:%d,pInstrument->ProductClass:%d\n", sInstr.c_str(), bIsLast, pInstrument->ProductClass);
		loggerv2::info("cffex_connection::OnRspQryInstrument sInstr：%s,bIsLast:%d,pInstrument->ProductClass:%d\n", sInstr.c_str(), bIsLast, pInstrument->ProductClass);
		if ((pInstrument->ProductClass != THOST_FTDC_PC_Options && pInstrument->ProductClass != THOST_FTDC_PC_Futures && pInstrument->ProductClass != THOST_FTDC_PC_SpotOption) || strlen(pInstrument->ExpireDate) < 1 /*|| strlen(pInstrument->ExchangeInstID) < 10*/)
		{
			if (bIsLast && this->get_is_last() == false)
			{
				this->set_is_last(true);
			}
			return;
		}
			
		if (pInstrument->ProductClass == THOST_FTDC_PC_Futures)
			return OnRspQryInstrument_Future(pInstrument,pRspInfo,nRequestID,bIsLast);

		std::string sSearch = "select * from Options where Code= '" + sInstr + "'";
		//const char* data = "Callback function called";
		char *zErrMsg = 0;

		std::string sUnderlying = pInstrument->UnderlyingInstrID; //
		std::string sInstClass = "O_" + get_instrument_class(sInstr);
		if (pInstrument->ProductClass == THOST_FTDC_PC_SpotOption)
		{
			if (strcmp(pInstrument->ProductID,"HO")==0)
			{
				sUnderlying = "IH";
				sInstClass = "O_IH";
			}
			if (strcmp(pInstrument->ProductID, "IO") == 0)
			{
				sUnderlying = "IF";
				sInstClass = "O_IF";
			}
		}
		std::string sCP = "C";  //"CallPut"
		switch (pInstrument->OptionsType)
		{
		case THOST_FTDC_CP_CallOptions:
			sCP = "C";
			break;
		case THOST_FTDC_CP_PutOptions:
		default:
			sCP = "P";
			break;
		}

		
		std::string strExecDate = pInstrument->ExpireDate;
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = pInstrument->ExchangeID;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Options VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + std::to_string(pInstrument->StrikePrice) + "',";
			sCmd += "'" + std::to_string(pInstrument->VolumeMultiple) + "',";
			sCmd += "'" + sCP + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());
			loggerv2::info("cffex_connection::OnRspQryInstrument_Option cmd:%s\n", sCmd.c_str());
			printf_ex("cffex_connection::OnRspQryInstrument_Option cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			if (rc == 0)
			{
				//loggerv2::info("failed to insert into database, ret is %d",rc);
				//sqlite3_free(zErrMsg);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@" + get_type();
			sCmd = "UPDATE Options SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "Strike = '" + std::to_string(pInstrument->StrikePrice) + "',";
			sCmd += "PointValue ='" + std::to_string(pInstrument->VolumeMultiple) + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "',";
			sCmd += "ConnectionCodes='" + sConnectionCodes + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());
			loggerv2::info("cffex_connection::OnRspQryInstrument_Option cmd:%s\n", sCmd.c_str());
			printf_ex("cffex_connection::OnRspQryInstrument_Option cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			if (rc == 0)
			{
				//loggerv2::info("failed to update the database,error is %d",rc);
				//sqlite3_free(zErrMsg);
			}
		}
		m_database->close_databse();
		if (bIsLast && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
	}
}


void cffex_connection::OnRtnExecOrder(CThostFtdcExecOrderField *pOrder)
{
	if (m_debug)
		loggerv2::info("cffex_connection::OnRtnExecOrder - "
		"BrokerID[%*.*s] "
		"InvestorID[%*.*s] "
		"InstrumentID[%*.*s] "
		"ExecOrderRef[%*.*s] "
		"UserID[%*.*s] "
		"Volume[%d]"
		"RequestID[%d] "

		"BusinessUnit[%*.*s] "
		"OffsetFlag[%c] "
		"HedgeFlag[%c] "
		"ActionType[%c] "
		"PosiDirection[%c] "
		"ReservePositionFlag[%c] "
		"CloseFlag[%c] "
		"ExecOrderLocalID[%*.*s] "
		"ExchangeID[%*.*s] "
		"ParticipantID[%*.*s] "
		"ClientID[%*.*s] "
		"ExchangeInstID[%*.*s] "
		"TraderID[%*.*s] "
		"InstallID[%d] "
		"OrderSubmitStatus*[%c] "
		"TradingDay[%*.*s] "
		"SettlementID[%d] "
		"ExecOrderSysID[%*.*s] "


		"InsertDate[%*.*s] "
		"InsertTime[%*.*s] "
		"CancelTime[%*.*s] "
		"ExecResult[%c] "
		"ClearingPartID[%*.*s] "


		"NotifySequence[%d] "
		"SequenceNo[%d] "
		"FrontID[%d] "
		"SessionID[%d] "
		"UserProductInfo[%*.*s] "
		"StatusMsg[%*.*s] "

		"ActiveUserID[%*.*s] "
		"BrokerExecOrderSeq[%d] "
		,

		sizeof(pOrder->BrokerID), sizeof(pOrder->BrokerID), pOrder->BrokerID,
		sizeof(pOrder->InvestorID), sizeof(pOrder->InvestorID), pOrder->InvestorID,
		sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID,
		sizeof(pOrder->ExecOrderRef), sizeof(pOrder->ExecOrderRef), pOrder->ExecOrderRef,
		sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID,
		pOrder->Volume,
		pOrder->RequestID,



		sizeof(pOrder->BusinessUnit), sizeof(pOrder->BusinessUnit), pOrder->BusinessUnit,
		pOrder->OffsetFlag,
		pOrder->HedgeFlag,
		pOrder->ActionType,
		pOrder->PosiDirection,


		pOrder->ReservePositionFlag,
		pOrder->CloseFlag,
		sizeof(pOrder->ExecOrderLocalID), sizeof(pOrder->ExecOrderLocalID), pOrder->ExecOrderLocalID,

		sizeof(pOrder->ExchangeID), sizeof(pOrder->ExchangeID), pOrder->ExchangeID,
		sizeof(pOrder->ParticipantID), sizeof(pOrder->ParticipantID), pOrder->ParticipantID,
		sizeof(pOrder->ClientID), sizeof(pOrder->ClientID), pOrder->ClientID,
		sizeof(pOrder->ExchangeInstID), sizeof(pOrder->ExchangeInstID), pOrder->ExchangeInstID,
		sizeof(pOrder->TraderID), sizeof(pOrder->TraderID), pOrder->TraderID,

		pOrder->InstallID,
		pOrder->OrderSubmitStatus,
		sizeof(pOrder->TradingDay), sizeof(pOrder->TradingDay), pOrder->TradingDay,
		pOrder->SettlementID,
		sizeof(pOrder->ExecOrderSysID), sizeof(pOrder->ExecOrderSysID), pOrder->ExecOrderSysID,
		sizeof(pOrder->InsertDate), sizeof(pOrder->InsertDate), pOrder->InsertDate,
		sizeof(pOrder->InsertTime), sizeof(pOrder->InsertTime), pOrder->InsertTime,
		sizeof(pOrder->CancelTime), sizeof(pOrder->CancelTime), pOrder->CancelTime,
		pOrder->ExecResult,
		sizeof(pOrder->ClearingPartID), sizeof(pOrder->ClearingPartID), pOrder->ClearingPartID,

		pOrder->NotifySequence,
		pOrder->SequenceNo,
		pOrder->FrontID,
		pOrder->SessionID,
		sizeof(pOrder->UserProductInfo), sizeof(pOrder->UserProductInfo), pOrder->UserProductInfo,
		sizeof(pOrder->StatusMsg), sizeof(pOrder->StatusMsg), pOrder->StatusMsg,
		sizeof(pOrder->ActiveUserID), sizeof(pOrder->ActiveUserID), pOrder->ActiveUserID,
		pOrder->BrokerExecOrderSeq
		);

	int orderId = get_order_id(pOrder->UserID);

	if (orderId == -1)
	{
		loggerv2::warn("cffex_connection::OnRtnExecOrder - cannot extract orderId from USerId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
		//return;
		orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->ExecOrderRef);
	}
	loggerv2::info("cffex_connection::OnRtnExecOrder,orderID:%d", orderId);

	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRtnExecOrder - message received on dead order[%d]...", orderId);
		break;

	case 2:

		o = cffex_order_aux::anchor(this, pOrder);
		if (o == NULL)
		{
			loggerv2::error("cffex_connection::OnRtnExecOrder cannot anchor order");
			return;
		}

		add_pending_order(o);
		break;
	default:
		break;
	}


	if (o == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRtnExecOrder - order recovered NULL");
		return;
	}


	cffex_order_aux::set_order_sys_id(o, pOrder->ExecOrderSysID);
	if (o->get_quantity() != pOrder->Volume)
	{
		if (m_debug)
			loggerv2::debug("cffex_connection::OnRtnExecOrder resetting order quantity to %d", pOrder->Volume);
		o->set_quantity(pOrder->Volume);
	}


	if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
	{
		if (m_debug)
			loggerv2::debug("cffex_connection::OnRtnExecOrder resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
		o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
	}

	// cancel 1: 3-3
	// cancel 2: 3-5 : sometimes 3-5 is not coming


	switch (pOrder->OrderSubmitStatus)
	{
	case THOST_FTDC_OSS_InsertSubmitted:
		//if (o->get_last_action() != AtsType::OrderAction::Created)
		//{
		//   o->set_last_action(AtsType::OrderAction::Created);
		//}
	{
		//special case, we could receive status 0-5 for cancellation from another application.
		if (pOrder->ExecResult == THOST_FTDC_OER_Canceled)
		{
			o->set_last_action(AtsType::OrderAction::Cancelled);

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
				loggerv2::info("cffex_connection::OnRtnExecOrder receive order ack while order status is exec, this is possibly during the resynchro process.");
				//we recvieve the exec earlier than ack , so order is already created.

				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}

		}

	}



	break;

	case THOST_FTDC_OSS_CancelSubmitted:
	{
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

		if (pOrder->ExecResult == THOST_FTDC_OER_Canceled)
			// '5','2','4'
		{
			o->set_last_action(AtsType::OrderAction::Cancelled);
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{

				update_instr_on_ack_from_market_cb(o);
				on_cancel_from_market_cb(o);
			}
			else if (o->get_status() == AtsType::OrderStatus::Ack)
			{
				loggerv2::info("Calling on_ack_from_market_cb.. this is possibly the resynchro process..");

				/*on_ack_from_market_cb(o);
				*///update_instr_on_ack_from_market_cb(o);


				update_instr_on_cancel_from_market_cb(o);
				on_cancel_from_market_cb(o);

				//m_statistics.incr_can();
				loggerv2::info("cffex_connection::OnRtnExecOrder Current cancel number is %d", m_statistics.get_can());
			}

			//earse the pointer from m_cancelOrdMap;
			//std::map<int, cffex_order*>::iterator it = m_cancelOrdMap.find(atoi(pOrder->OrderRef));
			//if (it != m_cancelOrdMap.end())
			//	m_cancelOrdMap.erase(it);
			//update_instr_on_ack_from_market_cb(o);
		}
	}
	break;

	case THOST_FTDC_OSS_InsertRejected:

		update_instr_on_nack_from_market_cb(o);
		on_nack_from_market_cb(o, pOrder->StatusMsg);
		break;

	case THOST_FTDC_OSS_CancelRejected:

	{
		on_nack_from_market_cb(o, pOrder->StatusMsg);
		if (o->get_status() == AtsType::OrderStatus::Nack) //this is nack of a cancel and we rolled back to order with book qty!=0
			update_instr_on_nack_from_market_cb(o);
	}
	break;

	case THOST_FTDC_OSS_ModifyRejected:
		on_nack_from_market_cb(o, pOrder->StatusMsg);
		//update_instr_on_nack_from_market_cb(o);
		break;


	default:
		break;
	}
}

void cffex_connection::OnRspExecOrderInsert(CThostFtdcInputExecOrderField * pOrder, TThostFtdcErrorIDType errorId)
{
	//cffex_order* o = NULL;

	// 0 - log
	if (m_debug)
		loggerv2::info("cffex_connection::OnRspExecOrderInsert - orderRef[%s] userId[%s] RequestID[%d] errorId[%d]", pOrder->ExecOrderRef, pOrder->UserID, pOrder->RequestID, errorId);

	// 1 - retrieve order
	int orderId = get_order_id(pOrder->UserID);
	loggerv2::info("cffex_connection::OnRspExecOrderInsert,orderID:%d", orderId);
	if (orderId == -1)
	{
		loggerv2::warn("cffex_connection::OnRspExecOrderInsert - cannot extract orderId from UserId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
		//return;
		//o->set_portfolio("UNKNOWN");
		orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->ExecOrderRef);
	}

	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRspExecOrderInsert - message received on dead order[%d]...", orderId);
		break;

	case 2:

		o = cffex_order_aux::anchor(this, pOrder);
		if (o == NULL)
		{
			loggerv2::error("cffex_connection::OnRspExecOrderInsert cannot anchor order");
			return;
		}

		add_pending_order(o);
		break;
	default:
		break;

	}


	if (o == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRspExecOrderInsert - order recovered NULL");
		return;
	}


	// 2 - treat message
	if (errorId != 0)
	{
		char szErrorMsg[32 + 1];
		snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", errorId);

		on_nack_from_market_cb(o, szErrorMsg);
		//bug fix. should not call update_instr_on_nack_from_market_cb, because we haven't ack the order before!
		//
		//update_instr_on_nack_from_market_cb(o);
	}
	else
	{
		loggerv2::error("OnRspExecOrderInsert - order[%d] errorId[0] ???", orderId);
	}
}

void cffex_connection::OnRtnQuoteAsync(CThostFtdcQuoteField* pQuote)
{

	if (m_debug)
		loggerv2::info("cffex_connection::OnRtnQuoteAsync - "
		"InstrumentID[%*.*s] "
		"QuoteRef[%*.*s] "
		"UserID[%*.*s] "
		"AskPrice[%f]"
		"BidPrice[%f] "
		"AskVolume[%d]"
		"BidVolume[%d] "

		"RequestID[%d] "
		"AskOffsetFlag[%c] "
		"BidOffsetFlag[%c] "

		"AskHedgeFlag[%c] "
		"BidHedgeFlag[%c] "
		"QuoteLocalID[%*.*s] "

		"OrderSubmitStatus*[%c] "
		"NotifySequence[%d] "
		"TradingDay[%*.*s] "
		"InsertDate[%*.*s] "
		"InsertTime[%*.*s] "
		"CancelTime[%*.*s] "
		"QuoteStatus[%c] "

		"AskOrderSysID[%*.*s] "
		"BidOrderSysID[%*.*s] "

		"StatusMsg[%*.*s] "
		"AskOrderRef[%*.*s] "
		"BidOrderRef[%*.*s] "
		"ForQuoteSysID[%*.*s] "
		,

		
		sizeof(pQuote->InstrumentID), sizeof(pQuote->InstrumentID), pQuote->InstrumentID,
		sizeof(pQuote->QuoteRef), sizeof(pQuote->QuoteRef), pQuote->QuoteRef,
		sizeof(pQuote->UserID), sizeof(pQuote->UserID), pQuote->UserID,
		pQuote->AskPrice,
		pQuote->BidPrice,
		pQuote->AskVolume,
		pQuote->BidVolume,
		pQuote->RequestID,
		pQuote->AskOffsetFlag,
		pQuote->BidOffsetFlag,

		pQuote->AskHedgeFlag,
		pQuote->BidHedgeFlag,
		

		sizeof(pQuote->QuoteLocalID), sizeof(pQuote->QuoteLocalID), pQuote->QuoteLocalID,
		pQuote->OrderSubmitStatus,
		pQuote->NotifySequence,
		sizeof(pQuote->TradingDay), sizeof(pQuote->TradingDay), pQuote->TradingDay,
		
		sizeof(pQuote->InsertDate), sizeof(pQuote->InsertDate), pQuote->InsertDate,
		sizeof(pQuote->InsertTime), sizeof(pQuote->InsertTime), pQuote->InsertTime,
		sizeof(pQuote->CancelTime), sizeof(pQuote->CancelTime), pQuote->CancelTime,
		pQuote->QuoteStatus,

		sizeof(pQuote->AskOrderSysID), sizeof(pQuote->AskOrderSysID), pQuote->AskOrderSysID,
		sizeof(pQuote->BidOrderSysID), sizeof(pQuote->BidOrderSysID), pQuote->BidOrderSysID,

		sizeof(pQuote->StatusMsg), sizeof(pQuote->StatusMsg), pQuote->StatusMsg,
		sizeof(pQuote->AskOrderRef), sizeof(pQuote->AskOrderRef), pQuote->AskOrderRef,
		sizeof(pQuote->BidOrderRef), sizeof(pQuote->BidOrderRef), pQuote->BidOrderRef,
		sizeof(pQuote->ForQuoteSysID), sizeof(pQuote->ForQuoteSysID), pQuote->ForQuoteSysID
	
		);
	//cffex_order* o = NULL;

	// 1 - retrieve order
	int account, bidId, askId, portfolioId, tradingType;
	get_user_info(pQuote->UserID, account, bidId, askId, portfolioId, tradingType);
	int orderId = bidId;

	if (orderId <= 0)
	{
		//abadon unknow quote now
		return;

		loggerv2::warn("cffex_connection::OnRtnQuoteAsync - cannot extract orderId from UserID[%*.*s]...", sizeof(pQuote->UserID), sizeof(pQuote->UserID), pQuote->UserID);
		//return;
		orderId = atoi(pQuote->BrokerID) * 100000 + atoi(pQuote->QuoteRef);
	}
	loggerv2::info("cffex_connection::OnRtnQuoteAsync,quoteID:%d", orderId);

	int ret;
	quote *q = get_quote_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRtnQuoteAsync - message received on dead quote[%d]...", orderId);
		break;

	case 2:

		q = cffex_order_aux::anchor(this, pQuote);
		if (q == NULL)
		{
			loggerv2::error("cffex_connection::OnRtnQuoteAsync cannot anchor quote");
			return;
		}

		add_pending_quote(q);
		break;
	default:
		break;
	}


	if (q == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRtnQuoteAsync - quote recovered NULL");
		return;
	}

	if ((int)(pQuote->ForQuoteSysID[0])>31)
		q->set_FQR_ID(pQuote->ForQuoteSysID);

	cffex_order_aux::set_quote_sys_id(q, pQuote->QuoteSysID);
	if (q->get_bid_order() && q->get_bid_order()->get_quantity() != pQuote->BidVolume)
	{
		if (m_debug)
			loggerv2::debug("cffex_connection::OnRtnQuoteAsync resetting bid order quantity to %d", pQuote->BidVolume);
		q->get_bid_order()->set_quantity(pQuote->BidVolume);
	}

	if (q->get_ask_order() && q->get_ask_order()->get_quantity() != pQuote->AskVolume)
	{
		if (m_debug)
			loggerv2::debug("cffex_connection::OnRtnQuoteAsync resetting ask order quantity to %d", pQuote->AskVolume);
		q->get_ask_order()->set_quantity(pQuote->AskVolume);
	}
	/*order* o1 = q->get_bid_order();

	if (o1 && o1->get_book_quantity() != o1->get_quantity() - o1->get_exec_quantity())
	{
	if (m_debug)
	loggerv2::debug("cffex_connection::OnRtnQuoteAsync resetting bid order book quantity to %d", o1->get_quantity() - o1->get_exec_quantity());
	o1->set_book_quantity(o1->get_quantity() - o1->get_exec_quantity());
	}

	o1 = q->get_ask_order();
	if (o1 && o1->get_book_quantity() != o1->get_quantity() - o1->get_exec_quantity())
	{
	if (m_debug)
	loggerv2::debug("cffex_connection::OnRtnQuoteAsync resetting ask order book quantity to %d", o1->get_quantity() - o1->get_exec_quantity());
	o1->set_book_quantity(o1->get_quantity() - o1->get_exec_quantity());
	}*/

	// cancel 1: 3-3
	// cancel 2: 3-5 : sometimes 3-5 is not coming


	switch (pQuote->OrderSubmitStatus)
	{
	case THOST_FTDC_OSS_InsertSubmitted:
		//if (o->get_last_action() != AtsType::OrderAction::Created)
		//{
		//   o->set_last_action(AtsType::OrderAction::Created);
		//}
	{
		//special case, we could receive status 0-5 for cancellation from another application.
		if (pQuote->QuoteStatus == THOST_FTDC_OST_Canceled)
		{
			q->set_last_action(AtsType::OrderAction::Cancelled);
			on_cancel_quote_from_market_cb(q);
			update_instr_on_ack_from_market_cb(q->get_bid_order());
			update_instr_on_ack_from_market_cb(q->get_ask_order());
			
			
			//on_cancel_quote_from_market_cb(q);
		}

		else if (q->get_last_action() == AtsType::OrderAction::Created || q->get_last_action() == AtsType::OrderAction::Cancelled)
		{
			if (q->get_status() == AtsType::OrderStatus::WaitMarket || q->get_status() == AtsType::OrderStatus::WaitServer)
			{

				update_instr_on_ack_from_market_cb(q->get_bid_order());
				update_instr_on_ack_from_market_cb(q->get_ask_order());
				on_ack_quote_from_market_cb(q);
			}
			//if (o->get_status() == AtsType::OrderStatus::Exec)
			//{
			//	loggerv2::info("cffex_connection::OnRtnQuoteAsync receive order ack while order status is exec, this is possibly during the resynchro process.");
			//	//we recvieve the exec earlier than ack , so order is already created.

			//	update_instr_on_ack_from_market_cb(o);
			//	on_ack_from_market_cb(o);
			//}

		}

	}



	break;

	case THOST_FTDC_OSS_CancelSubmitted:
	{
		if (q->get_last_action() != AtsType::OrderAction::Cancelled)
		{
			q->set_last_action(AtsType::OrderAction::Cancelled);
		}
		if (q->get_status() == AtsType::OrderStatus::WaitMarket || q->get_status() == AtsType::OrderStatus::WaitServer)
		{
			on_ack_quote_from_market_cb(q);
			//update_instr_on_ack_from_market_cb(o);
			//don't need to update because order will be updated on the second cancel confirmation.
		}
	}
	break;

	case THOST_FTDC_OSS_ModifySubmitted:
	{
		if (q->get_last_action() != AtsType::OrderAction::Modified)
		{
			q->set_last_action(AtsType::OrderAction::Modified);
		}
		if (q->get_status() == AtsType::OrderStatus::WaitMarket || q->get_status() == AtsType::OrderStatus::WaitServer)
		{

			update_instr_on_ack_from_market_cb(q->get_bid_order());
			update_instr_on_ack_from_market_cb(q->get_ask_order());
			on_ack_quote_from_market_cb(q);
		}
	}
	break;

	case THOST_FTDC_OSS_Accepted:

	{

		if (pQuote->QuoteStatus == THOST_FTDC_OST_Canceled || pQuote->QuoteStatus == THOST_FTDC_OST_PartTradedNotQueueing || pQuote->QuoteStatus == THOST_FTDC_OST_NoTradeNotQueueing)
			// '5','2','4'
		{
			q->set_last_action(AtsType::OrderAction::Cancelled);
			if (q->get_status() == AtsType::OrderStatus::WaitMarket || q->get_status() == AtsType::OrderStatus::WaitServer)
			{
				if (q->get_bid_order()!=nullptr)
				{
					update_instr_on_ack_from_market_cb(q->get_bid_order());
				}
				if (q->get_ask_order() != nullptr)
				{
					update_instr_on_ack_from_market_cb(q->get_ask_order());
				}
				
				on_cancel_quote_from_market_cb(q);
			}
			else if (q->get_status() == AtsType::OrderStatus::Ack)
			{
				loggerv2::info("Calling on_ack_from_market_cb.. this is possibly the resynchro process..");

				/*on_ack_from_market_cb(o);
				*///update_instr_on_ack_from_market_cb(o);

				on_cancel_quote_from_market_cb(q);
				update_instr_on_cancel_from_market_cb(q->get_bid_order());
				update_instr_on_cancel_from_market_cb(q->get_ask_order());
				//on_cancel_quote_from_market_cb(q);


				//m_statistics.incr_can();
				loggerv2::info("cffex_connection::OnRtnQuoteAsync Current cancel number is %d", m_statistics.get_can());
			}

			//earse the pointer from m_cancelOrdMap;
			//std::map<int, cffex_order*>::iterator it = m_cancelOrdMap.find(atoi(pOrder->OrderRef));
			//if (it != m_cancelOrdMap.end())
			//	m_cancelOrdMap.erase(it);
			//update_instr_on_ack_from_market_cb(o);
		}
	}
	break;

	case THOST_FTDC_OSS_InsertRejected:
		update_instr_on_nack_from_market_cb(q->get_bid_order());
		update_instr_on_nack_from_market_cb(q->get_ask_order());
		
		on_nack_quote_from_market_cb(q, pQuote->StatusMsg);
		break;

	case THOST_FTDC_OSS_CancelRejected:

	{
		on_nack_quote_from_market_cb(q, pQuote->StatusMsg);
		if (q->get_status() == AtsType::OrderStatus::Nack) //this is nack of a cancel and we rolled back to order with book qty!=0
		{
			update_instr_on_nack_from_market_cb(q->get_bid_order());
			update_instr_on_nack_from_market_cb(q->get_ask_order());
		}
	}
	break;

	case THOST_FTDC_OSS_ModifyRejected:
		on_nack_quote_from_market_cb(q, pQuote->StatusMsg);
		//update_instr_on_nack_from_market_cb(o);
		break;


	default:
		break;
	}
}

void cffex_connection::OnRspQuoteInsertAsync(CThostFtdcInputQuoteField* pOrder)
{
	//
	// used only for rejects.
	//
	int errorId = pOrder->RequestID;

	//cffex_order* o = NULL;

	// 0 - log
	if (m_debug)
		loggerv2::info("cffex_connection::OnRspQuoteInsertAsync - orderRef[%s] userId[%s] errorId[%d]", pOrder->QuoteRef, pOrder->UserID, errorId);

	// 1 - retrieve order
	//int orderId = get_order_id(pOrder->UserID);

	int account, bidId, askId, portfolioId, tradingType;
	get_user_info(pOrder->UserID, account, bidId, askId, portfolioId, tradingType);
	int orderId = bidId;


	loggerv2::info("cffex_connection::OnRspQuoteInsertAsync,quoteID:%d", orderId);
	if (orderId == -1)
	{
		loggerv2::warn("cffex_connection::OnRspQuoteInsertAsync - cannot extract quoteID from UserId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
		//return;
		//o->set_portfolio("UNKNOWN");
		orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->QuoteRef);
	}

	int ret;
	quote *q = get_quote_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("cffex_connection::OnRspQuoteInsertAsync - message received on dead quote[%d]...", orderId);
		break;

	case 2:

		q = cffex_order_aux::anchor(this, pOrder); 
		if (q == NULL)
		{
			loggerv2::error("cffex_connection::OnRspQuoteInsertAsync cannot anchor order");
			return;
		}

		add_pending_quote(q);
		break;
	default:
		break;

	}


	if (q == NULL) // should not happen
	{
		loggerv2::error("cffex_connection::OnRspQuoteInsertAsync - order recovered NULL");
		return;
	}


	// 2 - treat message
	if (errorId != 0)
	{
		char szErrorMsg[32 + 1];
		snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", errorId);

		on_nack_quote_from_market_cb(q, szErrorMsg);
		//bug fix. should not call update_instr_on_nack_from_market_cb, because we haven't ack the order before!
		//
		//update_instr_on_nack_from_market_cb(o);
	}
	else
	{
		loggerv2::error("OnRspQuoteInsertAsync - quote[%d] errorId[0] ???", orderId);
	}
}


}

