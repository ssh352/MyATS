#include "ib_connection.h"
#if 0
#include "EWrapper.h"
#include "EPosixClientSocket.h"
#else
#endif
#include "terra_logger.h"
#include "LockFreeWorkQueue.h"
#include "ib_order_aux.h"
#include <instrumentclass.h>
#include <boost/algorithm/string.hpp>
using namespace terra::common;
namespace ib
{
	//
	// ib_connection
	//



	void ib_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
	}


	ib_connection::ib_connection(bool checkSecurities) :
		ctpbase_connection(checkSecurities),
		m_pReader(0),
		m_osSignal(2000),
		m_extraAuth(false),
		m_pClient(new EClientSocket(this, &m_osSignal))
	{
		m_sName = "ib_connection";
		//m_debug = false;
		//
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 8)
			m_bTsession = true;
		else
			m_bTsession = false;
		//m_begin_Id = order_reference_provider::get_instance().get_current_int();
		//
	}

	ib_connection::~ib_connection()
	{
		//m_tListhen2IB.join();
		//release();
		if (m_pReader)
		{
			delete m_pReader;
			m_pReader = nullptr;
		}
		if (m_pClient)
		{
			delete m_pClient;
			m_pClient = nullptr;
		}
	}



#if 0
	bool ib_connection::init_config(const std::string &ini)
	{
		return false;
	}
#endif


	void ib_connection::init_connection(/*RTListener* pListener*/)
	{
		//m_pListener = pListener;

		//m_tListhen2IB = std::thread(&ib_connection::processMessages, this);
#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
	}

#ifdef Linux
	void  ib_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "ib_connection efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&ib_connection::process, this));
		
		m_outboundQueue.set_fd(efd);		
	}
#endif

	//void ib_connection::release_connection()
	//{



	//}

	void ib_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			if (m_debug)
			{
				loggerv2::info("connecting to ib...%s:%s,clientid:%s", m_sHostname.c_str(), m_sService.c_str(), get_login_id().c_str());
			}
			//on_status_changed(WAIT_CONNECT);

			printf_ex("ib_connection::connect %s:%s,clientid:%s\n", m_sHostname.c_str(), m_sService.c_str(), get_login_id().c_str());
			if (m_pClient->eConnect(m_sHostname.c_str(), boost::lexical_cast<int>(m_sService), atoi(get_login_id().c_str()), false))
			{
				on_status_changed(AtsType::ConnectionStatus::Connected);
				loggerv2::info("ib_connection::requesting open orders");
				requestOpenOrders();
				loggerv2::info("ib_connection::requesting executions");
				requestExecs();
				//to do ...
				m_pReader = new EReader(m_pClient, &m_osSignal);

				m_pReader->start();

				m_tListhen2IB = std::thread(&ib_connection::processMessages, this);
			}
		}
	}

	void ib_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			m_pClient->eDisconnect();
		}
	}


	int ib_connection::market_create_order_async(order* ord, char* pszReason)
	{
		char userId[32];
		memset(userId, 0, sizeof(userId));
		compute_userId(ord, userId, sizeof(userId));

#if 1
		Order ibOrd = Order();
		ibOrd.orderRef = userId;
#else	
		Order *ibOrd = ib_create_pool.get_mem();
		memset(ibOrd, 0, sizeof(Order));
		ibOrd->orderRef = userId;
#endif
		int nordID = ord->get_id();
		ibOrd.orderId = nordID;
		switch (ord->get_price_mode())
		{

		case AtsType::OrderPriceMode::Limit:
		{
			ibOrd.orderType = "LMT";
			ibOrd.lmtPrice = ord->get_price();
		}
		break;

		case AtsType::OrderPriceMode::Market:
			ibOrd.orderType = "MKT";
			break;

		default:
			if (pszReason != nullptr)
				strcpy(pszReason, "unsupported price mode");
			return 0;
		}

		switch (ord->get_way())
		{
		case AtsType::OrderWay::Buy:
			ibOrd.action = "BUY";
			break;
		case AtsType::OrderWay::Sell:
			ibOrd.action = "SELL";
			break;
		default:
			return 0;
		}

		ibOrd.totalQuantity = ord->get_quantity();

		Contract Con = Contract();
		switch (ord->get_instrument()->get_instr_type())
		{
		case AtsType::InstrType::Future:
			Con.secType = "FUT";
			break;
		case AtsType::InstrType::Index:
			Con.secType = "IND";
			break;
		case AtsType::InstrType::Stock:
			Con.secType = "STK";
			break;
		case AtsType::InstrType::Call:
		case AtsType::InstrType::Put:
			Con.secType = "OPT";
			break;
		case AtsType::InstrType::Forex:
			Con.secType = "CASH";
			break;
		default:
			break;
		}
		Con.currency = ord->get_instrument()->getInstrument()->get_class()->get_currency_name();
		Con.exchange = ord->get_instrument()->getMarket();
		Con.localSymbol = ord->get_instrument()->get_trading_code();
		//
		if (Con.secType == "CASH")
		{	
			Con.symbol = ord->get_instrument()->get_trading_code();
			Con.localSymbol = "";			
			//
			std::vector<std::string> sSymbol;
			boost::split(sSymbol,Con.symbol, boost::is_any_of("-"));
			Con.symbol   = sSymbol[0];
			Con.currency = sSymbol[1];
			//
		}
		//
		m_pClient->placeOrder(nordID, Con, ibOrd);		
		//
#if 0
		ib_create_pool.free_mem(ibOrd);
#endif
		return 1;
	}

	int ib_connection::market_cancel_order_async(order* ord, char* pszReason)
	{
		//loggerv2::info("ib_connection::market_cancel_order");
		int nordID = ord->get_id();

		m_pClient->cancelOrder(nordID);
		loggerv2::info("ib_connection:: market_cancel_order id %d", nordID);
		printf_ex("ib_connection:: market_cancel_order id %d\n", nordID);
		return 1;
	}

	void ib_connection::nextValidId(OrderId orderId)
	{
		printf_ex("ib_connection::nextValidId:%d\n",orderId);		
	}

	void ib_connection::connectAck()
	{
		printf_ex("ib_connection::connectAck\n");
		if (!m_extraAuth && m_pClient->asyncEConnect())
			m_pClient->startApi();
	}
	void ib_connection::error(const int id, const int errorCode, const std::string errorString)
	{

		printf_ex("ib_connection::error id:%d,errorCode: %d,errorString: %s \n", id, errorCode, errorString.c_str());
		if (m_debug)
		{
			loggerv2::info("ib_connection::error id:%d,errorCode: %d,errorString: %s \n", id, errorCode, errorString.c_str());
		}
		if (id == -1 && errorCode == 509)
		{
			//temprarally unavailable;api doesn't allow reconnect at this stage.
			//Sleep(100);
			on_status_changed(AtsType::ConnectionStatus::WaitConnect);
		}

		if (id > 0) //error related to an order
		{
			//loggerv2::info("ib_connection error related to order %d",id);
			//ib_order* o = nullptr;

			int ret;
			order *o = get_order_from_map(id, ret);

			if (ret!=2)
			{
				//ib_order* o = reinterpret_cast<ib_order*>(ord);

				if (o->get_last_action() != AtsType::OrderAction::Cancelled)
					on_nack_from_market_cb(o, errorString.c_str());

				else if (errorCode == 202) //order is cancelled.
					on_cancel_from_market_cb(o);

			}

			if (errorCode == 504) //not connected
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected);
				//loggerv2::info("reconnecting to IB..");
				if (m_pClient->eConnect(m_sHostname.c_str(), boost::lexical_cast<int>(m_sService), atoi(get_login_id().c_str()), false))
					on_status_changed(AtsType::ConnectionStatus::Connected);
			}

		}
	}
	/*
	The orderStatus callback

	The IBApi.EWrapper.orderStatus method contains all relevant information on the current status of the order execution-wise (i.e. amount filled and pending, filling price, etc.).
	*/
	void ib_connection::orderStatus(OrderId orderId, const std::string &status, int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld)
	{
		if (m_debug)
		{
			loggerv2::info("ib_connection::orderStatus orderId %d, status %s, filled %d, remaining %d,", orderId, status.c_str(), filled, remaining);
		}

		printf_ex("ib_connection::orderStatus orderId %d, status %s, filled %d, remaining %d\n", orderId, status.c_str(), filled, remaining);

		//order* o = NULL;

		int ret;
		order * o = get_order_from_map(orderId, ret);

		if (ret!=2)
		{
			//o = reinterpret_cast<ib_order*>(o);
			o->set_exec_quantity(filled);
			o->set_exec_price(lastFillPrice);
		}

		if (o == nullptr)
			return;

		if (status == std::string("Submitted") && (o && o->get_status() == AtsType::OrderStatus::WaitMarket))
		{
			on_ack_from_market_cb(o);
		}
		else if (status == std::string("Cancelled") && (o && o->get_status() == AtsType::OrderStatus::Ack))
		{
			if (o->get_last_action() == AtsType::OrderAction::Cancelled)
				on_cancel_from_market_cb(o);

			else if (o->get_last_action() == AtsType::OrderAction::Created)
				on_cancel_from_market_cb(o);
		}
		//else if (status == std::string("Cancelled") && (o->get_status() == S_ACK))
		//{
		//	on_cancel_from_market_cb(o);
		//}
	}


	void ib_connection::execDetails(int reqId, const Contract& contract, const Execution& execution)
	{
		//loggerv2::info("ib_connection::execDetails");
		printf_ex("ib_connection::execDetails orderId:%d\n", execution.orderId);

		if (m_debug)
		{
			loggerv2::info("ib_connection::execDetails,"
				"symbol:%s,"
				"secType:%s,"
				"localSymbol:%s,"
				"orderId:%d,"
				"cumQty:%d,"
				"avgPrice:%f,"
				"side:%s,"
				"shares:%f,"
				"orderRef:%s,"
				"execId:%s,"
				"acctNumber:%s,",
				contract.symbol.c_str(),
				contract.secType.c_str(),
				contract.localSymbol.c_str(),
				execution.orderId,
				execution.cumQty,
				execution.avgPrice,
				execution.side.c_str(),
				execution.shares,
				execution.orderRef.c_str(),
				execution.execId.c_str(),
				execution.acctNumber.c_str()
				);
		}
		//
		if (this->get_night_trade() == true)
		{			
			auto tp = string_to_lwtp(execution.time.c_str());
			int hour = get_hour_from_lwtp(tp);
			if ( hour > 8 && hour <16)
			{
				return;
			}
		}
		//
		int ordId = execution.orderId;

		int ret;
		order *o = get_order_from_map(ordId, ret);
		switch (ret)
		{
		case 0:
		case 1:
			//o = reinterpret_cast<ib_order*>(o);//it->second);
			break;

		case 2:
			o = ib_order_aux::anchor(this, contract, execution);
			if (o == nullptr)
			{
				loggerv2::error("ib_connection::execDetails cannot anchor order");
				return;
			}
			add_pending_order(o);
		default:
			break;
		}

		if (o == nullptr)
			return;

		int execQty = execution.shares;
		double execPrc = execution.price;
		//const char* pszExecRef = execution.execId.c_str();
		const char* pszTime = execution.time.c_str();
		pszTime = pszTime + 9;
		exec* e = new exec(o, execution.execId, execQty, execPrc, pszTime);
		on_exec_from_market_cb(o, e);
		//
		bool onlyUpdatePending = false;
		string ts = execution.time.c_str();
		ptime t(time_from_string(ts));
		lwtp tp = ptime_to_lwtp(t);				
		tp = tp + std::chrono::seconds(2);//ÔÊÐí2sµÄÎó²î
		if (o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;
		if (onlyUpdatePending)
		{
			loggerv2::info("ib_connection::execDetails will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time. tradeTime %s", execution.time.c_str());
		}
		update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
		//
	}

	void ib_connection::requestOpenOrders()
	{
		m_pClient->reqOpenOrders();
	}

	void ib_connection::requestExecs()
	{
		ExecutionFilter efilter = ExecutionFilter();
		m_pClient->reqExecutions(++m_requestId, efilter);
	}

	void ib_connection::requestAccountSummary()
	{
		//how to update these 2 strings?
		std::string group;
		std::string tag;
		m_pClient->reqAccountSummary(++m_requestId, group.c_str(), tag.c_str());

	}

	/*
	The openOrder callback

	The IBApi.EWrapper.openOrder method delivers an IBApi.Order object representing the open order within the system. 
	Additionally, an IBApi.OrderState object containing margin and commission information about this particular order.
	*/
	void ib_connection::openOrder(OrderId orderId, const Contract& contract, const Order& Ord, const OrderState& OrdStatus)
	{				
		printf_ex("ib_connection::openOrder orderId:%d,OrdStatus.status:%s\n", orderId, OrdStatus.status.c_str());		
		//
		if (m_debug)
		{
			loggerv2::info("ib_connection::openOrder "
				"symbol:%s,"
				"secType:%s,"
				"localSymbol:%s,"
				"orderId:%d,"	
				"orderRef:%s,"
				"totalQuantity:%f,"
				"lmtPrice:%f,"
				"OrdStatus:%s,",
				contract.symbol.c_str(),
				contract.secType.c_str(),
				contract.localSymbol.c_str(),
				Ord.orderId,
				Ord.orderRef.c_str(),
				Ord.totalQuantity,
				Ord.lmtPrice,
				OrdStatus.status.c_str()
				);
		}
		//
		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
		case 1:
			//o = reinterpret_cast<ib_order*>(o);//it->second);
			break;

		case 2:
		default:
			o = ib_order_aux::anchor(this, contract, Ord, OrdStatus);
			if (o == NULL)
			{
				loggerv2::error("ib_connection::openOrder cannot anchor order");
				return;
			}

			add_pending_order(o);
		}
		//
		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("ib_connection::openOrder resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}
		if (OrdStatus.status == std::string("Submitted"))
		{
			on_ack_from_market_cb(o);
		}
		else if (OrdStatus.status == std::string("Cancelled"))
		{
			on_cancel_from_market_cb(o);
		}
	}

	void ib_connection::openOrderEnd()
	{
		//loggerv2::info("End of Open Orders");
	}

	void ib_connection::execDetailsEnd(int reqId)
	{
		//loggerv2::info("End of Exec Details");

	}

	void ib_connection::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency)
	{
		//loggerv2::info("accountSummary reqId %d, account %s, tag %s, value %s, curency %s", reqId, account.c_str(), tag.c_str(), value.c_str(), curency.c_str());
	}
	void ib_connection::accountSummaryEnd(int reqId)
	{
		//loggerv2::info("accountSummary End");
	}
	void ib_connection::cancel_num_warning(tradeitem* i)
	{

	}
	void ib_connection::cancel_num_ban(tradeitem* i)
	{

	}
	void ib_connection::processMessages()
	{
		loggerv2::info("ib_connection processMessages");

		while (m_pClient->isConnected())
		{
			m_pReader->checkClient();
			m_osSignal.waitForSignal();
			m_pReader->processMsgs();
		}
	}
}