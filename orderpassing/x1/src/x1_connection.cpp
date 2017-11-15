#include "x1_connection.h"

#include <vector>
#include "tradeItem_gh.h"

#include <istream>
#include <fstream>
#include "order_reference_provider.h"
#include <sqlite3.h>
#include <boost/property_tree/ini_parser.hpp>


using namespace terra::common;

//#define POLL_NUM 10
namespace x1
{

	//Connect
	void x1_connection::init_connection()
	{
		m_sName = "x1_connection";

		m_orderRtnQueue.setHandler(boost::bind(&x1_connection::OnRtnOrderAsyn, this, _1));
		m_orderRspQueue.setHandler(boost::bind(&x1_connection::OnRspInsertOrderAsyn, this, _1));
		m_tradeQueue.setHandler(boost::bind(&x1_connection::OnRtnMatchedInfoAsyn, this, _1));
		m_ordCanRtnQueue.setHandler(boost::bind(&x1_connection::OnRtnCancelOrderAsyn, this, _1));

		loggerv2::info("x1_connection::init_connection create trader api..");
		m_pUserApi = CX1FtdcTraderApi::CreateCX1FtdcTraderApi();

#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
		time_duration dnow = microsec_clock::local_time().time_of_day();
		if (dnow.hours() < 16 && dnow.hours() > 4)
			m_bTsession = true;//开盘时间为T日
		else
			m_bTsession = false;//夜盘开盘，开盘时间T+1

		readLocal2Portfolio();
		m_timepoint = get_lwtp_now();
		//std::thread t(std::bind(&x1_connection::ackBackup, this));
		//t.detach();
		
	}

#ifdef Linux
	void  x1_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader,efd,std::bind(&x1_connection::process, this));
		m_orderRtnQueue.set_fd(efd);
		m_orderRspQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_ordCanRtnQueue.set_fd(efd);
		m_outboundQueue.set_fd(efd);
		m_outquoteboundQueue.set_fd(efd);
	}
#endif

	void x1_connection::connect()
	{
		loggerv2::info("calling x1_connection::connect");
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", m_sHostname.c_str(), m_sService.c_str());
			loggerv2::info("xs2_api::connect initializing api");
			int res = m_pUserApi->Init(addr, this,m_input_core,m_output_core);// TODO

			if (res != 0)
			{
				loggerv2::error("x1_connection::connect - api initalization failed,error %d", res);
				if (m_status != AtsType::ConnectionStatus::Disconnected)
					on_status_changed(AtsType::ConnectionStatus::Disconnected, "api initialization failed");
			}
			else
			{

			}
		}
		else
		{
			request_op_login();
		}
	}

	void x1_connection::release()
	{
		//is_alive(false);
		//m_thread.join();
		ctpbase_connection::release();
		m_pUserApi->Release();
	}

	void x1_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			//if (m_pUserApi->disconnect() == false)
			CX1FtdcReqUserLogoutField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.AccountID, m_sUsername.c_str());
			request.RequestID = m_nRequestId++;
			int res = m_pUserApi->ReqUserLogout(&request);
			if (res != 0)
			{
				loggerv2::error("x1_connection::disconnect failed");
			}
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "x1_connection - ReqUserLogout failed");
		}
	}

	void x1_connection::OnFrontConnected()
	{
		if (m_OnFrontConnected)
			return;
		m_OnFrontConnected = true;
		loggerv2::info("x1_connection::OnFrontConnected - x1_connection is UP");
		if (getStatus() != AtsType::ConnectionStatus::Connected)
		{
			loggerv2::info("x1_connection::OnFrontConnected subscribe private topic");
			//m_pUserApi->SubscribePrivateTopic(TERT_RESTART);
			loggerv2::info("x1_connection::OnFrontConnected,request user login");
			request_op_login();
		}
		else
		{
			loggerv2::info("x1_connection not asking for reconnect...");
		}
	}

	void x1_connection::OnFrontDisconnected(int nReason)
	{
		loggerv2::error("x1_connection::OnFrontDisconnected - x1_connection is down");
		char* pszMessage;
		switch (nReason)
		{
			// normal disconnection
		case 0x1001:
			pszMessage = "WLDSB";
			break;
		case 0x1000:
		default:
			pszMessage = "Unknown";
			break;
		}
		on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage);

	}

	void x1_connection::OnRtnErrorMsg(struct CX1FtdcRspErrorField* pErrorInfo)
	{
		std::cout << "OnRtnErrorMsg error msg:" << pErrorInfo->ErrorMsg << std::endl;
		loggerv2::error("x1_connection::OnRtnErrorMsg %s", pErrorInfo->ErrorMsg);
	}

	//login,logout
	void x1_connection::request_op_login()
	{
		struct CX1FtdcReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.AccountID, m_sUsername.c_str());
		strcpy(request.Password, m_sPassword.c_str());
		request.RequestID = m_nRequestId++;
		int res = m_pUserApi->ReqUserLogin(&request);
		if (res != 0)
		{
			loggerv2::error("x1_connection::request_op_login can not login,errorID:%d", res);
			cout << "errorID:res" << endl;
		}
		return;
	}

	void x1_connection::OnRspUserLogin(struct CX1FtdcRspUserLoginField * pUserLoginInfoRtn, struct CX1FtdcRspErrorField * pErrorInfo)
	{
		if (pErrorInfo != nullptr)
		{
			loggerv2::error("x1_connection::OnRspSOPUserLogin,error:%s", pErrorInfo->ErrorMsg);
			return;
		}

		if (pUserLoginInfoRtn != nullptr)
		{
			int temp_id = pUserLoginInfoRtn->InitLocalOrderID;
			if (temp_id > m_nCurrentOrderRef)
			{
				m_nCurrentOrderRef = pUserLoginInfoRtn->InitLocalOrderID;
			}
			loggerv2::info("x1_connection::OnRspSOPUserLogin localOrderId %d", pUserLoginInfoRtn->InitLocalOrderID);

			on_status_changed(AtsType::ConnectionStatus::Connected, "x1_connection::OnRspSOPUserLogin");
			loggerv2::info("x1_connection::OnRspSOPUserLogin login succeed");

			request_op_positions();
			OnQryOpOrders();
			//OnQryFutureOrders();
			//OnQryOpMatches();
			//OnQryFutureMatches();
			//todo comment this line later.
			if (this->getRequestInstruments() == true)
			{
				if (m_bRequestFuture == true)
				{
					requset_future_instruments();
				}
				else
				{
				requset_op_instruments();
				}
			}

		}
		else
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected, std::to_string(pErrorInfo->ErrorID).c_str());
			loggerv2::info("x1_connection::OnRspSOPUserLogin login failed error %d", pErrorInfo->ErrorID);
		}
	}
	void x1_connection::OnRspUserLogout(struct CX1FtdcRspUserLogoutInfoField * pUserLogoutInfoRtn, struct CX1FtdcRspErrorField * pErrorInfo)
	{
		if (pUserLogoutInfoRtn != nullptr)
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "x1_connection::OnRspUserLogout Receive Logout Msg");
		else
			loggerv2::error("x1_connection::OnRspSOPUserLogout logout failed ErrId[%d]", pErrorInfo->ErrorID);
	}

	//init
	x1_connection::x1_connection(const std::string &path,bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "x1_connection";
		m_nRequestId = 0;
		m_path = path;
		
		m_startTime = m_last_time;
		m_begin_Id = order_reference_provider::get_instance().get_current_int();
		m_nCurrentOrderRef = m_begin_Id;
	}

	bool x1_connection::init_config(const std::string &name, const std::string & strConfigFile)
	{
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		//if (strConfigFile.length() < 1)
		//	return false;

		boost::filesystem::path p(strConfigFile);
		if (!boost::filesystem::exists(p))
		{
			printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
			return false;
		}
		boost::property_tree::ptree root;
		boost::property_tree::ini_parser::read_ini(strConfigFile, root);
		//m_bRequestdico = root.get<bool>(name + ".req_dico", true);

		string Futures = root.get<string>(name + ".Futures", "");
		if (Futures == "")
		{
			m_bRequestFuture = false;
		}
		else
		{
			m_bRequestFuture = true;
		}

		m_input_core = root.get<int>(name + ".input_core", -1);
		m_output_core = root.get<int>(name + ".output_core", -1);
		cout << "X1 con " << " input core:" << m_input_core << " output_core:" << m_output_core << endl;

		xs_create_pool.init(32);
		xs_cancel_pool.init(32);
		quote_create_pool.init(32);
		quote_cancel_pool.init(32);

		std::list<CX1FtdcInsertOrderField*> mlist1;
		std::list<CX1FtdcCancelOrderField*> mlist2;

		unsigned int i = 0;
		while (i<xs_create_pool.mlen)
		{
			CX1FtdcInsertOrderField* ptr = xs_create_pool.get_mem();
			memset((void *)ptr, 0, sizeof(CX1FtdcInsertOrderField));
			strcpy(ptr->AccountID, m_sUsername.c_str());
			//xs_create_pool.free_mem(ptr);
			++i;
			mlist1.push_back(ptr);
		}

		i = 0;
		while (i<xs_cancel_pool.mlen)
		{
			CX1FtdcCancelOrderField* ptr2 = xs_cancel_pool.get_mem();
			memset((void *)ptr2, 0, sizeof(CX1FtdcCancelOrderField));
			strcpy(ptr2->AccountID, m_sUsername.c_str());
			//xs_cancel_pool.free_mem(ptr2);
			++i;
			mlist2.push_back(ptr2);
		}
		for (auto &it : mlist1)
			xs_create_pool.free_mem(it);
		for (auto &it : mlist2)
			xs_cancel_pool.free_mem(it);

		return true;
	}

	void x1_connection::readLocal2Portfolio()
	{		
		if (!boost::filesystem::exists(m_path))
		{
			boost::filesystem::create_directory(m_path);
		}
		std::string fn = m_path + "/x1Id2Msg.csv";
		std::ifstream f1(fn.data(), std::ios::in);
		if (f1.is_open())
		{
			std::string buf;
			std::vector<std::string>vec;
			while (getline(f1, buf))
			{
				boost::split(vec, buf, boost::is_any_of(":"));
				if (vec.size() < 5)
					continue;
				x1OrderMsg msg;
				msg.x1Id = boost::lexical_cast<int>(vec[0]);
				msg.orderId = boost::lexical_cast<int> (vec[1]);
				msg.accId = boost::lexical_cast<int> (vec[2]);
				msg.nTradingType = boost::lexical_cast<int> (vec[3]);
				msg.portfolio = vec[4];
				m_x1Id2Portfolio.insert(std::make_pair(msg.x1Id, msg));
			}
			f1.close();
		}

		std::string fn2 = m_path + "/localId2Msg.csv";
		std::ifstream f2(fn2.data(), std::ios::in);
		if (f2.is_open())
		{
			std::string buf;
			std::vector<std::string>vec;
			while (getline(f2, buf))
			{
				boost::split(vec, buf, boost::is_any_of(":"));
				if (vec.size() < 5)
					continue;
				localOrderMsg msg;
				msg.localId = boost::lexical_cast<int>(vec[0]);
				msg.orderId = boost::lexical_cast<int> (vec[1]);
				msg.accId = boost::lexical_cast<int> (vec[2]);
				msg.nTradingType = boost::lexical_cast<int> (vec[3]);
				msg.portfolio = vec[4];
				m_localId2Portfolio.insert(std::make_pair(msg.localId, msg));
			}
			f2.close();
		}

		cout << "load fin,size of m_x1Id2Portfolio is:" << m_x1Id2Portfolio.size()<<endl;
		cout << "load fin,size of m_localId2Portfolio is:" << m_localId2Portfolio.size() << endl;
		
	}

	void x1_connection::write_data2disk()
	{
		if (x1MsgQueue.empty() && localMsgQueue.empty())
			return;

		lwtp now = get_lwtp_now();
		if ((now - m_timepoint)<std::chrono::milliseconds(1000))
			return;

		m_timepoint = now;
		std::thread t(std::bind(&x1_connection::ackBackup, this));
		t.detach();
	}

	void x1_connection::ackBackup()
	{
		std::string fn = m_path + "/x1Id2Msg.csv";
		std::ofstream f1(fn.data(), std::ios::app);
		if (!f1.bad())
		{
			int tnum = 0;
			while (!x1MsgQueue.empty() || tnum < 20)
			{
				std::shared_ptr<x1OrderMsg>msg;
				if (x1MsgQueue.try_pop(msg))
				{
					f1 << msg->x1Id << ":" << msg->orderId << ":" << msg->accId << ":"
						<< msg->nTradingType << ":" << msg->portfolio << std::endl;
				}
				++tnum;
			}
			f1.close();
		}

		std::string fn2 = m_path + "/localId2Msg.csv";
		std::ofstream f2(fn2.data(), std::ios::app);
		if (!f2.bad())
		{
			int tnum = 0;
			while (!localMsgQueue.empty() || tnum < 20)
			{
				std::shared_ptr<localOrderMsg>msg;
				if (localMsgQueue.try_pop(msg))
				{
					f2 << msg->localId << ":" << msg->orderId << ":" << msg->accId << ":"
						<< msg->nTradingType << ":" << msg->portfolio << std::endl;
				}
				++tnum;
			}
			f2.close();
		}
	}


	void x1_connection::request_investor_full_positions()
	{
		if (last_req_fut_pos)
		{
			request_op_positions();
			last_req_fut_pos = false;

		}
		else
		{
			request_future_positions();
			last_req_fut_pos = true;
		}
		
	}

	void x1_connection::request_op_positions()
	{
		//loggerv2::info("");
		CX1FtdcQryPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.AccountID, m_sUsername.c_str());

		pRequest.RequestID = m_nRequestId++;
		pRequest.InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;
		if (m_pUserApi->ReqQryPosition(&pRequest) != 0)
			loggerv2::error("x1_connection::request_op_positions failed");
		//if (m_debug)
			//loggerv2::info("x1_connection:: calling OnRspQryInvestorPosition ");
	}

	void x1_connection::requset_op_instruments()
	{
		loggerv2::info("x1_connection::requset_op_instruments");
		CX1FtdcQryExchangeInstrumentField request;
		memset(&request, 0, sizeof(CX1FtdcQryExchangeInstrumentField));
		std::string exchangeID = "DCE";
		strcpy(request.AccountID, m_sUsername.c_str());                //客户号(Y)
		strcpy(request.ExchangeID, exchangeID.c_str());
		request.RequestID = m_nRequestId++;
		request.InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;

		m_database->open_database();
		int ret = m_pUserApi->ReqQryExchangeInstrument(&request);
		printf_ex("x1_connection::requset_op_instruments ret:%d\n", ret);
	}

	void x1_connection::request_future_positions()
	{
		CX1FtdcQryPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.AccountID, m_sUsername.c_str());

		pRequest.RequestID = m_nRequestId++;
		pRequest.InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;


		if (m_pUserApi->ReqQryPosition(&pRequest) != 0)
			loggerv2::error("x1_connection::request_future_positions failed");
		//if (m_debug)
			//loggerv2::info("x1_connection:: calling OnRspQryInvestorPosition ");
	}

	void x1_connection::requset_future_instruments()
	{
		loggerv2::info("x1_connection::requset_future_instruments");
		CX1FtdcQryExchangeInstrumentField request;
		memset(&request, 0, sizeof(CX1FtdcQryExchangeInstrumentField));
		strcpy(request.AccountID, m_sUsername.c_str());                //客户号(Y)

		request.RequestID = m_nRequestId++;
		request.InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;
		m_database->open_database();
		int ret = m_pUserApi->ReqQryExchangeInstrument(&request);
		printf_ex("x1_connection::requset_future_instruments ret:%d\n",ret);
	}

	void x1_connection::req_RiskDegree()
	{
		CX1FtdcQryCapitalField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.AccountID, m_sUsername.c_str());
		pRequest.RequestID = m_nRequestId++;

		if (m_pUserApi->ReqQryCustomerCapital(&pRequest) != 0)
			loggerv2::error("x1_connection::request_future_positions failed");
	}

	void x1_connection::OnRspCustomerCapital(struct CX1FtdcRspCapitalField* pCapitalInfoRtn, struct CX1FtdcRspErrorField* pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo!=nullptr)
			loggerv2::error("x1_connection::OnRspCustomerCapital failed,error:%s", pErrorInfo->ErrorMsg);
		else
		{
			this->set_RiskDegree(pCapitalInfoRtn->RiskDegree);
		}
	}

	void log_position(struct CX1FtdcRspPositionField *pData, bool bIsLast)
	{
		loggerv2::info("x1_connection::OnRspSOPQryPosition [ReqID]:[%d],[accountID]:[%s],[exchangeID]:[%s],[instrumentID]:[%s],[buySellType]:[%d],[openAvgPrice]:[%f],[positionAvgPrice]:[%f],[positionAmount]:[%ld],[totalAvaiAmount]:[%ld],"
					   					   "[todayAvaiAmount]:[%ld],[lastAvaiAmount]:[%ld],[todayAmount]:[%ld],[lastAmount]:[%ld],[tradingAmount]:[%ld],[datePositionProfitLoss]:[%f],[dateCloseProfitLoss]:[%f],[floatProfitLoss]:[%f],[dMargin]:[%f],[speculator]:[%d],[clientID]:[%s],"
										   					   "[preSettlementPrice]:[%f],[instrumentType]:[%d],[dPremium]:[%f],[isLast]:[%d]",
															   pData->RequestID, //ReqID
															   //pData->counterID,  //柜台编号
															   pData->AccountID,  //资金帐号ID
															   pData->ExchangeID, //交易所编码
															   pData->InstrumentID,//InstrumentID
															   pData->BuySellType,//买卖
															   pData->OpenAvgPrice,//开仓均价
															   pData->PositionAvgPrice,//持仓均价
															   pData->PositionAmount,//持仓量
															   pData->TotalAvaiAmount,//总可用
															   pData->TodayAvaiAmount,//今可用
															   pData->LastAvaiAmount, //昨可用
															   pData->TodayAmount,    //今仓
															   pData->LastAmount,     //昨仓
															   pData->TradingAmount,  //挂单量
															   pData->DatePositionProfitLoss, // 盯市持仓盈亏
															   pData->DateCloseProfitLoss,   // 盯市平仓盈亏
															   pData->ProfitLoss,        //浮动盈亏
															   pData->Margin,                // 占用保证金
															   pData->Speculator,             //投保类别
															   pData->ClientID,               //交易编码
															   pData->PreSettlementPrice,              //昨结算价
															   pData->InstrumentType,        //合约类型
															   pData->Premium,          //权利金 
															   bIsLast);
	}

	void x1_connection::OnRspQryPosition(struct CX1FtdcRspPositionField *pData, struct CX1FtdcRspErrorField *pRspInfo, bool bIsLast)
	{
		if (pData == nullptr)
		{
			loggerv2::error("x1_connection::OnRspSOPQryPosition errorid %d,%s", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			return;
		}

		if (bIsLast == true)
		{
			if (this->getRequestInstruments() == true)
			{
				sleep_by_milliseconds(2000);
				if (m_bRequestFuture == true)
				{
					requset_future_instruments();
				}
				else
				{
					requset_op_instruments();
				}				
			}
		}

		std::string instr = pData->InstrumentID;
		if (instr.size()==0)
		{
			return;
		}
		
		if (pData->InstrumentID)
		{

			//std::string instr = std::string(pData->InstrumentID) + "." + std::string(pData->ExchangeID);
			std::string instr = std::string(pData->InstrumentID) + "." + std::string(pData->ExchangeID) + "@" + getName();

			//std::string instr = std::string(pData->contractID) + "@" + getName();
			auto con = tradeitem_gh::get_instance().container();
			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(instr.c_str());
			if (i)
			{
				//loggerv2::info("lts_api::OnRspQryInvestorPosition found tradeitem %s", pData->InstrumentID);
				if (pData->BuySellType == X1_FTDC_SPD_BUY)//long
				{
					long freezeQty = pData->PositionAmount - pData->TotalAvaiAmount;
					if (i->get_today_long_position() != pData->TodayAmount|| i->get_tot_long_position()!=pData->PositionAmount  || freezeQty != i->get_pending_short_close_qty())
					{
						if (m_debug)
							i->dumpinfo();
						i->set_tot_long_position(pData->PositionAmount);//总持仓
						i->set_today_long_position(pData->TodayAmount);//今仓
						i->set_yst_long_position(pData->PositionAmount - pData->TodayAmount);//昨仓
						i->set_pending_short_close_qty(freezeQty);//总挂单 = 总持仓 - 总可用
						i->set_pending_short_close_today_qty(pData->TodayAmount - pData->TodayAvaiAmount);//今日挂单 = 今仓 - 今可用
						log_position(pData, bIsLast);
						if (m_debug)
							i->dumpinfo();
					}	
				}


				//3+4 = total short position in options
				else if (pData->BuySellType == X1_FTDC_SPD_SELL)//short，字段含义同上
				{
					long freezeQty = pData->PositionAmount - pData->TotalAvaiAmount;
					if (i->get_today_short_position() != pData->TodayAmount || i->get_tot_short_position() != pData->PositionAmount || freezeQty != i->get_pending_long_close_qty())
					{
						if (m_debug)
							i->dumpinfo();
						i->set_tot_short_position(pData->PositionAmount);
						i->set_today_short_position(pData->TodayAmount);
						i->set_yst_short_position(pData->PositionAmount - pData->TodayAmount);
						i->set_pending_long_close_qty(freezeQty);
						i->set_pending_long_close_today_qty(pData->TodayAmount - pData->TodayAvaiAmount);
						log_position(pData, bIsLast);
						if (m_debug)
							i->dumpinfo();
					}
									
				}
				i->set_last_sychro_timepoint(get_lwtp_now());

			}
			else
				loggerv2::warn("x1_connection::OnRspSOPQryPosition cannot find tradeitem %s", pData->InstrumentID);
		}



	}
	void x1_connection::OnRspQryExchangeInstrument(struct CX1FtdcRspExchangeInstrumentField *pData, struct CX1FtdcRspErrorField *pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != nullptr)
		{
			loggerv2::error("OnRspQryExchangeInstrument Error,msg:%s", pErrorInfo->ErrorMsg);
			return;
		}		
		if (pData == nullptr)
		{
			loggerv2::info("交易所合约查询响应发生错误![accountID]:[%s],[会话ID]:[%ld],[LocalOrderID]:[%ld],[柜台委托号]:[%ld],[ReqID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->AccountID,
				pErrorInfo->SessionID,
				pErrorInfo->LocalOrderID,
				pErrorInfo->X1OrderID,
				pErrorInfo->RequestID,
				pErrorInfo->ErrorID,
				pErrorInfo->ErrorMsg
				);
		}
		else
		{
			loggerv2::info("交易所合约查询响应[RequestID]:[%d],[exchangeID]:[%s],[instrumentID]:[%s],[品种]:[%s],[是否结束]:[%d]\n",
				pData->RequestID,
				pData->ExchangeID,
				pData->InstrumentID,
				pData->VarietyName,
				bIsLast);
		}
		std::string sInstr = std::string(pData->InstrumentID);
		boost::trim(sInstr);
		std::string sCmd, sCP;
		std::string sUnderlying = pData->Underlying;
		std::string sExcge = pData->ExchangeID;
		std::string sMat = getMaturity(pData->InstrumentMaturity);

		std::string prefix = pData->InstrumentPrefix;
		std::string sInstClass;
		if (pData->InstrumentType == X1FTDC_INSTRUMENT_TYPE_COMM)
			sInstClass = "F_" + boost::to_upper_copy(prefix);
		else
			sInstClass = "O_" + boost::to_upper_copy(prefix);

		{
			std::string sSearch = "select * from InstrumentClass where ClassName= '" + sInstClass + "'";
			char *zErrMsg = 0;
			std::string TickRule = "0_" + std::to_string(pData->MinPriceFluctuation);
			std::string PointValue = std::to_string(pData->ContractMultiplier);
			std::string Cur = "CNY";

			std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());
			if (!prefix.empty() && pTree->size() == 0)
			{
				sqlite3_free(zErrMsg);
				sCmd = "INSERT INTO InstrumentClass VALUES (";
				sCmd += "'" + sInstClass + "',";
				sCmd += "'" + TickRule + "',";
				sCmd += "'" + PointValue + "',";
				sCmd += "'" + Cur + "',";
				sCmd += "' ',";
				sCmd += "' ')";
				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					//loggerv2::info("failed to insert into database, ret is %d",rc);
					sqlite3_free(zErrMsg);
				}
			}

		}

		if (pData->InstrumentType == X1FTDC_INSTRUMENT_TYPE_COMM)
		{
			std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";
			char *zErrMsg = 0;

			std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

			if (pTree->size() == 0) //tradeitem doesn't exist
			{
				//loggerv2::info("Could not find tradeitem %s", std::string(pInstrument->InstrumentID).c_str());
				sqlite3_free(zErrMsg);

				sCmd = "INSERT INTO Futures VALUES (";
				sCmd += "'" + sInstr + "',";
				sCmd += "'" + sExcge + "',";
				sCmd += "' ',";
				sCmd += "' ',";				
				sCmd += "'" + sInstr + "@CTP|" + sInstr + "@XS2" + "',";
				sCmd += "'" + sInstr + "@CTP|" + sInstr + "@XS2" + "',";
				sCmd += "'" + boost::to_upper_copy(prefix) + "',";
				sCmd += "'" + sMat + "',";
				sCmd += "'" + sInstClass + "')";

				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					//loggerv2::info("failed to insert into database, ret is %d",rc);
					sqlite3_free(zErrMsg);
				}

			}

			else //exists
			{
				sqlite3_free(zErrMsg);
				std::string sConnectionCodes = sInstr + "." + sExcge + "@"+get_type();
				sCmd = "UPDATE Futures SET ";
				sCmd += "Code = '" + sInstr + "',";
				sCmd += "Exchange = '" + sExcge + "',";
				sCmd += "ISIN = ' ',";
				sCmd += "RIC = ' ',";				
				sCmd += "FeedCodes ='" + sInstr + "@CTP|" + sInstr + "@XS2" + "',";
				sCmd += "ConnectionCodes='" + sInstr + "@CTP|" + sInstr + "@XS2" + "',";
				sCmd += "Underlying = '" + boost::to_upper_copy(prefix) + "',";
				sCmd += "Maturity = '" + sMat + "',";
				sCmd += "InstrumentClass = '" + sInstClass + "'";
				sCmd += " where Code='" + sInstr + "';";

				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					//loggerv2::info("failed to update the database,error is %d",rc);
					sqlite3_free(zErrMsg);
				}
			}
		}
		else if (pData->InstrumentType == X1FTDC_INSTRUMENT_TYPE_OPT)
		{
			std::string sSearch = "select * from Options where Code= '" + sInstr + "'";
			char *zErrMsg = 0;

			if (sInstr.find("-C-") != std::string::npos)
			{
				sCP = "C";
			}
			else if (sInstr.find("-P-") != std::string::npos)
			{
				sCP = "P";
			}
			else
			{
				return;
			}

			std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

			if (pTree->size() == 0) //tradeitem doesn't exist
			{
				//loggerv2::info("Could not find tradeitem %s", std::string(pInstrument->InstrumentID).c_str());
				sqlite3_free(zErrMsg);

				sCmd = "INSERT INTO Options VALUES (";
				sCmd += "'" + sInstr + "',";
				sCmd += "'" + sExcge + "',";
				sCmd += "'" + sInstr + "',";
				sCmd += "' ',";				
				sCmd += "'" + sInstr + "@XS2|" + sInstr + "@CTP" + "',";
				sCmd += "'" + sInstr + "." + sExcge + "@XS2|" + sInstr + "@CTP" + "',";
				sCmd += "'" + sUnderlying + "',";
				sCmd += "'" + sMat + "',";
				sCmd += "'" + std::to_string(pData->StrikePrice) + "',";
				sCmd += "'" + std::to_string(pData->ContractMultiplier) + "',";
				sCmd += "'" + sCP + "',";
				sCmd += "'" + sInstClass + "')";

				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					//loggerv2::info("failed to insert into database, ret is %d",rc);
					sqlite3_free(zErrMsg);
				}

			}

			else //exists
			{
				sqlite3_free(zErrMsg);				
				std::string sConnectionCodes = sInstr + "." + sExcge + "@XS2|" + sInstr + "@CTP";
				sCmd = "UPDATE Options SET ";
				sCmd += "Code = '" + sInstr + "',";
				sCmd += "ISIN = '" + sInstr + "',";
				sCmd += "Maturity = '" + sMat + "',";
				sCmd += "Strike = '" + std::to_string(pData->StrikePrice) + "',";
				sCmd += "PointValue ='" + std::to_string(pData->ContractMultiplier) + "',";				
				sCmd += "FeedCodes ='" + sInstr + "@XS2|" + sInstr + "@CTP" + "',";
				sCmd += "ConnectionCodes='" + sConnectionCodes + "'";
				sCmd += " where Code='" + sInstr + "';";

				int rc = m_database->executeNonQuery(sCmd.c_str());

				if (rc == 0)
				{
					//loggerv2::info("failed to update the database,error is %d",rc);
					sqlite3_free(zErrMsg);
				}
			}
		}
		if (bIsLast)
		{
			//m_bIsDicoRdy = true;
			m_database->close_databse();
			this->set_is_last(true);
		}
	}
	
	void x1_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_orderRspQueue.Pops_Handle(0);
		m_orderRtnQueue.Pops_Handle(0);
		m_tradeQueue.Pops_Handle(0);
		m_ordCanRtnQueue.Pops_Handle(0);
		m_outquoteboundQueue.Pops_Handle_Keep(10);
		write_data2disk();

#ifdef Linux
		bool vaild = m_outboundQueue.read_available() || m_orderRspQueue.read_available() || m_orderRtnQueue.read_available()
			|| m_tradeQueue.read_available() || m_ordCanRtnQueue.read_available() || m_outquoteboundQueue.read_available();
		if (vaild)
		{
			uint64_t buf = 1;
			int wlen = 0;
			while(1)
			{
				wlen = write(efd, &buf, sizeof(buf));
				if (wlen >= 0)
					break;
				else
				{
					if (errno == EAGAIN || errno == EINTR)
					{
						continue;
					}
					else
					{
						loggerv2::error("write efd fail");
						break;
					}
				}
			}
		}
#endif
	}

	int x1_connection::market_create_order_async(order* o, char* pszReason)
	{
		if (o == NULL)
		{
			return 0;
		}

		CX1FtdcInsertOrderField* request = xs_create_pool.get_mem();//xs_create_pool.get_mem();//new DFITCInsertOrderField;// 

		//memset(request, 0, sizeof(DFITCInsertOrderField));
		//strcpy(request->AccountID, m_sUsername.c_str());

		request->LocalOrderID = ++m_nCurrentOrderRef;

		tradeitem* i = o->get_instrument();
		if (i == nullptr)
		{
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}
		switch (i->get_instr_type())
		{
		case AtsType::InstrType::Future:
			request->InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;
			break;
		case AtsType::InstrType::Call:
		case AtsType::InstrType::Put:
			request->InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;
			break;
		default:
		{
			xs_create_pool.free_mem(request);
			//delete request;
			snprintf(pszReason, REASON_MAXLENGTH, " incorrect tradeitem type\n");
			o->set_status(AtsType::OrderStatus::Reject);
			return 0;
		}
		break;
		}

		//strcpy(request->AccountID, m_sUsername.c_str());
		memset(request->InstrumentID, 0, sizeof(request->InstrumentID));
		strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());

		request->InsertType = 1;
		request->OrderAmount = o->get_quantity();
		request->InsertPrice = o->get_price();

		if (o->get_way() == AtsType::OrderWay::Buy)
			request->BuySellType = X1_FTDC_SPD_BUY;
		else if (o->get_way() == AtsType::OrderWay::Sell)
			request->BuySellType = X1_FTDC_SPD_SELL;

		if (o->get_restriction() == AtsType::OrderRestriction::None)
			request->OrderProperty = X1_FTDC_SP_NON; // or GFS ???
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
			request->OrderProperty = X1_FTDC_SP_FOK;
		else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			request->OrderProperty = X1_FTDC_SP_FAK;
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}

		TX1FtdcOpenCloseTypeType oc = X1_FTDC_SPD_OPEN;
		if (o->get_open_close() == OrderOpenClose::Undef)
		{
			o->set_open_close(compute_open_close(o, m_bCloseToday));
		}
		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			break;

		case AtsType::OrderOpenClose::Close:
			oc = X1_FTDC_SPD_CLOSE;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			oc = X1_FTDC_SPD_CLOSETODAY;
			break;

		default:
			break;


		}

		if (o->get_way() == AtsType::OrderWay::Exercise)
			oc = X1_FTDC_SPD_EXECUTE;

		request->OpenCloseType = oc;
		if (m_debug)
			loggerv2::info("x1_connection::market_create_order openCloseFlag is %d,order_way is:%d", oc, o->get_way());


		if (!compute_userId(o, request->CustomCategory, sizeof(request->CustomCategory)))
		{
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}
		loggerv2::info("market_crate_order reqest.devDecInfo [%s],localID [%d]", request->CustomCategory, request->LocalOrderID);

		switch (o->get_price_mode())
		{
		case AtsType::OrderPriceMode::Limit:
		{
			request->OrderType = X1_FTDC_LIMITORDER;
			request->InsertPrice = o->get_price();
		}
		break;
		case AtsType::OrderPriceMode::Market:
		{
			request->OrderType = X1_FTDC_MKORDER;
		}
		break;
		default:
		{
			snprintf(pszReason, REASON_MAXLENGTH, "undefined price mode.\n");
			o->set_status(AtsType::OrderStatus::Reject);
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}
		}

		request->RequestID = m_nRequestId++;

		x1_order_aux::set_locId(o, request->LocalOrderID);
		insert_localId2order(request->LocalOrderID,o);

		std::shared_ptr<localOrderMsg> localmsg(new localOrderMsg());
		localmsg->localId = request->LocalOrderID;
		localmsg->portfolio = o->get_portfolio();
		localmsg->orderId = o->get_id();
		localmsg->accId = o->get_account_num();
		localmsg->nTradingType = o->get_trading_type();
		localMsgQueue.push(localmsg);

		int res = m_pUserApi->ReqInsertOrder(request);
		if (res != 0)
		{
			snprintf(pszReason, REASON_MAXLENGTH, "api returns %d", res);
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}
		xs_create_pool.free_mem(request);
		//delete request;
		return 1;
	}

	int x1_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());
		//x1_order* o = dynamic_cast<x1_order*>(ord);
		if (o == NULL)
		{
			//snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to x1_order*...\n");
			//ord->set_status(AtsType::OrderStatus::Nack);
			//ord->rollback();
			return 0;
		}

		tradeitem* i = o->get_instrument();
		if (i == nullptr)
			return 0;


		CX1FtdcCancelOrderField* request = xs_cancel_pool.get_mem(); //new DFITCCancelOrderField;//xs_cancel_pool.get_mem();
		
		//memset(request, 0, sizeof(DFITCCancelOrderField));
		//strcpy(request->AccountID, m_sUsername.c_str());
		memset(request->InstrumentID, 0, sizeof(request->InstrumentID));
		strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());

		request->X1OrderID = x1_order_aux::get_spdId(o);
		request->RequestID = m_nRequestId++;
		//request->LocalOrderID = abs(x1_order_aux::get_locId(o));

		loggerv2::info("market_cancel_of_order,id:%d,num:%d", o->get_id(), o->get_quantity());

		if (m_pUserApi->ReqCancelOrder(request) != 0)
		{
			snprintf(pszReason, REASON_MAXLENGTH, "api reject\n");
			o->set_status(AtsType::OrderStatus::Nack);
			xs_cancel_pool.free_mem(request);
			//delete request;
			return 0;
		}
		xs_cancel_pool.free_mem(request);
		//delete request;
		return 1;
	}

	//quote
	int x1_connection::market_create_quote_async(quote* q, char* pszReason)
	{
		
		CX1FtdcQuoteInsertField *request = quote_create_pool.get_mem();
		memset(request, 0, sizeof(CX1FtdcQuoteInsertField));

		strcpy(request->AccountID, m_sUsername.c_str());
		strcpy(request->InstrumentID, q->get_instrument()->get_trading_code());
		strcpy(request->QuoteID, q->get_FQR_ID().c_str());
		request->InsertType = 1;
		request->LocalOrderID = ++m_nCurrentOrderRef;
		request->RequestID = m_nRequestId++;

		request->SellInsertPrice = q->get_ask_order()->get_price();
		request->BuyInsertPrice = q->get_bid_order()->get_price();

		request->SellOrderAmount = q->get_ask_order()->get_quantity();
		request->BuyOrderAmount = q->get_bid_order()->get_quantity();

		if (q->get_bid_order()->get_open_close() == OrderOpenClose::Undef)
		{
			q->get_bid_order()->set_open_close(compute_open_close(q->get_bid_order(), m_bCloseToday));
		}

		switch (q->get_bid_order()->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			request->BuyOpenCloseType = X1_FTDC_SPD_OPEN;
			break;

		case AtsType::OrderOpenClose::Close:
			request->BuyOpenCloseType = X1_FTDC_SPD_CLOSE;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->BuyOpenCloseType = X1_FTDC_SPD_CLOSETODAY;
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
			request->SellOpenCloseType = X1_FTDC_SPD_OPEN;
			break;

		case AtsType::OrderOpenClose::Close:
			request->SellOpenCloseType = X1_FTDC_SPD_CLOSE;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->SellOpenCloseType = X1_FTDC_SPD_CLOSETODAY;
			break;

		default:

			break;
		}

		request->SellSpeculator = X1_FTDC_SPD_SPECULATOR;
		request->BuySpeculator = X1_FTDC_SPD_SPECULATOR;

		if (!compute_userId(q, request->CustomCategory, sizeof(request->CustomCategory)))
		{
			quote_create_pool.free_mem(request);
			return 0;
		}

		if (!m_pUserApi->ReqQuoteInsert(request))
		{


			snprintf(pszReason, REASON_MAXLENGTH, "cffex api reject!\n");
			quote_create_pool.free_mem(request);
			return 0;
		}
		quote_create_pool.free_mem(request);

		return 1;
	}

	int x1_connection::market_cancel_quote_async(quote* q, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_quote_async : %d", q->get_id());


		CX1FtdcCancelOrderField *request = quote_cancel_pool.get_mem();
		memset(request, 0, sizeof(CX1FtdcCancelOrderField));

		strcpy(request->AccountID, m_sUsername.c_str());
		request->X1OrderID = x1_order_aux::get_spdId(q);
		request->RequestID = m_nRequestId++;
		request->LocalOrderID = ++m_nCurrentOrderRef;

		if (!m_pUserApi->ReqQuoteCancel(request))
		{
			quote_cancel_pool.free_mem(request);
			return 0;
		}

		//
		quote_cancel_pool.free_mem(request);
		return 1;
	}

	void x1_connection::OnRtnQuoteInsert(struct CX1FtdcQuoteRtnField * pRtnQuoteData)
	{
		if (m_debug)
			loggerv2::info("x1_connection::OnRtnQuoteInsert-",
			"X1OrderID[%d]"
			"LocalOrderID[%d]"
			"QuoteID[%s]"
			"InstrumentID[%s]"
			"InstrumentType[%d]"
			"BuyOpenCloseType[%d]"
			"BuyOrderAmount[%d]"
			"SellOpenCloseType[%d]"
			"SellOrderAmount[%d]"
			"StatusMsg[%s]"
			"ErrorID[%d]"
			"CustomCategory[%s]"
			"OrderStatus[%s]"
			,
			pRtnQuoteData->X1OrderID,
			pRtnQuoteData->LocalOrderID,
			pRtnQuoteData->QuoteID,
			pRtnQuoteData->InstrumentID,
			pRtnQuoteData->InstrumentType,
			pRtnQuoteData->BuyOpenCloseType,
			pRtnQuoteData->BuyOrderAmount,
			pRtnQuoteData->SellOpenCloseType,
			pRtnQuoteData->SellOrderAmount,
			pRtnQuoteData->StatusMsg,
			pRtnQuoteData->ErrorID,
			pRtnQuoteData->CustomCategory,
			pRtnQuoteData->OrderStatus
			);

		int account, bidId, askId, portfolioId, tradingType;
		get_user_info(pRtnQuoteData->CustomCategory, account, bidId, askId, portfolioId, tradingType);

		int orderId = bidId;
		bool isRebuild = false;
		if (m_begin_Id >= orderId)
			isRebuild = true;

		quote *q = nullptr;
		if (orderId <= 0)
		{
			q = get_localId2quote(pRtnQuoteData->LocalOrderID);
		}
		else
		{
			int ret;
			quote *q = get_quote_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<cffex_order*>(ord);
				break;
			case 1:
				//o = reinterpret_cast<cffex_order*>(ord);
				loggerv2::info("x1_connection::OnRtnQuoteAsync - message received on dead quote[%d]...", orderId);
				break;

			case 2:

				q = x1_order_aux::anchor(this, pRtnQuoteData);
				if (q == NULL)
				{
					loggerv2::error("x1_connection::OnRtnQuoteAsync cannot anchor quote");
					return;
				}

				add_pending_quote(q);
				break;
			default:
				break;
			}
		}
		if (q == NULL) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnQuoteInsert - quote recovered NULL");
			return;
		}

		std::string FQR_ID = std::to_string(pRtnQuoteData->X1OrderID);
		q->set_FQR_ID(FQR_ID);

		x1_order_aux::set_spdId(q, pRtnQuoteData->X1OrderID);
		if (q->get_bid_order() && q->get_bid_order()->get_quantity() != pRtnQuoteData->BuyOrderAmount)
		{
			if (m_debug)
				loggerv2::debug("x1_connection::OnRtnQuoteInsert resetting bid order quantity to %d", pRtnQuoteData->BuyOrderAmount);
			q->get_bid_order()->set_quantity(pRtnQuoteData->BuyOrderAmount);
		}

		if (q->get_ask_order() && q->get_ask_order()->get_quantity() != pRtnQuoteData->SellOrderAmount)
		{
			if (m_debug)
				loggerv2::debug("x1_connection::OnRtnQuoteInsert resetting ask order quantity to %d", pRtnQuoteData->SellOrderAmount);
			q->get_ask_order()->set_quantity(pRtnQuoteData->SellOrderAmount);
		}

		switch (pRtnQuoteData->OrderStatus)
		{
		case X1_FTDC_SPD_TRIGGERED:
		case X1_FTDC_SPD_IN_QUEUE:
		case X1_FTDC_SPD_PLACED:
		case X1_FTDC_SPD_PARTIAL:
		{
			if (isRebuild == false)//历史回包不参与pending计算
			{
				update_instr_on_ack_from_market_cb(q->get_bid_order());
				update_instr_on_ack_from_market_cb(q->get_ask_order());
			}
			on_ack_quote_from_market_cb(q);
			break;
		}
		case X1_FTDC_SPD_FILLED: //    
			break;
		case X1_FTDC_SPD_ERROR:
		{
			on_nack_quote_from_market_cb(q, "OrderRtn api reject");
			break;
		}
		case X1_FTDC_SPD_PARTIAL_CANCELED:
		case X1_FTDC_SPD_CANCELED:
		{
			q->set_last_action(AtsType::OrderAction::Cancelled);
			if (isRebuild == false)
			{
				update_instr_on_ack_from_market_cb(q->get_bid_order());
				update_instr_on_ack_from_market_cb(q->get_ask_order());
			}
			on_ack_quote_from_market_cb(q);
		}
		break;

		default:
			break;
		}

	}

	void x1_connection::OnRtnQuoteCancel(struct CX1FtdcQuoteCanceledRtnField * pRtnQuoteCanceledData)
	{
		if (m_debug)
			loggerv2::info("x1_connection::OnRtnQuoteCancel-",
			"X1OrderID[%d]"
			"LocalOrderID[%d]"
			"QuoteID[%s]"
			"InstrumentID[%s]"
			"CancelAmount[%d]"
			"BuyOpenCloseType[%d]"
			"OrderStatus[%s]"
			"SellOpenCloseType[%d]"
			"CanceledTime[%s]"
			"StatusMsg[%s]"
			"ErrorID[%d]"
			"CustomCategory[%s]"
			,
			pRtnQuoteCanceledData->X1OrderID,
			pRtnQuoteCanceledData->LocalOrderID,
			pRtnQuoteCanceledData->QuoteID,
			pRtnQuoteCanceledData->InstrumentID,
			pRtnQuoteCanceledData->CancelAmount,
			pRtnQuoteCanceledData->BuyOpenCloseType,
			pRtnQuoteCanceledData->OrderStatus,
			pRtnQuoteCanceledData->SellOpenCloseType,
			pRtnQuoteCanceledData->CanceledTime,
			pRtnQuoteCanceledData->StatusMsg,
			pRtnQuoteCanceledData->ErrorID,
			pRtnQuoteCanceledData->CustomCategory
			);

		int account, bidId, askId, portfolioId, tradingType;
		get_user_info(pRtnQuoteCanceledData->CustomCategory, account, bidId, askId, portfolioId, tradingType);

		int orderId = bidId;
		bool isRebuild = false;
		if (m_begin_Id >= orderId)
			isRebuild = true;

		quote *q = nullptr;
		if (orderId <= 0&&isRebuild==false)
		{
			q = get_localId2quote(pRtnQuoteCanceledData->LocalOrderID);
		}
		else
		{
			int ret;
			quote *q = get_quote_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<cffex_order*>(ord);
				break;
			case 1:
				//o = reinterpret_cast<cffex_order*>(ord);
				loggerv2::info("x1_connection::OnRtnQuoteCancel - message received on dead quote[%d]...", orderId);
				break;

			case 2:

				q = x1_order_aux::anchor(this, pRtnQuoteCanceledData);
				if (q == NULL)
				{
					loggerv2::error("x1_connection::OnRtnQuoteCancel cannot anchor quote");
					return;
				}

				add_pending_quote(q);
				break;
			default:
				break;
			}
		}
		if (q == NULL) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnQuoteCancel - quote recovered NULL");
			return;
		}

		std::string FQR_ID = std::to_string(pRtnQuoteCanceledData->X1OrderID);
		q->set_FQR_ID(FQR_ID);

		x1_order_aux::set_spdId(q, pRtnQuoteCanceledData->X1OrderID);

		switch (pRtnQuoteCanceledData->OrderStatus)
		{
		
		case X1_FTDC_SPD_PARTIAL_CANCELED:
		case X1_FTDC_SPD_CANCELED:
		{

			if (isRebuild == false)
			{
				update_instr_on_cancel_from_market_cb(q->get_bid_order());
				update_instr_on_cancel_from_market_cb(q->get_ask_order());
			}
			q->set_status(AtsType::OrderStatus::Cancel);

			//on_nack_from_market_cb(o, NULL);
			on_cancel_quote_from_market_cb(q);
			break;
		}
		case X1_FTDC_SPD_ERROR:
		{
			q->rollback();
			q->set_status(OrderStatus::Ack);
			loggerv2::error("x1_connection::OnRtnQuoteCancel,cancel quote fail,orderID:%d", q->get_id());
			break;
		}

		default:
			break;
		}
	}

	void x1_connection::OnRtnQuoteMatchedInfo(struct CX1FtdcQuoteMatchRtnField * pRtnQuoteMatchedData)
	{
		if (m_debug)
			loggerv2::info("x1_connection::OnRtnQuoteMatchedInfo-",
			"X1OrderID[%d]"
			"LocalOrderID[%d]"
			"QuoteID[%s]"
			"InstrumentID[%s]"
			"MatchedAmount[%d]"
			"MatchedPrice[%f]"
			"OrderStatus[%s]"
			"BuySellType[%d]"
			"MatchedTime[%s]"
			"OrderAmount[%d]"
			"DateCloseProfitLoss[%d]"
			"CustomCategory[%s]"
			,
			pRtnQuoteMatchedData->X1OrderID,
			pRtnQuoteMatchedData->LocalOrderID,
			pRtnQuoteMatchedData->QuoteID,
			pRtnQuoteMatchedData->InstrumentID,
			pRtnQuoteMatchedData->MatchedAmount,
			pRtnQuoteMatchedData->MatchedPrice,
			pRtnQuoteMatchedData->OrderStatus,
			pRtnQuoteMatchedData->BuySellType,
			pRtnQuoteMatchedData->MatchedTime,
			pRtnQuoteMatchedData->OrderAmount,
			pRtnQuoteMatchedData->DateCloseProfitLoss,
			pRtnQuoteMatchedData->CustomCategory
			);

		int account, bidId, askId, portfolioId, tradingType;
		get_user_info(pRtnQuoteMatchedData->CustomCategory, account, bidId, askId, portfolioId, tradingType);

		int orderId = bidId;
		bool isRebuild = false;
		if (m_begin_Id >= orderId)
			isRebuild = true;

		quote *q = nullptr;
		if (orderId <= 0 && isRebuild == false)
		{
			q = get_localId2quote(pRtnQuoteMatchedData->LocalOrderID);
		}
		else
		{
			int ret;
			quote *q = get_quote_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<cffex_order*>(ord);
				break;
			case 1:
				//o = reinterpret_cast<cffex_order*>(ord);
				loggerv2::info("x1_connection::OnRtnQuoteCancel - message received on dead quote[%d]...", orderId);
				break;

			case 2:

				q = x1_order_aux::anchor(this, pRtnQuoteMatchedData);
				if (q == NULL)
				{
					loggerv2::error("x1_connection::OnRtnQuoteCancel cannot anchor quote");
					return;
				}

				add_pending_quote(q);
				break;
			default:
				break;
			}
		}
		if (q == NULL) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnQuoteCancel - quote recovered NULL");
			return;
		}

		std::string FQR_ID = std::to_string(pRtnQuoteMatchedData->X1OrderID);
		q->set_FQR_ID(FQR_ID);

		x1_order_aux::set_spdId(q, pRtnQuoteMatchedData->X1OrderID);

		//switch (pRtnQuoteMatchedData->OrderStatus)
		//{

		//case X1_FTDC_SPD_PARTIAL_CANCELED:
		//case X1_FTDC_SPD_CANCELED:
		//{

		//	if (isRebuild == false)
		//	{
		//		update_instr_on_cancel_from_market_cb(q->get_bid_order());
		//		update_instr_on_cancel_from_market_cb(q->get_ask_order());
		//	}
		//	q->set_status(AtsType::OrderStatus::Cancel);

		//	//on_nack_from_market_cb(o, NULL);
		//	on_cancel_quote_from_market_cb(q);
		//	break;
		//}
		//case X1_FTDC_SPD_ERROR:
		//{
		//	q->rollback();
		//	q->set_status(OrderStatus::Ack);
		//	loggerv2::error("x1_connection::OnRtnQuoteCancel,cancel quote fail,orderID:%d", q->get_id());
		//	break;
		//}

		//default:
		//	break;
		//}
	}

	void x1_connection::OnRspQuoteInsert(struct CX1FtdcQuoteRspField * pRspQuoteData, struct CX1FtdcRspErrorField * pErrorInfo)
	{

		if (pErrorInfo != nullptr)
		{
			loggerv2::error("x1_connection::OnRspQuoteInsert,localID:%d,x1ID:%d,ErrorID:%d,errorMsg:%s", pErrorInfo->LocalOrderID, pErrorInfo->X1OrderID, pErrorInfo->ErrorID, pErrorInfo->ErrorMsg);
			
			quote *q = get_localId2quote(pRspQuoteData->LocalOrderID);

			if (q == NULL) // should not happen
			{
				loggerv2::error("x1_connection::OnRspQuoteActionAsync - quote recovered NULL");
				return;
			}

			on_nack_quote_from_market_cb(q, NULL);
		}
	}

	//Sec Trade Ack
	void x1_connection::OnRspInsertOrder(struct CX1FtdcRspOperOrderField * pData, struct CX1FtdcRspErrorField * pRspInfo)
	{
		if (pRspInfo != nullptr)
		{
			loggerv2::info("x1_connection::OnRspInsertOrder -->  localOrderID %d spdOrderID %d errorID %d errorMsg %s", pRspInfo->LocalOrderID, pRspInfo->X1OrderID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
			//todo find order and nack
			int localOrderID = pRspInfo->LocalOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				if (o->get_status() == AtsType::OrderStatus::Ack)
				{
					return;
				}
				char pszReason[REASON_MAXLENGTH + 1];
				memset(pszReason, 0, sizeof(pszReason));
				pszReason[REASON_MAXLENGTH] = '\0';

				snprintf(pszReason, REASON_MAXLENGTH, "error id %d.", pRspInfo->ErrorID);
				on_nack_from_market_cb(o, pszReason);


				m_localId2order.erase(localOrderID);
			}
		}//error return

		//m_orderRspQueue.CopyPush(pData);
	}
	void x1_connection::OnRspCancelOrder(struct CX1FtdcRspOperOrderField * pOrderCanceledRtn, struct CX1FtdcRspErrorField * pErrorInfo)
	{
		return;//temp
		if (pOrderCanceledRtn != nullptr)
		{
			loggerv2::info("委托撤单响应[LocalOrderID]:[%ld],[柜台委托号]:[%ld],[委托状态]:[%d]\n",
				pOrderCanceledRtn->LocalOrderID,
				pOrderCanceledRtn->X1OrderID,
				pOrderCanceledRtn->OrderStatus
				);

			int localOrderID = pOrderCanceledRtn->LocalOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				if (m_debug)
					loggerv2::info("x1_connection::OnRspSOPWithdrawOrder localOrderID %d", pOrderCanceledRtn->LocalOrderID);
				/*x1_order* o = it->second;
				if (o != nullptr)
				{
				on_ack_from_market_cb(o);

				}*/
				m_localId2order.erase(localOrderID);
			}
		}

		else if (pErrorInfo != nullptr)
		{
			loggerv2::info("委托撤单响应发生错误![accountID]:[%s],[会话ID]:[%ld],[LocalOrderID]:[%ld],[柜台委托号]:[%ld],[ReqID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->AccountID,
				pErrorInfo->SessionID,
				pErrorInfo->LocalOrderID,
				pErrorInfo->X1OrderID,
				pErrorInfo->RequestID,
				pErrorInfo->ErrorID,
				pErrorInfo->ErrorMsg
				);

			//cancel is bad

			int localOrderID = pErrorInfo->LocalOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				char pszReason[REASON_MAXLENGTH + 1];
				memset(pszReason, 0, sizeof(pszReason));
				pszReason[REASON_MAXLENGTH] = '\0';

				snprintf(pszReason, REASON_MAXLENGTH, "error id %d.", pErrorInfo->ErrorID);
				on_nack_from_market_cb(o, pszReason);


				m_localId2order.erase(localOrderID);
			}
		}
	}

	//Exchange Trade Ack
	void x1_connection::OnRtnOrder(struct CX1FtdcRspPriOrderField * pData)
	{
		m_orderRtnQueue.CopyPush(pData);
	}

	void x1_connection::OnRtnCancelOrder(struct CX1FtdcRspPriCancelOrderField * pData)
	{
		m_ordCanRtnQueue.CopyPush(pData);
	}

	void x1_connection::OnRtnMatchedInfo(struct CX1FtdcRspPriMatchInfoField * pData)
	{
		m_tradeQueue.CopyPush(pData);
	}

	//Handler
	void x1_connection::OnRtnOrderAsyn(CX1FtdcRspPriOrderField * pOrder)
	{
		std::string statusMsg;
		if (pOrder->OrderStatus == 3)
			statusMsg = "order Stauts:ACK";
		if (pOrder->OrderStatus == 6)
			statusMsg = "status:on cancel";
		loggerv2::info("x1_connection::OnRtnOrderAsyn[localID]:[%ld],[X1OrderID]:[%ld],%s,[OrderStatus]:[%d],[OrderSysID]:[%s],[SessionID]:[%ld],[SuspendTime]:[%s],\
			[InstrumentID]:[%s],[ExchangeIDID]:[%s],[BuySellType]:[%d],[OpenCloseType]:[%d],[InstrumentType]:[%d],[Speculator]:[%d],[AccountIDID]:[%s],[CancelAmount]:[%ld],[InsertPrice]:[%lf],[OrderAmount]:[%ld],[CustomCategory]:[%s]",
			pOrder->LocalOrderID,
			pOrder->X1OrderID,
			statusMsg.c_str(),
			pOrder->OrderStatus,
			pOrder->OrderSysID,
			pOrder->SessionID,
			pOrder->SuspendTime,
			pOrder->InstrumentID,
			pOrder->ExchangeID,
			pOrder->BuySellType,
			pOrder->OpenCloseType,
			pOrder->InstrumentType,
			pOrder->Speculator,
			pOrder->AccountID,
			pOrder->CancelAmount,
			pOrder->InsertPrice,
			pOrder->OrderAmount,
			pOrder->CustomCategory
			);
		//x1_order* o = NULL;
		//order *o = nullptr;
		bool isRebuild = false;

		int localID = pOrder->LocalOrderID;
		int orderId = get_order_id(pOrder->CustomCategory);

		if (m_nCurrentOrderRef < localID)
			m_nCurrentOrderRef = localID;

		//temp
		//if (contain_used_locId(localID))
		//{
		//	return;//该localId已经处理过。
		//}
		
		order *o = nullptr;
		if (0 >= orderId)
		{
			o = get_localId2order(localID);
			if (o == nullptr)
			{
				loggerv2::error("x1_connection::OnRtnOrderAsync - order recovered NULL,rebuild");
				o = x1_order_aux::anchor(this, pOrder);
			}
			orderId = o->get_id();
		}
		else
		{
			int ret;
			o = get_order_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<x1_order*>(ord);
				break;
			case 1:
				loggerv2::info("x1_connection::OnRtnOrderAsyn - message received on dead order[%d]...", orderId);
				//o = reinterpret_cast<x1_order*>(ord);
				break;

			case 2:

				o = x1_order_aux::anchor(this, pOrder);
				if (o == NULL)
				{
					loggerv2::error("x1_connection::OnRtnOrderAsyn cannot anchor order");
					return;
				}

				add_pending_order(o);
				break;
			default:
				break;
			}
		}
	

		if (o == NULL) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}

		if (m_begin_Id >= orderId)
			isRebuild = true;

		insert_used_locId(orderId);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		insert_spId2order(pOrder->X1OrderID, o);

		std::shared_ptr<x1OrderMsg> x1msg(new x1OrderMsg());
		x1msg->x1Id = pOrder->X1OrderID;
		x1msg->portfolio = o->get_portfolio();
		x1msg->orderId = o->get_id();
		x1msg->accId = o->get_account_num();
		x1msg->nTradingType = o->get_trading_type();
		x1MsgQueue.push(x1msg);

		/*ptime tradeTime(day_clock::local_day(), duration_from_string(pOrder->SuspendTime));
		ptime last = o->get_instrument()->get_last_sychro_timepoint();
		last += m_interval;*/

		int ackQty = pOrder->OrderAmount;

		switch (pOrder->OrderStatus)
		{
		case X1_FTDC_SPD_TRIGGERED:
		case X1_FTDC_SPD_IN_QUEUE:
		case X1_FTDC_SPD_PLACED:
		case X1_FTDC_SPD_PARTIAL:
		{
			//if (tradeTime > last&&isRebuild == false)//历史回包不参与pending计算
			{
				update_instr_on_ack_from_market_cb(o, ackQty);
			}
			on_ack_from_market_cb(o);
			break;
		}
		case X1_FTDC_SPD_FILLED: //    
			break;
		case X1_FTDC_SPD_ERROR:
		{
			on_nack_from_market_cb(o, "OrderRtn api reject");
			break;
		}
		case X1_FTDC_SPD_PARTIAL_CANCELED:
		case X1_FTDC_SPD_CANCELED:
		{
			o->set_last_action(AtsType::OrderAction::Cancelled);
			//if (tradeTime > last&&isRebuild == false)
			{
				update_instr_on_ack_from_market_cb(o, ackQty);
			}
			on_cancel_from_market_cb(o);
		}
		break;

		default:
			break;
		}

	}
	void x1_connection::OnRspInsertOrderAsyn(CX1FtdcRspOperOrderField * pData)
	{
		return;//temp,pData->X1OrderID is null

		if (pData != nullptr)
		{
			int localOrderID = pData->LocalOrderID;

			//auto itr = m_used_locId.find(localOrderID);
			if (contain_used_locId(localOrderID))
			{
				return;//该localId已经处理过。
			}

			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				loggerv2::info("x1_connection::OnRspInsertOrderAsyn localOrderID %d", pData->LocalOrderID);

				x1_order_aux::set_spdId(o, pData->X1OrderID);

				//auto t1 = std::chrono::system_clock::now();//由于DFITCOrderRspDataRtnField不提供挂单时间，所以以本地时间为准，每次都会去更新pending。
				auto t2 = o->get_instrument()->get_last_sychro_timepoint();



				int ackQty = o->get_quantity();//券商响应里没有委托数量，这里认为全部下单都ack了。

				if (get_lwtp_now() > t2)
				{
					update_instr_on_ack_from_market_cb(o, ackQty);
				}
				on_ack_from_market_cb(o);



				m_localId2order.erase(localOrderID);
				insert_used_locId(localOrderID);

			}

		}


	}
	void x1_connection::OnRtnCancelOrderAsyn(struct CX1FtdcRspPriCancelOrderField * pCancelOrderData)
	{
		loggerv2::info("x1_connection::OnRtnCancelOrderAsyn[LocalOrderID]:[%ld],[OrderSysID]:[%s],[InstrumentID]:[%s],[BuySellType]:[%d],[OpenCloseType]:[%d],[OrderAmount]:[%ld],[CanceledTime]:[%s],[InsertPrice]:[%lf],\
					   		[X1OrderID]:[%ld],[StatusMsg]:[%s],[Speculator]:[%d],[ExchangeID]:[%s],[Fee]:[%lf],[sessionID]:[%ld],[InstrumentType]:[%d],[AccountID]:[%s],[OrderStatus]:[%d],[CustomCategory]:[%s]\n",

							pCancelOrderData->LocalOrderID,
							pCancelOrderData->OrderSysID,
							pCancelOrderData->InstrumentID,
							pCancelOrderData->BuySellType,
							pCancelOrderData->OpenCloseType,
							pCancelOrderData->OrderAmount,
							pCancelOrderData->CanceledTime,
							pCancelOrderData->InsertPrice,
							pCancelOrderData->X1OrderID,
							pCancelOrderData->StatusMsg,
							pCancelOrderData->Speculator,
							pCancelOrderData->ExchangeID,
							pCancelOrderData->Fee,
							pCancelOrderData->SessionID,
							pCancelOrderData->InstrumentType,
							pCancelOrderData->AccountID,
							pCancelOrderData->OrderStatus,
							pCancelOrderData->CustomCategory
							);
		int nOrderId = get_order_id(pCancelOrderData->CustomCategory);
		long X1OrderID = pCancelOrderData->X1OrderID;

		bool isOldPacket = false;

		order *o = nullptr;
		if (0 >= nOrderId)
		{
			o = get_spId2order(X1OrderID);
			if (o == nullptr)
			{
				loggerv2::error("x1_connection::OnRtnOrderAsync - order recovered NULL,rebuild");
				o = x1_order_aux::anchor(this, pCancelOrderData);
			}
			else
				nOrderId = o->get_id();
		}
		else
		{
			int ret;
			o = get_order_from_map(nOrderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<x1_order*>(ord);//it->second);
				break;
			case 1:
				//o = reinterpret_cast<x1_order*>(ord);//it->second);
				loggerv2::info("x1_connection::OnRtnCancelOrderAsyn - message received on dead order[%d]...", nOrderId);
				break;
			default:
				break;
			}

		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnCancelOrderAsyn - order recovered NULL");
			return;
		}

		if (m_begin_Id >= nOrderId)
			isOldPacket = true;

		int cancelQty = pCancelOrderData->CancelAmount;

		switch (pCancelOrderData->OrderStatus)
		{

		case X1_FTDC_SPD_PARTIAL_CANCELED:
		case X1_FTDC_SPD_CANCELED:
		{

			if (get_lwtp_now() > o->get_instrument()->get_last_sychro_timepoint() && isOldPacket == false)
			{
				update_instr_on_cancel_from_market_cb(o, cancelQty);
			}
			o->set_status(AtsType::OrderStatus::Cancel);

			//on_nack_from_market_cb(o, NULL);
			on_cancel_from_market_cb(o);
			break;
		}
		case X1_FTDC_SPD_ERROR:
		{
			if (o->get_status() == OrderStatus::WaitServer || o->get_status() == OrderStatus::WaitMarket)
			{
				o->rollback();
				o->set_status(OrderStatus::Ack);
			}
			loggerv2::error("xs_connection::OnSOPWithdrawOrderRtnAsyn,cancel order fail,orderID:%d", o->get_id());
			break;
		}

		default:
			break;
		}

		//if (tradeTime > last && isOldPacket == false)
		//{
		//	update_instr_on_cancel_from_market_cb(o, cancelQty);
		//}
		//o->set_status(AtsType::OrderStatus::Cancel);

		//on_cancel_from_market_cb(o);

	}
	void x1_connection::OnRtnMatchedInfoAsyn(struct CX1FtdcRspPriMatchInfoField* pTrade)
	{
		loggerv2::info("x1_connection::OnRtnMatchedInfoAsyn[LocalOrderID]:[%ld],[OrderSysID]:[%s],[MatchID]:[%s],[InstrumentID]:[%s],[BuySellType]:[%d],[OpenCloseType]:[%d],[matchPrice]:[%lf],[OrderAmount]:[%ld],[MatchedAmount]:[%ld],[MatchedTime]:[%s],[InsertPrice]:[%lf],\
		[X1OrderID]:[%ld],[MatchType]:[%ld],[Speculator]:[%d],[ExchangeID]:[%s],[Fee]:[%lf],[sessionID]:[%ld],[InstrumentType]:[%d],[accountID]:[%s],[OrderStatus]:[%d],[CustomCategory]:[%s]\n",
		
		pTrade->LocalOrderID,
		pTrade->OrderSysID,
		pTrade->MatchID,
		pTrade->InstrumentID,
		pTrade->BuySellType,
		pTrade->OpenCloseType,
		pTrade->MatchedPrice,
		pTrade->OrderAmount,
		pTrade->MatchedAmount,
		pTrade->MatchedTime,
		pTrade->InsertPrice,
		pTrade->X1OrderID,
		pTrade->MatchType,
		pTrade->Speculator,
		pTrade->ExchangeID,
		pTrade->Fee,
		pTrade->SessionID,
		pTrade->InstrumentType,
		pTrade->AccountID,
		pTrade->OrderStatus,
		pTrade->CustomCategory
		);

		//x1_order* o = NULL;
		bool duplicat = false;
		bool isOldPacket = false;
		// 1 - retrieve order
		int orderId = get_order_id(pTrade->CustomCategory);
		int localID = pTrade->LocalOrderID;
		long X1OrderID = pTrade->X1OrderID;

		order *o = nullptr;
		if (0 >= orderId)
		{
			o = get_spId2order(X1OrderID);
			if (o == nullptr)
			{
				o = get_localId2order(localID);
				if (o == nullptr)
				{
					loggerv2::error("x1_connection::OnRtnMatchedInfoAsyn - order recovered NULL,Rebuild");
					o = x1_order_aux::anchor(this, pTrade);
				}
			}
			else
				orderId = o->get_id();
		}
		else
		{
			int ret;
			o = get_order_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<x1_order*>(ord);//it->second);
				break;
			case 1:
				loggerv2::info("x1_connection::OnRtnMatchedInfoAsyn - message received on dead order[%d]...", orderId);
				//o = reinterpret_cast<x1_order*>(ord);//it->second);
				break;
			case 2:

				o = x1_order_aux::anchor(this, pTrade);
				if (o == NULL)
				{
					loggerv2::error("x1_connection::OnRtnMatchedInfoAsyn cannot anchor order");
					return;
				}

				add_pending_order(o);
				break;
			default:
				break;
			}
		}
		if (o == NULL) // should not happen
		{
			loggerv2::error("x1_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}

		if (m_begin_Id >= orderId)
			isOldPacket = true;

		// 2 - treat message
		int execQty = pTrade->MatchedAmount;
		double execPrc = pTrade->MatchedPrice;
		const char* pszTime = pTrade->MatchedTime;

		exec* e = new exec(o, std::string(pTrade->MatchID), execQty, execPrc, pszTime);

		on_exec_from_market_cb(o, e, duplicat);
		if (duplicat)
		{
			loggerv2::info("duplicat packet,drop");
			return;
		}

		update_instr_on_exec_from_market_cb(o, e, false);//这个API的报文全都不是历史回报

	}


	int x1_connection::get_xs_instype(AtsType::InstrType::type _type)
	{
		switch (_type)
		{
		case AtsType::InstrType::Call:
		case AtsType::InstrType::Put:
			return 1;
		case AtsType::InstrType::ETF:
			return 0;
		default:
			return 2;
		}

		return 0;
	}

	std::string x1_connection::getMaturity(std::string sMat)
	{
		std::string newMat;
		if (sMat.size() < 8)
			return sMat;
		newMat = sMat.substr(0, 4);
		newMat += "-";
		newMat += sMat.substr(5, 2);
		newMat += "-";
		newMat += sMat.substr(8, 2);
		return newMat;
	}

	//history query
	void x1_connection::OnRspQryOrderInfo(struct CX1FtdcRspOrderField *pRtnOrderData, struct CX1FtdcRspErrorField * pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != NULL)
		{
			loggerv2::error("OnRspQryOrderInfo error![accountID]:[%s],[sessionID]:[%ld],[localOrderID]:[%ld],[spdOrderID]:[%ld],[requestID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->AccountID,
				pErrorInfo->SessionID,
				pErrorInfo->LocalOrderID,
				pErrorInfo->X1OrderID,
				pErrorInfo->RequestID,
				pErrorInfo->ErrorID,
				pErrorInfo->ErrorMsg); //打印“错误ID,错误信息,LocalOrderID”

		}

		loggerv2::error("OnRspQryOrderInfo[localOrderID]:[%ld],[lRequestID]:[%ld],[instrumentID]:[%s],[spdOrderID]:[%ld],[orderStatus]:[%d],[exchangeID]:[%s],[buySellType]:[%d],[openClose]:[%d],[insertPrice]:[%lf],[orderAmount]:[%ld],[matchedPrice]:[%lf],\
[matchedAmount]:[%ld],[cancelAmount]:[%ld],[insertType]:[%d],[speculator]:[%d],[commTime]:[%s],[clientID]:[%s],[exchangeID]:[%s],[submitTime]:[%s],[operStation]:[%s],[accountID]:[%s],[instrumentType]:[%d],[bIsLast]:[%d],[CustomCategory]:[%s]\n", 
								pRtnOrderData->LocalOrderID,//本地ID
								pRtnOrderData->RequestID,//ReqID
								pRtnOrderData->InstrumentID,//InstrumentID
								pRtnOrderData->X1OrderID,//柜台委托号
								pRtnOrderData->OrderStatus,//申报结果
								pRtnOrderData->ExchangeID,
								pRtnOrderData->BuySellType,
								pRtnOrderData->OpenClose,
								pRtnOrderData->InsertPrice,
								pRtnOrderData->OrderAmount,
								pRtnOrderData->MatchedPrice,
								pRtnOrderData->MatchedAmount,
								pRtnOrderData->CancelAmount,
								pRtnOrderData->InsertType,
								pRtnOrderData->Speculator,
								pRtnOrderData->CommTime,
								pRtnOrderData->ClientID,
								pRtnOrderData->ExchangeID,
								pRtnOrderData->SubmitTime,
								pRtnOrderData->OperStation,
								pRtnOrderData->AccountID,
								pRtnOrderData->InstrumentType,
								bIsLast,
								pRtnOrderData->CustomCategory
								);
																																													

		//x1_order* o = NULL;
		bool isRebuild = true;

		int locID = pRtnOrderData->LocalOrderID;
		if (m_nCurrentOrderRef < abs(locID))
			m_nCurrentOrderRef = abs(locID);

		order* o = nullptr;
		if (pRtnOrderData->X1OrderID>0)
			o = x1_order_aux::anchor(this, pRtnOrderData);

		if (nullptr!= o)//&&o->get_id()>0)
		{
			add_pending_order(o);

			insert_used_locId(locID);
			x1_order_aux::set_spdId(o, pRtnOrderData->X1OrderID);

			int ackQty = pRtnOrderData->OrderAmount;

			switch (pRtnOrderData->OrderStatus)
			{
			case X1_FTDC_SPD_TRIGGERED:
			case X1_FTDC_SPD_IN_QUEUE:
			case X1_FTDC_SPD_PLACED://TO CHECK
			case X1_FTDC_SPD_PARTIAL:
			{
				on_ack_from_market_cb(o);
				break;
			}
			case X1_FTDC_SPD_FILLED: //    
				on_ack_from_market_cb(o);
				break;
			case X1_FTDC_SPD_ERROR:
			{
				on_nack_from_market_cb(o, "OrderRtn api reject");
				break;
			}
			case X1_FTDC_SPD_PARTIAL_CANCELED:
			case X1_FTDC_SPD_CANCELED:
			{
				o->set_last_action(AtsType::OrderAction::Cancelled);
				on_cancel_from_market_cb(o);
			}
			break;

			default:
				break;
			}



		}
		else
		{
			loggerv2::error("x1_connection::OnRspQryOrderInfo cannot anchor order");
		}

		if (bIsLast == true)
		{
			if (m_bFuOrder == false)
			{
				sleep_by_milliseconds(1000);
				loggerv2::info("query Future orderinsert");
				OnQryFutureOrders();
				m_bFuOrder = true;
			}
			else if (m_bOpTrade == false)
			{
				sleep_by_milliseconds(1000);
				loggerv2::info("query Option match");
				OnQryOpMatches();
				m_bOpTrade = true;
			}
		}

	}

	void x1_connection::OnRspQryMatchInfo(struct CX1FtdcRspMatchField *pTrade, struct CX1FtdcRspErrorField * pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != NULL){

			loggerv2::error("OnRspQryMatchInfo error![accountID]:[%s],[sessionID]:[%ld],[localOrderID]:[%ld],[spdOrderID]:[%ld],[requestID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->AccountID,
				pErrorInfo->SessionID,
				pErrorInfo->LocalOrderID,
				pErrorInfo->X1OrderID,
				pErrorInfo->RequestID,
				pErrorInfo->ErrorID,
				pErrorInfo->ErrorMsg); //打印“错误ID,错误信息,LocalOrderID”

		}

		loggerv2::info("OnRspQryMatchInfo[lRequestID]:[%ld],[spdOrderID]:[%ld],[exchangeID]:[%s],[instrumentID]:[%s],[buySellType]:[%d],[openClose]:[%d],[matchedPrice]:[%lf],[matchedAmount]:[%ld],[matchedMort]:[%lf],[speculator]:[%d],[matchedTime]:[%s],[matchedID]:[%s],[localOrderID]:[%ld],[clientID]:[%s],[matchType]:[%d],[instrumentType]:[%d],[bIsLast]:[%d]",
																																						 pTrade->RequestID,//ReqID
																																						 pTrade->X1OrderID,//柜台委托号
																																						 pTrade->ExchangeID,//ExchangeID
																																						 pTrade->InstrumentID,//InstrumentID
																																						 pTrade->BuySellType,//买卖
																																						 pTrade->OpenClose,//OpenCloseType
																																						 pTrade->MatchedPrice,//matchPrice
																																						 pTrade->MatchedAmount,//成交数量
																																						 pTrade->MatchedMort,//成交金额
																																						 pTrade->Speculator,//投保类别
																																						 pTrade->MatchedTime,//成交时间
																																						 pTrade->MatchedID,//成交编号
																																						 pTrade->LocalOrderID,//LocalOrderID
																																						 pTrade->ClientID,//交易编码
																																						 pTrade->MatchType,//成交类型
																																						 pTrade->InstrumentType,//合约类型
																																						 bIsLast);


		bool duplicat = false;
		order *o = nullptr;
		// 1 - retrieve order
		int orderId = get_order_id(pTrade->CustomCategory);
		if (orderId <= 0)
		{
			if (pTrade->X1OrderID > 0)
			{
				o = get_spId2order(pTrade->X1OrderID);
			}

		}
		else
		{
			int localID = pTrade->LocalOrderID;

			if (m_nCurrentOrderRef < abs(localID))
				m_nCurrentOrderRef = abs(localID);


			int ret;
			o = get_order_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				break;
			case 1:
				loggerv2::info("x1_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
				break;

			case 2:
			default:
				break;
			}
		}
		if (o != NULL)
		{
			int execQty = pTrade->MatchedAmount;
			double execPrc = pTrade->MatchedPrice;
			const char* pszTime = pTrade->MatchedTime;

			exec* e = new exec(o, string(pTrade->MatchedID), execQty, execPrc, pszTime);
			on_exec_from_market_cb(o, e, duplicat);

		}
		else
		{
			cout << "OnRspQryMatchInfo cannot find order" << endl;
		}

		if (bIsLast == true)
		{
			if (m_bFuTrade == false)
			{
				sleep_by_milliseconds(1000);
				loggerv2::info("query Futures match");
				OnQryFutureMatches();
				m_bFuTrade = true;
			}
		}

	}

	void x1_connection::OnQryOpOrders()
	{
		struct CX1FtdcQryOrderField data;
		memset(&data, 0, sizeof(data));

		data.RequestID = m_nRequestId++;
		data.InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;
		strcpy(data.AccountID, m_sUsername.c_str());

		m_pUserApi->ReqQryOrderInfo(&data);
	}
	void x1_connection::OnQryFutureOrders()
	{
		struct CX1FtdcQryOrderField data;
		memset(&data, 0, sizeof(data));

		data.RequestID = m_nRequestId++;
		data.InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;
		strcpy(data.AccountID, m_sUsername.c_str());

		m_pUserApi->ReqQryOrderInfo(&data);
	}
	void x1_connection::OnQryOpMatches()
	{
		struct CX1FtdcQryMatchField data;
		memset(&data, 0, sizeof(data));
		data.RequestID = m_nRequestId++;
		strcpy(data.AccountID, m_sUsername.c_str());
		data.InstrumentType = X1FTDC_INSTRUMENT_TYPE_OPT;

		m_pUserApi->ReqQryMatchInfo(&data);
		return;
	}
	void x1_connection::OnQryFutureMatches()
	{
		struct CX1FtdcQryMatchField data;
		memset(&data, 0, sizeof(data));
		data.RequestID = m_nRequestId++;
		strcpy(data.AccountID, m_sUsername.c_str());
		data.InstrumentType = X1FTDC_INSTRUMENT_TYPE_COMM;

		m_pUserApi->ReqQryMatchInfo(&data);
		return;
	}

	//order* x1_connection::create_order()
	//{
	//	return new x1_order(this);
	//}

	/*void x1_connection::cancel_num_warning(tradeitem* i)
	{
		loggerv2::warn("tradeitem:%s cancel num is more than warning lev", i->getCode().c_str());
		this->setTradingAllowed(false);
	}

	void x1_connection::cancel_num_ban(tradeitem* i)
	{
		if (i->get_instr_type() == InstrType::Future)
		{
			loggerv2::warn("tradeitem:%s cancel num is more than forbid lev", i->getCode().c_str());
			this->setTradingAllowed(false);
			i->set_cancel_forbid(true);
		}
	}*/

	void x1_connection::insert_localId2order(int id, order* o)
	{
		tbb::concurrent_hash_map<int, order*>::accessor wa;
		m_localId2order.insert(wa, id);
		wa->second = o;
	}

	order *x1_connection::get_localId2order(int id)
	{
		tbb::concurrent_hash_map<int, order *>::const_accessor ra;
		if (m_localId2order.find(ra, id))
		{
			if (ra->second != nullptr)
				return ra->second;
			else
			{
				ra.release();
				m_localId2order.erase(id);
				return nullptr;
			}
		}
		else
			return nullptr;
	}

	void x1_connection::insert_spId2order(long id, order* o)
	{
		tbb::concurrent_hash_map<int, order*>::accessor wa;
		m_spId2order.insert(wa, id);
		wa->second = o;
	}

	order *x1_connection::get_spId2order(long id)
	{
		tbb::concurrent_hash_map<int, order *>::const_accessor ra;
		if (m_spId2order.find(ra, id))
		{
			if (ra->second != nullptr)
				return ra->second;
			else
			{
				ra.release();
				m_spId2order.erase(id);
				return nullptr;
			}
		}
		else
			return nullptr;
	}

	void x1_connection::insert_localId2quote(int id, quote* o)
	{
		tbb::concurrent_hash_map<int, quote*>::accessor wa;
		m_localId2quote.insert(wa, id);
		wa->second = o;
	}

	quote *x1_connection::get_localId2quote(int id)
	{
		tbb::concurrent_hash_map<int, quote *>::const_accessor ra;
		if (m_localId2quote.find(ra, id))
		{
			if (ra->second != nullptr)
				return ra->second;
			else
			{
				ra.release();
				m_localId2quote.erase(id);
				return nullptr;
			}
		}
		else
			return nullptr;
	}

	void x1_connection::insert_spId2quote(long id, quote* o)
	{
		tbb::concurrent_hash_map<int, quote*>::accessor wa;
		m_spId2quote.insert(wa, id);
		wa->second = o;
	}
	quote *x1_connection::get_spId2quote(long id)
	{
		tbb::concurrent_hash_map<int, quote *>::const_accessor ra;
		if (m_localId2quote.find(ra, id))
		{
			if (ra->second != nullptr)
				return ra->second;
			else
			{
				ra.release();
				m_localId2quote.erase(id);
				return nullptr;
			}
		}
		else
			return nullptr;
	}

	void x1_connection::insert_used_locId(int id)
	{
		tbb::concurrent_hash_map<int, int>::accessor wa;
		m_used_locId.insert(wa, id);
		wa->second = id;
	}

	bool x1_connection::contain_used_locId(int id)
	{
		tbb::concurrent_hash_map<int, int>::const_accessor ra;
		if (m_used_locId.find(ra, id))
			return true;
		else
			return false;
	}

}



