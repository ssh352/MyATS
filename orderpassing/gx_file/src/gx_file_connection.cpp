#include "gx_file_connection.h"
#include "string_tokenizer.h"
#include <boost/algorithm/string.hpp>
#include "tradeItem_gh.h"
#include <boost/property_tree/ini_parser.hpp>
using namespace terra::common;
namespace gx_file
{
	gx_file_connection::gx_file_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName              = "gx_file_connection";
		m_quant_proxy = new gx_file_quant_proxy();
		m_quant_proxy->m_con = this;
	}

	gx_file_connection::~gx_file_connection()
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
	bool gx_file_connection::init_config(const std::string &name, const std::string &strConfigFile)
	{
		// general
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 4)
			m_bTsession = true;
		else
			m_bTsession = false;
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
		//
		m_strOrderDir = root.get<string>(name + ".order_dir", "");
		printf_ex("gx_file_connection::init_config m_strOrderDir:%s\n", m_strOrderDir.c_str());
		//
		init_process(io_service_type::trader, 10);
		m_bKey_with_exchange = false;
		return true;
	}
	void gx_file_connection::init_connection()
	{
		//m_outboundQueue.setHandler(boost::bind(&lts_file_connection::process_outbound_msg_cb, this));
		m_userInfoQueue.setHandler(boost::bind(&gx_file_connection::OnUserInfoAsync, this, _1));
	}
	void gx_file_connection::release()
	{		
		ctpbase_connection::release();		
	}

	void gx_file_connection::connect()
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

	void gx_file_connection::disconnect()
	{		
		if (this->m_quant_proxy->disconnect() == true)
		{
			on_status_changed(AtsType::ConnectionStatus::Disconnected);
		}
		this->m_quant_proxy->stop();
	}
	void gx_file_connection::request_instruments()
	{
		loggerv2::info("gx_file_connection::requesting instruments");
		m_database->open_database();		
	}


	void gx_file_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_userInfoQueue.Pops_Handle(0);
	}

	int gx_file_connection::market_create_order_async(order* o, char* pszReason)
	{		
		if (m_quant_proxy)
		{			
			/*
			输入文件格式为csv文件，包括下面几列：证券代码,证券名称,方向,数量,价格,备注
			local_entrust_no,fund_account,exchange_type,stock_code,entrust_bs,entrust_prop,entrust_price,entrust_amount,client_filed1,clientfield2
            1,               23076653,    1,            601866,    1,         0,           1.53,         200,           client_field1,
			*/
			char userID[64];
			memset(userID, 0, sizeof(userID));
			compute_userId(o, userID, sizeof(userID));
			if (o->get_open_close() == OrderOpenClose::Undef)
			{
				o->set_open_close(compute_open_close(o, m_bCloseToday));
			}
			int localId = m_quant_proxy->ReqOrderInsert(o->get_instrument()->get_trading_code(), o->get_instrument()->getInstrument()->get_exchange().c_str(), o->get_way(), o->get_quantity(), o->get_price(), o->get_restriction(), o->get_open_close(), o->get_price_mode(),userID);
			this->create_user_info(localId,userID);
			gx_file_order_aux::set_order_local_id(o,localId);
		}	
		return 1;
	}


	int gx_file_connection::market_cancel_order_async(order* o, char* pszReason)
	{	
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());		

		if (o->get_instrument()->get_instr_type() == AtsType::InstrType::Stock)
		{
			if (m_quant_proxy)
			{
				m_quant_proxy->cancel(gx_file_order_aux::get_order_local_id(o));
			}
		}
		if (m_debug)
			loggerv2::info("order cancel sent ok! order_status is %d", o->get_status());
		
		return 1;
	}

	void gx_file_connection::OnRtnOrderAsync(string line)
	{
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		tokenizer.break_line(line.c_str(), szSeparators);

		//OrderDate[0], OrderTime[1], FilledDate[2], FilledTime[3], OrderID[4], Stock[5], buy / sell[6], OrderType[7], OrderPrice[8], OrderNumber[9], EnterNumber[10], NodealNumber[11], MatchPrice[12], OrderStatue[13], Market[14], Account[15]		
		//printf_ex("gx_file_connection::OnRtnOrderAsync line:%s\n",line.c_str());

		string OrderDate   = tokenizer[0];
		string OrderTime   = tokenizer[1];

		string FilledDate  = tokenizer[2];
		string FilledTime  = tokenizer[3];

		string LocalID     = tokenizer[4];//local id
		string Stock       = tokenizer[5];//000001.SZ

		string buy_sell    = tokenizer[6];
		string OrderType   = tokenizer[7];

		string OrderPrice  = tokenizer[8];
		string OrderNumber = tokenizer[9];

		string EnterNumber = tokenizer[10];
		string NodealNumber= tokenizer[11];

		string MatchPrice  = tokenizer[12];
		string OrderStatus = tokenizer[13];

		string Market      = tokenizer[14];

		int nAccount     = 0;
		int nUserOrdId   = -1;
		int nInternalRe  = -1;
		int nPortfolio   = 0;
		int nTradingType = 0;
		ctpbase_connection::get_user_info(this->get_user_id(atoi(LocalID.c_str())).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);

		int ret;		
		OrderWay::type way = buy_sell == "buy" ? OrderWay::Buy : OrderWay::Sell;
		int orderId = (way == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
		if (orderId < 1)
		{
			orderId = FAKE_ID_MIN + atoi(LocalID.c_str());
		}
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("gx_file_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			//anchor
			o = gx_file_order_aux::anchor_order(this, line);
			if (o == nullptr)
			{
				loggerv2::error("gx_file_connection::OnRtnOrderAsync cannot anchor order:%d", orderId);
				return;
			}
			add_pending_order(o);
			break;
		default:
			{
			loggerv2::info("gx_file_connection::OnRtnOrderAsync ret:%d\n", ret);
			break;
			}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("gx_file_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}

		//
		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("gx_file_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}
		//

		if (OrderStatus == "rejected")
		{
			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			//sprintf(buffer, "%d[%s][%s]", pReject->m_ReasonCode, pReject->m_GrammerText, pReject->m_RiskText);
			on_nack_from_market_cb(o, buffer);
			update_instr_on_nack_from_market_cb(o);
		}				
		else if (OrderStatus == "received" || OrderStatus == "partiallyfilled" || OrderStatus == "filled")
		{
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{
				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}
		}
		else if (OrderStatus == "canceled" || OrderStatus == "partiallyfilledurout")
		{
			update_instr_on_cancel_from_market_cb(o);
			on_cancel_from_market_cb(o);
		}
		else
		{
			//printf_ex("gx_file_connection::OnRtnOrderAsync didn't with the status:%s\n",OrderStatus.c_str());
			loggerv2::debug("gx_file_connection::OnRtnOrderAsync didn't with the status:%s\n", OrderStatus.c_str());
		}
	}
	/*
	资金账号0,    买/卖1,委托期限2,委托数量3,委托日期4,委托时间5,成交数量6,成交日期7,成交时间8,成交价格9,委托编号10,                   委托状态11,委托状态明细12,成交价格13,股票代码14,委托类型15,委托ID16
	410001174092,buy,  gfd,     100,     2017-07-27,11:16:21,100,     2017-07-27,11:16:21,36.450,  0_41-0001-1740-92_2-0170-727_-3001-5593,filled, filled,      36.45,   002032.sz,limit,  3	
	*/
	void gx_file_connection::OnRtnTradeAsync(string line)
	{
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		tokenizer.break_line(line.c_str(), szSeparators);
		//printf_ex("gx_file_connection::OnRtnTradeAsync line:%s\n",line.c_str());

		string buy_sell  = tokenizer[1];
		int LocalID      = atoi(tokenizer[16]);
		int nAccount     = 0;
		int nUserOrdId   = -1;
		int nInternalRe  = -1;
		int nPortfolio   = 0;
		int nTradingType = 0;
		ctpbase_connection::get_user_info(this->get_user_id(LocalID).c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);

		int ret;
		OrderWay::type way = buy_sell == "buy" ? OrderWay::Buy : OrderWay::Sell;
		int orderId = (way == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
		if (orderId < 1)
		{
			orderId = FAKE_ID_MIN + LocalID;
		}
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sl_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sl_order*>(ord);
			loggerv2::warn("gx_file_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			//anchor
			o = gx_file_order_aux::anchor_trade(this, line);
			if (o == nullptr)
			{
				loggerv2::error("gx_file_connection::OnRtnTradeAsync cannot anchor order:%d", orderId);
				return;
			}
			add_pending_order(o);
			break;
		default:
		{
			loggerv2::info("gx_file_connection::OnRtnTradeAsync ret:%d\n", ret);
			break;
		}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("gx_file_connection::OnRtnTradeAsync - order recovered NULL");
			return;
		}
			
		int execQty            = atoi(tokenizer[6]);
		double execPrc         = atof(tokenizer[9]);
		const char* pszExecRef = tokenizer[10];
		const char * pszExecTime = strlen(tokenizer[8]) > 0 ? tokenizer[8] : tokenizer[5];
		/*
		std::string ts("2002-01-20 23:59:59.000");
		ptime t(time_from_string(ts))
		*/
		char pszTime[256];
		memset(pszTime, 0, sizeof(pszTime));
		sprintf(pszTime, "%s %s", tokenizer[7], tokenizer[8]);
		lwtp tp;
		if (strlen(pszTime) < 10)
		{			
			tp = get_lwtp_now();
		}
		else
		{
			tp = string_to_lwtp(pszTime);
		}		
		exec* e = new exec(o, pszExecRef, execQty, execPrc,pszExecTime);
		on_exec_from_market_cb(o, e);
		bool onlyUpdatePending = false;		
		int hour = get_hour_from_lwtp(tp);
		tp = tp + std::chrono::seconds(2);//允许2s的误差
		if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			onlyUpdatePending = true;
		if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;		
		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);		
	}

	void gx_file_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{

	}

	void gx_file_connection::request_investor_full_positions()
	{					
	}
	/*
	Position.CSV
	Account,Stock,Quantity,QuantityAvailable
	410001174092,000001.SZ,100,100
	410001174092,000002.SZ,299,299
	*/
	void gx_file_connection::OnRspQryPosition(string line)
	{
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		tokenizer.break_line(line.c_str(), szSeparators);
		//loggerv2::info("gx_file_connection::OnRspQryPosition line:%s\n", line.c_str());
		string Stock    = tokenizer[1];
		string quantity = tokenizer[2];
		string QuantityAvailable = tokenizer[3];
		std::string sInstrCode = Stock.substr(0, 6) + "@" + this->getName();
		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("gx_file_order_aux::OnRspQryPosition - tradeitem:%s not found", sInstrCode.c_str());
			return ;
		}
		if (instr->get_tot_long_position() != atoi(quantity.c_str()))
		{
			loggerv2::info("gx_file_connection::OnRspQryPosition,tot_long_position change,%s", instr->getCode().c_str());
			instr->dumpinfo();
			instr->set_tot_long_position(atoi(quantity.c_str()));
			instr->dumpinfo();
			instr->set_last_sychro_timepoint(get_lwtp_now());
		}
		if (instr->get_yst_long_position() != atoi(QuantityAvailable.c_str()))
		{
			loggerv2::info("gx_file_connection::OnRspQryPosition,yst_long_position change,%s", instr->getCode().c_str());
			instr->dumpinfo();
			instr->set_yst_long_position(atoi(QuantityAvailable.c_str()));
			instr->dumpinfo();
			instr->set_last_sychro_timepoint(get_lwtp_now());
		}
	}
	OrderOpenClose::type gx_file_connection::compute_open_close(order* ord, bool hasCloseToday /*= false*/)
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
	void gx_file_connection::init_user_info(char * user_info_file)
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
		printf("gx_file_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
			user_info * info = nullptr;
			if (m_user_info_map.find(atoi(tokenizer[0])) == m_user_info_map.end())
			{
				info = new user_info();		
				//
				info->LocalID = atoi(tokenizer[0]);
				//
				m_user_info_map.emplace(info->LocalID, info);
			}
			else
			{
				info = m_user_info_map[atoi(tokenizer[0])];
			}				
			info->UserID              = tokenizer[1];						
		}
		stream.close();
	}
	void gx_file_connection::create_user_info(int localid, string userid)
	{
		if (m_user_info_map.find(localid) == m_user_info_map.end())
		{
			user_info * info = new user_info();
			info->LocalID    = localid;
			info->UserID     = userid;
			m_user_info_map.emplace(info->LocalID, info);
			//to do ... append the file every day
			m_userInfoQueue.CopyPush(info);
		}
		else
		{
			
		}
	}
	string gx_file_connection::get_user_id(int localid)
	{
		if (m_user_info_map.find(localid) != m_user_info_map.end())
		{
			user_info * info = m_user_info_map[localid];
			return info->UserID;
		}
		return "";
	}
	void gx_file_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));		
		sprintf(buffer, "%d,%s\n", info->LocalID, info->UserID.c_str());
		stream << buffer;
		stream.close();
	}
	void gx_file_connection::OnUserInfoAsync(user_info* pInfo)//异步记录userinfo
	{
		this->append(pInfo);
	}	
}

