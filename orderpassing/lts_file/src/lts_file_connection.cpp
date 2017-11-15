#include "lts_file_connection.h"
#include "string_tokenizer.h"
#include <boost/algorithm/string.hpp>
#include "tradeItem_gh.h"
#include <boost/property_tree/ini_parser.hpp>


using namespace terra::common;

namespace lts_file
{

	lts_file_connection::lts_file_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "lts_file_connection";
		m_quant_proxy = new lts_quant_proxy();
		m_quant_proxy->m_con = this;
	}

	lts_file_connection::~lts_file_connection()
	{
		delete m_pltsTrdApi;
		delete m_pltsReqApi;
	}
#if 0//def _WIN32
	BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam)
	{
		lts_file::lts_file_connection *con = (lts_file::lts_file_connection*)lParam;
		char str[100] = { 0 };
		::GetWindowText(hwnd, str, sizeof(str));
		std::string buff = str;
		if (buff.find("盈佳证券") != std::string::npos)
		{
			con->set_HWND(hwnd);//获取盈佳客户端的窗口句柄
			//::PostMessage(hwnd, WM_KEYDOWN, VK_F2, 1);
		}
		return true;
	}

	void lts_file_connection::post_keyboard_msg()
	{
		sleep_by_milliseconds(std::chrono::seconds(1));
		while (1)
		{
			if (m_hwnd != NULL)
			{
				::PostMessage(m_hwnd, WM_KEYDOWN, VK_F2, 1);//发送键盘消息给盈佳客户端
				sleep_by_milliseconds(std::chrono::milliseconds(200));
				::PostMessage(m_hwnd, WM_KEYDOWN, VK_F8, 1);
				
			}
		}
	}
#endif
	bool lts_file_connection::init_config(const std::string &name, const std::string &strConfigFile)
	{
		// general
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		if (strConfigFile.length() < 1)
			return false;

		boost::filesystem::path p(strConfigFile);
		if (!boost::filesystem::exists(p))
		{
			printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
			return false;
		}
		boost::property_tree::ptree root;
		boost::property_tree::ini_parser::read_ini(strConfigFile, root);


		// req
		m_sHostnameQry = root.get<string>(name + ".hostname_req", "");

		m_sServiceQry = root.get<string>(name + ".service_req", "");

		m_sPasswordQry = root.get<string>(name + ".password_req", "");

		// trd
		m_sHostnameTrd = root.get<string>(name + ".hostname_trd", "");

		m_sServiceTrd = root.get<string>(name + ".service_trd", "");

		m_sPasswordTrd = root.get<string>(name + ".password_trd", "");

		m_sProductInfo = root.get<string>(name + ".product_info", "");

		m_sAuthCode = root.get<string>(name + ".auth_code", "");

		m_bAutoDetectCoveredSell = root.get<bool>(name + ".auto_detect_covered_sell", false);

		m_blts_wrapper = root.get<bool>(name + ".lts_wrapper", false);

		m_str_ltsSrv = root.get<string>(name + ".lts_servers", "test");

		m_sStockAccountName = root.get<string>(name + ".stock_account", "test");

		m_pltsTrdApi = new lts_file_trdapi(this);
		m_pltsReqApi = new lts_file_reqapi(this);
		//CMySecurityFtdcTraderSpi* pBase = new CMySecurityFtdcTraderSpi();
		//m_pltsTrdApi = ptr;
		//CLTSAgent::CreateTraderApi("060000000038", "123321", "互联网1", m_pltsTrdApi, SECURITY_TERT_RESTART, SECURITY_TERT_RESTART);

		if (m_pltsTrdApi == nullptr || m_pltsReqApi == nullptr)
		{
			return false;
		}

		// lts API
#if 0
		init_req_api();
		init_trd_api();
#endif
		//std::thread th(boost::bind(&lts_file_connection::set_kernel_timer_thread, this));
		//m_thread.swap(th);
		init_process(io_service_type::trader, 10);

#if 0//def _WIN32
		auto hwnd = FindWindow("winner", "XXXX - 盈佳证券交易终端");
		if (NULL != hwnd)
		{
			m_hwnd = hwnd;
		}
		else
		{
			EnumWindows(lpEnumFunc, (LPARAM)this);
		}
		std::thread t(boost::bind(&lts_file_connection::post_keyboard_msg, this));
		t.detach();
#endif
		m_bKey_with_exchange = false;
		return true;
	}





	void lts_file_connection::init_connection()
	{
		//m_outboundQueue.setHandler(boost::bind(&lts_file_connection::process_outbound_msg_cb, this));
		m_userInfoQueue.setHandler(boost::bind(&lts_file_connection::OnUserInfoAsync, this, _1));
	}

	void lts_file_connection::release()
	{

		//is_alive(false);
		//m_thread.join();
		ctpbase_connection::release();
		// lts API
		m_pltsTrdApi->release();
		m_pltsReqApi->release();
	}

	void lts_file_connection::connect()
	{
		//#if 0
		if (m_blts_wrapper)
			return;
#if 0
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			if (m_debug)
				loggerv2::info("connecting to lts...");

			on_status_changed(AtsType::ConnectionStatus::WaitConnect);

			loggerv2::info("connecting to lts trade api");
			connect_trd_api();
			loggerv2::info("connecting to lts request api");
			connect_req_api();
		}
#endif
		//#else
		if (this->m_quant_proxy->connect() == true)
		{
			on_status_changed(AtsType::ConnectionStatus::Connected);
		}
		else
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}
		//#endif
	}

	void lts_file_connection::disconnect()
	{
		//#if 0
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			if (m_pltsTrdApi->disconnect() == true || m_pltsReqApi->disconnect() == true)
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "lts_file_connection - ReqUserLogout failed");
			}
		}
		//#else
		if (this->m_quant_proxy->disconnect() == true)
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}
		//#endif
		this->m_quant_proxy->stop();
	}
	void lts_file_connection::request_instruments()
	{
		loggerv2::info("lts_file_connection::requesting instruments");
		m_database->open_database();
		m_pltsReqApi->request_instruments();
	}


	void lts_file_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_pltsTrdApi->Process();
		m_userInfoQueue.Pops_Handle(0);
	}

	int lts_file_connection::market_create_order_async(order* o, char* pszReason)
	{

		CSecurityFtdcInputOrderField *request = xs_create_pool.get_mem();
		memset(request, 0, sizeof(CSecurityFtdcInputOrderField));

		strcpy(request->BrokerID, m_sBrokerId.c_str());
		strcpy(request->InvestorID, m_sUsername.c_str());

		strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());

		//need to overwrite the following part.
		strcpy(request->ExchangeID, o->get_instrument()->getMarket().c_str());


		switch (o->get_way())
		{
		case AtsType::OrderWay::Buy:
		{
			request->Direction = SECURITY_FTDC_D_Buy;
			if (m_bAutoDetectCoveredSell)
			{
				if (o->get_instrument()->get_instr_type() == AtsType::InstrType::Call)
				{
					tradeitem* i = o->get_instrument();
					if (i)
					{

						if (i->get_covered_sell_open_position() - i->get_pending_covered_sell_close_qty() >= o->get_quantity())
						{
							request->Direction = SECURITY_FTDC_D_Covered;
							o->set_way(AtsType::OrderWay::CoveredBuy);
							o->set_open_close(AtsType::OrderOpenClose::Close);
						}


					}
				}
			}
		}

		break;
		case AtsType::OrderWay::CoveredBuy:
		case AtsType::OrderWay::CoveredSell:
			request->Direction = SECURITY_FTDC_D_Covered;
			break;
		case AtsType::OrderWay::Freeze:
		case AtsType::OrderWay::Unfreeze:
			request->Direction = SECURITY_FTDC_D_Freeze;
			break;
		case AtsType::OrderWay::Sell:
		{
			request->Direction = SECURITY_FTDC_D_Sell;
			if (m_bAutoDetectCoveredSell)
			{
				if (o->get_instrument()->get_instr_type() == AtsType::InstrType::Call)
				{
					tradeitem* i = o->get_instrument();
					tradeitem* iUnderlying = i->get_underlying();
					if (iUnderlying)
					{
						//loggerv2::info("iUnderlying->get_frozen_long_position[%d], i->get_pending_covered_sell_open_qty[%d],i->get_point_value[%f],o->get_quantity[%d]", iUnderlying->get_frozen_long_position(), i->get_pending_covered_sell_open_qty(), i->get_point_value(), o->get_quantity());
						double pv = i->get_point_value();
						if (iUnderlying->get_frozen_long_position() - i->get_pending_covered_sell_open_qty()*pv >= o->get_quantity()*pv)
						{
							//loggerv2::info("auto detect order should be covered sell. open");
							request->Direction = SECURITY_FTDC_D_Covered;
							o->set_way(AtsType::OrderWay::CoveredSell);
							o->set_open_close(AtsType::OrderOpenClose::Open);
						}
					}
				}
			}

			break;
		}
		case AtsType::OrderWay::PLEDGE_BOND_IN:
		{
			request->Direction = SECURITY_FTDC_D_PledgeBondIn;
		}
		break;
		case AtsType::OrderWay::PLEDGE_BOND_OUT:
		{
			request->Direction = SECURITY_FTDC_D_PledgeBondOut;
		}
		break;

		case AtsType::OrderWay::ETFPur:	//基金申购 货币基金需要区分 实时还是 交易型

		{
			request->Direction = SECURITY_FTDC_D_ETFPur;
		}
		break;

		case AtsType::OrderWay::ETFRed: //基金赎回 货币基金需要区分 实时还是 交易型
		{
			request->Direction = SECURITY_FTDC_D_ETFRed;
		}
		break;


		case AtsType::OrderWay::OFPur:	//基金申购 货币基金需要区分 实时还是 交易型

		{
			request->Direction = SECURITY_FTDC_D_OFPur;
		}
		break;

		case AtsType::OrderWay::OFRed: //基金赎回 货币基金需要区分 实时还是 交易型
		{
			request->Direction = SECURITY_FTDC_D_OFRed;
		}
		break;


		default:
		{
			snprintf(pszReason, REASON_MAXLENGTH, "unknown order way.\n");
			return false;
		}
		break;
		}

		switch (o->get_price_mode())
		{
		case AtsType::OrderPriceMode::Limit:
		{
			request->OrderPriceType = SECURITY_FTDC_OPT_LimitPrice;
			//bug in the api. though the size is 10, we can only copy 8 chars.
			strncpy(request->LimitPrice, std::to_string(o->get_price()).c_str(), 8);
			request->LimitPrice[8] = '\0';

		}
		break;
		case AtsType::OrderPriceMode::Market:
		{
			request->OrderPriceType = SECURITY_FTDC_OPT_BestPrice;
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

		request->VolumeTotalOriginal = o->get_quantity();
		if (o->get_restriction() == AtsType::OrderRestriction::None)
		{
			request->TimeCondition = SECURITY_FTDC_TC_GFD; // or GFS ???
			strcpy(request->GTDDate, "");
		}
		else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)
			request->TimeCondition = SECURITY_FTDC_TC_IOC;
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			xs_create_pool.free_mem(request);
			return 0;
		}


		//open _close

		if (o->get_open_close() == OrderOpenClose::Undef)
			o->set_open_close(compute_open_close(o));

		TSecurityFtdcOffsetFlagType oc = SECURITY_FTDC_OF_Open;
		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			oc = SECURITY_FTDC_OF_Open;
			break;

		case AtsType::OrderOpenClose::Close:
			oc = SECURITY_FTDC_OF_Close;
			break;

		case AtsType::OrderOpenClose::CloseToday:
			oc = SECURITY_FTDC_OF_CloseToday;
			break;

			//case AtsType::OrderOpenClose::Undef:
		default:
			//oc = compute_open_close(o);
			break;
		}


		request->CombOffsetFlag[0] = oc;

		if (!compute_userId(o, request->UserID, sizeof(request->UserID)))
		{
			xs_create_pool.free_mem(request);
			return 0;
		}

		//loggerv2::info("market_crate_order reqest.UserId [%s]", request->UserID);


		request->CombHedgeFlag[0] = SECURITY_FTDC_HF_Speculation;


		request->VolumeCondition = SECURITY_FTDC_VC_AV;
		request->ContingentCondition = SECURITY_FTDC_CC_Immediately;
		request->ForceCloseReason = SECURITY_FTDC_FCC_NotForceClose;

		request->MinVolume = 1;
		request->IsAutoSuspend = 0;
		request->UserForceClose = 0;
#if 1
		if (o->get_instrument()->get_instr_type() != AtsType::InstrType::Stock)
		{
			if (!m_pltsTrdApi->ReqOrderInsert(request))
			{
				if (m_debug)
					loggerv2::info("error when sending order");
				xs_create_pool.free_mem(request);
				return 0;
			}
		}
		else//stock
#endif
		{
			if (m_quant_proxy)
			{
				this->create_user_info(o);
				/*
				输入文件格式为csv文件，包括下面几列：证券代码,证券名称,方向,数量,价格,备注
				bool ReqOrderInsert(int order_id,const string & feedCode,const string & name,OrderWay::type way, int quantity, double price, OrderRestriction::type restriction = OrderRestriction::None, OrderOpenClose::type openClose = OrderOpenClose::Undef, OrderPriceMode::type priceMode = OrderPriceMode::Limit);
				*/
				m_quant_proxy->ReqOrderInsert(o->get_instrument()->get_trading_code(), o->get_instrument()->getInstrument()->get_exchange().c_str(), o->get_way(), o->get_quantity(), o->get_price(), o->get_restriction(), o->get_open_close(), o->get_price_mode(), request->UserID);
			}
		}
		xs_create_pool.free_mem(request);
		return 1;
	}


	int lts_file_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		//#endif

		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());

		//lts_order* o = dynamic_cast<lts_order*>(ord);
		//if (o == NULL)
		//{
		//	snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to lts_order*...\n");
		//	o->set_status(AtsType::OrderStatus::Nack);
		//	o->rollback();
		//	return 0;
		//}

		switch (o->get_way())
		{

		case AtsType::OrderWay::Freeze:
		case AtsType::OrderWay::Unfreeze:
			snprintf(pszReason, REASON_MAXLENGTH, "cannot cancel order with way = (un)freeze.\n");
			return 0;
		default:
			break;

		}


		CSecurityFtdcInputOrderActionField *request = xs_cancel_pool.get_mem();
		memset(request, 0, sizeof(CSecurityFtdcInputOrderActionField));

		strcpy(request->BrokerID, m_sBrokerId.c_str());
		strcpy(request->InvestorID, m_sUsername.c_str());
		strcpy(request->OrderRef, std::to_string(lts_file_order_aux::get_order_ref(o)).c_str());
		strcpy(request->ExchangeID, o->get_exchange_id().c_str());
		request->ActionFlag = SECURITY_FTDC_AF_Delete;



		if (!compute_userId(o, request->UserID, sizeof(request->UserID)))
		{
			xs_cancel_pool.free_mem(request);
			return 0;
		}
		strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());
		// //mandatory for securities.
		strcpy(request->BranchPBU, lts_file_order_aux::get_trader_id(o).c_str());
		strcpy(request->OrderLocalID, lts_file_order_aux::get_order_local_id(o).c_str());

		if (o->get_instrument()->get_instr_type() != AtsType::InstrType::Stock)
		{
			if (!m_pltsTrdApi->ReqOrderAction(request))
			{
				snprintf(pszReason, REASON_MAXLENGTH, "market_cancel order - api sending error\n");
				xs_cancel_pool.free_mem(request);
				return 0;
			}
		}

		int nOrdRef = lts_file_order_aux::get_order_ref(o);
		//m_cancelOrdMap.insert(std::pair<int, lts_order*>(nOrdRef, ord));
		m_cancelOrdMap.emplace(nOrdRef, o);

		if (o->get_instrument()->get_instr_type() == AtsType::InstrType::Stock)
		{
			if (m_quant_proxy)
			{
				m_quant_proxy->cancel(nOrdRef);
			}
		}

		if (m_debug)
			loggerv2::info("order cancel sent ok! order_status is %d", o->get_status());

		xs_cancel_pool.free_mem(request);
		return 1;
	}


	//
	// lts callbacks
	//
	void lts_file_connection::OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pOrder, int errorId)
	{
		//
		// used only for rejects.
		//
		// loggerv2::info("OnRspOrderInsertAsync");

		// 0 - log
		loggerv2::info("lts_file_connection::OnRspOrderInsertAsync - order[%s] errorId[%d]", pOrder->OrderRef, errorId);

		//lts_order* o = NULL;
		// 1 - retrieve order
		int orderId = get_order_id(pOrder->UserID);
		if (orderId == -1)
		{
			loggerv2::warn("lts_file_connection::OnRspOrderInsertAsync - cannot extract orderId from UserId[%*.*s]...", sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID);
			//return;
			//o->set_portfolio("UNKNOWN");
			orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderRef);
		}

		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<lts_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<lts_order*>(ord);//it->second);
			loggerv2::warn("lts_file_connection::OnRspOrderInsertAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:

			o = lts_file_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("lts_file_connection::OnRspOrderInsertAsync cannot anchor order");
				return;
			}

			add_pending_order(o);
			treat_freeze_order(o);
			break;
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("lts_file_connection::OnRspOrderInsertAsync - order recovered NULL");
			return;
		}


		// 2 - treat message
		if (errorId != 0)
		{
			char szErrorMsg[32 + 1];
			snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", errorId);

			on_nack_from_market_cb(o, szErrorMsg);
		}
		else
		{
			loggerv2::error("OnRspOrderInsertAsync - order[%d] errorId[0] ???", orderId);
		}
	}



	void lts_file_connection::OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pOrder, int errorId)
	{
		if (m_debug)
			loggerv2::info("lts_file_connection::OnRspOrderActionAsync");

		int nOrdRef = atoi(pOrder->OrderRef);
		auto it = m_cancelOrdMap.find(nOrdRef);
		if (it != m_cancelOrdMap.end())
		{
			order* o = it->second; // or dynamic_cast??
			if (o != nullptr)
			{
				if (errorId != 0)
				{
					char szErrorMsg[32 + 1];
					snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", errorId);

					on_nack_from_market_cb(o, szErrorMsg);
				}
			}

			m_cancelOrdMap.erase(it);
		}
		else
			loggerv2::warn("lts_file_connection::OnRspOrderActionAsync could not find order ref %d", nOrdRef);
	}



	void lts_file_connection::request_trading_account()
	{
		if (m_debug)
			loggerv2::info("lts_file_connection:: calling ReqQryTradingAccount ");

		CSecurityFtdcQryTradingAccountField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		//strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pltsReqApi->ReqQryTradingAccount(&pRequest);
		return;

	}


	void lts_file_connection::OnRtnOrderAsync(CSecurityFtdcOrderField* pOrder)
	{

		//loggerv2::info("calling lts_file_connection::OnRtnOrderAsync");
		if (m_debug)
			loggerv2::info("lts_file_connection::OnRtnOrderAsync - "
			"InstrumentID[%*.*s] "
			"OrderRef[%*.*s] "
			"UserID[%*.*s] "
			"ExChgID[%*.*s]"
			"OrderPriceType[%c] "
			"Direction[%c] "
			"LimitPrice[%*.*s] "
			"VolumeTotalOriginal[%d] "
			"ExchangeInstID[%*.*s]"
			"OrderSubmitStatus[%c] "
			"OrderStatus[%c]"
			"NotifySequence[%d]"
			"OrderSysID[%*.*s] "
			"VolumeTraded[%d] "
			"VolumeTotal[%d] "
			// "SequenceNo[%d] "
			"FrontID[%d]"
			"SessionID[%d]"
			"BrokerOrderSeq[%d] "
			"OrderLocalID[%*.*s] "
			"TraderID[%*.*s]"
			"RelativeOrderSysID[%*.*s]"
			"IsETF[%c]"
			,
			sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID,
			sizeof(pOrder->OrderRef), sizeof(pOrder->OrderRef), pOrder->OrderRef,
			sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID,
			sizeof(pOrder->ExchangeID), sizeof(pOrder->ExchangeID), pOrder->ExchangeID,
			pOrder->OrderPriceType,
			pOrder->Direction,
			sizeof(pOrder->LimitPrice), sizeof(pOrder->LimitPrice), pOrder->LimitPrice,
			pOrder->VolumeTotalOriginal,

			sizeof(pOrder->ExchangeInstID), sizeof(pOrder->ExchangeInstID), pOrder->ExchangeInstID,

			pOrder->OrderSubmitStatus,
			pOrder->OrderStatus,
			pOrder->NotifySequence,
			sizeof(pOrder->OrderSysID), sizeof(pOrder->OrderSysID), pOrder->OrderSysID,
			pOrder->VolumeTraded,
			pOrder->VolumeTotal,
			// pOrder->SequenceNo,
			pOrder->FrontID,
			pOrder->SessionID,
			pOrder->BrokerOrderSeq,
			sizeof(pOrder->OrderLocalID), sizeof(pOrder->OrderLocalID), pOrder->OrderLocalID,
			sizeof(pOrder->BranchPBU), sizeof(pOrder->BranchPBU), pOrder->BranchPBU,
			sizeof(pOrder->RelativeOrderSysID), sizeof(pOrder->RelativeOrderSysID), pOrder->RelativeOrderSysID,
			pOrder->IsETF ? 'Y' : 'N'

			);


		//std::string insertDate = pOrder->InsertDate;
		//if (insertDate != m_sCurrentBizDate)
		//{
		//	if (m_debug)
		//		loggerv2::warn("lts_file_connection::OnRtnOrderAsync - order insert date [%s] doesn't match current business date[%s].", insertDate.c_str(), m_sCurrentBizDate.c_str());
		//	return;
		//}

		//lts_order* o = NULL;
		// 1 - retrieve order
		int orderId = get_order_id(pOrder->UserID);
		if (orderId < 1)
		{
			orderId = FAKE_ID_MIN + atoi(pOrder->OrderRef);
		}
		//
		update_user_info(orderId,pOrder->UserID,atoi(pOrder->OrderRef));
		//
		std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + getName();
		AtsType::InstrType::type isntrType = AtsType::InstrType::Undef;
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr != NULL)
			isntrType = instr->get_instr_type();


		bool isFromActiveBook = false, /*isFromDeadBook = false,*/ isNewlyCreated = false, isEtf = (pOrder->IsETF && isntrType == AtsType::InstrType::Stock); //ETF realated but not ETF itself

		int ret;
		order *o = get_order_from_map(orderId, ret);
		//
		if (o&&pOrder->VolumeTotalOriginal<=0&&instr&&instr->get_instr_type() == AtsType::InstrType::Stock)
		{
			this->get_user_info_ex(o);
			loggerv2::info("lts_file_connection::OnRtnOrderAsync %s,VolumeTotalOriginal:%d,pOrder->OrderSubmitStatus:%d\n", pOrder->InstrumentID, pOrder->VolumeTotalOriginal, pOrder->OrderSubmitStatus);
		}
		//
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<lts_order*>(ord);//it->second);
			isFromActiveBook = true;
			break;
		case 1:
			//o = reinterpret_cast<lts_order*>(ord);//it->second);
			loggerv2::warn("lts_file_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			if (isEtf)
				return;
			break;
		case 2:
			if (instr&&instr->get_instr_type() == AtsType::InstrType::Stock)
			{
				o = lts_file_order_aux::anchor(this, pOrder, true);
				this->get_user_info_ex(o);
				//
				if (o->get_quantity() < 1)
				{
					o = nullptr;
				}
				//
			}
			else
			{
				o = lts_file_order_aux::anchor(this, pOrder);
			}
			if (o == NULL)
			{
				loggerv2::error("lts_file_connection::OnRtnOrderAsync cannot anchor order");
				return;
			}
			isNewlyCreated = true;
			add_pending_order(o);
			break;
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			if (m_debug)
				loggerv2::error("lts_file_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}


		// lts BUG:
		// on a cancel, first message has OrderSubmitStatus = THOST_FTDC_OSS_Accepted instead of THOST_FTDC_OSS_CancelSubmitted
		// -> trash
		//if (o->get_last_action() == AtsType::OrderAction::Cancelled && pOrder->OrderSubmitStatus == THOST_FTDC_OSS_Accepted && pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
		//{
		//   loggerv2::info("lts_file_connection::OnRtnOrderAsync - lts bug, TRASH");
		//   return;
		//}


		// 2 - treat message

		//if (!isEtf || isNewlyCreated || isFromDeadBook)
		if (!isEtf || isNewlyCreated)
		{

			//o->set_exchange_id(pOrder->ExchangeID);
			lts_file_order_aux::set_order_ref(o, atoi(pOrder->OrderRef));
			lts_file_order_aux::set_order_local_id(o, pOrder->OrderLocalID);
			lts_file_order_aux::set_trader_id(o, pOrder->BranchPBU);
			//o->set_front_id(pOrder->FrontID);
			//o->set_session_id(pOrder->SessionID);
			lts_file_order_aux::set_order_sys_id(o, pOrder->OrderSysID);

			if ((pOrder->VolumeTotalOriginal >0 )&&(o->get_quantity() != pOrder->VolumeTotalOriginal))
			{
				if (m_debug)
					loggerv2::debug("reseting order quantity to %d", pOrder->VolumeTotalOriginal);
				o->set_quantity(pOrder->VolumeTotalOriginal);
			}

			//if (o->get_exec_quantity() != pOrder->VolumeTraded)
			//{
			//	if (m_debug)
			//		loggerv2::debug("reseting order exec quantity to %d", pOrder->VolumeTraded);
			//	o->set_exec_quantity(pOrder->VolumeTraded);
			//}


			if ((pOrder->VolumeTotal>0)&&(o->get_book_quantity() != pOrder->VolumeTotal))
			{

				if (m_debug)
					loggerv2::debug("resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());

				//if (m_debug)
				//	loggerv2::debug("resetting order book quantity to %d", pOrder->VolumeTotal);
				//o->set_book_quantity(pOrder->VolumeTotal);
			}



			//loggerv2::info("order get_status is %d", o->get_status());

			switch (pOrder->OrderSubmitStatus)
			{

			case SECURITY_FTDC_OSS_InsertSubmitted:
			{
				//if (o->get_last_action() != AtsType::OrderAction::Created)
				//{
				//	o->set_last_action(AtsType::OrderAction::Created);
				//}


				if (o->get_last_action() == AtsType::OrderAction::Created)
				{
					if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
					{
						if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
						{
							//loggerv2::info("calling update_instr_on_ack_from_market_cb, because order rebuild time %s is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
							//if (!isFromDeadBook)
							update_instr_on_ack_from_market_cb(o);
						}

															  {
																  on_ack_from_market_cb(o);
																  treat_freeze_order(o);
															  }


					}


				}


			}
			break;

			case SECURITY_FTDC_OSS_CancelSubmitted:
			{
				if (o->get_last_action() != AtsType::OrderAction::Cancelled)
				{
					o->set_last_action(AtsType::OrderAction::Cancelled);
				}
				//loggerv2::info(" ::THOST_FTDC_OSS_CancelSubmitted isFromActiveBook");
#if 0
				if (!isNewlyCreated){
					if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
					{

						if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
						{
							//loggerv2::info("calling update_instr_on_ack_from_market_cb, because order rebuild time %s is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
							//if (!isFromDeadBook)
							update_instr_on_ack_from_market_cb(o);
						}

						on_cancel_from_market_cb(o);
						treat_freeze_order(o);
					}
				}
#else
				update_instr_on_cancel_from_market_cb(o);
				on_cancel_from_market_cb(o);
#endif
			}
			break;

			case SECURITY_FTDC_OSS_ModifySubmitted:
			{
				if (o->get_last_action() != AtsType::OrderAction::Modified)
				{
					o->set_last_action(AtsType::OrderAction::Modified);
				}
				if (!isNewlyCreated)
				{
					if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
					{
						//update_instr_on_ack_from_market_cb(o);

						if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
						{
							//loggerv2::info("calling update_instr_on_ack_from_market_cb, because order rebuild time %s is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
							//if (!isFromDeadBook)
							update_instr_on_ack_from_market_cb(o);
						}

						on_ack_from_market_cb(o);
						treat_freeze_order(o);
					}
				}
			}
			break;

			case SECURITY_FTDC_OSS_Accepted:
			{
				// FIX lts bug on resynchro
				if (pOrder->OrderStatus == SECURITY_FTDC_OST_Canceled || pOrder->OrderStatus == SECURITY_FTDC_OST_PartTradedNotQueueing || pOrder->OrderStatus == SECURITY_FTDC_OST_NoTradeNotQueueing) // '5','2','4'
				{
					if (o->get_last_action() == AtsType::OrderAction::Cancelled)
					{
						if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
						{
							//update_instr_on_ack_from_market_cb(o);

							if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
							{
								//loggerv2::info("calling update_instr_on_ack_from_market_cb, because order rebuild time %s is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
								//if (!isFromDeadBook)
								update_instr_on_ack_from_market_cb(o);
							}

							on_cancel_from_market_cb(o);
							treat_freeze_order(o);
						}
					}
					else

					{
						if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
						{
							//loggerv2::info("calling update_instr_on_cancel_from_market_cb, because order rebuild time %s is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
							//if (!isFromDeadBook)
							update_instr_on_cancel_from_market_cb(o);
						}


						on_cancel_from_market_cb(o);

						if (pOrder->OrderStatus == SECURITY_FTDC_OST_Canceled)
						{
							//m_statistics.incr_can();
							//loggerv2::info("lts_file_connection::OnRtnOrderAsync Current cancel number is %d", m_statistics.get_can());
						}
					}



					//earse the pointer from m_cancelOrdMap;
					auto it = m_cancelOrdMap.find(atoi(pOrder->OrderRef));
					if (it != m_cancelOrdMap.end())
						m_cancelOrdMap.erase(it);
				}



			}
			break;

			case SECURITY_FTDC_OSS_InsertRejected:
			{

				if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
				{

					update_instr_on_nack_from_market_cb(o);
				}

				on_nack_from_market_cb(o, pOrder->StatusMsg);
			}
			break;

			case SECURITY_FTDC_OSS_CancelRejected:
			{
				//update_instr_on_nack_from_market_cb(o);
				if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
				{
					//loggerv2::info("calling update_instr_on_nack_from_market_cb, because order rebuild time %S is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
					//update_instr_on_ack_from_market_cb(o);
					//if (!isFromDeadBook)
					update_instr_on_nack_from_market_cb(o);
				}
				on_nack_from_market_cb(o, pOrder->StatusMsg);
			}
			break;

			case SECURITY_FTDC_OSS_ModifyRejected:
			{
				//update_instr_on_nack_from_market_cb(o);
				if (o->get_rebuild_time() < o->get_instrument()->get_last_sychro_timepoint())
				{
					//loggerv2::info("calling update_instr_on_nack_from_market_cb, because order rebuild time %S is before tradeitem re-synchro time %s", o->get_rebuild_time().get_string(terra::common::date_time::date_format::ISO), o->get_instrument()->get_last_sychro_time().get_string(terra::common::date_time::date_format::ISO));
					//update_instr_on_ack_from_market_cb(o);
					//if (!isFromDeadBook)
					update_instr_on_nack_from_market_cb(o);
				}


				on_nack_from_market_cb(o, pOrder->StatusMsg);
			}
			break;


			default:
				break;
			}

		}


	}

	void lts_file_connection::OnRtnTradeAsync(CSecurityFtdcTradeField* pTrade)
	{
		//boost::lock_guard<boost::mutex> lock(m_mtrade);

		// 0 - log
		//loggerv2::info();
		//loggerv2::info("calling lts_file_connection::OnRtnTradeAsync...");
		if (m_debug)
			loggerv2::info("lts_file_connection::OnRtnTradeAsync : Dump Exec Info - "
			"InstrumentID[%*.*s]"
			"OrderRef[%*.*s]"
			"UserID[%*.*s] "
			"ExChgID[%*.*s]"
			"Direction[%c]"
			"TradeID[%*.*s]"
			//"OffsetFlag[%c]"
			"Price[%*.*s]"
			"Volume[%d]"
			"OrderLocalID[%*.*s]"
			"SequenceNo[%d]"
			//"SettlementID[%d]"
			"BrokerOrderSeq[%d] "
			"TradeDate[%s]"
			"TradeTime[%s]"
			"TradeType[%c]"
			"TradeIndex[%d]"
			,
			sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID,
			sizeof(pTrade->OrderRef), sizeof(pTrade->OrderRef), pTrade->OrderRef,
			sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID,
			sizeof(pTrade->ExchangeID), sizeof(pTrade->ExchangeID), pTrade->ExchangeID,
			pTrade->Direction,
			sizeof(pTrade->TradeID), sizeof(pTrade->TradeID), pTrade->TradeID,
			//pTrade->OffsetFlag,
			sizeof(pTrade->Price), sizeof(pTrade->Price), pTrade->Price,
			pTrade->Volume,
			sizeof(pTrade->OrderLocalID), sizeof(pTrade->OrderLocalID), pTrade->OrderLocalID,
			pTrade->SequenceNo,
			//pTrade->SettlementID,
			pTrade->BrokerOrderSeq,
			pTrade->TradeDate,
			pTrade->TradeTime,
			pTrade->TradeType,
			pTrade->TradeIndex
			);
		order* o = NULL;
		//order* o1 = NULL;
		bool isETFPurRed = (pTrade->Direction == SECURITY_FTDC_D_ETFPur) || (pTrade->Direction == SECURITY_FTDC_D_ETFRed);

		std::string sInstrCode = std::string(pTrade->InstrumentID) + "@" + this->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("lts_file_connection::OnRtnTradeAsync - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return;
		}

#if 0
		if (/*isETFPurRed && */instr->get_instr_type() == AtsType::InstrType::Stock)
		{
			//we should create new order
			loggerv2::info("lts_file_connection::OnRtnTradeAsync - order is ETF pur/red related execution - need to build new order");
			o = lts_file_order_aux::anchor(this, pTrade, true);
			if (o == NULL)
				return;
		}
		else
		{
#endif
			// 1 - retrieve order
			string userID(pTrade->UserID);
			if (strlen(pTrade->UserID) == 0)
			{
				userID = get_user_id(atoi(pTrade->OrderRef));				
			}
			int orderId = get_order_id(userID.c_str());
			if (orderId < 1)
			{
				orderId = FAKE_ID_MIN + atoi(pTrade->OrderRef);
			}
			int ret;
			o = get_order_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<lts_order*>(ord);//it->second);
				break;
			case 1:
				//o = reinterpret_cast<lts_order*>(ord);//it->second);
				loggerv2::warn("lts_file_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
				if (std::string(o->get_instrument()->getName()) != std::string(pTrade->InstrumentID) && (o->get_way() == AtsType::OrderWay::PLEDGE_BOND_IN || o->get_way() == AtsType::OrderWay::PLEDGE_BOND_OUT))
					return;
				break;
			case 2:
				if (/*isETFPurRed && */instr->get_instr_type() == AtsType::InstrType::Stock)
				{
					o = lts_file_order_aux::anchor(this, pTrade, true);
				}
				else
				{
					o = lts_file_order_aux::anchor(this, pTrade);
				}
				if (o == NULL)
				{
					loggerv2::error("lts_file_connection::OnRtnTradeAsync cannot anchor order");
					return;
				}
				add_pending_order(o);
				break;
			default:
				break;

			}




			if (o == NULL) // should not happen
			{
				if (m_debug)
					loggerv2::error("lts_file_connection::OnRtnTradeAsync - order recovered NULL");
				return;
			}
#if 0
		}
#endif
		// 2 - treat message
		int execQty = pTrade->Volume;
		double execPrc = strtod(pTrade->Price, NULL);

		std::string sExecNum = std::string(pTrade->TradeID);
		boost::trim_left(sExecNum);
		//std::string sExecRef = sExecNum + (o1 != nullptr ? std::to_string(o1->get_way()) : o != nullptr ? std::to_string(o->get_way()) : "?");
		//const char* pszExecRef = sExecRef.c_str();
		const char* pszTime = pTrade->TradeTime;

		exec* e;
		if (!isETFPurRed)
		{
			e = new exec(o, sExecNum, execQty, execPrc, pszTime);
			on_exec_from_market_cb(o, e);
		}

		else if (isETFPurRed && instr->get_instr_type() == AtsType::InstrType::ETF)
		{
			//if (std::string(instr->get_instrument_id()) == "510050")
			//	execQty = execQty * 900000;
			execQty *= instr->get_etf_unitisize();


			e = new exec(o, sExecNum, execQty, execPrc, pszTime);
			on_exec_from_market_cb(o, e);
		}

		else if (isETFPurRed && instr->get_instr_type() == AtsType::InstrType::Stock)
		{
			e = new exec(o, sExecNum, execQty, execPrc, pszTime);
			on_exec_from_market_cb(o, e);
		}

		//terra::common::date_time tradeTime;
		//tradeTime.set_date(pTrade->TradeDate, terra::common::date_time::FN2);
		//tradeTime.set_time(pszTime);

		lwtp tp = string_to_lwtp(from_undelimited_string(pTrade->TradeDate), (pTrade->TradeTime));
		tp = tp + std::chrono::seconds(2);


		if (!isETFPurRed || instr->get_instr_type() == AtsType::InstrType::ETF)
		{
			if (o->get_instrument()->get_last_sychro_timepoint() <= tp)
			{
				update_instr_on_exec_from_market_cb(o, e);
			}
		}

		else
		{
			if (o->get_instrument()->get_last_sychro_timepoint() <= tp)
			{
				update_instr_on_exec_from_market_cb(o, e);
			}
		}
	}

	void lts_file_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{
		CSecurityFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pltsReqApi->ReqQryInvestorPosition(&pRequest);

		if (m_debug)
			loggerv2::info("lts_file_connection::request_investor_position requesting investor position for tradeitem %s", i->get_trading_code());


		return;
	}

	void lts_file_connection::request_investor_full_positions()
	{
		CSecurityFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		m_pltsReqApi->ReqQryInvestorPosition(&pRequest);
		return;
	}



	OrderOpenClose::type lts_file_connection::compute_open_close(order* ord, bool hasCloseToday /*= false*/)
	{

		tradeitem* i = ord->get_instrument();

		if (i->get_instr_type() == AtsType::InstrType::Stock)
		{
			switch (ord->get_way())
			{
				case AtsType::OrderWay::Buy:
						return OrderOpenClose::Open;
						break;
				case AtsType::OrderWay::Sell:
						return OrderOpenClose::Close;
						break;
				default:
					break;
			}
		}

		if (!(i->get_instr_type() == AtsType::InstrType::Put || i->get_instr_type() == AtsType::InstrType::Call))
		{
			if (!(ord->get_way() == AtsType::OrderWay::Freeze || ord->get_way() == AtsType::OrderWay::Unfreeze))
			{
				return OrderOpenClose::Open;
			}
		}
		switch (ord->get_way())
		{
		case AtsType::OrderWay::Freeze:

			return OrderOpenClose::Open; //for underlying only
			break;

		case AtsType::OrderWay::Unfreeze:
			return OrderOpenClose::Close;//for underlying only
			break;
		case AtsType::OrderWay::CoveredBuy:
			return OrderOpenClose::Close;
			break;
		case AtsType::OrderWay::CoveredSell:
			return OrderOpenClose::Open;
			break;
		case AtsType::OrderWay::ETFPur:
			if (i->get_instr_type() == AtsType::InstrType::ETF)
				return OrderOpenClose::Open;
			return OrderOpenClose::Close;
			break;
		case AtsType::OrderWay::ETFRed:
			if (i->get_instr_type() == AtsType::InstrType::ETF)
				return OrderOpenClose::Close;
			return OrderOpenClose::Open;
			break;

		default:
			return connection::compute_open_close(ord, false);
			break;
		}

	}




	void lts_file_connection::treat_freeze_order(terra::marketaccess::orderpassing::order* ord)
	{

		if (ord->get_way() == AtsType::OrderWay::Freeze || ord->get_way() == AtsType::OrderWay::Unfreeze)
		{
			std::string sFlag = ord->get_way() == AtsType::OrderWay::Freeze ? "F" : "U";
			//lts_order* ltsOrd = dynamic_cast<lts_order*>(ord);
			std::string pszExecRef = std::to_string(ord->get_id()) + sFlag;
			//const char* pszTime = ord->get_last_time().get_string(terra::common::date_time::date_format::TI1).c_str();
			//int execQty = ord->get_quantity();
			//double execPrc = ord->get_price();
			exec* e = new exec(ord, pszExecRef.c_str(), ord->get_quantity(), ord->get_price(), lwtp_to_simple_time_string(ord->get_last_time()).c_str());
			on_exec_from_market_cb(ord, e);
		}
	}
	
	void lts_file_connection::cancel_num_warning(tradeitem* i)
	{

	}

	void lts_file_connection::cancel_num_ban(tradeitem* i)
	{

	}
	
	void lts_file_connection::init_user_info(char * user_info_file)
	{
		m_quant_proxy->init_user_info(user_info_file);
		//to do ...
		if (user_info_file == nullptr)
			return;
		boost::filesystem::path p;
		p.clear();
		p.append(user_info_file);
		p.append("user_info.csv");
		m_user_info_file_name = p.string();
		printf("lts_file_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
				int OrderID;
				int VolumeTotalOriginal;
				double LimitPrice;
			};*/
			user_info * info = nullptr;
			if (m_user_info_map.find(atoi(tokenizer[0])) == m_user_info_map.end())
			{
				info = new user_info();		
				//
				info->OrderID = atoi(tokenizer[0]);
				//
				m_user_info_map.emplace(info->OrderID, info);
			}
			else
			{
				info = m_user_info_map[atoi(tokenizer[0])];
			}
			//info->OrderID             = atoi(tokenizer[0]);
			info->VolumeTotalOriginal = atoi(tokenizer[1]);
			info->LimitPrice          = atof(tokenizer[2]);
			info->UserID              = tokenizer[3];
			info->OrderRef            = atoi(tokenizer[4]);
			//
			if (info->OrderRef > 0)
			{
				m_order_ref_map.emplace(info->OrderRef, info);
			}
			//			
		}
		stream.close();
	}
	void lts_file_connection::create_user_info(order * o)
	{
		if (o == nullptr)
			return;
		if (m_user_info_map.find(o->get_id()) == m_user_info_map.end())
		{
			user_info * info = new user_info();
			info->OrderID = o->get_id();
			info->VolumeTotalOriginal = o->get_quantity();
			info->LimitPrice = o->get_price();
			m_user_info_map.emplace(info->OrderID, info);
			//to do ... append the file every day
			m_userInfoQueue.CopyPush(info);
		}
		else
		{
			
		}
	}
	void lts_file_connection::update_user_info(int orderId, string userID, int orderRef)
	{	
		if (m_user_info_map.find(orderId) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[orderId];			
			//rewrite the file
			if (info->OrderRef != orderRef)
			{
				info->OrderRef = orderRef;
				info->UserID   = userID;
				m_userInfoQueue.CopyPush(info);				
				//			 
				m_order_ref_map.emplace(orderRef, info);
				//
			}					
		}
	}
	string lts_file_connection::get_user_id(int orderRef)
	{
		if (m_order_ref_map.find(orderRef) != m_order_ref_map.end())
		{
			user_info * info = m_order_ref_map[orderRef];
			return info->UserID;
		}
		return "";
	}
	void lts_file_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		//sprintf(buffer, "%d,%d,%f\n", info->OrderID,info->VolumeTotalOriginal, info->LimitPrice);
		sprintf(buffer, "%d,%d,%f,%s,%d\n", info->OrderID, info->VolumeTotalOriginal, info->LimitPrice,info->UserID.c_str(),info->OrderRef);
		stream << buffer;
		stream.close();
	}
	void lts_file_connection::OnUserInfoAsync(user_info* pInfo)//异步记录userinfo
	{
		this->append(pInfo);
	}
	void lts_file_connection::get_user_info_ex(order * o)
	{
		if (m_user_info_map.find(o->get_id()) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[o->get_id()];
			o->set_quantity(info->VolumeTotalOriginal);
			o->set_price(info->LimitPrice);
		}
	}
}

