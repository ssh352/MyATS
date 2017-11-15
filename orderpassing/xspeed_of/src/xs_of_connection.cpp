#include "xs_of_connection.h"

#include <vector>
#include "tradeItem_gh.h"

#include <istream>
#include <fstream>

#include "order_reference_provider.h"
#include <sqlite3.h>
#include <boost/property_tree/ini_parser.hpp>


using namespace terra::common;

//#define POLL_NUM 10
namespace xs_of
{

	//Connect
	void xs_of_connection::init_connection()
	{
		m_sName = "xs_of_connection";

		m_orderRtnQueue.setHandler(boost::bind(&xs_of_connection::OnRtnOrderAsyn, this, _1));
		m_orderRspQueue.setHandler(boost::bind(&xs_of_connection::OnRspInsertOrderAsyn, this, _1));
		m_tradeQueue.setHandler(boost::bind(&xs_of_connection::OnRtnMatchedInfoAsyn, this, _1));
		m_ordCanRtnQueue.setHandler(boost::bind(&xs_of_connection::OnRtnCancelOrderAsyn, this, _1));

		loggerv2::info("xs_of_connection::init_connection create trader api..");
		m_pUserApi = DFITCTraderApi::CreateDFITCTraderApi();

		//std::thread th(boost::bind(&xs_of_connection::set_kernel_timer_thread, this));
		//m_thread.swap(th);
		//init_process(io_service_type::trader, 10);
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
	}

#ifdef Linux
	void  xs_of_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&xs_of_connection::process, this));
		m_orderRtnQueue.set_fd(efd);
		m_orderRspQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_ordCanRtnQueue.set_fd(efd);
		m_outboundQueue.set_fd(efd);
		m_outquoteboundQueue.set_fd(efd);
	}
#endif

	void xs_of_connection::connect()
	{
		loggerv2::info("calling xs_of_connection::connect");
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", m_sHostname.c_str(), m_sService.c_str());
			loggerv2::info("xs2_api::connect initializing api");
			int res = m_pUserApi->Init(addr, this);
			//int res = m_pMdApi->connect(addr);
			if (res != 0)
			{
				loggerv2::error("xs_of_connection::connect - api initalization failed,error %d", res);
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

	void xs_of_connection::OnRtnErrorMsg(struct DFITCErrorRtnField * pErrorInfo)
	{
		std::cout << "xs_of_connection::OnRtnErrorMsg:" << pErrorInfo->errorMsg << std::endl;
		loggerv2::error("xs_of_connection::OnRtnErrorMsg:%s", pErrorInfo->errorMsg);
	}

	void xs_of_connection::release()
	{
		//is_alive(false);
		//m_thread.join();
		ctpbase_connection::release();
		m_pUserApi->Release();
	}
	void xs_of_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			//if (m_pUserApi->disconnect() == false)
			DFITCUserLogoutField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.accountID, m_sUsername.c_str());
			request.lRequestID = m_nRequestId++;
			int res = m_pUserApi->ReqUserLogout(&request);
			if (res != 0)
			{
				loggerv2::error("xs_of_connection::disconnect failed");
			}
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "xs_of_connection - ReqUserLogout failed");
		}
	}
	void xs_of_connection::OnFrontConnected()
	{
		loggerv2::info("xs_of_connection::OnFrontConnected - xs_of_connection is UP");
		if (getStatus() != AtsType::ConnectionStatus::Connected)
		{
			loggerv2::info("xs_of_connection::OnFrontConnected subscribe private topic");
			//m_pUserApi->SubscribePrivateTopic(TERT_RESTART);
			loggerv2::info("request user login");
			request_op_login();
		}
		else
		{
			loggerv2::info("xs_of_connection not asking for reconnect...");
		}
	}
	void xs_of_connection::OnFrontDisconnected(int nReason)
	{
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

	//login,logout
	void xs_of_connection::request_op_login()
	{
		struct DFITCUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.accountID, m_sUsername.c_str());
		strcpy(request.passwd, m_sPassword.c_str());
		request.lRequestID = m_nRequestId++;
		int res = m_pUserApi->ReqUserLogin(&request);
		if (res != 0)
		{
			loggerv2::error("can not login");
		}
		return;
	}
	void xs_of_connection::OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pUserLoginInfoRtn, struct DFITCErrorRtnField * pErrorInfo)
	{
		if (pErrorInfo != nullptr)
		{
			loggerv2::error("xs_of_connection::OnRspSOPUserLogin,error:%s", pErrorInfo->errorMsg);
			return;
		}

		if (pUserLoginInfoRtn != nullptr)
		{
			int temp_id = pUserLoginInfoRtn->initLocalOrderID;
			if (temp_id > m_nCurrentOrderRef)
			{
				m_nCurrentOrderRef = pUserLoginInfoRtn->initLocalOrderID;
			}
			loggerv2::info("xs_of_connection::OnRspSOPUserLogin localOrderId %d", pUserLoginInfoRtn->initLocalOrderID);

			on_status_changed(AtsType::ConnectionStatus::Connected, "xs_of_connection::OnRspSOPUserLogin");
			loggerv2::info("xs_of_connection::OnRspSOPUserLogin login succeed");

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
			on_status_changed(AtsType::ConnectionStatus::Disconnected, std::to_string(pErrorInfo->nErrorID).c_str());
			loggerv2::info("xs_of_connection::OnRspSOPUserLogin login failed error %d", pErrorInfo->nErrorID);
		}
	}
	void xs_of_connection::OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pUserLogoutInfoRtn, struct DFITCErrorRtnField * pErrorInfo)
	{
		if (pUserLogoutInfoRtn != nullptr)
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "xs_of_connection::OnRspUserLogout Receive Logout Msg");
		else
			loggerv2::error("xs_of_connection::OnRspSOPUserLogout logout failed ErrId[%d]", pErrorInfo->nErrorID);
	}

	//init
	xs_of_connection::xs_of_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "xs_of_connection";
		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		m_startTime = m_last_time;
		m_begin_Id = order_reference_provider::get_instance().get_current_int();
	}
	//xs_of_connection::~xs_of_connection()
	//{
	//	//delete m_pUserApi;
	//}

	bool xs_of_connection::init_config(const std::string &name, const std::string & strConfigFile)
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
		xs_create_pool.init(32);
		xs_cancel_pool.init(32);

		quote_create_pool.init(32);
		quote_cancel_pool.init(32);

		std::list<DFITCInsertOrderField*> mlist1;
		std::list<DFITCCancelOrderField*> mlist2;

		unsigned int i = 0;
		while (i<xs_create_pool.mlen)
		{
			DFITCInsertOrderField* ptr = xs_create_pool.get_mem();
			memset((void *)ptr, 0, sizeof(DFITCInsertOrderField));
			strcpy(ptr->accountID, m_sUsername.c_str());
			//xs_create_pool.free_mem(ptr);
			++i;
			mlist1.push_back(ptr);
		}

		i = 0;
		while (i<xs_cancel_pool.mlen)
		{
			DFITCCancelOrderField* ptr2 = xs_cancel_pool.get_mem();
			memset((void *)ptr2, 0, sizeof(DFITCCancelOrderField));
			strcpy(ptr2->accountID, m_sUsername.c_str());
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

	void xs_of_connection::readLocal2Portfolio()
	{
		std::stringstream ss;
		ss << ".." << OSD << "Data" << OSD << getCurrentBizDate() << OSD;
		std::string path = ss.str();

		if (!boost::filesystem::exists(path))
		{
			boost::filesystem::create_directory(path);
		}
		std::string fn = path + "xs2_" + getCurrentBizDate() + "_locIdtoPro.csv";
		std::ifstream f1(fn.data(), std::ios::in);
		if (f1.is_open())
		{
			std::string buf;
			while (getline(f1, buf))
			{
				//fgets(buff, 100, fp);
				int index = buf.find(':');
				if (index != std::string::npos)
				{
					int localID = stoi(buf.substr(0, index));
					std::string porfo = buf.substr(index + 1, buf.length() - index - 1);
					if (porfo.at(porfo.length() - 1) == '\n' || porfo.at(porfo.length() - 1) == '\r\n')
					{
						porfo[porfo.length() - 1] = '\0';
					}
					m_localId2Portfolio.insert(std::make_pair(localID, porfo));
				}
			}
			//fclose(fp);
			f1.close();
		}
		if (m_localId2Portfolio.size() == 0)
		{
			std::string fn2 = path + "xs2_" + getCurrentBizDate() + "_locIdtoPro_Backup.csv";
			f1.open(fn2.data(), std::ios::in);

			if (f1.is_open())
			{
				std::string buf;

				while (getline(f1, buf))
				{
					int index = buf.find(':');
					if (index != std::string::npos)
					{
						int localID = stoi(buf.substr(0, index));
						std::string porfo = buf.substr(index + 1, buf.length() - index - 1);
						if (porfo.at(porfo.length() - 1) == '\n' || porfo.at(porfo.length() - 1) == '\r\n')
						{
							porfo[porfo.length() - 1] = '\0';
						}
						m_localId2Portfolio.insert(std::make_pair(localID, porfo));
					}
				}
				//fclose(fp);
				f1.close();
			}
		}


	}
	void xs_of_connection::ackBackup()
	{
		//boost::posix_time::ptime pt = microsec_clock::local_time() + boost::posix_time::time_duration(boost::posix_time::milliseconds(3));
		//boost_write_lock wlock(m_rwmutex, boost::posix_time::time_duration(boost::posix_time::milliseconds(3)));
		/*std::stringstream ss;
		ss << ".." << OSD << "data" << OSD << getCurrentBizDate() << OSD;
		std::string path = ss.str();
		while (1)
		{
			boost_write_lock wlock(m_rwmutex);
			std::unordered_map<int, std::string> temp;
			temp.swap(m_localId2Portfolio);
			wlock.unlock();

			if (!boost::filesystem::exists(path))
			{
				boost::filesystem::create_directory(path);
			}

			std::string fn = path + "xs2_" + getCurrentBizDate() + "_locIdtoPro.csv";
			std::ofstream f1(fn.data(), std::ios::app);
			if (!f1.bad())
			{
				for (auto it = temp.begin(); it != temp.end(); ++it)
				{
					std::string str = it->second;
					if (str.length() == 0)
						str = "unknown";
					f1 << it->first << ":" << str << std::endl;
				}
				f1.close();
			}
			std::string fn2 = path + "xs2_" + getCurrentBizDate() + "_locIdtoPro_Backup.csv";
			std::ofstream f2(fn2.data(), std::ios::app);
			if (!f2.bad())
			{
				for (auto it = temp.begin(); it != temp.end(); ++it)
				{
					std::string str = it->second;
					if (str.length() == 0)
						str = "unknown";
					f2 << it->first << ":" << str << std::endl;
				}
				f2.close();
			}
			sleep_by_milliseconds(std::chrono::microseconds(21000000));

		}
		return;*/

	}

	void xs_of_connection::req_RiskDegree()
	{
		DFITCCapitalField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.accountID, m_sUsername.c_str());
		pRequest.lRequestID = m_nRequestId++;

		if (m_pUserApi->ReqQryCustomerCapital(&pRequest) != 0)
			loggerv2::error("x1_connection::request_future_positions failed");
	}

	void xs_of_connection::OnRspCustomerCapital(struct DFITCCapitalInfoRtnField * pCapitalInfo, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != nullptr)
			loggerv2::error("x1_connection::OnRspCustomerCapital failed,error:%s", pErrorInfo->errorMsg);
		else
		{
			loggerv2::error("x1_connection::OnRspCustomerCapital, riskDegree:%lf", pCapitalInfo->riskDegree);
			this->set_RiskDegree(pCapitalInfo->riskDegree);
		}
	}

	void xs_of_connection::request_investor_full_positions()
	{
		request_op_positions();
		request_future_positions();
	}
	void xs_of_connection::request_op_positions()
	{
		//loggerv2::info("");
		DFITCPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.accountID, m_sUsername.c_str());

		pRequest.lRequestID = m_nRequestId++;
		pRequest.instrumentType = DFITC_OPT_TYPE;
		if (m_pUserApi->ReqQryPosition(&pRequest) != 0)
			loggerv2::error("xs_of_connection::request_op_positions failed");
		if (m_debug)
			loggerv2::info("xs_of_connection:: calling OnRspQryInvestorPosition ");
	}
	void xs_of_connection::requset_op_instruments()
	{
		loggerv2::info("xs_of_connection::requset_op_instruments");
		DFITCExchangeInstrumentField request;
		memset(&request, 0, sizeof(DFITCExchangeInstrumentField));
		std::string exchangeID = "DCE";
		strcpy(request.accountID, m_sUsername.c_str());                //客户号(Y)
		strcpy(request.exchangeID, exchangeID.c_str());
		request.lRequestID = m_nRequestId++;
		request.instrumentType = DFITC_OPT_TYPE;

		m_database->open_database();
		int ret = m_pUserApi->ReqQryExchangeInstrument(&request);
		printf_ex("xs_of_connection::requset_op_instruments ret:%d\n", ret);
	}

	void xs_of_connection::request_future_positions()
	{
		DFITCPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.accountID, m_sUsername.c_str());

		pRequest.lRequestID = m_nRequestId++;
		pRequest.instrumentType = DFITC_COMM_TYPE;


		if (m_pUserApi->ReqQryPosition(&pRequest) != 0)
			loggerv2::error("xs_of_connection::request_op_positions failed");
		if (m_debug)
			loggerv2::info("xs_of_connection:: calling OnRspQryInvestorPosition ");
	}
	void xs_of_connection::requset_future_instruments()
	{
		loggerv2::info("xs_of_connection::requset_future_instruments");
		DFITCExchangeInstrumentField request;
		memset(&request, 0, sizeof(DFITCExchangeInstrumentField));
		strcpy(request.accountID, m_sUsername.c_str());                //客户号(Y)

		request.lRequestID = m_nRequestId++;
		request.instrumentType = DFITC_COMM_TYPE;
		m_database->open_database();
		int ret = m_pUserApi->ReqQryExchangeInstrument(&request);
		printf_ex("xs_of_connection::requset_future_instruments ret:%d\n",ret);
	}

	void xs_of_connection::OnRspQryPosition(struct DFITCPositionInfoRtnField *pData, struct DFITCErrorRtnField *pRspInfo, bool bIsLast)
	{
		if (pData == nullptr)
		{
			loggerv2::error("xs_of_connection::OnRspSOPQryPosition errorid %d,%s", pRspInfo->nErrorID, pRspInfo->errorMsg);
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
		loggerv2::info("xs_of_connection::OnRspSOPQryPosition [ReqID]:[%d],[accountID]:[%s],[exchangeID]:[%s],[instrumentID]:[%s],[buySellType]:[%d],[openAvgPrice]:[%f],[positionAvgPrice]:[%f],[positionAmount]:[%ld],[totalAvaiAmount]:[%ld],\
					   [todayAvaiAmount]:[%ld],[lastAvaiAmount]:[%ld],[todayAmount]:[%ld],[lastAmount]:[%ld],[tradingAmount]:[%ld],[datePositionProfitLoss]:[%f],[dateCloseProfitLoss]:[%f],[floatProfitLoss]:[%f],[dMargin]:[%f],[speculator]:[%d],[clientID]:[%s],\
					   [preSettlementPrice]:[%f],[instrumentType]:[%d],[dPremium]:[%f],[isLast]:[%d]",
																														   pData->lRequestID, //ReqID
																														   //pData->counterID,  //柜台编号
																														   pData->accountID,  //资金帐号ID
																														   pData->exchangeID, //交易所编码
																														   pData->instrumentID,//合约代码
																														   pData->buySellType,//买卖
																														   pData->openAvgPrice,//开仓均价
																														   pData->positionAvgPrice,//持仓均价
																														   pData->positionAmount,//持仓量
																														   pData->totalAvaiAmount,//总可用
																														   pData->todayAvaiAmount,//今可用
																														   pData->lastAvaiAmount, //昨可用
																														   pData->todayAmount,    //今仓
																														   pData->lastAmount,     //昨仓
																														   pData->tradingAmount,  //挂单量
																														   pData->datePositionProfitLoss, // 盯市持仓盈亏
																														   pData->dateCloseProfitLoss,   // 盯市平仓盈亏
																														   pData->floatProfitLoss,        //浮动盈亏
																														   pData->dMargin,                // 占用保证金
																														   pData->speculator,             //投保类别
																														   pData->clientID,               //交易编码
																														   pData->preSettlementPrice,              //昨结算价
																														   pData->instrumentType,        //合约类型
																														   pData->dPremium,          //权利金 
																														   bIsLast);



		if (pData->instrumentID)
		{

			//std::string instr = std::string(pData->InstrumentID) + "." + std::string(pData->ExchangeID);
			std::string instr = std::string(pData->instrumentID) + "." + std::string(pData->exchangeID) + "@" + getName();

			//std::string instr = std::string(pData->contractID) + "@" + getName();
			auto con = tradeitem_gh::get_instance().container();
			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(instr.c_str());
			if (i)
			{
				//loggerv2::info("lts_api::OnRspQryInvestorPosition found tradeitem %s", pData->InstrumentID);
				if (pData->buySellType == DFITC_SPD_BUY)//long
				{
					i->set_tot_long_position(pData->positionAmount);//总持仓
					i->set_today_long_position(pData->todayAmount);//今仓
					i->set_yst_long_position(pData->lastAmount);//昨仓
					long freezeQty = pData->positionAmount - pData->totalAvaiAmount;
					i->set_pending_short_close_qty(freezeQty);//总挂单 = 总持仓 - 总可用
					i->set_pending_short_close_today_qty(pData->todayAmount - pData->todayAvaiAmount);//今日挂单 = 今仓 - 今可用

				}


				//3+4 = total short position in options
				else if (pData->buySellType == DFITC_SPD_SELL)//short，字段含义同上
				{
					i->set_tot_short_position(pData->positionAmount);
					i->set_today_short_position(pData->todayAmount);
					i->set_yst_short_position(pData->lastAmount);
					long freezeQty = pData->positionAmount - pData->totalAvaiAmount;
					i->set_pending_long_close_qty(freezeQty);
					i->set_pending_long_close_today_qty(pData->todayAmount - pData->todayAvaiAmount);
					//if (pData->coveredFlag == DFITCSEC_CF_Covered) // covered sell
					//{
					// i->set_covered_sell_open_position(pData->totalQty);
					//}
				}
				i->set_last_sychro_timepoint(get_lwtp_now());

				if (m_debug)
					i->dumpinfo();

			}
			else
				loggerv2::warn("xs_of_connection::OnRspSOPQryPosition cannot find tradeitem %s", pData->instrumentID);
		}



	}
	void xs_of_connection::OnRspQryExchangeInstrument(struct DFITCExchangeInstrumentRtnField *pData, struct DFITCErrorRtnField *pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != nullptr)
		{
			loggerv2::error("OnRspQryExchangeInstrument Error,msg:%s", pErrorInfo->errorMsg);
			return;
		}		
		if (pData == nullptr)
		{
			loggerv2::info("交易所合约查询响应发生错误![accountID]:[%s],[会话ID]:[%ld],[本地委托号]:[%ld],[柜台委托号]:[%ld],[ReqID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->accountID,
				pErrorInfo->sessionID,
				pErrorInfo->localOrderID,
				pErrorInfo->spdOrderID,
				pErrorInfo->requestID,
				pErrorInfo->nErrorID,
				pErrorInfo->errorMsg
				);
		}
		else
		{
			loggerv2::info("交易所合约查询响应[RequestID]:[%d],[exchangeID]:[%s],[instrumentID]:[%s],[品种]:[%s],[是否结束]:[%d]\n",
				pData->lRequestID,
				pData->exchangeID,
				pData->instrumentID,
				pData->VarietyName,
				bIsLast);
		}
		std::string sInstr = std::string(pData->instrumentID);
		boost::trim(sInstr);
		std::string sCmd, sCP;
		std::string sUnderlying = pData->underlying;
		std::string sExcge = pData->exchangeID;
		std::string sMat = getMaturity(pData->instrumentMaturity);

		std::string prefix = pData->instrumentPrefix;
		std::string sInstClass;
		if (pData->instrumentType == DFITC_COMM_TYPE)
			sInstClass = "F_" + boost::to_upper_copy(prefix);
		else
			sInstClass = "O_" + boost::to_upper_copy(prefix);

		{
			std::string sSearch = "select * from InstrumentClass where ClassName= '" + sInstClass + "'";
			char *zErrMsg = 0;
			std::string TickRule = "0_" + std::to_string(pData->minPriceFluctuation);
			std::string PointValue = std::to_string(pData->contractMultiplier);
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

		if (pData->instrumentType == DFITC_COMM_TYPE)
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
		else if (pData->instrumentType == DFITC_OPT_TYPE)
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
				sCmd += "'" + std::to_string(pData->strikePrice) + "',";
				sCmd += "'" + std::to_string(pData->contractMultiplier) + "',";
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
				sCmd += "Strike = '" + std::to_string(pData->strikePrice) + "',";
				sCmd += "PointValue ='" + std::to_string(pData->contractMultiplier) + "',";				
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

	void xs_of_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_orderRspQueue.Pops_Handle(0);
		m_orderRtnQueue.Pops_Handle(0);
		m_tradeQueue.Pops_Handle(0);
		m_ordCanRtnQueue.Pops_Handle(0);
		m_outquoteboundQueue.Pops_Handle_Keep(10);

	}

	int xs_of_connection::market_create_order_async(order* o, char* pszReason)
	{
		//xs_of_order* o = dynamic_cast<xs_of_order*>(ord);
		if (o == NULL)
		{
			//snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to xs_of_order*...\n");
			//ord->set_status(AtsType::OrderStatus::Reject);
			return 0;
		}

		DFITCInsertOrderField* request = xs_create_pool.get_mem();//xs_create_pool.get_mem();//new DFITCInsertOrderField;// 

		//memset(request, 0, sizeof(DFITCInsertOrderField));
		//strcpy(request->accountID, m_sUsername.c_str());

		request->localOrderID = ++m_nCurrentOrderRef;

		tradeitem* i = o->get_instrument();
		if (i == nullptr)
		{
			//xs_create_pool.free_mem(request);
			delete request;
			return 0;
		}
		switch (i->get_instr_type())
		{
		case AtsType::InstrType::Future:
			request->instrumentType = DFITC_COMM_TYPE;
			break;
		case AtsType::InstrType::Call:
		case AtsType::InstrType::Put:
			request->instrumentType = DFITC_OPT_TYPE;
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

		//strcpy(request->accountID, m_sUsername.c_str());
		strcpy(request->instrumentID, o->get_instrument()->get_trading_code());


		request->orderAmount = o->get_quantity();
		request->insertPrice = o->get_price();

		if (o->get_way() == AtsType::OrderWay::Buy)
			request->buySellType = DFITC_SPD_BUY;
		else if (o->get_way() == AtsType::OrderWay::Sell)
			request->buySellType = DFITC_SPD_SELL;

		if (o->get_restriction() == AtsType::OrderRestriction::None)
			request->orderProperty = DFITC_SP_NON; // or GFS ???
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
			request->orderProperty = DFITC_SP_FOK;
		else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			request->orderProperty = DFITC_SP_FAK;
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}

		DFITCOpenCloseTypeType oc = DFITC_SPD_OPEN;
		if (o->get_open_close() == OrderOpenClose::Undef)
		{
			o->set_open_close(compute_open_close(o, m_bCloseToday));
		}
		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			break;

		case AtsType::OrderOpenClose::Close:
			oc = DFITC_SPD_CLOSE;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			oc = DFITC_SPD_CLOSETODAY;
			break;

		default:
			break;


		}

		request->openCloseType = oc;
		if (m_debug)
			loggerv2::info("xs_of_connection::market_create_order openCloseFlag is %d,order_way is:%d", oc, o->get_way());


		if (!compute_userId(o, request->customCategory, sizeof(request->customCategory)))
		{
			xs_create_pool.free_mem(request);
			//delete request;
			return 0;
		}
		loggerv2::info("market_crate_order reqest.devDecInfo [%s],localID [%d]", request->customCategory, request->localOrderID);

		switch (o->get_price_mode())
		{
		case AtsType::OrderPriceMode::Limit:
		{
			request->orderType = DFITC_LIMITORDER;
			request->insertPrice = o->get_price();
		}
		break;
		case AtsType::OrderPriceMode::Market:
		{
			request->orderType = DFITC_MKORDER;
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

		request->lRequestID = m_nRequestId++;

		xs_of_order_aux::set_locId(o, request->localOrderID);

		insert_localId2order(request->localOrderID, o);

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

	int xs_of_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());
		//xs_of_order* o = dynamic_cast<xs_of_order*>(ord);
		if (o == NULL)
		{
			//snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to xs_of_order*...\n");
			//ord->set_status(AtsType::OrderStatus::Nack);
			//ord->rollback();
			return 0;
		}

		tradeitem* i = o->get_instrument();
		if (i == nullptr)
			return 0;


		DFITCCancelOrderField* request = xs_cancel_pool.get_mem(); //new DFITCCancelOrderField;//xs_cancel_pool.get_mem();
		
		//memset(request, 0, sizeof(DFITCCancelOrderField));
		//strcpy(request->accountID, m_sUsername.c_str());
		
		strcpy(request->instrumentID, o->get_instrument()->get_trading_code());

		request->spdOrderID = xs_of_order_aux::get_spdId(o);
		request->lRequestID = m_nRequestId++;
		request->localOrderID = abs(xs_of_order_aux::get_locId(o));

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
	int xs_of_connection::market_create_quote_async(quote* q, char* pszReason)
	{

		DFITCQuoteInsertField *request = quote_create_pool.get_mem();
		memset(request, 0, sizeof(DFITCQuoteInsertField));

		strcpy(request->accountID, m_sUsername.c_str());
		strcpy(request->instrumentID, q->get_instrument()->get_trading_code());
		strcpy(request->quoteID, q->get_FQR_ID().c_str());
		request->insertType = 1;
		request->instrumentType = DFITC_OPT_TYPE;
		request->localOrderID = ++m_nCurrentOrderRef;
		request->lRequestID = m_nRequestId++;

		request->sInsertPrice = q->get_ask_order()->get_price();
		request->bInsertPrice = q->get_bid_order()->get_price();

		request->sOrderAmount = q->get_ask_order()->get_quantity();
		request->bOrderAmount = q->get_bid_order()->get_quantity();

		if (q->get_bid_order()->get_open_close() == OrderOpenClose::Undef)
		{
			q->get_bid_order()->set_open_close(compute_open_close(q->get_bid_order(), m_bCloseToday));
		}

		switch (q->get_bid_order()->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			request->bOpenCloseType = DFITC_SPD_OPEN;
			break;

		case AtsType::OrderOpenClose::Close:
			request->bOpenCloseType = DFITC_SPD_CLOSE;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->bOpenCloseType = DFITC_SPD_CLOSETODAY;
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
			request->sOpenCloseType = DFITC_SPD_OPEN;
			break;

		case AtsType::OrderOpenClose::Close:
			request->sOpenCloseType = DFITC_SPD_CLOSE;
			break;
		case AtsType::OrderOpenClose::CloseToday:
			request->sOpenCloseType = DFITC_SPD_CLOSETODAY;
			break;

		default:

			break;
		}

		request->sSpeculator = DFITC_SPD_SPECULATOR;
		request->bSpeculator = DFITC_SPD_SPECULATOR;

		if (!compute_userId(q, request->customCategory, sizeof(request->customCategory)))
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
		insert_localId2quote(request->localOrderID, q);
		return 1;
	}

	int xs_of_connection::market_cancel_quote_async(quote* q, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_quote_async : %d", q->get_id());


		DFITCCancelOrderField *request = quote_cancel_pool.get_mem();
		memset(request, 0, sizeof(DFITCCancelOrderField));

		strcpy(request->accountID, m_sUsername.c_str());
		request->spdOrderID = xs_of_order_aux::get_spdId(q);
		request->lRequestID = m_nRequestId++;
		request->localOrderID = ++m_nCurrentOrderRef;

		if (!m_pUserApi->ReqQuoteCancel(request))
		{
			quote_cancel_pool.free_mem(request);
			return 0;
		}

		quote_cancel_pool.free_mem(request);
		return 1;
	}

	void xs_of_connection::OnRspQuoteInsert(struct DFITCQuoteRspField * pRspQuote, struct DFITCErrorRtnField * pErrorInfo)
	{
		if (pErrorInfo != nullptr)
		{
			loggerv2::error("xs_of_connection::OnRspQuoteInsert,localID:%d,spdID:%d,ErrorID:%d,errorMsg:%s", pErrorInfo->localOrderID, pErrorInfo->spdOrderID, pErrorInfo->nErrorID, pErrorInfo->errorMsg);

			quote *q = get_localId2quote(pErrorInfo->localOrderID);

			if (q == NULL) // should not happen
			{
				loggerv2::error("x1_connection::OnRspQuoteActionAsync - quote recovered NULL");
				return;
			}

			on_nack_quote_from_market_cb(q, NULL);
		}
	}

	void  xs_of_connection::OnRspQuoteCancel(struct DFITCQuoteRspField * pRspQuoteCancel, struct DFITCErrorRtnField * pErrorInfo)
	{
	
	}

	void  xs_of_connection::OnRtnQuoteInsert(struct DFITCQuoteRtnField * pRtnQuoteData)
	{
		if (m_debug)
			loggerv2::info("x1_connection::OnRtnQuoteInsert-",
			"spdOrderID[%d]"
			"orderSysID[%d]"
			"LocalOrderID[%d]"
			"QuoteID[%s]"
			"InstrumentID[%s]"
			"InstrumentType[%d]"
			"BuyOpenCloseType[%d]"
			"BuyOrderAmount[%d]"
			"SellOpenCloseType[%d]"
			"SellOrderAmount[%d]"
			"errorMsg[%s]"
			"CustomCategory[%s]"
			"OrderStatus[%s]"
			,
			pRtnQuoteData->spdOrderID,
			pRtnQuoteData->orderSysID,
			pRtnQuoteData->localOrderID,
			pRtnQuoteData->quoteID,
			pRtnQuoteData->instrumentID,
			pRtnQuoteData->instrumentType,
			pRtnQuoteData->bOpenCloseType,
			pRtnQuoteData->bOrderAmount,
			pRtnQuoteData->sOpenCloseType,
			pRtnQuoteData->sOrderAmount,
			pRtnQuoteData->errorMsg,
			pRtnQuoteData->customCategory,
			pRtnQuoteData->orderStatus
			);

		int account, bidId, askId, portfolioId, tradingType;
		get_user_info(pRtnQuoteData->customCategory, account, bidId, askId, portfolioId, tradingType);

		int orderId = bidId;
		bool isRebuild = false;
		if (m_begin_Id >= orderId)
			isRebuild = true;

		quote *q = nullptr;
		if (orderId <= 0)
		{
			q = get_localId2quote(pRtnQuoteData->localOrderID);
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

				q = xs_of_order_aux::anchor(this, pRtnQuoteData);
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

		//std::string FQR_ID = std::to_string(pRtnQuoteData->spdOrderID);
		//q->set_FQR_ID(FQR_ID);

		xs_of_order_aux::set_spdId(q, pRtnQuoteData->spdOrderID);
		if (q->get_bid_order() && q->get_bid_order()->get_quantity() != pRtnQuoteData->bOrderAmount)
		{
			if (m_debug)
				loggerv2::debug("x1_connection::OnRtnQuoteInsert resetting bid order quantity to %d", pRtnQuoteData->bOrderAmount);
			q->get_bid_order()->set_quantity(pRtnQuoteData->bOrderAmount);
		}

		if (q->get_ask_order() && q->get_ask_order()->get_quantity() != pRtnQuoteData->sOrderAmount)
		{
			if (m_debug)
				loggerv2::debug("x1_connection::OnRtnQuoteInsert resetting ask order quantity to %d", pRtnQuoteData->sOrderAmount);
			q->get_ask_order()->set_quantity(pRtnQuoteData->sOrderAmount);
		}

		switch (pRtnQuoteData->orderStatus)
		{
		case DFITC_SPD_TRIGGERED:
		case DFITC_SPD_IN_QUEUE:
		case DFITC_SPD_PLACED:
		case DFITC_SPD_PARTIAL:
		{
			if (isRebuild == false)//历史回包不参与pending计算
			{
				update_instr_on_ack_from_market_cb(q->get_bid_order());
				update_instr_on_ack_from_market_cb(q->get_ask_order());
			}
			on_ack_quote_from_market_cb(q);
			break;
		}
		case DFITC_SPD_FILLED: //    
			break;
		case DFITC_SPD_ERROR:
		{
			on_nack_quote_from_market_cb(q, "OrderRtn api reject");
			break;
		}
		case DFITC_SPD_PARTIAL_CANCELED:
		case DFITC_SPD_CANCELED:
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

	void  xs_of_connection::OnRtnQuoteCancel(struct DFITCQuoteCanceledRtnField * pRtnQuoteCanceledData)
	{
		if (m_debug)
			loggerv2::info("xs_of_connection::OnRtnQuoteCancel-",
			"spdOrderID[%d]"
			"LocalOrderID[%d]"
			"QuoteID[%s]"
			"InstrumentID[%s]"
			"CancelAmount[%d]"
			"BuyOpenCloseType[%d]"
			"OrderStatus[%s]"
			"SellOpenCloseType[%d]"
			"CanceledTime[%s]"
			"orderSysID[%s]"
			"CustomCategory[%s]"
			,
			pRtnQuoteCanceledData->spdOrderID,
			pRtnQuoteCanceledData->localOrderID,
			pRtnQuoteCanceledData->quoteID,
			pRtnQuoteCanceledData->instrumentID,
			pRtnQuoteCanceledData->cancelAmount,
			pRtnQuoteCanceledData->bOpenCloseType,
			pRtnQuoteCanceledData->orderStatus,
			pRtnQuoteCanceledData->sOpenCloseType,
			pRtnQuoteCanceledData->canceledTime,
			pRtnQuoteCanceledData->orderSysID,
			pRtnQuoteCanceledData->customCategory
			);

		int account, bidId, askId, portfolioId, tradingType;
		get_user_info(pRtnQuoteCanceledData->customCategory, account, bidId, askId, portfolioId, tradingType);

		int orderId = bidId;
		bool isRebuild = false;
		if (m_begin_Id >= orderId)
			isRebuild = true;

		quote *q = nullptr;
		if (orderId <= 0 && isRebuild == false)
		{
			q = get_localId2quote(pRtnQuoteCanceledData->localOrderID);
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

				//q = xs_of_order_aux::anchor(this, pRtnQuoteCanceledData);
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

		//std::string FQR_ID = std::to_string(pRtnQuoteCanceledData->spdOrderID);
		//q->set_FQR_ID(FQR_ID);

		xs_of_order_aux::set_spdId(q, pRtnQuoteCanceledData->spdOrderID);

		switch (pRtnQuoteCanceledData->orderStatus)
		{

		case DFITC_SPD_PARTIAL_CANCELED:
		case DFITC_SPD_CANCELED:
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
		case DFITC_SPD_ERROR:
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
	void  xs_of_connection::OnRtnQuoteMatchedInfo(struct DFITCQuoteMatchRtnField * pRtnQuoteMatchedData)
	{
		if (m_debug)
			loggerv2::info("x1_connection::OnRtnQuoteMatchedInfo-",
			"spdOrderID[%d]"
			"LocalOrderID[%d]"
			"QuoteID[%s]"
			"InstrumentID[%s]"
			"MatchedAmount[%d]"
			"MatchedPrice[%f]"
			"OrderStatus[%s]"
			"BuySellType[%d]"
			"MatchedTime[%s]"
			"OrderAmount[%d]"
			"insertPrice[%f]"
			"CustomCategory[%s]"
			,
			pRtnQuoteMatchedData->spdOrderID,
			pRtnQuoteMatchedData->localOrderID,
			pRtnQuoteMatchedData->quoteID,
			pRtnQuoteMatchedData->instrumentID,
			pRtnQuoteMatchedData->matchedAmount,
			pRtnQuoteMatchedData->matchedPrice,
			pRtnQuoteMatchedData->orderStatus,
			pRtnQuoteMatchedData->buySellType,
			pRtnQuoteMatchedData->matchedTime,
			pRtnQuoteMatchedData->matchedAmount,
			pRtnQuoteMatchedData->insertPrice,
			pRtnQuoteMatchedData->customCategory
			);

		int account, bidId, askId, portfolioId, tradingType;
		get_user_info(pRtnQuoteMatchedData->customCategory, account, bidId, askId, portfolioId, tradingType);

		int orderId = bidId;
		bool isRebuild = false;
		if (m_begin_Id >= orderId)
			isRebuild = true;

		quote *q = nullptr;
		if (orderId <= 0 && isRebuild == false)
		{
			q = get_localId2quote(pRtnQuoteMatchedData->localOrderID);
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

				//q = x1_order_aux::anchor(this, pRtnQuoteMatchedData);
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

		//std::string FQR_ID = std::to_string(pRtnQuoteMatchedData->spdOrderID);
		//q->set_FQR_ID(FQR_ID);

		xs_of_order_aux::set_spdId(q, pRtnQuoteMatchedData->spdOrderID);
	}

	//Sec Trade Ack
	void xs_of_connection::OnRspInsertOrder(struct DFITCOrderRspDataRtnField * pData, struct DFITCErrorRtnField * pRspInfo)
	{
		if (pRspInfo != nullptr)
		{
			loggerv2::info("xs_of_connection::OnRspSOPEntrustOrder -->  localOrderID %d spdOrderID %d errorID %d errorMsg %s", pRspInfo->localOrderID, pRspInfo->spdOrderID, pRspInfo->nErrorID, pRspInfo->errorMsg);
			//todo find order and nack
			int localOrderID = pRspInfo->localOrderID;
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

				snprintf(pszReason, REASON_MAXLENGTH, "error id %d.", pRspInfo->nErrorID);
				on_nack_from_market_cb(o, pszReason);

				m_localId2order.erase(localOrderID);
			}
			
			
		}//error return

		m_orderRspQueue.CopyPush(pData);
	}
	void xs_of_connection::OnRspCancelOrder(struct DFITCOrderRspDataRtnField * pOrderCanceledRtn, struct DFITCErrorRtnField * pErrorInfo)
	{
		if (pOrderCanceledRtn != nullptr)
		{
			loggerv2::info("委托撤单响应[本地委托号]:[%ld],[柜台委托号]:[%ld],[委托状态]:[%d]\n",
				pOrderCanceledRtn->localOrderID,
				pOrderCanceledRtn->spdOrderID,
				pOrderCanceledRtn->orderStatus
				);

			int localOrderID = pOrderCanceledRtn->localOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				if (m_debug)
					loggerv2::info("xs_of_connection::OnRspSOPWithdrawOrder localOrderID %d", pOrderCanceledRtn->localOrderID);
				/*xs_of_order* o = it->second;
				if (o != nullptr)
				{
				on_ack_from_market_cb(o);

				}*/
				m_localId2order.erase(localOrderID);
			}
		}

		else if (pErrorInfo != nullptr)
		{
			loggerv2::info("委托撤单响应发生错误![accountID]:[%s],[会话ID]:[%ld],[本地委托号]:[%ld],[柜台委托号]:[%ld],[ReqID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->accountID,
				pErrorInfo->sessionID,
				pErrorInfo->localOrderID,
				pErrorInfo->spdOrderID,
				pErrorInfo->requestID,
				pErrorInfo->nErrorID,
				pErrorInfo->errorMsg
				);

			//cancel is bad

			int localOrderID = pErrorInfo->localOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				char pszReason[REASON_MAXLENGTH + 1];
				memset(pszReason, 0, sizeof(pszReason));
				pszReason[REASON_MAXLENGTH] = '\0';

				snprintf(pszReason, REASON_MAXLENGTH, "error id %d.", pErrorInfo->nErrorID);
				on_nack_from_market_cb(o, pszReason);


				m_localId2order.erase(localOrderID);
			}
		}
	}
	void xs_of_connection::OnRspError(DFITCErrorRtnField *pRspInfo)
	{
		loggerv2::error("OnRspError : need to implement");
	}

	//Exchange Trade Ack
	void xs_of_connection::OnRtnOrder(struct DFITCOrderRtnField * pData)
	{
		//push to queue

		//DFITCOrderRtnField* o = xs_order_rtn_pool.get_mem();//new DFITCOrderRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCOrderRtnField));
		//m_orderRtnQueue.Push(o);
		m_orderRtnQueue.CopyPush(pData);
	}
	void xs_of_connection::OnRtnCancelOrder(struct DFITCOrderCanceledRtnField * pData)
	{
		//push to queue
		//DFITCOrderCanceledRtnField* o = xs_order_can_rtn_pool.get_mem();//new DFITCOrderCanceledRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCOrderCanceledRtnField));
		//m_ordCanRtnQueue.Push(o);
		m_ordCanRtnQueue.CopyPush(pData);
	}
	void xs_of_connection::OnRtnMatchedInfo(struct DFITCMatchRtnField * pData)
	{
		//push to queue
		//DFITCMatchRtnField* trade = xs_trade_pool.get_mem();//new DFITCMatchRtnField;
		//memcpy_lw(trade, pData, sizeof(DFITCMatchRtnField));
		//m_tradeQueue.Push(trade);
		m_tradeQueue.CopyPush(pData);
	}

	//Handler
	void xs_of_connection::OnRtnOrderAsyn(DFITCOrderRtnField * pOrder)
	{
		std::string statusMsg;
		if (pOrder->orderStatus == 3)
			statusMsg = "报单状态：未成交还在队列中！";
		if (pOrder->orderStatus == 6)
			statusMsg = "报单状态：正在撤单中！";
		loggerv2::info("委托回报[本地委托号]:[%ld],[柜台委托号]:[%ld],%s,[委托状态]:[%d],[交易所报单编号]:[%s],[会话ID]:[%ld],[申报时间]:[%s],\
					   					   					   					   					   					   					   					 					[合约代码]:[%s],[交易所ID]:[%s],[买卖]:[%d],[开平]:[%d],[合约类型]:[%d],[投保]:[%d],[资金帐号ID]:[%s],[撤单数量]:[%ld],[委托价格]:[%lf],[委托数量]:[%ld]",
																																													pOrder->localOrderID,
																																													pOrder->spdOrderID,
																																													statusMsg.c_str(),
																																													pOrder->orderStatus,
																																													pOrder->OrderSysID,
																																													pOrder->sessionID,
																																													pOrder->SuspendTime,
																																													pOrder->instrumentID,
																																													pOrder->exchangeID,
																																													pOrder->buySellType,
																																													pOrder->openCloseType,
																																													pOrder->instrumentType,
																																													pOrder->speculator,
																																													pOrder->accountID,
																																													pOrder->cancelAmount,
																																													pOrder->insertPrice,
																																													pOrder->orderAmount
																																													);
		//xs_of_order* o = NULL;
		//order *o = nullptr;

		int localID = pOrder->localOrderID;
		int orderId = get_order_id(pOrder->customCategory);

		if (m_nCurrentOrderRef < localID)
			m_nCurrentOrderRef = localID;

		//auto itr = m_used_locId.find(localID);
		if (contain_used_locId(localID))
		{
			return;//该localId已经处理过。
		}

		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_of_order*>(ord);
			break;
		case 1:
			loggerv2::info("xs_of_connection::OnRtnOrderAsyn - message received on dead order[%d]...", orderId);
			//o = reinterpret_cast<xs_of_order*>(ord);
			break;

		case 2:

			o = xs_of_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("xs_of_connection::OnRtnOrderAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;
		}



		if (o == NULL) // should not happen
		{
			loggerv2::error("xs_of_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}

		insert_used_locId(orderId);
		xs_of_order_aux::set_spdId(o, pOrder->spdOrderID);

		int ackQty = pOrder->orderAmount;

		switch (pOrder->orderStatus)
		{
		case DFITC_SPD_TRIGGERED:
		case DFITC_SPD_IN_QUEUE:
		case DFITC_SPD_PLACED:
		case DFITC_SPD_PARTIAL:
		{

			//if (tradeTime > last)//历史回包不参与pending计算
			{
				update_instr_on_ack_from_market_cb(o, ackQty);
			}
			on_ack_from_market_cb(o);
			break;
		}
		case DFITC_SPD_FILLED: //    
			break;
		case DFITC_SPD_ERROR:
		{
			on_nack_from_market_cb(o, "OrderRtn api reject");
			break;
		}
		case DFITC_SPD_PARTIAL_CANCELED:
		case DFITC_SPD_CANCELED:
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
	void xs_of_connection::OnRspInsertOrderAsyn(DFITCOrderRspDataRtnField * pData)
	{
		if (pData != nullptr)
		{
			int localOrderID = pData->localOrderID;

			//auto itr = m_used_locId.find(localOrderID);
			if (contain_used_locId(localOrderID))
			{
				return;//该localId已经处理过。
			}

			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{

				xs_of_order_aux::set_spdId(o, pData->spdOrderID);

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
	void xs_of_connection::OnRtnCancelOrderAsyn(struct DFITCOrderCanceledRtnField * pCancelOrderData)
	{
		if (m_debug)
			loggerv2::info("xs_of_connection::OnRspOrderActionAsync");
		int nOrderId = get_order_id(pCancelOrderData->customCategory);
		int localID = pCancelOrderData->localOrderID;
		bool isOldPacket = false;

		if (m_begin_Id >= nOrderId)
			isOldPacket = true;

		//xs_of_order* o = nullptr;

		int ret;
		order *o = get_order_from_map(nOrderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			loggerv2::info("xs_of_connection::OnRtnCancelOrderAsyn - message received on dead order[%d]...", nOrderId);
			break;
		default:
			break;
		}


		if (o == nullptr) // should not happen
		{
			loggerv2::error("xs_of_connection::OnRspOrderActionAsync - order recovered NULL");
			return;
		}

		int cancelQty = pCancelOrderData->cancelAmount;

		switch (pCancelOrderData->orderStatus)
		{

		case DFITC_SPD_PARTIAL_CANCELED:
		case DFITC_SPD_CANCELED:
		{

			if (get_lwtp_now() > o->get_instrument()->get_last_sychro_timepoint() && isOldPacket == false)
			{
				update_instr_on_cancel_from_market_cb(o, cancelQty);
			}
			//else
				//m_statistics.incr_can();
			o->set_status(AtsType::OrderStatus::Cancel);

			//on_nack_from_market_cb(o, NULL);
			on_cancel_from_market_cb(o);
			break;
		}
		/*case DFITCSEC_DR_Declaring:
		case DFITCSEC_DR_PartTrade:
		case DFITCSEC_DR_UnTrade:
		case DFITCSEC_DR_Confirm:
		case DFITCSEC_DR_UnDeclare:
		{
		o->rollback();
		o->set_status(OrderStatus::Ack);
		loggerv2::warn("xs_connection::OnSOPWithdrawOrderRtnAsyn,waiting for cancle,orderID:%d", o->get_id());
		break;
		}*/
		case DFITC_SPD_ERROR:
		{
			o->rollback();
			o->set_status(OrderStatus::Ack);
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
	void xs_of_connection::OnRtnMatchedInfoAsyn(struct DFITCMatchRtnField* pTrade)
	{
		loggerv2::info("成交回报[本地委托号]:[%ld],[交易所报单编号]:[%s],[成交编号]:[%s],[合约代码]:[%s],[买卖]:[%d],[开平]:[%d],[matchPrice]:[%lf],[委托数量]:[%ld],[成交数量]:[%ld],[成交时间]:[%s],[委托价格]:[%lf],\
					   					   					   					 					 					[柜台委托号]:[%ld],[成交类型]:[%ld],[投保]:[%d],[交易所ID]:[%s],[手续费]:[%lf],[sessionID]:[%ld],[合约类型]:[%d],[accountID]:[%s],[申报结果]:[%d]\n",
																														pTrade->localOrderID,
																														pTrade->OrderSysID,
																														pTrade->matchID,
																														pTrade->instrumentID,
																														pTrade->buySellType,
																														pTrade->openCloseType,
																														pTrade->matchedPrice,
																														pTrade->orderAmount,
																														pTrade->matchedAmount,
																														//pTrade->leftAmount, [剩余数量]:[%ld],
																														pTrade->matchedTime,
																														pTrade->insertPrice,
																														pTrade->spdOrderID,
																														pTrade->matchType,
																														pTrade->speculator,
																														pTrade->exchangeID,
																														pTrade->fee,
																														pTrade->sessionID,
																														pTrade->instrumentType,
																														pTrade->accountID,
																														pTrade->orderStatus
																														);

		//xs_of_order* o = NULL;
		bool duplicat = false;
		// 1 - retrieve order
		int orderId = get_order_id(pTrade->customCategory);
		int localID = pTrade->localOrderID;

		if (orderId == -1)
		{
			return;
		}

		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			break;
		case 1:
			loggerv2::info("xs_of_connection::OnRtnMatchedInfoAsyn - message received on dead order[%d]...", orderId);
			//o = reinterpret_cast<xs_of_order*>(ord);//it->second);
			break;
		case 2:

			o = xs_of_order_aux::anchor(this, pTrade);
			if (o == NULL)
			{
				loggerv2::error("xs_of_connection::OnRtnMatchedInfoAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;
		}





		if (o == NULL) // should not happen
		{
			loggerv2::error("xs_of_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}


		// 2 - treat message
		int execQty = pTrade->matchedAmount;
		double execPrc = pTrade->matchedPrice;
		//const char* pszExecRef = pTrade->matchID;
		//std::string pszExecRef = std::string(pTrade->matchID) + std::to_string(o->get_way());

		const char* pszTime = pTrade->matchedTime;


		exec* e = new exec(o, std::string(pTrade->matchID), execQty, execPrc, pszTime);
		//std::shared_ptr<exec> e(new exec(o, pszExecRef, execQty, execPrc, pszTime));

		on_exec_from_market_cb(o, e, duplicat);
		if (duplicat)
		{
			loggerv2::info("duplicat packet,drop");
			return;
		}

		update_instr_on_exec_from_market_cb(o, e, false);

	}


	int xs_of_connection::get_xs_instype(AtsType::InstrType::type _type)
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

	std::string xs_of_connection::getMaturity(std::string sMat)
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
	void xs_of_connection::OnRspQryOrderInfo(struct DFITCOrderCommRtnField *pRtnOrderData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != NULL)
		{
			loggerv2::error("OnRspQryOrderInfo error![accountID]:[%s],[sessionID]:[%ld],[localOrderID]:[%ld],[spdOrderID]:[%ld],[requestID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->accountID,
				pErrorInfo->sessionID,
				pErrorInfo->localOrderID,
				pErrorInfo->spdOrderID,
				pErrorInfo->requestID,
				pErrorInfo->nErrorID,
				pErrorInfo->errorMsg); //打印“错误ID,错误信息,本地委托号”

		}

		loggerv2::error("OnRspQryOrderInfo[localOrderID]:[%ld],[lRequestID]:[%ld],[instrumentID]:[%s],[spdOrderID]:[%ld],[orderStatus]:[%d],[exchangeID]:[%s],[buySellType]:[%d],[openClose]:[%d],[insertPrice]:[%lf],[orderAmount]:[%ld],[matchedPrice]:[%lf],\
																																													 [matchedAmount]:[%ld],[cancelAmount]:[%ld],[insertType]:[%d],[speculator]:[%d],[commTime]:[%s],[clientID]:[%s],[exchangeID]:[%s],[submitTime]:[%s],[operStation]:[%s],[accountID]:[%s],[instrumentType]:[%d],[bIsLast]:[%d]\n",
																																													 pRtnOrderData->localOrderID,//本地ID
																																													 pRtnOrderData->lRequestID,//ReqID
																																													 pRtnOrderData->instrumentID,//合约代码
																																													 pRtnOrderData->spdOrderID,//柜台委托号
																																													 pRtnOrderData->orderStatus,//申报结果
																																													 pRtnOrderData->exchangeID,
																																													 pRtnOrderData->buySellType,
																																													 pRtnOrderData->openClose,
																																													 pRtnOrderData->insertPrice,
																																													 pRtnOrderData->orderAmount,
																																													 pRtnOrderData->matchedPrice,
																																													 pRtnOrderData->matchedAmount,
																																													 pRtnOrderData->cancelAmount,
																																													 pRtnOrderData->insertType,
																																													 pRtnOrderData->speculator,
																																													 pRtnOrderData->commTime,
																																													 pRtnOrderData->clientID,
																																													 pRtnOrderData->exchangeID,
																																													 pRtnOrderData->submitTime,
																																													 pRtnOrderData->operStation,
																																													 pRtnOrderData->accountID,
																																													 pRtnOrderData->instrumentType,
																																													 bIsLast);

		//xs_of_order* o = NULL;
		bool isRebuild = true;

		int locID = pRtnOrderData->localOrderID;
		if (m_nCurrentOrderRef < abs(locID))
			m_nCurrentOrderRef = abs(locID);

		order* o = xs_of_order_aux::anchor(this, pRtnOrderData);

		if (o != NULL&&o->get_id()>0)
		{
			add_pending_order(o);

			insert_used_locId(locID);
			xs_of_order_aux::set_spdId(o, pRtnOrderData->spdOrderID);

			int ackQty = pRtnOrderData->orderAmount;

			switch (pRtnOrderData->orderStatus)
			{
			case DFITC_SPD_TRIGGERED:
			case DFITC_SPD_IN_QUEUE:
			case DFITC_SPD_PLACED:
			case DFITC_SPD_PARTIAL:
			{
				on_ack_from_market_cb(o);
				break;
			}
			case DFITC_SPD_FILLED: //    
				break;
			case DFITC_SPD_ERROR:
			{
				on_nack_from_market_cb(o, "OrderRtn api reject");
				break;
			}
			case DFITC_SPD_PARTIAL_CANCELED:
			case DFITC_SPD_CANCELED:
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
			loggerv2::error("xs_of_connection::OnRspQryOrderInfo cannot anchor order");
		}

		if (bIsLast == true)
		{
			if (m_bFuOrder == false)
			{
				sleep_by_milliseconds(2000);
				loggerv2::info("query Future orderinsert");
				OnQryFutureOrders();
				m_bFuOrder = true;
			}
			else if (m_bOpTrade == false)
			{
				sleep_by_milliseconds(2000);
				loggerv2::info("query Option match");
				OnQryOpMatches();
				m_bOpTrade = true;
			}
		}

	}

	void xs_of_connection::OnRspQryMatchInfo(struct DFITCMatchedRtnField *pTrade, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast)
	{
		if (pErrorInfo != NULL){

			loggerv2::error("OnRspQryMatchInfo error![accountID]:[%s],[sessionID]:[%ld],[localOrderID]:[%ld],[spdOrderID]:[%ld],[requestID]:[%d],[ErrorID]:[%d],[ErrorMsg]:[%s]",
				pErrorInfo->accountID,
				pErrorInfo->sessionID,
				pErrorInfo->localOrderID,
				pErrorInfo->spdOrderID,
				pErrorInfo->requestID,
				pErrorInfo->nErrorID,
				pErrorInfo->errorMsg); //打印“错误ID,错误信息,本地委托号”

		}

		loggerv2::info("OnRspQryMatchInfo[lRequestID]:[%ld],[spdOrderID]:[%ld],[exchangeID]:[%s],[instrumentID]:[%s],[buySellType]:[%d],[openClose]:[%d],[matchedPrice]:[%lf],[matchedAmount]:[%ld],\
					   					   					   					   				   					   				   					 [matchedMort]:[%lf],[speculator]:[%d],[matchedTime]:[%s],[matchedID]:[%s],[localOrderID]:[%ld],[clientID]:[%s],[matchType]:[%d],[instrumentType]:[%d],[bIsLast]:[%d]",
																																						 pTrade->lRequestID,//ReqID
																																						 pTrade->spdOrderID,//柜台委托号
																																						 pTrade->exchangeID,//交易所ID
																																						 pTrade->instrumentID,//合约代码
																																						 pTrade->buySellType,//买卖
																																						 pTrade->openClose,//开平
																																						 pTrade->matchedPrice,//matchPrice
																																						 pTrade->matchedAmount,//成交数量
																																						 pTrade->matchedMort,//成交金额
																																						 pTrade->speculator,//投保类别
																																						 pTrade->matchedTime,//成交时间
																																						 pTrade->matchedID,//成交编号
																																						 pTrade->localOrderID,//本地委托号
																																						 pTrade->clientID,//交易编码
																																						 pTrade->matchType,//成交类型
																																						 pTrade->instrumentType,//合约类型
																																						 bIsLast);


		bool duplicat = false;
		bool isOldPacket = true;
		// 1 - retrieve order
		int orderId = get_order_id(pTrade->customCategory);
		if (orderId > 0)
		{
			int localID = pTrade->localOrderID;

			if (m_nCurrentOrderRef < abs(localID))
				m_nCurrentOrderRef = abs(localID);

			int ret;
			order *o = get_order_from_map(orderId, ret);
			switch (ret)
			{
			case 0:
				//o = reinterpret_cast<xs_of_order*>(ord);
				break;
			case 1:
				//o = reinterpret_cast<xs_of_order*>(ord);
				loggerv2::info("cffex_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
				break;

			case 2:
				o = xs_of_order_aux::anchor(this, pTrade);
				if (o)
				add_pending_order(o);
				break;
			default:
				break;
			}

			if (o != NULL)
			{
				// 2 - treat message

				int execQty = pTrade->matchedAmount;
				double execPrc = pTrade->matchedPrice;
				//const char* pszExecRef = pTrade->matchedID;
				const char* pszTime = pTrade->matchedTime;


				exec* e = new exec(o, string(pTrade->matchedID), execQty, execPrc, pszTime);
				//std::shared_ptr<exec> e(new exec(o, pszExecRef, execQty, execPrc, pszTime));

				on_exec_from_market_cb(o, e, duplicat);

			}
		}

		//历史成交，不参与持仓更新。
		//update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending, isOldPacket);
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

	void xs_of_connection::OnQryOpOrders()
	{
		struct DFITCOrderField data;
		memset(&data, 0, sizeof(data));

		data.lRequestID = m_nRequestId++;
		data.instrumentType = DFITC_OPT_TYPE;
		strcpy(data.accountID, m_sUsername.c_str());

		m_pUserApi->ReqQryOrderInfo(&data);
	}
	void xs_of_connection::OnQryFutureOrders()
	{
		struct DFITCOrderField data;
		memset(&data, 0, sizeof(data));

		data.lRequestID = m_nRequestId++;
		data.instrumentType = DFITC_COMM_TYPE;
		strcpy(data.accountID, m_sUsername.c_str());

		m_pUserApi->ReqQryOrderInfo(&data);
	}
	void xs_of_connection::OnQryOpMatches()
	{
		struct DFITCMatchField data;
		memset(&data, 0, sizeof(data));
		data.lRequestID = m_nRequestId++;
		strcpy(data.accountID, m_sUsername.c_str());
		data.instrumentType = DFITC_OPT_TYPE;

		m_pUserApi->ReqQryMatchInfo(&data);
		return;
	}
	void xs_of_connection::OnQryFutureMatches()
	{
		struct DFITCMatchField data;
		memset(&data, 0, sizeof(data));
		data.lRequestID = m_nRequestId++;
		strcpy(data.accountID, m_sUsername.c_str());
		data.instrumentType = DFITC_COMM_TYPE;

		m_pUserApi->ReqQryMatchInfo(&data);
		return;
	}

	//order* xs_of_connection::create_order()
	//{
	//	return new xs_of_order(this);
	//}

	//void xs_of_connection::cancel_num_warning(tradeitem* i)
	//{
	//	loggerv2::warn("tradeitem:%s cancel num is more than warning lev", i->getCode().c_str());
	//	this->setTradingAllowed(false);
	//}

	//void xs_of_connection::cancel_num_ban(tradeitem* i)
	//{
	//	if (i->get_instr_type() == InstrType::Future)
	//	{
	//		loggerv2::warn("tradeitem:%s cancel num is more than forbid lev", i->getCode().c_str());
	//		this->setTradingAllowed(false);
	//		i->set_cancel_forbid(true);
	//	}
	//}

	void xs_of_connection::insert_localId2order(int id, order* o)
	{
		tbb::concurrent_hash_map<int, order*>::accessor wa;
		m_localId2order.insert(wa, id);
		wa->second = o;
	}

	order *xs_of_connection::get_localId2order(int id)
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

	void xs_of_connection::insert_used_locId(int id)
	{
		tbb::concurrent_hash_map<int, int>::accessor wa;
		m_used_locId.insert(wa, id);
		wa->second = id;
	}

	bool xs_of_connection::contain_used_locId(int id)
	{
		tbb::concurrent_hash_map<int, int>::const_accessor ra;
		if (m_used_locId.find(ra, id))
			return true;
		else
			return false;
	}

	void xs_of_connection::insert_spId2order(long id, order* o)
	{
		tbb::concurrent_hash_map<int, order*>::accessor wa;
		m_spId2order.insert(wa, id);
		wa->second = o;
	}

	order *xs_of_connection::get_spId2order(long id)
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

	void xs_of_connection::insert_localId2quote(int id, quote* o)
	{
		tbb::concurrent_hash_map<int, quote*>::accessor wa;
		m_localId2quote.insert(wa, id);
		wa->second = o;
	}

	quote *xs_of_connection::get_localId2quote(int id)
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

	void xs_of_connection::insert_spId2quote(long id, quote* o)
	{
		tbb::concurrent_hash_map<int, quote*>::accessor wa;
		m_spId2quote.insert(wa, id);
		wa->second = o;
	}
	quote *xs_of_connection::get_spId2quote(long id)
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
}



