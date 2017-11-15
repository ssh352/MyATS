#include "gx_connection.h"
#include "string_tokenizer.h"
#include <boost/algorithm/string.hpp>
#include "tradeItem_gh.h"
#include <boost/property_tree/ini_parser.hpp>
#include "defaultdatetimepublisher.h"
using namespace terra::common;
namespace gx
{	
	gx_connection::gx_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "gx_connection";
		m_quant_proxy = new gx_quant_proxy();
		m_quant_proxy->m_con = this;
	}

	gx_connection::~gx_connection()
	{
		if (m_quant_proxy != nullptr)
		{
			delete m_quant_proxy;
		}
		m_quant_proxy = nullptr;
	}

	bool gx_connection::init_config(const std::string &name, const std::string &strConfigFile)
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

		m_bKey_with_exchange = false;
#if 0
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

		init_req_api();
		init_trd_api();
#endif
		//std::thread th(boost::bind(&gx_connection::set_kernel_timer_thread, this));
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
		std::thread t(boost::bind(&gx_connection::post_keyboard_msg, this));
		t.detach();
#endif
		//
		//if (m_quant_proxy != nullptr)
		//{
		//	m_quant_proxy->query_position(get_stock_account_name(),"000753.SZ");
		//}
		return true;
	}





	void gx_connection::init_connection()
	{
		//m_outboundQueue.setHandler(boost::bind(&gx_connection::process_outbound_msg_cb, this));
#if 1
		m_userInfoQueue.setHandler(boost::bind(&gx_connection::OnUserInfoAsync, this, _1));
		m_pdu_queue.setHandler(boost::bind(&gx_connection::OnRtnOrderAsync, this, _1));
#endif
	}

	void gx_connection::release()
	{
		ctpbase_connection::release();

		m_quant_proxy->disconnect();
	}

	void gx_connection::connect()
	{
		if (this->m_quant_proxy->connect() == true)
		{
			on_status_changed(AtsType::ConnectionStatus::Connected);
			request_investor_full_positions();
		}
		else
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}
	}

	void gx_connection::disconnect()
	{
		if (this->m_quant_proxy->disconnect() == true)
		{
		on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}		
	}
	void gx_connection::request_instruments()
	{
		loggerv2::info("gx_connection::requesting instruments");
		m_database->open_database();
	}


	void gx_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
#if 1
		m_userInfoQueue.Pops_Handle(0);
		m_pdu_queue.Pops_Handle(0);
#endif
	}

	int gx_connection::market_create_order_async(order* o, char* pszReason)
	{
		printf_ex("gx_connection::market_create_order_async %s,qty:%d\n", o->get_instrument()->get_trading_code(),o->get_quantity());
		char userID[256];
		if (!compute_userId(o, userID, sizeof(userID)))
		{			
			return 0;
		}	
		if (m_quant_proxy)
		{
			//this->create_user_info(o);
			/*
			输入文件格式为csv文件，包括下面几列：证券代码,证券名称,方向,数量,价格,备注
			bool ReqOrderInsert(int order_id,const string & feedCode,const string & name,OrderWay::type way, int quantity, double price, OrderRestriction::type restriction = OrderRestriction::None, OrderOpenClose::type openClose = OrderOpenClose::Undef, OrderPriceMode::type priceMode = OrderPriceMode::Limit);
			*/
			m_quant_proxy->ReqOrderInsert(o->get_instrument()->get_trading_code(), o->get_instrument()->get_trading_code(), o->get_way(), o->get_quantity(), o->get_price(), o->get_restriction(), o->get_open_close(), o->get_price_mode(), userID);
		}
		return 1;
	}

	int gx_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (o == nullptr)
			return -1;
		string clientID;
		if (m_order_id_map.find(o->get_id()) != m_order_id_map.end())
		{
			clientID = m_order_id_map[o->get_id()];			
		}

		if (m_quant_proxy)
		{
			m_quant_proxy->cancel(clientID);
		}
		return -1;
	}
	void gx_connection::request_trading_account()
	{
#if 0
		if (m_debug)
			loggerv2::info("gx_connection:: calling ReqQryTradingAccount ");

		CSecurityFtdcQryTradingAccountField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));

		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		//strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pltsReqApi->ReqQryTradingAccount(&pRequest);
#endif
		return;

	}
	void gx_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{
		if (m_quant_proxy != nullptr && i != nullptr)
		{
			m_quant_proxy->query_position(get_stock_account_name(),i->get_trading_code());
		}
		return;
	}

	void gx_connection::request_investor_full_positions()
	{
		if (m_quant_proxy != nullptr)
		{
			m_quant_proxy->query_position(get_stock_account_name());
		}
		return;
	}	
	
	void gx_connection::cancel_num_warning(tradeitem* i)
	{

	}

	void gx_connection::cancel_num_ban(tradeitem* i)
	{

	}
	

	void gx_connection::init_user_info(char * user_info_file)
	{		
		//to do ...
		if (user_info_file == nullptr)
			return;
		boost::filesystem::path p;
		p.clear();
		p.append(user_info_file);
		p.append("user_info.csv");
		m_user_info_file_name = p.string();
		printf("gx_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
			user_info * info = new user_info();
			info->clientID =tokenizer[0];
			info->userID   = tokenizer[1];			
			info->way      = atoi(tokenizer[2]);
			m_user_info_map.emplace(info->clientID, info);
			//to do ...
			int id = -1;
			int nAccount = 0;
			int nUserOrdId = -1;
			int nInternalRe = -1;
			int nPortfolio = 0;
			int nTradingType = 0;
			get_user_info(info->userID.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
			if (m_order_id_map.find(id) == m_order_id_map.end())
			{
				m_order_id_map.emplace(id, info->clientID);
			}

			//query
			if (m_quant_proxy != nullptr)
			{
				m_quant_proxy->query(info->clientID);
			}
		}
		stream.close();		
	}
	void gx_connection::create_user_info(const string & clientID, const string & userID, int way)
	{
		if (m_user_info_map.find(clientID) == m_user_info_map.end())
		{
			user_info * info = new user_info();
			info->clientID   = clientID;
			info->userID     = userID;			
			info->way        = way;
			m_user_info_map.emplace(info->clientID, info);
			//
			m_userInfoQueue.CopyPush(info);
		}		
		//to do ...
		int id = -1;
		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		get_user_info(userID.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (m_order_id_map.find(id) == m_order_id_map.end())
		{
			m_order_id_map.emplace(id, clientID);
		}
	}
	void gx_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s,%s,%d\n", info->clientID.c_str(),info->userID.c_str(),info->way);
		stream << buffer;
		stream.close();
	}
	void gx_connection::OnUserInfoAsync(user_info* pInfo)//异步记录userinfo
	{
		this->append(pInfo);
	}
	string gx_connection::get_user_info_ex(const string & clientID)
	{
		if (m_user_info_map.find(clientID) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[clientID];
			return info->userID;
		}
		return "";
	}
	int gx_connection::get_way(const string & clientID)
	{
		if (m_user_info_map.find(clientID) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[clientID];
			return info->way;
		}
		return 0;
	}
	void gx_connection::onRtnOrder(string * pdu)
	{
		m_pdu_queue.Push(pdu);
	}
	void gx_connection::OnRspOrderInsertAsync(int id)
	{
		int ret = -1;
		order * o = nullptr;
		o = get_order_from_map(id, ret);
		if (o == nullptr)
		{
			printf_ex("gx_connection::OnRspOrderInsertAsync didn't find the order id:%d,no anchor\n", id);
			return;
		}
		if (o->get_status() == AtsType::OrderStatus::WaitServer)
		{
			o->set_status(AtsType::OrderStatus::WaitMarket);
			connection::update_pending_order(o);
		}
	}
	void gx_connection::OnRtnOrderAsync(string * pdu)
	{
		string msg_type;
		string instrumentID;
		string state;
		int    enteredQuantity=-1;
		int    filledQuantity=-1;
		int    leftQuantity = -1;
		double avgFilledPrice = 0;
		double limitPrice = 0;
		string orderID;		
		string enteredTime;
		string filledTime;
		string clientID;
		string message;
		
		int   quantity = -1;

		int id = -1;
		int nAccount     =  0;
		int nUserOrdId   = -1;
		int nInternalRe  = -1;
		int nPortfolio   =  0;
		int nTradingType =  0;

		if (pdu != nullptr)
		{
			printf_ex("-----------gx_connection::OnRtnOrderAsync,%s\n", pdu->c_str());
			vector<string> arr;
			boost::split(arr, *pdu, boost::is_any_of(";"));
			for (auto &it : arr)
			{
				//printf_ex("%s\n", it.c_str());
				vector<string> arr1;
				boost::split(arr1, it, boost::is_any_of("="));
				if (arr1[0] == "Msg")
				{
					msg_type = arr1[1];
					printf_ex("msg_type:%s\n", msg_type.c_str());
				}
				else if (arr1[0] == "Symbol")
				{
					instrumentID = arr1[1];

					vector<string> arr2;
					boost::split(arr2, instrumentID, boost::is_any_of("."));
					instrumentID = arr2[0];

					printf_ex("instrumentID:%s\n", instrumentID.c_str());										
				}
				else if (arr1[0] == "State")
				{
					state = arr1[1];
					//printf_ex("state:%s\n", state.c_str());
					/*
					Canceled 				9
					Cancelpending   		10
					Expired					12
					Filled  				8
					Partiallyfilled			6
					Partiallyfilledurout    7
					Queued   				5
					Received				4
					Rejected				11
					Sendfailed				3
					Sending					1
					Sent					2
					unsent                  0
					*/
				}
				else if (arr1[0] == "EnteredQuantity")
				{
					enteredQuantity = atoi(arr1[1].c_str());
					//printf_ex("enteredQuantity:%d\n",enteredQuantity);
				}
				else if (arr1[0] == "LimitPrice")
				{
					limitPrice = atof(arr1[1].c_str());
					//printf_ex("limitPrice:%f\n", limitPrice);
				}
				else if (arr1[0] == "FilledQuantity")
				{
					filledQuantity = atoi(arr1[1].c_str());
					//printf_ex("filledQuantity:%d\n", filledQuantity);
				}
				else if (arr1[0] == "LeftQuantity")
				{
					leftQuantity = atoi(arr1[1].c_str());
					//printf_ex("leftQuantity:%d\n", leftQuantity);
				}
				else if (arr1[0] == "AvgFilledPrice")
				{
					avgFilledPrice = atof(arr1[1].c_str());
					//printf_ex("avgFilledPrice:%f\n", avgFilledPrice);
				}
				else if (arr1[0] == "OrderID")
				{
					orderID = arr1[1];
					//printf_ex("orderID:%s\n", orderID.c_str());
				}
				else if (arr1[0] == "EnteredTime")
				{
					enteredTime = arr1[1];
					//printf_ex("enteredTime:%s\n", enteredTime.c_str());
				}
				else if (arr1[0] == "FilledTime")
				{
					filledTime = arr1[1];
					//printf_ex("filledTime:%s\n", filledTime.c_str());
				}
				else if (arr1[0] == "ClientID")
				{
					clientID = arr1[1];
					//printf_ex("clientID:%s\n",clientID.c_str());
					string & userID = this->get_user_info_ex(clientID);
					get_user_info(userID.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
					id = nInternalRe;
				}
				else if (arr1[0] == "Message")
				{
					message = arr1[1];
					//printf_ex("message:%s\n", message.c_str());
				}
				else if (arr1[0] == "Quantity")
				{
					quantity = atoi(arr1[1].c_str());
					printf_ex("Quantity:%d\n",quantity);
				}
				else
				{
					//printf_ex("Did not do with the field:%s\n",arr1[0].c_str());
				}
			}
		}
		//
		if (msg_type == "QueryPosition")
		{
			OnRspQryInvestorPosition(instrumentID,quantity);
			return;
		}
		//to do ...
	    if (state == "received")
		{
			if (enteredQuantity == -1)
			{
				OnRspOrderInsertAsync(id);
			}
			else //same to the "queued" state
			{
				int ret = -1;
				order * o = nullptr;
				o = get_order_from_map(id, ret);
				switch (ret)
				{
				case 0:
				case 1:
					break;
				case 2:
					o = gx_order_aux::anchor(this, clientID, instrumentID, enteredQuantity, limitPrice,enteredTime);
					if (o == nullptr)
					{
						loggerv2::error("gx_connection::OnRtnOrderAsync cannot anchor order");
						return;
					}					
					add_pending_order(o);
					break;
				default:
					break;
				}
				if (o == nullptr)
				{
					printf_ex("gx_connection::OnRtnOrderAsync didn't find the order id:%d\n", id);
					return;
				}
				o->set_quantity(enteredQuantity);
				o->set_book_quantity(leftQuantity);
				o->set_status(AtsType::OrderStatus::Ack);
				on_ack_from_market_cb(o);
				update_instr_on_ack_from_market_cb(o);
			}
		}
		else if (state == "queued")
		{
			int ret = -1;
			order * o = nullptr;
			o = get_order_from_map(id, ret);
			switch (ret)
			{
			case 0:
			case 1:
				break;
			case 2:
				o = gx_order_aux::anchor(this, clientID, instrumentID, enteredQuantity, limitPrice, enteredTime);
				if (o == nullptr)
				{
					loggerv2::error("gx_connection::OnRtnOrderAsync cannot anchor order");
					return;
				}
				add_pending_order(o);
				break;
			default:
				break;
			}
			if (o == nullptr)
			{
				printf_ex("gx_connection::OnRtnOrderAsync didn't find the order id:%d\n", id);
				return;
			}			
			o->set_quantity(enteredQuantity);
			o->set_book_quantity(leftQuantity);
			o->set_status(AtsType::OrderStatus::Ack);
			on_ack_from_market_cb(o);
			update_instr_on_ack_from_market_cb(o);
		}
		else if (state == "filled" || state == "partiallyfilled")
		{
			int ret = -1;
			order * o = nullptr;
			o = get_order_from_map(id, ret);
			switch (ret)
			{
			case 0:
			case 1:
				break;
			case 2:
				o = gx_order_aux::anchor(this, clientID, instrumentID, enteredQuantity, limitPrice, enteredTime);
				if (o == nullptr)
				{
					loggerv2::error("gx_connection::OnRtnOrderAsync cannot anchor order");
					return;
				}			
				o->set_book_quantity(enteredQuantity);
				o->set_status(AtsType::OrderStatus::Ack);
				add_pending_order(o);
				break;
			default:
				break;
			}
			if (o == nullptr)
			{
				printf_ex("gx_connection::OnRtnOrderAsync didn't find the order id:%d\n", id);
				return;
			}
			OnRtnTradeAsync(o->get_id(), instrumentID, leftQuantity, avgFilledPrice, orderID + date_time_publisher_gh::get_instance()->get_now_str(), filledTime, filledTime);
		}
		else if (state == "partiallyfilledurout" || state=="canceled")
		{
			int ret = -1;
			order * o = nullptr;
			o = get_order_from_map(id, ret);
			if (o == nullptr)
			{
				printf_ex("gx_connection::OnRtnOrderAsync didn't find the order id:%d\n", id);
				return;
			}
			on_cancel_from_market_cb(o);
			update_instr_on_cancel_from_market_cb(o);			
		}
		else if (state == "rejected" || state == "sendfailed")
		{
			int ret = -1;
			order * o = nullptr;
			o = get_order_from_map(id, ret);
			if (o == nullptr)
			{
				printf_ex("gx_connection::OnRtnOrderAsync didn't find the order id:%d\n", id);
				return;
			}
			on_nack_from_market_cb(o, message.c_str());
		}
		else
		{
			printf_ex("gx_connection::OnRtnOrderAsync:Didn't with the state:%s\n",state.c_str());
		}
	}

	void gx_connection::OnRspQryInvestorPosition(const string & instrumentID, int quantity)
	{
		std::string sInstrCode = instrumentID + "@" + this->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("gx_connection::OnRspQryInvestorPosition - tradeitem:%s not found", sInstrCode.c_str());
			return;
		}
		instr->set_tot_long_position(quantity);
	}
	void gx_connection::OnRtnTradeAsync(int id,const string & instrumentID,int quantity,double price,const string & tradeID,const string & tradeDate,const string & tradeTime)
	{		
		//1.get the tradeitem
		std::string sInstrCode = instrumentID + "@" + this->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("gx_connection::OnRtnTradeAsync - tradeitem:%s not found", sInstrCode.c_str());
			return;
		}
		//2.get the order id
		int ret = -1;
		order * o = nullptr;
		o = get_order_from_map(id,ret);
		if (o == nullptr)
		{
			printf_ex("gx_connection::OnRtnTradeAsync didn't find the order id:%d\n",id);
			return;
		}
		//
		if (o->get_book_quantity() == 0)
			return;
		//3.
		exec * e = new exec(o,tradeID,o->get_book_quantity()- quantity, price, tradeTime.c_str());
		on_exec_from_market_cb(o, e);
		update_instr_on_exec_from_market_cb(o, e);
	}
	void gx_connection::OnRspOrderActionAsync(string * pdu)
	{

	}
}

