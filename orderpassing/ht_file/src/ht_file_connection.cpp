#include "ht_file_connection.h"
#include "string_tokenizer.h"
#include <boost/algorithm/string.hpp>
#include "tradeItem_gh.h"
#include <boost/property_tree/ini_parser.hpp>
using namespace terra::common;
namespace ht_file
{
	ht_file_connection::ht_file_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName              = "ht_file_connection";
		m_quant_proxy        = new ht_quant_proxy();
		m_quant_proxy->m_con = this;
	}

	ht_file_connection::~ht_file_connection()
	{

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
	bool ht_file_connection::init_config(const std::string &name, const std::string &strConfigFile)
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
		init_process(io_service_type::trader, 10);
		m_bKey_with_exchange = false;
		return true;
	}
	void ht_file_connection::init_connection()
	{
		//m_outboundQueue.setHandler(boost::bind(&lts_file_connection::process_outbound_msg_cb, this));
		m_userInfoQueue.setHandler(boost::bind(&ht_file_connection::OnUserInfoAsync, this, _1));
	}
	void ht_file_connection::release()
	{		
		ctpbase_connection::release();		
	}

	void ht_file_connection::connect()
	{		
		if (this->m_quant_proxy->connect() == true)
		{
			on_status_changed(AtsType::ConnectionStatus::Connected);
		}
		else
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}	
	}

	void ht_file_connection::disconnect()
	{		
		if (this->m_quant_proxy->disconnect() == true)
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}
		//#endif
		this->m_quant_proxy->stop();
	}
	void ht_file_connection::request_instruments()
	{
		loggerv2::info("ht_file_connection::requesting instruments");
		m_database->open_database();		
	}


	void ht_file_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		//m_pltsTrdApi->Process();
		m_userInfoQueue.Pops_Handle(0);
	}

	int ht_file_connection::market_create_order_async(order* o, char* pszReason)
	{		
		if (m_quant_proxy)
		{
			this->create_user_info(o);
			/*
			输入文件格式为csv文件，包括下面几列：证券代码,证券名称,方向,数量,价格,备注
			local_entrust_no,fund_account,exchange_type,stock_code,entrust_bs,entrust_prop,entrust_price,entrust_amount,client_filed1,clientfield2
            1,               23076653,    1,            601866,    1,         0,           1.53,         200,           client_field1,
			*/
			char userID[64];
			memset(userID, 0, sizeof(userID));
			compute_userId(o, userID, sizeof(userID));
			m_quant_proxy->ReqOrderInsert(o->get_id(),o->get_instrument()->get_trading_code(), 
				o->get_instrument()->getInstrument()->get_exchange().c_str(), 
				o->get_way(), o->get_quantity(), 
				o->get_price(), o->get_restriction(), 
				o->get_open_close(), 
				o->get_price_mode(),userID);
		}	
		return 1;
	}


	int ht_file_connection::market_cancel_order_async(order* o, char* pszReason)
	{	
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());		

		if (o->get_instrument()->get_instr_type() == AtsType::InstrType::Stock)
		{
			if (m_quant_proxy)
			{
				//m_quant_proxy->cancel(nOrdRef);
			}
		}
		if (m_debug)
			loggerv2::info("order cancel sent ok! order_status is %d", o->get_status());
		
		return 1;
	}

	void ht_file_connection::OnRtnOrderAsync(result_orde& pOrder)
	{
		if (m_debug)
			loggerv2::info("ht_file_connection::OnRtnOrderAsync - "
			"file_name[%s] "
			"local_entrust_no[%s] "
			"batch_no[%s] "
			"entrust_no[%s]"
			"fund_account[%s] "
			"result[%s] "
			"remark[%s] "
			
			,
			pOrder.file_name.data(),
			pOrder.local_entrust_no.data(),
			pOrder.batch_no.data(),
			pOrder.entrust_no.data(),
			pOrder.fund_account.data(),
			pOrder.result.data(),
			pOrder.remark.data()

			);

		int orderId = boost::lexical_cast<int>(pOrder.local_entrust_no);
		int eId = boost::lexical_cast<int>(pOrder.entrust_no);
		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			break;
		case 1:
			loggerv2::info("xs_of_connection::OnRtnOrderAsyn - message received on dead order[%d]...", orderId);
			break;

		case 2:
			loggerv2::error("xs_of_connection::OnRtnOrderAsyn cannot anchor order :[%d]",orderId);
			return;
			
		default:
			return;
		}

		int ackQty = o->get_quantity();
		update_instr_on_ack_from_market_cb(o, ackQty);
		on_ack_from_market_cb(o);
		insert_spId2order(eId, orderId);
	}

	void ht_file_connection::OnRtnOrderHisAsync(result_orde& pOrder)
	{
		if (m_debug)
			loggerv2::info("ht_file_connection::OnRtnOrderHisAsync - "
			"file_name[%s] "
			"local_entrust_no[%s] "
			"batch_no[%s] "
			"entrust_no[%s]"
			"fund_account[%s] "
			"result[%s] "
			"remark[%s] "

			,
			pOrder.file_name.data(),
			pOrder.local_entrust_no.data(),
			pOrder.batch_no.data(),
			pOrder.entrust_no.data(),
			pOrder.fund_account.data(),
			pOrder.result.data(),
			pOrder.remark.data()

			);

		std::list<order *> mlist;
		std::string path = "./";
		ht_file_order_aux::anchor(this, &pOrder, path, mlist);
		int orderId = boost::lexical_cast<int>(pOrder.local_entrust_no);
		int eId = boost::lexical_cast<int>(pOrder.entrust_no);

		for (auto it : mlist)
		{
			int ackQty = it->get_quantity();
			update_instr_on_ack_from_market_cb(it, ackQty);
			on_ack_from_market_cb(it);
			insert_spId2order(eId, orderId);
		}
	}

	void ht_file_connection::OnRtnTradeAsync(result_trader &pTrade)
	{
		if (m_debug)
			loggerv2::info("ht_file_connection::OnRtnTradeAsync - "
			"id[%s] "
			"business_price[%s] "
			"fund_account[%s] "
			"entrust_no[%s]"
			"business_amount[%s] "
			"real_status[%s] "
			"stock_name[%s] "
			,
			pTrade.id.data(),
			pTrade.business_price.data(),
			pTrade.fund_account.data(),
			pTrade.entrust_no.data(),
			pTrade.business_amount.data(),
			pTrade.real_status.data(),
			pTrade.stock_name.data()
			);

		int eid = boost::lexical_cast<int>(pTrade.entrust_no);
		int oid = this->get_spId2order(eid);
		if (oid == -1)
		{
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - can not find eid %d", eid);
			return;
		}
		order *o = nullptr;

		int ret;
		o = get_order_from_map(oid, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			loggerv2::info("xs_of_connection::OnRtnCancelOrderAsyn - message received on dead order[%d]...", oid);
			break;
		default:
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - can not find oid %d", oid);
			break;
		}

		if (o == nullptr) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - order recovered NULL");
			return;
		}

		int execQty = boost::lexical_cast<int>(pTrade.business_amount);
		double execPrc = boost::lexical_cast<double>(pTrade.business_price);

		std::string sExecNum = std::string(pTrade.orig_order_id);
		boost::trim_left(sExecNum);

		exec* e;
		e = new exec(o, sExecNum, execQty, execPrc, pTrade.business_time.data());
		bool duplicat = false;
		on_exec_from_market_cb(o, e, duplicat);
		if (duplicat)
		{
			loggerv2::info("duplicat packet,drop");
			return;
		}

		update_instr_on_exec_from_market_cb(o, e, false);//这个API的报文全都不是历史回报

	}

	void ht_file_connection::OnRtnCancelOrderAsyn(result_cancel &pOrder)
	{
		if (m_debug)
			loggerv2::info("ht_file_connection::OnRtnCancelOrderAsyn - "
			"file_name[%s] "
			"local_withdraw_no[%s] "
			"fund_account[%s] "
			"entrust_no[%s]"
			"entrust_no_old[%s] "
			"result[%s] "
			"remark[%s] "
			,
			pOrder.file_name.data(),
			pOrder.local_withdraw_no.data(),
			pOrder.fund_account.data(),
			pOrder.entrust_no.data(),
			pOrder.entrust_no_old.data(),
			pOrder.result.data(),
			pOrder.remark.data()
			);


		int eid =boost::lexical_cast<int>(pOrder.entrust_no_old);
		int oid = this->get_spId2order(eid);
		if (oid == -1)
		{
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - can not find eid %d",eid);
			return;
		}
		order *o = nullptr;

		int ret;
		o = get_order_from_map(oid, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			loggerv2::info("xs_of_connection::OnRtnCancelOrderAsyn - message received on dead order[%d]...", oid);
			break;
		default:
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - can not find oid %d", oid);
			break;
		}
		
		if (o == nullptr) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - order recovered NULL");
			return;
		}

		int cancelQty = o->get_book_quantity();
		update_instr_on_cancel_from_market_cb(o, cancelQty);
		o->set_status(AtsType::OrderStatus::Cancel);
		on_cancel_from_market_cb(o);

	}

#if 0
	//
	// lts callbacks
	//
	void ht_file_connection::OnRspOrderInsertAsync(CSecurityFtdcInputOrderField* pOrder, int errorId)
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



	void ht_file_connection::OnRspOrderActionAsync(CSecurityFtdcInputOrderActionField* pOrder, int errorId)
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



	void ht_file_connection::request_trading_account()
	{
		if (m_debug)
			loggerv2::info("ht_file_connection:: calling ReqQryTradingAccount ");
		return;
	}





	void ht_file_connection::OnRtnTradeAsync(CSecurityFtdcTradeField* pTrade)
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
#endif
	void ht_file_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{
#if 0
		CSecurityFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pltsReqApi->ReqQryInvestorPosition(&pRequest);
		if (m_debug)
			loggerv2::info("lts_file_connection::request_investor_position requesting investor position for tradeitem %s", i->get_trading_code());
		return;
#endif
	}

	void ht_file_connection::request_investor_full_positions()
	{		
		return;
	}

	OrderOpenClose::type ht_file_connection::compute_open_close(order* ord, bool hasCloseToday /*= false*/)
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
	void ht_file_connection::init_user_info(char * user_info_file)
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
	void ht_file_connection::create_user_info(order * o)
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
	void ht_file_connection::update_user_info(int orderId, string userID, int orderRef)
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
	string ht_file_connection::get_user_id(int orderRef)
	{
		if (m_order_ref_map.find(orderRef) != m_order_ref_map.end())
		{
			user_info * info = m_order_ref_map[orderRef];
			return info->UserID;
		}
		return "";
	}
	void ht_file_connection::append(user_info * info)
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
	void ht_file_connection::OnUserInfoAsync(user_info* pInfo)//异步记录userinfo
	{
		this->append(pInfo);
	}
	void ht_file_connection::get_user_info_ex(order * o)
	{
		if (m_user_info_map.find(o->get_id()) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[o->get_id()];
			o->set_quantity(info->VolumeTotalOriginal);
			o->set_price(info->LimitPrice);
		}
	}

	void ht_file_connection::insert_spId2order(int eid, int oid)
	{
		tbb::concurrent_hash_map<int, int>::accessor wa;
		m_spId2orderId.insert(wa, eid);
		wa->second = oid;
	}

	int ht_file_connection::get_spId2order(int eid)
	{
		tbb::concurrent_hash_map<int, int>::const_accessor ra;
		if (m_spId2orderId.find(ra, eid))
		{
			return ra->second;
		}
		else
			return -1;
	}
}

