#include "xs_connection.h"
#include "sqliteclient.h"
#include <vector>
#include "tradeItem_gh.h"
#include "boost/lexical_cast.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/algorithm/string.hpp"
#include <istream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "terra_logger.h"
#include "order_reference_provider.h"
#include <boost/property_tree/ini_parser.hpp>

using namespace terra::common;

namespace xs
{
	xs_connection::xs_connection(const std::string &path,bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "xs_connection";

		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		//m_debug = false;
		auto now = get_lwtp_now();
		//m_last_time=now;

		m_path = path;
		m_startTime = now;
		m_begin_Id = order_reference_provider::get_instance().get_current_int();

	}

	bool xs_connection::init_config(const string &name, const std::string &strConfigFile)
	{
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

		m_sStockUsername = root.get<string>(name + ".stock_username", "");
		m_sStockPassword = root.get<string>(name + ".stock_password", "");

		//readLocal2Portfolio();
		//std::thread t1(std::bind(&xs_connection::ackBackup, this));
		xs_create_pool.init(32);
		xs_cancel_pool.init(32);

		std::list<DFITCSOPReqEntrustOrderField*> mlist1;
		std::list<DFITCSECReqWithdrawOrderField*> mlist2;

		unsigned int i = 0;
		while (i < xs_create_pool.mlen)
		{
			DFITCSOPReqEntrustOrderField* ptr = xs_create_pool.get_mem();
			memset((void *)ptr, 0, sizeof(DFITCSOPReqEntrustOrderField));
			strcpy(ptr->accountID, m_sUsername.c_str());
			//xs_create_pool.free_mem(ptr);
			++i;
			mlist1.push_back(ptr);
		}

		i = 0;
		while (i < xs_cancel_pool.mlen)
		{
			DFITCSECReqWithdrawOrderField* ptr2 = xs_cancel_pool.get_mem();
			memset((void *)ptr2, 0, sizeof(DFITCSECReqWithdrawOrderField));
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

	int xs_connection::get_xs_instype(AtsType::InstrType::type _type)
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

	void xs_connection::request_investor_full_positions()
	{
		request_op_positions();
		//request_stock_positions();
	}
	void xs_connection::req_RiskDegree()
	{
		/*
		/**
		* SOP-资金信息查询请求
		* @param p:指向用户资金信息查询请求结构的地址
		* @return 0表示请求发送成功，其他值表示请求发送失败，具体错误请对照error.xml
		*/
		//virtual int ReqSOPQryCapitalAccountInfo(DFITCSOPReqQryCapitalAccountField *p) = 0;
		

		DFITCSOPReqQryCapitalAccountField QryCapitalAccountField;
		memset(&QryCapitalAccountField, 0, sizeof(DFITCSOPReqQryCapitalAccountField));
		strcpy(QryCapitalAccountField.accountID, m_sUsername.c_str());
		QryCapitalAccountField.requestID = ++m_nRequestId;
		int ret = m_pUserApi->ReqSOPQryCapitalAccountInfo(&QryCapitalAccountField);
		loggerv2::info("xs_connection::req_RiskDegree call the ReqSOPQryCapitalAccountInfo ret:%d",ret);
	}
	void xs_connection::OnRspSOPQryCapitalAccountInfo(DFITCSOPRspQryCapitalAccountField *pData, DFITCSECRspInfoField *pRspInfo)
	{
		/*
		//SOP-客户资金查询响应
		struct APISTRUCT DFITCSOPRspQryCapitalAccountField
		{
		DFITCSECRequestIDType                requestID;                //请求ID
		DFITCSECAccountIDType                accountID;                //客户号
		DFITCSECBranchIDType                 branchID;                 //机构编码
		DFITCSECCurrencyType                 currency;                 //币种
		DFITCSECFundsType                    accountBanlance;          //账户余额
		DFITCSECFundsType                    availableFunds;           //可用资金
		DFITCSECFundsType                    freezeFunds;              //冻结资金
		DFITCSECFundsType                    anticipatedInterest;      //预计利息
		DFITCSECFundsType                    usedDeposit;              //占用保证金
		DFITCSECAccountStatusType            accountStatus;            //客户状态
		DFITCSECFundsType                    totalFunds;               //总资金
		DFITCSECFundsType                    totalMarket;              //总市值
		DFITCSECFundsType                    cashAssets;               //现金资产
		DFITCSECFundsType                    execGuaranteeRatio;       //履约担保比例
		DFITCSECFundsType                    buyLimits;                //买入额度
		};
		*/
		if (pData != nullptr)
		{
			double ratio = 0;
			if (pData->availableFunds + pData->usedDeposit != 0)
			{
				ratio = pData->usedDeposit / (pData->availableFunds + pData->usedDeposit);
			}
			loggerv2::info("xs_connection::OnRspSOPQryCapitalAccountInfo,"
				          "accountID:%s,"
						  "availableFunds:%f,"
						  "usedDeposit:%f,"
						  "accountStatus:%d,"
						  "ratio:%f", pData->accountID, pData->availableFunds, pData->usedDeposit, pData->accountStatus,ratio);

			this->set_RiskDegree(ratio);
		}
	}
	void xs_connection::request_op_positions()
	{
		//loggerv2::info("");
		DFITCSOPReqQryPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.accountID, m_sUsername.c_str());
		//strcpy(pRequest.exchangeID, "SH");
		pRequest.requestID = ++m_nRequestId;
		if (m_pUserApi->ReqSOPQryPosition(&pRequest) != 0)
			loggerv2::error("xs_connection::request_op_positions failed");
		//if (m_debug)
		//	loggerv2::info("xs_connection:: calling OnRspQryInvestorPosition ");
	}

	void xs_connection::request_stock_positions()
	{
		DFITCStockReqQryPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.accountID, m_sStockUsername.c_str());
		pRequest.requestID = ++m_nRequestId;
		if (m_pUserApi->ReqStockQryPosition(&pRequest) != 0)
			loggerv2::error("xs_connection::request_stock_positions failed");
		//if (m_debug)
		// loggerv2::info("xs_connection:: calling OnRspQryInvestorPosition ");
	}

	void xs_connection::init_connection()
	{

		m_orderRtnQueue.setHandler(boost::bind(&xs_connection::OnSOPEntrustOrderRtnAsyn, this, _1));
		m_orderRspQueue.setHandler(boost::bind(&xs_connection::OnRspSOPEntrustOrderRtnAsyn, this, _1));
		m_tradeQueue.setHandler(boost::bind(&xs_connection::OnSOPTradeRtnAsyn, this, _1));
		m_ordCanRtnQueue.setHandler(boost::bind(&xs_connection::OnSOPWithdrawOrderRtnAsyn, this, _1));

		m_stockorderRtnQueue.setHandler(boost::bind(&xs_connection::OnStockEntrustOrderRtnAsyn, this, _1));
		m_stocktradeQueue.setHandler(boost::bind(&xs_connection::OnStockTradeRtnAsyn, this, _1));
		m_stockordCanRtnQueue.setHandler(boost::bind(&xs_connection::OnStockWithdrawOrderRtnAsyn, this, _1));

		loggerv2::info("xs_connection::init_connection create trader api..");
		m_pUserApi = DFITCSECTraderApi::CreateDFITCSECTraderApi(NULL,NULL);

		//init_process(io_service_type::trader, 10);
#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif

	}
	
#ifdef Linux
	void  xs_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&xs_connection::process, this));
		m_orderRtnQueue.set_fd(efd);
		m_orderRspQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_ordCanRtnQueue.set_fd(efd);
		m_outboundQueue.set_fd(efd);
	}
#endif

	void xs_connection::write_data2disk()
	{
		if ( localMsgQueue.empty())
			return;

		lwtp now = get_lwtp_now();
		if ((now - m_timepoint)<std::chrono::milliseconds(1000))
			return;

		m_timepoint = now;
		std::thread t(std::bind(&xs_connection::ackBackup, this));
		t.detach();
	}

	void xs_connection::readLocal2Portfolio()
	{
		if (!boost::filesystem::exists(m_path))
		{
			boost::filesystem::create_directory(m_path);
		}
		
		std::string fn2 = m_path + "/localId2Msg_XS.csv";
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

		cout << "load fin,size of m_localId2Portfolio is:" << m_localId2Portfolio.size() << endl;
	}

	void xs_connection::ackBackup()
	{
		std::string fn2 = m_path + "/localId2Msg_XS.csv";
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

	void xs_connection::release()
	{
		//is_alive(false);
		////RTThread::Join();

		//m_thread.join();
		ctpbase_connection::release();

		m_pUserApi->Release();
	}

	void xs_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			//if (m_pUserApi->Disconnect() == false)
			DFITCSECReqUserLogoutField request;
			memset(&request, 0, sizeof(request));
			strcpy(request.accountID, m_sUsername.c_str());
			request.requestID = ++m_nRequestId;
			int res = m_pUserApi->ReqSOPUserLogout(&request);
			if (res != 0)
			{
				loggerv2::error("xs_connection::Disconnect failed");
			}
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "xs_connection - ReqUserLogout failed");
		}
	}

	void xs_connection::request_op_login()
	{
		DFITCSECReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.accountID, m_sUsername.c_str());
		strcpy(request.password, m_sPassword.c_str());
		request.requestID = ++m_nRequestId;
		m_pUserApi->ReqSOPUserLogin(&request);
		return;
	}

	void xs_connection::request_stock_login()
	{
		DFITCSECReqUserLoginField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.accountID, m_sUsername.c_str());
		strcpy(request.password, m_sPassword.c_str());
		request.requestID = ++m_nRequestId;
		m_pUserApi->ReqStockUserLogin(&request);
		return;
	}

	void xs_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_orderRspQueue.Pops_Handle(0);
		m_orderRtnQueue.Pops_Handle(0);
		m_tradeQueue.Pops_Handle(0);
		m_ordCanRtnQueue.Pops_Handle(0);

		m_stockorderRtnQueue.Pops_Handle(0);
		m_stocktradeQueue.Pops_Handle(0);
		m_stockordCanRtnQueue.Pops_Handle(0);
		write_data2disk();
	}

	void xs_connection::connect()
	{
		loggerv2::info("calling xs_connection::Connect");
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", m_sHostname.c_str(), m_sService.c_str());
			loggerv2::info("xs_api::Connect initializing api");
			int res = m_pUserApi->Init(addr, this);
			if (res != 0)
			{
				loggerv2::error("xs_connection::Connect - api initalization failed,error %d", res);
				if (m_status != AtsType::ConnectionStatus::Disconnected)
					on_status_changed(AtsType::ConnectionStatus::Disconnected, "api initialization failed");
			}
			else
			{
				//? subscribe private topic

			}
		}
		else
		{
			request_op_login();
			request_stock_login();
			request_srv_time();
		}
	}

	void xs_connection::OnFrontConnected()
	{
		loggerv2::info("xs_connection::OnFrontConnected - xs_connection is UP");
		if (getStatus() != AtsType::ConnectionStatus::Connected)
		{
			loggerv2::info("xs_connection::OnFrontConnected subscribe private topic");
			//m_pUserApi->SubscribePrivateTopic(TERT_RESTART);
			switch (getResynchronizationMode())
			{
			case ResynchronizationMode::None:
				m_pUserApi->SubscribePrivateTopic(TERT_QUICK);
				break;

			case ResynchronizationMode::Last:
				m_pUserApi->SubscribePrivateTopic(TERT_RESUME);
				break;

			default:
			case ResynchronizationMode::Full:
				m_pUserApi->SubscribePrivateTopic(TERT_RESTART);
				break;
			}
			loggerv2::info("request user login");
			request_op_login();
			request_stock_login();
			request_srv_time();
		}
		else
		{
			loggerv2::info("xs_connection not asking for reconnect...");
		}
	}

	int xs_connection::market_create_order_async(order* o, char* pszReason)
	{
		//xs_order* o = dynamic_cast<xs_order*>(ord);
		//if (o == NULL)
		//{
		//	snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to xs_order*...\n");
		//	ord->set_status(AtsType::OrderStatus::Reject);
		//	return 0;
		//}

		terra::marketaccess::orderpassing::tradeitem* i = o->get_instrument();
		if (i == nullptr)
			return 0;

		int res = 1;
		switch (i->get_instr_type())
		{
		case AtsType::InstrType::ETF:
		case AtsType::InstrType::Stock:
			res = market_create_stock_order(o, pszReason);
			break;
		case AtsType::InstrType::Call:
		case AtsType::InstrType::Put:
			res = market_create_op_order(o, pszReason);
			break;
		default:
		{
			snprintf(pszReason, REASON_MAXLENGTH, " incorrect tradeitem type\n");
			o->set_status(AtsType::OrderStatus::Reject);
			return 0;
		}
		break;
		}
		return res;
	}

	int xs_connection::market_create_op_order(order* o, char* pszReason)
	{
		DFITCSOPReqEntrustOrderField *request = xs_create_pool.get_mem();;
		//memset(request, 0, sizeof(DFITCSOPReqEntrustOrderField));

		request->localOrderID = o->get_id();//++m_nCurrentOrderRef;

		strcpy(request->accountID, m_sUsername.c_str());
		strcpy(request->exchangeID, o->get_instrument()->getMarket().c_str());
		strcpy(request->securityID, o->get_instrument()->get_trading_code());

		request->entrustQty = o->get_quantity();
		request->entrustPrice = o->get_price();

		if (o->get_way() == AtsType::OrderWay::Buy)
			request->entrustDirection = DFITCSEC_ED_Buy;
		else if (o->get_way() == AtsType::OrderWay::Sell)
			request->entrustDirection = DFITCSEC_ED_Sell;

		if (o->get_restriction() == AtsType::OrderRestriction::None)
			request->orderExpiryDate = DFITCSEC_OE_NONE; // or GFS ???
		else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
			request->orderExpiryDate = DFITCSEC_OE_FOK;
		else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
			request->orderExpiryDate = DFITCSEC_OE_FAK;
		else
		{
			snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
			xs_create_pool.free_mem(request);
			return 0;
		}

		DFITCSECOpenCloseFlagType oc = DFITCSEC_OCF_Open;
		if (o->get_open_close() == OrderOpenClose::Undef)
			o->set_open_close(compute_open_close(o, false));

		switch (o->get_open_close())
		{
		case AtsType::OrderOpenClose::Open:
			break;

		case AtsType::OrderOpenClose::Close:
		case AtsType::OrderOpenClose::CloseToday:
			oc = DFITCSEC_OCF_Close;
			break;

		default:
			break;
		}

		request->openCloseFlag = oc;
		if (m_debug)
			loggerv2::info("xs_connection::market_create_order openCloseFlag is %d", oc);


		if (!compute_userId(o, request->devDecInfo, sizeof(request->devDecInfo)))
		{
			xs_create_pool.free_mem(request);
			return 0;
		}
		//format %2x-%5x%5x%d%x (2+1+5+5+1+1=15) account - userOrdId internalId tradingtype portfolio
		//snprintf(request->devDecInfo, sizeof(request->devDecInfo), "%2x-%5x%5x%1d%2x", nAccountNum, nUserOrderId, nInternalRef, o->get_trading_type(), nPortfolio);
		loggerv2::info("market_crate_order reqest.devDecInfo [%s],localID [%d]", request->devDecInfo, request->localOrderID);
		
		switch (o->get_price_mode())
		{
		case AtsType::OrderPriceMode::Limit:
		{
			request->orderType = DFITCSEC_SOP_LimitPrice;
			//strcpy(request->LimitPrice, std::to_string(o->get_price()).c_str());
			request->entrustPrice = o->get_price();
		}
		break;
		case AtsType::OrderPriceMode::Market:
		{
			request->orderType = DFITCSEC_SOP_LastPrice;
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
		request->orderCategory = DFITCSEC_OC_GeneralOrder;
		request->requestID = ++m_nRequestId;

		xs_order_aux::set_locId(o, request->localOrderID);

		int res = m_pUserApi->ReqSOPEntrustOrder(request);
		if (res != 0)
		{
			snprintf(pszReason, REASON_MAXLENGTH, "api returns %d\n", res);
			xs_create_pool.free_mem(request);
			return 0;
		}
		insert_localId2order(request->localOrderID, o);
		xs_create_pool.free_mem(request);
		return 1;
	}

	int xs_connection::market_create_stock_order(order* o, char* pszReason)
	{
		DFITCStockReqEntrustOrderField request;
		memset(&request, 0, sizeof(DFITCStockReqEntrustOrderField));

		strcpy(request.accountID, m_sUsername.c_str());
		request.localOrderID = o->get_id();// ++m_nCurrentOrderRef;

		strcpy(request.exchangeID, o->get_instrument()->getMarket().c_str());
		if (o->get_way() == AtsType::OrderWay::Buy)
			request.entrustDirection = DFITCSEC_ED_Buy;
		else if (o->get_way() == AtsType::OrderWay::Sell)
			request.entrustDirection = DFITCSEC_ED_Sell;
		request.entrustQty = o->get_quantity();
		request.entrustPrice = o->get_price();
		strcpy(request.securityID, o->get_instrument()->get_trading_code());

		o->set_price_mode(AtsType::OrderPriceMode::Market);//股票只支持市价交易模式
		switch (o->get_price_mode())
		{
		case AtsType::OrderPriceMode::Limit:
		{
			if (o->get_instrument()->getMarket() == "SH")
				request.orderType = DFITCSEC_OT_LimitPrice;
			else
				request.orderType = DFITCSEC_OT_SZOtherBestPrice;
		}
		break;
		case AtsType::OrderPriceMode::Market:
		{
			if (o->get_instrument()->getMarket() == "SH")
				request.orderType = DFITCSEC_OT_SHBESTFRTradeLeftWithdraw;
			else
				request.orderType = DFITCSEC_OT_SZBestPrice;
		}
		break;
		default:
		{
			snprintf(pszReason, REASON_MAXLENGTH, "undefined price mode.\n");
			o->set_status(AtsType::OrderStatus::Reject);
			return 0;
		}
		break;
		}

		request.requestID = ++m_nRequestId;
		xs_order_aux::set_locId(o, request.localOrderID);

		int res = m_pUserApi->ReqStockEntrustOrder(&request);

		if (res != 0)
		{
			snprintf(pszReason, REASON_MAXLENGTH, "api returns %d\n", res);
			loggerv2::error("xs_connection::market_create_stock_order create stock order failed, api reject res= %d", res);
			return 0;
		}

		insert_localId2order(request.localOrderID, o);

		std::shared_ptr<localOrderMsg> localmsg(new localOrderMsg());
		localmsg->localId = request.localOrderID;
		localmsg->portfolio = o->get_portfolio();
		localmsg->orderId = o->get_id();
		localmsg->accId = o->get_account_num();
		localmsg->nTradingType = o->get_trading_type();
		localMsgQueue.push(localmsg);
		return 1;
	}

	int xs_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());
		//xs_order* o = dynamic_cast<xs_order*>(ord);
		//if (o == NULL)
		//{
		//	snprintf(pszReason, REASON_MAXLENGTH, "cannot cast order* to xs_order*...\n");
		//	ord->set_status(AtsType::OrderStatus::Nack);
		//	ord->rollback();
		//	return 0;
		//}

		terra::marketaccess::orderpassing::tradeitem* i = o->get_instrument();
		if (i == nullptr)
			return 0;

		int res = 1;
		switch (i->get_instr_type())
		{
		case AtsType::InstrType::ETF:
		case AtsType::InstrType::Stock:
			res = market_cancel_stock_order(o, pszReason);
			break;
		case AtsType::InstrType::Call:
		case AtsType::InstrType::Put:
			res = market_cancel_op_order(o, pszReason);
			break;
		default:
		{
			snprintf(pszReason, REASON_MAXLENGTH, " incorrect tradeitem type\n");
			o->set_status(AtsType::OrderStatus::Nack);
			o->rollback();
			return 0;
		}
		break;
		}

		return res;
	}

	int xs_connection::market_cancel_op_order(order* ord, char* pszReason)
	{
		DFITCSECReqWithdrawOrderField *request = xs_cancel_pool.get_mem();
		memset(request, 0, sizeof(DFITCSECReqWithdrawOrderField));

		strcpy(request->accountID, m_sUsername.c_str());
		request->spdOrderID = xs_order_aux::get_spdId(ord);
		request->requestID = ++m_nRequestId;
		request->localOrderID = abs(xs_order_aux::get_locId(ord));
		if (m_pUserApi->ReqSOPWithdrawOrder(request) != 0)
		{
			snprintf(pszReason, REASON_MAXLENGTH, "api reject\n");
			ord->set_status(AtsType::OrderStatus::Nack);
			xs_cancel_pool.free_mem(request);
			return 0;
		}
		xs_cancel_pool.free_mem(request);
		return 1;
	}

	int xs_connection::market_cancel_stock_order(order* ord, char* pszReason)
	{

		DFITCSECReqWithdrawOrderField request;
		memset(&request, 0, sizeof(request));
		strcpy(request.accountID, m_sUsername.c_str());
		request.spdOrderID = xs_order_aux::get_spdId(ord);
		request.requestID = ++m_nRequestId;
		request.localOrderID = abs(xs_order_aux::get_locId(ord));
		if (m_pUserApi->ReqStockWithdrawOrder(&request) != 0)
		{
			snprintf(pszReason, REASON_MAXLENGTH, "api reject\n");
			ord->set_status(AtsType::OrderStatus::Nack);
			return 0;
		}

		return 1;
	}

	void xs_connection::OnSOPWithdrawOrderRtnAsyn(DFITCSOPWithdrawOrderRtnField * pCancelOrderData)
	{
		loggerv2::info("xs_connection::OnSOPWithdrawOrderRtnAsyn "
			" localOrderID %d"
			" spdOrderID %d"
			" devDecInfo %s"
			" devID %s"
			" declareResult %d"
			" exchangeID %s"
			" noteMsg %s"
			" securityID %s",

			pCancelOrderData->localOrderID,
			pCancelOrderData->spdOrderID,
			pCancelOrderData->devDecInfo,
			pCancelOrderData->devID,
			pCancelOrderData->declareResult,
			pCancelOrderData->exchangeID,
			pCancelOrderData->noteMsg,
			pCancelOrderData->securityID

			);

		int nOrderId = get_order_id(pCancelOrderData->devDecInfo);
		int localID = pCancelOrderData->localOrderID;
		bool isOldPacket = false;

		if (m_begin_Id >= nOrderId)
			isOldPacket = true;

		if (abs(localID) > m_nCurrentOrderRef)
			m_nCurrentOrderRef = abs(localID);

		//xs_order* o = nullptr;

		int ret;
		order *o = get_order_from_map(nOrderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<xs_order*>(ord);//it->second);
			loggerv2::info("xs_connection::OnSOPWithdrawOrderRtnAsyn - message received on dead order[%d]...", nOrderId);
			break;
		case 2:
			o = xs_order_aux::anchor(this, pCancelOrderData);
			if (o == NULL)
			{
				loggerv2::error("xs_connection::OnSOPWithdrawOrderRtnAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);
		default:
			break;
		}

		if (o == nullptr) // should not happen
		{
			loggerv2::error("xs_connection::OnSOPWithdrawOrderRtnAsyn - order recovered NULL");
			return;
		}

		/*std::chrono::system_clock::time_point Now = std::chrono::system_clock::now();
		auto t2 = o->get_instrument()->get_last_sychro_timepoint();*/
		int cancelQty = pCancelOrderData->withdrawQty;
		//terra::common::date_time dnow = terra::common::date_time(time(NULL));
		switch (pCancelOrderData->declareResult)
		{

		case DFITCSEC_DR_TradeAWithdraw:
		case DFITCSEC_DR_TotalWithdraw:
		{

			if (get_lwtp_now() > o->get_instrument()->get_last_sychro_timepoint() && isOldPacket == false)
			{
				update_instr_on_cancel_from_market_cb(o, cancelQty);
			}
			//else
				//m_statistics.incr_can();
			if (pCancelOrderData->withdrawQty + pCancelOrderData->tradeQty == o->get_quantity())
				o->set_status(AtsType::OrderStatus::Cancel);

			//on_nack_from_market_cb(o, NULL);
			on_cancel_from_market_cb(o);
			break;
		}
		case DFITCSEC_DR_EntrustFail:
		case DFITCSEC_DR_WithdrawFail://
		{
			auto sta = o->get_status();
			if (sta==OrderStatus::Exec||sta==OrderStatus::Reject||sta==OrderStatus::Cancel)
				return;

			if (o->get_status() == OrderStatus::WaitServer)//历史回报重建后的状态都是waitmarket，不会进行回滚。
				o->rollback();
			//o->rollback();
			//o->set_status(OrderStatus::Ack);
			//on_nack_from_market_cb(o, "Cancel Order fail");
			loggerv2::error("xs_connection::OnSOPWithdrawOrderRtnAsyn,cancel order fail,orderID:%d", o->get_id());
			break;
		}

		default:
			break;
		}

	}

	void xs_connection::OnSOPEntrustOrderRtnAsyn(DFITCSOPEntrustOrderRtnField* pOrder)
	{


		loggerv2::info("xs_connection::OnSOPEntrustOrderRtnAsyn "
			" localOrderID %d"
			" spdOrderID %d"
			" devDecInfo %s"
			" devID %s"
			" declareResult %d"
			" exchangeID %s"
			" noteMsg %s"
			" securityID %s",

			pOrder->localOrderID,
			pOrder->spdOrderID,
			pOrder->devDecInfo,
			pOrder->devID,
			pOrder->declareResult,
			pOrder->exchangeID,
			pOrder->noteMsg,
			pOrder->securityID

			);
		//order* o = NULL;
		bool isRebuild = false;

		int localID = pOrder->localOrderID;
		int orderId = get_order_id(pOrder->devDecInfo);

		if (m_nCurrentOrderRef < localID)
			m_nCurrentOrderRef = localID;

		if (m_begin_Id >= orderId)
			isRebuild = true;

		//tbb::concurrent_hash_map<int, int>::const_accessor ra;
		//if (m_used_locId.find(ra, localID))
		//{
		//	loggerv2::info("xs_connection::OnSOPEntrustOrderRtnAsyn,localID %d has been processed", localID);
		//	return;//该localId已经处理过。
		//}
		//ra.release();

		int ret;
		AtsType::OrderStatus::type status = OrderStatus::Ack;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<xs_order*>(ord);//it->second);
			loggerv2::info("xs_connection::OnSOPEntrustOrderRtnAsyn - message received on dead order[%d]...", orderId);
			status = o->get_status();
			if (status == OrderStatus::Cancel || status == OrderStatus::Nack || status == OrderStatus::Reject)
			{
				o->set_quantity(pOrder->entrustQty);
				loggerv2::error("xs_connection::OnSOPWithdrawOrderRtnAsyn order[%d] has been exec/cancel/rej/nack/,return");
				return;
			}
			else if (status == OrderStatus::Exec)//成交回报先到
			{
				if (pOrder->entrustQty > o->get_quantity())//这种情况属于部分成交
				{
					o->set_quantity(pOrder->entrustQty);
					o->set_status(OrderStatus::Ack);
				}
				else//非部分成交，return
					return;
			}
			break;

		case 2:

			o = xs_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("xs_of_connection::OnSOPEntrustOrderRtnAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("xs_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}

		/*tbb::concurrent_hash_map<int, int>::accessor wa;
		m_used_locId.insert(wa, localID);
		wa->second = orderId;
		wa.release();*/

		xs_order_aux::set_spdId(o, pOrder->spdOrderID);

		/*auto t1 = microsec_clock::local_time();
		auto t2 = o->get_instrument()->get_last_sychro_timepoint();*/
		int ackQty = pOrder->entrustQty;

		switch (pOrder->declareResult)
		{
		case DFITCSEC_DR_Declaring:
		case DFITCSEC_DR_UnTrade:
		case DFITCSEC_DR_PartTrade:
		{

			if (get_lwtp_now() > o->get_instrument()->get_last_sychro_timepoint() && isRebuild == false)//历史回包不参与pending计算
			{
				update_instr_on_ack_from_market_cb(o, ackQty);
			}
			on_ack_from_market_cb(o);
			break;
		}
		case DFITCSEC_DR_TotalTrade: //    
			break;
		case DFITCSEC_DR_EntrustFail:
		{
			//on_nack_from_market_cb(o, "OrderRtn api reject");
			on_nack_from_market_cb(o, pOrder->noteMsg);
			break;
		}
		case DFITCSEC_DR_WithdrawFail://
		{
			loggerv2::error("xs_connection::OnSOPEntrustOrderRtnAsyn,cancel order fail,orderID:%d", o->get_id());
			auto sta = o->get_status();
			if (sta == OrderStatus::Exec || sta == OrderStatus::Reject || sta == OrderStatus::Cancel)
				return;
			
			if (o->get_status() == OrderStatus::WaitServer)
				o->rollback();

			if (o->get_status() == OrderStatus::WaitServer)
				o->rollback();

			break;
		}
		case DFITCSEC_DR_TradeAWithdraw:
		case DFITCSEC_DR_TotalWithdraw:
		{
			//o->set_last_action(AtsType::OrderAction::Cancelled);
			if (get_lwtp_now() > o->get_instrument()->get_last_sychro_timepoint() && isRebuild == false)
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

	void xs_connection::OnSOPTradeRtnAsyn(DFITCSOPTradeRtnField* pTrade)
	{
		std::string sInstrCode = compute_second_key(std::string(pTrade->securityID), std::string(pTrade->exchangeID));
		loggerv2::info("xs_connection::OnSOPTradeRtnAsyn"
			" InstrCode %s"
			" securityID %s"
			" localOrderID %d"
			" tradeID %s"
			" openCloseFlag %d"
			" tradePrice %f"
			" tradeQty %d"
			" devDecInfo %s"
			" entrustDirection %d",
			sInstrCode.c_str(),
			pTrade->securityID,
			pTrade->localOrderID,
			pTrade->tradeID,
			pTrade->openCloseFlag,
			pTrade->tradePrice,
			pTrade->tradeQty,
			pTrade->devDecInfo,
			pTrade->entrustDirection
			);

		//xs_order* o = NULL;
		lwtp Now = get_lwtp_now();
		bool duplicat = false;
		bool isOldPacket = false;
		// 1 - retrieve order
		int orderId = get_order_id(pTrade->devDecInfo);
		int localID = pTrade->localOrderID;

		if (m_nCurrentOrderRef < abs(localID))
			m_nCurrentOrderRef = abs(localID);

		if (m_begin_Id >= orderId)
			isOldPacket = true;

		if (orderId == -1)
		{
			loggerv2::error("xs_connection::OnSOPTradeRtnAsyn,can not get orderid,localID is %d", localID);
			return;
		}

		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<xs_order*>(ord);//it->second);
			break;
		case 1:
			//o = reinterpret_cast<xs_order*>(ord);//it->second);
			loggerv2::info("xs_connection::OnSOPTradeRtnAsyn - message received on dead order[%d]...", orderId);
			break;

		case 2:

			loggerv2::warn("xs_connection::OnRtnTradeAsync,can not find order in order book,rebuild");
			o = xs_order_aux::anchor(this, pTrade);
			if (o == NULL)
			{
				loggerv2::error("xs_of_connection::OnSOPTradeRtnAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);
			break;
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("xs_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}


		// 2 - treat message
		int execQty = pTrade->tradeQty;
		double execPrc = pTrade->tradePrice;
		//const char* pszExecRef = pTrade->tradeID;

		exec* e = new exec(o, string(pTrade->tradeID), execQty, execPrc, pTrade->tradeTime);

		int cumulatedQty = o->get_exec_quantity() + e->getQuantity();
		int totQty = o->get_quantity();

		on_exec_from_market_cb(o, e, duplicat);

		if (duplicat)
		{
			loggerv2::info("duplicat packet,drop");
			return;
		}

		bool onlyUpdatePending = false;
		auto last = o->get_instrument()->get_last_sychro_timepoint();

		if (Now < last)
			return;
		if (isOldPacket )
		{
			onlyUpdatePending = true;
		}

		if (onlyUpdatePending)
		{
			loggerv2::info("xs_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time.");
		}

		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);

	}

	void xs_connection::requset_op_instruments()
	{
		loggerv2::info("xs_connection::requset_op_instruments");
		DFITCSOPReqQryContactField request;
		memset(&request, 0, sizeof(DFITCSOPReqQryContactField));
		strcpy(request.accountID, m_sUsername.c_str());                //客户号(Y)
		//strcpy(request.exchangeID, "SH"); //?
		request.requestID = ++m_nRequestId;
		m_database->open_database();
		m_pUserApi->ReqSOPQryContactInfo(&request);
	}

	void xs_connection::OnFrontDisconnected(int nReason)
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
	void xs_connection::OnRspError(DFITCSECRspInfoField *pRspInfo)
	{
		loggerv2::error("OnRspError : need to implement");
	}
	void xs_connection::OnRspSOPUserLogin(DFITCSECRspUserLoginField *pData, DFITCSECRspInfoField *pRspInfo)
	{
		if (pData != nullptr)
		{
			int temp_id = pData->localOrderID;
			if (temp_id > m_nCurrentOrderRef)
			{
				m_nCurrentOrderRef = pData->localOrderID;
			}
			loggerv2::info("xs_connection::OnRspSOPUserLogin localOrderId %d", pData->localOrderID);

			on_status_changed(AtsType::ConnectionStatus::Connected, "xs_connection::OnRspSOPUserLogin");
			loggerv2::info("xs_connection::OnRspSOPUserLogin login succeed");

			request_op_positions();
			//todo comment this line later.
			/*if (!m_bIsDicoRdy && m_bRequestdico)
			{
			m_database->open_database();
			requset_op_instruments();
			}*/
			if (this->getRequestInstruments() == true)
			{
				requset_op_instruments();
			}
		}
		else
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected, std::to_string(pRspInfo->errorID).c_str());
			loggerv2::info("xs_connection::OnRspSOPUserLogin login failed error %d", pRspInfo->errorID);
		}
	}
	void xs_connection::OnRspSOPUserLogout(DFITCSECRspUserLogoutField *pData, DFITCSECRspInfoField *pRspInfo)
	{
		if (pData != nullptr)
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "xs_connection::OnRspUserLogout Receive Logout Msg");
		else
			loggerv2::error("xs_connection::OnRspSOPUserLogout logout failed ErrId[%d]", pRspInfo->errorID);
	}

	void xs_connection::OnRspSOPEntrustOrder(DFITCSOPRspEntrustOrderField *pData, DFITCSECRspInfoField *pRspInfo)
	{
		
		if (pRspInfo != nullptr)
		{
			loggerv2::info("xs_connection::OnRspSOPEntrustOrder -->  localOrderID %d spdOrderID %d errorID %d errorMsg %s", pRspInfo->localOrderID, pRspInfo->spdOrderID, pRspInfo->errorID, pRspInfo->errorMsg);
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

				snprintf(pszReason, REASON_MAXLENGTH, "error_id:%ld.emsg:%s", pRspInfo->errorID, pRspInfo->errorMsg);
				on_nack_from_market_cb(o, pszReason);

				m_localId2order.erase(localOrderID);
			}
		}//error return

		return;

		m_orderRspQueue.CopyPush(pData);
	}

	void xs_connection::OnRspSOPEntrustOrderRtnAsyn(DFITCSOPRspEntrustOrderField * pData)
	{
		return;
		//if (pData != nullptr)
		//{
		//	int localOrderID = pData->localOrderID;
		//	if (contain_used_locId(localOrderID))
		//	{
		//		return;//该localId已经处理过。
		//	}

		//	order* o = get_localId2order(localOrderID);
		//	if (o != nullptr)
		//	{

		//		xs_order_aux::set_spdId(o, pData->spdOrderID);

		//		auto t1 = microsec_clock::local_time();
		//		auto t2 = o->get_instrument()->get_last_sychro_timepoint();
		//		int ackQty = o->get_quantity();//券商响应里没有委托数量，这里认为全部下单都ack了。

		//		if (t1 > t2)
		//		{
		//			update_instr_on_ack_from_market_cb(o, ackQty);
		//		}

		//		m_localId2order.erase(localOrderID);
		//		insert_used_locId(localOrderID);

		//	}

		//}


	}
	
	void xs_connection::OnRspSOPWithdrawOrder(DFITCSECRspWithdrawOrderField *pData, DFITCSECRspInfoField *pRspInfo)
	{


		if (pData != nullptr)
		{
			loggerv2::info("xs_connection::OnRspSOPWithdrawOrder "
				" requestID %d"
				" accountID %s"
				" localOrderID %d"
				" spdOrderID %d"
				" entrustTime %s"
				" cancelMsg %s",
				pData->requestID,
				pData->accountID,
				pData->localOrderID,
				pData->spdOrderID,
				pData->entrustTime,
				pData->cancelMsg
				);

			int localOrderID = pData->localOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				if (m_debug)
					loggerv2::info("xs_connection::OnRspSOPWithdrawOrder localOrderID %d", pData->localOrderID);

				m_localId2order.erase(localOrderID);
			}


		}

		else if (pRspInfo != nullptr)
		{
			loggerv2::info("xs_connection::OnRspSOPWithdrawOrder "
				" requestID %d"
				" sessionID %d"
				" accountID %s"
				" errorID %d"
				" localOrderID %d"
				" spdOrderID %d"
				" errorMsg %s",
				pRspInfo->requestID,
				pRspInfo->sessionID,
				pRspInfo->accountID,
				pRspInfo->errorID,
				pRspInfo->localOrderID,
				pRspInfo->spdOrderID,
				pRspInfo->errorMsg
				);

			//cancel is bad

			int localOrderID = pRspInfo->localOrderID;

			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				char pszReason[REASON_MAXLENGTH + 1];
				memset(pszReason, 0, sizeof(pszReason));
				pszReason[REASON_MAXLENGTH] = '\0';

				snprintf(pszReason, REASON_MAXLENGTH, "error id %ld.", pRspInfo->errorID);
				on_nack_from_market_cb(o, pszReason);

				m_localId2order.erase(localOrderID);
			}
		}
	}

	void log_OnRspSOPQryPosition(DFITCSOPRspQryPositionField *pData)
	{
		if (pData == NULL)
			return;
		loggerv2::info("xs_connection::OnRspSOPQryPosition accountID[%s]"
			"exchangeID[%s]"
			"securityOptionID[%s]"
			"contractID[%s]"
			"contractName[%s] "
			"entrustDirection[%d]"
			"coveredFlag[%d]"
			"executeDate[%f]"
			"totalQty[%d]"
			"availableQty[%d]"
			"latestPrice[%f]"
			"freezeQty[%d]"
			"executeQty[%d]"
			"openEntrustQty[%f]"
			"openTradeQty[%f]"
			"prePosition[%f]"
			"closeEntrustQty[%f]"
			"closeTradeQty[%f]",

			pData->accountID,
			pData->exchangeID,
			pData->securityOptionID,
			pData->contractID,
			pData->contractName,
			pData->entrustDirection,
			pData->coveredFlag,
			pData->executeDate,
			pData->totalQty,
			pData->availableQty,
			pData->latestPrice,
			pData->freezeQty,
			pData->executeQty,
			pData->openEntrustQty,
			pData->openTradeQty,
			pData->prePosition,
			pData->closeEntrustQty,
			pData->closeTradeQty
			);
	}

	void xs_connection::OnRspSOPQryPosition(DFITCSOPRspQryPositionField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast)
	{
		if (pData == nullptr)
		{
			loggerv2::error("xs_connection::OnRspSOPQryPosition errorid %d,%s", pRspInfo->errorID, pRspInfo->errorMsg);
			return;
		}
		/*loggerv2::info("xs_connection::OnRspSOPQryPosition accountID[%s]"
		"exchangeID[%s]"
		"securityOptionID[%s]"
		"contractID[%s]"
		"contractName[%s] "
		"entrustDirection[%d]"
		"coveredFlag[%d]"
		"executeDate[%f]"
		"totalQty[%d]"
		"availableQty[%d]"
		"latestPrice[%f]"
		"freezeQty[%d]"
		"executeQty[%d]"
		"openEntrustQty[%f]"
		"openTradeQty[%f]"
		"prePosition[%f]"
		"closeEntrustQty[%f]"
		"closeTradeQty[%f]",

		pData->accountID,
		pData->exchangeID,
		pData->securityOptionID,
		pData->contractID,
		pData->contractName,
		pData->entrustDirection,
		pData->coveredFlag,
		pData->executeDate,
		pData->totalQty,
		pData->availableQty,
		pData->latestPrice,
		pData->freezeQty,
		pData->executeQty,
		pData->openEntrustQty,
		pData->openTradeQty,
		pData->prePosition,
		pData->closeEntrustQty,
		pData->closeTradeQty
		);*/

		if (pData->contractID)
		{

			//std::string instr = std::string(pData->InstrumentID) + "." + std::string(pData->ExchangeID);
			std::string instr = std::string(pData->contractID) + "." + std::string(pData->exchangeID) + "@" + getName();

			//std::string instr = std::string(pData->contractID) + "@" + GetName();

			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(instr.c_str());
			if (i)
			{
				//loggerv2::info("lts_api::OnRspQryInvestorPosition found tradeitem %s", pData->InstrumentID);
				if (pData->entrustDirection == DFITCSEC_ED_Buy)//long
				{
					if (i->get_tot_long_position() != pData->totalQty || i->get_pending_short_close_qty() != pData->freezeQty)
					{
						if (m_debug)
							i->dumpinfo();
						i->set_tot_long_position(pData->totalQty);
						i->set_today_long_position(pData->executeQty);
						i->set_pending_short_close_qty(pData->freezeQty);
						i->set_yst_long_position(pData->prePosition);
						log_OnRspSOPQryPosition(pData);
						if (m_debug)
							i->dumpinfo();
					}
				}

				else if (pData->entrustDirection == DFITCSEC_ED_Sell)//short
				{
					if (i->get_tot_short_position() != pData->totalQty || i->get_pending_long_close_qty() != pData->freezeQty)
					{
						if (m_debug)
							i->dumpinfo();
						i->set_tot_short_position(pData->totalQty);
						i->set_today_short_position(pData->executeQty);
						i->set_pending_long_close_qty(pData->freezeQty);

						if (pData->coveredFlag == DFITCSEC_CF_Covered) // covered sell
						{
							i->set_covered_sell_open_position(pData->totalQty);

							//Todo update these qtys
							/*i->set_pending_covered_sell_close_qty(pData->);
							i->set_pending_covered_sell_open_qty(pData->);*/
						}
						log_OnRspSOPQryPosition(pData);
						if (m_debug)
							i->dumpinfo();
					}
				}
				i->set_last_sychro_timepoint(get_lwtp_now());



			}
			else
				loggerv2::warn("xs_connection::OnRspSOPQryPosition cannot find tradeitem %s", pData->contractID);
		}
		if (bIsLast == true)
		{
			if (this->getRequestInstruments() == true && this->get_is_last() == false)
			{
				sleep_by_milliseconds(2000);
				requset_op_instruments();
			}
		}
	}

	void xs_connection::OnRspSOPQryContactInfo(DFITCSOPRspQryContactField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast)
	{
		if (pData == nullptr)
			return;
		loggerv2::info("DFITCSOPRspQryContactField exchangeID[%s]"
			"securityOptionID[%s]"		   //期权交易代码(10000031)
			"contractID[%s]"                //期权标识代码(600104C1406M01200  )
			"securityID[%s]"                //标的代码(600104)
			"optType[%d]"                   //期权类型
			"execDate[%d]"                  //行权日期
			"execPrice[%f]",                //行权价格
			pData->exchangeID,
			pData->securityOptionID,
			pData->contractID,
			pData->securityID,
			pData->optType,
			pData->execDate,
			pData->execPrice
			);

		std::string sInstr = std::string(pData->contractID);
		boost::trim(sInstr);

		std::string sSearch = "select * from Options where Code= '" + std::string(pData->securityOptionID) + "'";
		//const char* data = "Callback function called";
		char *zErrMsg = 0;

		std::string sUnderlying = pData->securityID; //
		std::string sCP = "C";  //"CallPut"
		switch (pData->optType)
		{
		case DFITCSEC_OT_CALL:
			sCP = "C";
			break;
		case DFITCSEC_OT_PUT:
		default:
			sCP = "P";
			break;
		}

		std::string sInstClass = "O_" + sUnderlying;
		std::string strExecDate = std::to_string(pData->execDate);
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = (std::string(pData->exchangeID) == "SH") ? std::string("SSE") : std::string("SZE");


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
			sCmd += "'" + std::string(pData->exchangeID) + std::string(pData->securityOptionID) + "@XS|" + std::string(pData->securityOptionID) + "@LTSUDP|" + std::string(pData->securityOptionID) + "." + sExcge + "@LTS|" + std::string(pData->securityOptionID) + "." + std::string(pData->exchangeID) + "@TDF" + "',";
			sCmd += "'" + std::string(pData->securityOptionID) + "." + sExcge + "@LTS|" + std::string(pData->securityOptionID) + "." + std::string(pData->exchangeID) + "@XS" + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + std::to_string(pData->execPrice) + "',";
			sCmd += "'" + std::to_string(pData->contactUnit) + "',";
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
			std::string sConnectionCodes = std::string(pData->securityOptionID) + sExcge + "@" + get_type();
			sCmd = "UPDATE Options SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "Strike = '" + std::to_string(pData->execPrice) + "',";
			sCmd += "PointValue ='" + std::to_string(pData->contactUnit) + "'";
			sCmd += "FeedCodes='SH" + sConnectionCodes + "'";
			sCmd += "ConnectionCodes='" + sConnectionCodes + "'";
			sCmd += " where Code='" + std::string(pData->securityOptionID) + "';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				//loggerv2::info("failed to update the database,error is %d",rc);
				sqlite3_free(zErrMsg);
			}
		}

		if (bIsLast && this->get_is_last() == false)
		{
			//m_bIsDicoRdy = true;
			m_database->close_databse();
			this->set_is_last(true);
		}
	}
	
	void xs_connection::OnSOPEntrustOrderRtn(DFITCSOPEntrustOrderRtnField * pData)
	{
		//push to queue

		//DFITCSOPEntrustOrderRtnField* o = new DFITCSOPEntrustOrderRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCSOPEntrustOrderRtnField));
		//m_orderRtnQueue.Push(o);
		m_orderRtnQueue.CopyPush(pData);
	}

	void xs_connection::OnSOPTradeRtn(DFITCSOPTradeRtnField * pData)
	{
		//push to queue
		//DFITCSOPTradeRtnField* trade = new DFITCSOPTradeRtnField;
		//memcpy_lw(trade, pData, sizeof(DFITCSOPTradeRtnField));
		//m_tradeQueue.Push(trade);
		m_tradeQueue.CopyPush(pData);

	}
	void xs_connection::OnSOPWithdrawOrderRtn(DFITCSOPWithdrawOrderRtnField * pData)
	{
		//push to queue
		//DFITCSOPWithdrawOrderRtnField* o = new DFITCSOPWithdrawOrderRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCSOPWithdrawOrderRtnField));
		//m_ordCanRtnQueue.Push(o);
		m_ordCanRtnQueue.CopyPush(pData);
	}
	
	void xs_connection::OnRspStockUserLogin(DFITCSECRspUserLoginField *pData, DFITCSECRspInfoField *pRspInfo)
	{

		if (pData != nullptr)
		{
			int temp_id = pData->localOrderID;
			if (temp_id > m_nCurrentOrderRef)
			{
				m_nCurrentOrderRef = pData->localOrderID;
			}
			loggerv2::info("xs_connection::OnRspStockUserLogin localOrderId %d", pData->localOrderID);
			on_status_changed(AtsType::ConnectionStatus::Connected, "xs_connection::OnRspStockUserLogin");
			loggerv2::info("xs_connection::OnRspStockUserLogin login succeed");
			request_stock_positions();
		}
		else
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected, std::to_string(pRspInfo->errorID).c_str());
			loggerv2::info("xs_connection::OnRspStockUserLogin login failed error %d", pRspInfo->errorID);
		}

	}
	void xs_connection::OnRspStockUserLogout(DFITCSECRspUserLogoutField *pData, DFITCSECRspInfoField *pRspInfo)
	{
		if (pData != nullptr)
			on_status_changed(AtsType::ConnectionStatus::Disconnected, "xs_connection::OnRspStockUserLogout Receive Logout Msg");
		else
			loggerv2::error("xs_connection::OnRspStockUserLogout logout failed ErrId[%d]", pRspInfo->errorID);

	}
	void xs_connection::OnRspStockEntrustOrder(DFITCStockRspEntrustOrderField *pData, DFITCSECRspInfoField *pRspInfo)
	{
		if (pRspInfo != nullptr)
		{
			loggerv2::info("xs_connection::OnRspSOPEntrustOrder -->  localOrderID %d spdOrderID %d errorID %d errorMsg %s", pRspInfo->localOrderID, pRspInfo->spdOrderID, pRspInfo->errorID, pRspInfo->errorMsg);
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

				snprintf(pszReason, REASON_MAXLENGTH, "error id %ld.", pRspInfo->errorID);
				on_nack_from_market_cb(o, pszReason);

				m_localId2order.erase(localOrderID);
				insert_used_locId(localOrderID);

			}
		}//error return

		if (pData != nullptr)
		{
			int localOrderID = pData->localOrderID;

			if (contain_used_locId(localOrderID))
			{
				return;//该localId已经处理过。
			}

			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				xs_order_aux::set_spdId(o, pData->spdOrderID);

				auto t1 = get_lwtp_now();
				auto t2 = o->get_instrument()->get_last_sychro_timepoint();
				int ackQty = o->get_quantity();//券商响应里没有委托数量，这里认为全部下单都ack了。

				if (t1 > t2)
				{
					update_instr_on_ack_from_market_cb(o, ackQty);
				}
				on_ack_from_market_cb(o);

				insert_used_locId(localOrderID);

			}

		}

	}

	void xs_connection::OnRspStockWithdrawOrder(DFITCSECRspWithdrawOrderField *pData, DFITCSECRspInfoField *pRspInfo)
	{

		if (pData != nullptr)
		{
			loggerv2::info("xs_connection::OnRspStockWithdrawOrder "
				" requestID %d"
				" accountID %s"
				" localOrderID %d"
				" spdOrderID %d"
				" entrustTime %s"
				" cancelMsg %S",
				pData->requestID,
				pData->accountID,
				pData->localOrderID,
				pData->spdOrderID,
				pData->entrustTime,
				pData->cancelMsg
				);

			int localOrderID = pData->localOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				if (m_debug)
					loggerv2::info("xs_connection::OnRspStockWithdrawOrder localOrderID %d", pData->localOrderID);

				on_ack_from_market_cb(o);

				m_localId2order.erase(localOrderID);
			}

		}

		else if (pRspInfo != nullptr)
		{
			loggerv2::info("xs_connection::OnRspStockWithdrawOrder "
				" requestID %d"
				" sessionID %d"
				" accountID %s"
				" errorID %d"
				" localOrderID %d"
				" spdOrderID %d"
				" errorMsg %s",
				pRspInfo->requestID,
				pRspInfo->sessionID,
				pRspInfo->accountID,
				pRspInfo->errorID,
				pRspInfo->localOrderID,
				pRspInfo->spdOrderID,
				pRspInfo->errorMsg
				);

			//cancel is bad

			int localOrderID = pRspInfo->localOrderID;
			order* o = get_localId2order(localOrderID);
			if (o != nullptr)
			{
				char pszReason[REASON_MAXLENGTH + 1];
				memset(pszReason, 0, sizeof(pszReason));
				pszReason[REASON_MAXLENGTH] = '\0';

				snprintf(pszReason, REASON_MAXLENGTH, "error id %ld.", pRspInfo->errorID);
				o->set_status(AtsType::OrderStatus::Cancel);//yanglei add 临时解决飞创bug的方案。
				on_nack_from_market_cb(o, pszReason);

				m_localId2order.erase(localOrderID);
			}
		}
	}
		
	void xs_connection::OnRspStockQryPosition(DFITCStockRspQryPositionField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast)
	{

		if (pData == nullptr)
		{
			loggerv2::error("xs_connection::OnRspStockQryPosition errorid %d,%s", pRspInfo->errorID, pRspInfo->errorMsg);

			return;
		}

		loggerv2::info("xs_connection::OnRspStockQryPosition accountID[%s]"
			"exchangeID[%s]"
			"ableSellQty[%d]"
			"securityID[%s]"
			"securityName[%s] "
			"securityQty[%d]"
			"position[%d]"
			"freezeQty[%d]"
			"totalBuyQty[%d]"
			"totalSellQty[%d]",

			pData->accountID,
			pData->exchangeID,
			pData->ableSellQty,
			pData->securityID,
			pData->securityName,
			pData->securityQty,
			pData->position,
			pData->freezeQty,
			pData->totalBuyQty,
			pData->totalSellQty
			);

		if (pData->securityID)
		{
			std::string instr = std::string(pData->securityID) + "." + std::string(pData->exchangeID) + "@" + getName();
			tradeitem* i = tradeitem_gh::get_instance().container().get_by_second_key(instr.c_str());
			if (i)
			{
				i->set_tot_long_position(pData->totalBuyQty);
				i->set_pending_short_close_qty(pData->freezeQty);
				i->set_tot_short_position(pData->totalSellQty);

				i->set_last_sychro_timepoint(get_lwtp_now());

				if (m_debug)
					i->dumpinfo();
			}
			else
				loggerv2::warn("xs_connection::OnRspStockQryPosition cannot find tradeitem %s", pData->securityID);
		}
	}
	void xs_connection::OnStockEntrustOrderRtn(DFITCStockEntrustOrderRtnField * pData)
	{
		//DFITCStockEntrustOrderRtnField* o = new DFITCStockEntrustOrderRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCStockEntrustOrderRtnField));
		//m_stockorderRtnQueue.Push(o);
		m_stockorderRtnQueue.CopyPush(pData);
	}
	void xs_connection::OnStockTradeRtn(DFITCStockTradeRtnField * pData)
	{
		//DFITCStockTradeRtnField* o = new DFITCStockTradeRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCStockTradeRtnField));
		//m_stocktradeQueue.Push(o);
		m_stocktradeQueue.CopyPush(pData);
	}
	void xs_connection::OnStockWithdrawOrderRtn(DFITCStockWithdrawOrderRtnField * pData)
	{
		//DFITCStockWithdrawOrderRtnField* o = new DFITCStockWithdrawOrderRtnField;
		//memcpy_lw(o, pData, sizeof(DFITCStockWithdrawOrderRtnField));
		//m_stockordCanRtnQueue.Push(o);
		m_stockordCanRtnQueue.CopyPush(pData);
	}
	
	void xs_connection::OnStockEntrustOrderRtnAsyn(DFITCStockEntrustOrderRtnField* pOrder)
	{

		loggerv2::info("xs_connection::OnStockEntrustOrderRtnAsyn "
			" localOrderID %d"
			" spdOrderID %d"
			" declareResult %d"
			" exchangeID %s"
			" securityID %s",

			pOrder->localOrderID,
			pOrder->spdOrderID,
			pOrder->declareResult,
			pOrder->exchangeID,
			pOrder->securityID

			);
		order* o = NULL;
		bool isRebuild = false;

		int localID = pOrder->localOrderID;
		if (m_nCurrentOrderRef < abs(localID))
			m_nCurrentOrderRef = abs(localID);
		//int orderId = get_order_id(pOrder->devDecInfo);
		if (localID < 0)
		{
			localID = -localID;

			isRebuild = true;
		}

		if (contain_used_locId(localID))
		{
			return;//该localId已经处理过。
		}

		if (isRebuild == false)
		{
			order* o = get_localId2order(localID);
			if (o != nullptr)
			{
				if (o->get_status() == AtsType::OrderStatus::Ack)
				{
					return;
				}
			}
			
		}
		else
		{
			o = xs_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("xs_connection::OnSOPEntrustOrderRtnAsyn cannot anchor order");
				return;
			}

			add_pending_order(o);

		}
		if (o == NULL) // should not happen
		{
			loggerv2::error("xs_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}

		insert_used_locId(localID);
		xs_order_aux::set_spdId(o, pOrder->spdOrderID);

		auto t1 = get_lwtp_now();
		auto t2 = o->get_instrument()->get_last_sychro_timepoint();
		int ackQty = pOrder->entrustQty;

		switch (pOrder->declareResult)
		{
		case DFITCSEC_DR_Declaring:
		case DFITCSEC_DR_UnTrade:
		case DFITCSEC_DR_PartTrade:
		{

			if (t1 > t2&&isRebuild == false)//历史回包不参与pending计算
			{
				update_instr_on_ack_from_market_cb(o, ackQty);
			}
			on_ack_from_market_cb(o);
			break;
		}
		case DFITCSEC_DR_TotalTrade: //    
			break;
		case DFITCSEC_DR_EntrustFail:
		case DFITCSEC_DR_WithdrawFail://
		{
			on_nack_from_market_cb(o, "OrderRtn api reject");
			break;
		}
		case DFITCSEC_DR_TradeAWithdraw:
		case DFITCSEC_DR_TotalWithdraw:
		{
			o->set_last_action(AtsType::OrderAction::Cancelled);
			if (t1 > t2&&isRebuild == false)
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
	void xs_connection::OnStockWithdrawOrderRtnAsyn(DFITCStockWithdrawOrderRtnField* pCancelOrderData)
	{

		if (m_debug)
			loggerv2::info("xs_connection::OnRspOrderActionAsync");
		//int nOrderId = get_order_id(pCancelOrderData->devDecInfo);
		int localID = pCancelOrderData->localOrderID;
		bool isOldPacket = false;
		if (localID < 0)
		{
			isOldPacket = true;
		}
		if (abs(localID)>m_nCurrentOrderRef)
			m_nCurrentOrderRef = abs(localID);
		order* o = nullptr;

		tbb::concurrent_hash_map<int, order*>::const_accessor _ra;
		if (m_localId2order.find(_ra, localID))
		{
			o = _ra->second;
		}
		else
		{

		}
		_ra.release();

		if (o == nullptr) // should not happen
		{
			loggerv2::error("xs_connection::OnRspOrderActionAsync - order recovered NULL");
			return;
		}

		//std::chrono::system_clock::time_point Now = std::chrono::system_clock::now();
		auto t2 = o->get_instrument()->get_last_sychro_timepoint();
		int cancelQty = pCancelOrderData->withdrawQty;
		//terra::common::date_time dnow = terra::common::date_time(time(NULL));

		if (get_lwtp_now() > o->get_instrument()->get_last_sychro_timepoint() && isOldPacket == false)
		{
			update_instr_on_cancel_from_market_cb(o, cancelQty);
		}
		o->set_status(AtsType::OrderStatus::Cancel);

		on_nack_from_market_cb(o, NULL);//on_cancel_from_market_cb(o);

		m_localId2order.erase(localID);

	}
	void xs_connection::OnStockTradeRtnAsyn(DFITCStockTradeRtnField* pTrade)
	{

		loggerv2::info("xs_connection::OnStockTradeRtnAsyn"
			" localOrderID %d"
			" tradeID %s",
			pTrade->localOrderID,
			pTrade->tradeID
			);

		order* o = NULL;
		bool duplicat = false;
		bool isOldPacket = false;
		// 1 - retrieve order
		int localID = pTrade->localOrderID;
		if (localID < 0)
		{
			localID = -localID;

			isOldPacket = true;
		}
		if (m_nCurrentOrderRef < abs(localID))
			m_nCurrentOrderRef = abs(localID);

		tbb::concurrent_hash_map<int, order*>::const_accessor ra;
		if (m_localId2order.find(ra, localID))
		{
			o = ra->second;
		}
		if (o == NULL) // should not happen
		{
			loggerv2::error("xs_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}

		exec* e = new exec(o, std::string(pTrade->tradeID), pTrade->tradeQty, pTrade->tradePrice, pTrade->tradeTime);

		int cumulatedQty = o->get_exec_quantity() + e->getQuantity();
		int totQty = o->get_quantity();
		if (cumulatedQty > totQty)
		{
			int i = 0;
		}
		on_exec_from_market_cb(o, e, duplicat);
		if (duplicat)
		{
			loggerv2::info("duplicat packet,drop");
			return;
		}

		auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->tradeTime));
		bool onlyUpdatePending = false;
		auto last = o->get_instrument()->get_last_sychro_timepoint();
		//last += m_interval;//Trade_Time的BUG服务器依旧没修复
		/*临时方案*/
		//terra::common::date_time Now = terra::common::date_time(time(NULL));
		if (isOldPacket)
		{
			onlyUpdatePending = true;
		}
		else
		{
			if (get_lwtp_now() < o->get_instrument()->get_last_sychro_timepoint())
			{
				onlyUpdatePending = true;
			}
		}
		if (onlyUpdatePending)
		{
			loggerv2::info("xs_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time.");
		}

		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
	}

	void xs_connection::request_srv_time()
	{

		DFITCSOPReqQryTradeTimeField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.accountID, m_sUsername.c_str());
		pRequest.requestID = ++m_nRequestId;
		if (m_pUserApi->ReqSOPQryTradeTime(&pRequest) != 0)
			loggerv2::error("xs_connection::request_op_positions failed");
	}

	void xs_connection::OnRspSOPQryTradeTime(DFITCSOPRspQryTradeTimeField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast)
	{
		if (pRspInfo != nullptr)
		{
			loggerv2::error("xs_connection::OnRspSOPQryTradeTime errorid %d,%s", pRspInfo->errorID, pRspInfo->errorMsg);
			return;
		}

	}

	std::string xs_connection::getMaturity(std::string& sMat)
	{
		std::string newMat;
		newMat = sMat.substr(0, 4);
		newMat += "-";
		newMat += sMat.substr(4, 2);
		newMat += "-";
		newMat += sMat.substr(6, 2);
		return newMat.c_str();
	}

	void xs_connection::cancel_num_warning(tradeitem* i)
	{

	}

	void xs_connection::cancel_num_ban(tradeitem* i)
	{

	}

	void xs_connection::insert_localId2order(int id, order* o)
	{
		tbb::concurrent_hash_map<int, order*>::accessor wa;
		m_localId2order.insert(wa, id);
		wa->second = o;
	}

	order *xs_connection::get_localId2order(int id)
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

	void xs_connection::insert_used_locId(int id)
	{
		tbb::concurrent_hash_map<int, int>::accessor wa;
		m_used_locId.insert(wa, id);
		wa->second = id;
	}

	bool xs_connection::contain_used_locId(int id)
	{
		tbb::concurrent_hash_map<int, int>::const_accessor ra;
		if (m_used_locId.find(ra, id))
			return true;
		else
			return false;
	}

}
