#if 0
#include "EPosixClientSocket.h"
#endif
#include "EPosixClientSocketPlatform.h"
#include "ib_source.h"
#include "ib_feed_update.h"
#include "Contract.h"
#include "Order.h"
#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include "database_factory.h"
//
//#include "Execution.h"
//int g_orderId = 0;
//
namespace feed
{
	namespace ib
	{
		std::string ib_source::getInstrTypeShortName(std::string instType)
		{
			std::string res = "STK";
			if (instType == "Futures") res = "FUT";
			else if (instType == "Options") res = "OPT";
			else if (instType == "Forex") res = "CASH";
			return res;
		}
		void ib_source::loadSingleInstType(abstract_database* db, std::string instType)
		{			
			std::string cmd = "select Code,FeedCodes,Exchange,Currency,Maturity from " + instType + ",InstrumentClass where (" + instType + ".instrumentClass=instrumentClass.ClassName and " + instType + ".FeedCodes like '%IB%')";
			if (instType == "Stocks" || instType == "Forex")
			{
				cmd = "select Code,FeedCodes,Exchange,Currency from " + instType + ",InstrumentClass where (" + instType + ".instrumentClass=instrumentClass.ClassName and " + instType + ".FeedCodes like '%IB%')";
			}
			std::vector<boost::property_tree::ptree>* res = db->get_table(cmd.c_str());
			int i = 0;
			for (std::vector<boost::property_tree::ptree>::iterator it = res->begin(); it != res->end(); ++it)
			{
				std::string stemp = (*it).get("Code", "");
				std::string sExchange = (*it).get("Exchange", "");
				std::string sInstType = getInstrTypeShortName(instType);
				std::string sCurrency = (*it).get("Currency", "");
				std::string sLocalSymbol = (*it).get("FeedCodes", "IB");

				std::string sMaturity = (*it).get("Maturity", "");
				char buffer[32];
				memset(buffer, 0, sizeof(buffer));
				int j = 0;
				for (int i = 0; i < sMaturity.length(); i++)
				{
					if (sMaturity[i] != '-')
					{
						buffer[j] = sMaturity[i];
						j++;
					}
				}
				sMaturity = buffer;

				std::vector<std::string> sSymbol;
				if (!sLocalSymbol.empty())
				{
					boost::split(sSymbol, sLocalSymbol, boost::is_any_of("@"));
					ib_contract_info* iCI = new ib_contract_info(sSymbol[0], sExchange, sInstType, sCurrency);
					iCI->expire = sMaturity;
					m_contracts[stemp] = iCI;
					//m_contracts[sSymbol[0]] = iCI;
					++i;
				}
			}
			loggerv2::info("ib_source %d %s loaded from database", i, instType.c_str());
			delete res;
		}

		ib_source::ib_source(const string & sourceName, const string & hostname, const string & service, const string & user, const string & dbName)
			: feed_source("IB", sourceName, hostname, service, "", user, "", dbName),
#if 0
			m_pClient(new EPosixClientSocket(this))
#else
			m_pReader(0),
			m_osSignal(2000),
			m_extraAuth(false),
			m_pClient(new EClientSocket(this, &m_osSignal))
#endif
		{

		}
		ib_source::~ib_source()
		{
			release_source();
			
			//to do ...
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
			//
		}
		void ib_source::processMessages()
		{
			loggerv2::info("ib_source processMessages");
#if 0
			while (is_alive())
			{				
				m_pClient->checkMessages();
			}
#else
			while (m_pClient->isConnected())
			{
				//to do ...
#if 0
				static int i = 0;
				if (g_orderId > 0 && i == 0)
				{
					Contract contract;
					contract.symbol = "EUR";
					contract.secType = "CASH";
					contract.currency = "USD";
					contract.exchange = "IDEALPRO";
					Order ibOrd;
					ibOrd.orderId = g_orderId;
					ibOrd.action = "BUY";
					ibOrd.orderType = "LMT";
					ibOrd.totalQuantity = 20000;
					ibOrd.lmtPrice = 1.2;
					m_pClient->placeOrder(g_orderId, contract, ibOrd);
					i++;
				}
#endif
				//
				m_pReader->checkClient();
				m_osSignal.waitForSignal();
				m_pReader->processMsgs();
			}
#endif
		}
		void ib_source::init_source()
		{
			get_queue()->setHandler(boost::bind(&ib_source::process_msg, this, _1));

			/*std::thread t(std::bind(&ib_source::process, this));
			t.detach();*/
#ifdef Linux
			init_epoll_eventfd();
#else
			init_process(io_service_type::feed);
#endif
			//
			feed_source::load_database();
			//

			//load contract info from DB
			abstract_database* db = database_factory::create("sqlite", get_database_name().c_str());

			if (db->open_database())
			{
				loggerv2::info("ib_source connected to database %s", get_database_name().c_str());
			}

			std::vector<std::string> sInsVec = { "Futures", "Stocks", "Options","Forex" };
			for (std::string s : sInsVec)
			{
				loadSingleInstType(db, s);
			}

			db->close_databse();
			loggerv2::info("ib_source close database");

#if 0
			m_tListhen2IB = std::thread(&ib_source::processMessages, this);
#endif
			if (m_pClient->eConnect(get_service_name().c_str(), atoi(get_port().c_str()), atoi(get_user_name().c_str()), false))
			{
				update_state(AtsType::FeedSourceStatus::Up, "");
				//to do ...
				m_pReader = new EReader(m_pClient, &m_osSignal);

				m_pReader->start();
				//
				m_tListhen2IB = std::thread(&ib_source::processMessages, this);
			}
		}
#ifdef Linux
		void  ib_source::init_epoll_eventfd()
		{
			efd = eventfd(0, EFD_NONBLOCK);
			if (-1 == efd)
			{
				cout << "ib_source efd create fail" << endl;
				exit(1);
			}

			add_fd_fun_to_io_service(io_service_type::feed, efd, std::bind(&ib_source::process, this));
			m_queue.set_fd(efd);
		}
#endif
		void ib_source::release_source()
		{

			feed_source::release_source();
			m_tListhen2IB.join();
		}
		bool ib_source::subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || get_status() == AtsType::FeedSourceStatus::Down)
				return false;
			if (feed_item->is_subsribed() == true)
				return true;
			add_feed_item(feed_item);
			feed_item->subscribe();
			if (m_contracts[feed_item->get_code()] == nullptr)
			{
				return false;
			}
			return this->subscribe_item(feed_item);
		}
		bool ib_source::un_subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || get_status() == AtsType::FeedSourceStatus::Down)
				return false;
			feed_item->un_subscribe();
			if (m_contracts[feed_item->get_code()] == nullptr)
			{
				return false;
			}
			return this->unsubscribe_item(feed_item);
		}

		bool ib_source::subscribe_item(feed_item* pItem)
		{
			if (pItem == nullptr)
				return false;

			Contract con = Contract();
			if (!CreateContract(pItem, con))
			{
				loggerv2::error("ib_source::subscribe_item ib_source cannot create contract for instrument %s, subscription failed.", pItem->get_code().c_str());
				return false;
			}

			TagValueListSPtr tvl = TagValueListSPtr();
			m_pClient->reqMktData(++m_requestId, con, "", false, tvl);
			m_requestFeedItems.insert(std::make_pair(m_requestId, pItem));
			loggerv2::info("ib_source::subscribe_item item %s - request id %d has been subscribed", con.symbol.c_str(), m_requestId);
			return true;

		}

		Contract ib_source::CreateContract(feed_item* ib)
		{
			Contract con = Contract();
			std::map<std::string, ib_contract_info*>::iterator it = m_contracts.find(ib->get_code());
			if (it != m_contracts.end())
			{
				con.secType = it->second->secType;
				con.exchange = it->second->exchange;
				con.currency = it->second->currency;				
				con.localSymbol = it->second->localSymbol;
#if 0
				con.expiry = it->second->expire;
#else
				con.lastTradeDateOrContractMonth = it->second->expire;
				con.symbol = it->first;
				if (it->second->secType == "CASH")
				{
					con.localSymbol = "";
					//
					std::vector<std::string> sSymbol;
					boost::split(sSymbol,it->second->localSymbol, boost::is_any_of("-"));
					con.symbol   = sSymbol[0];
					con.currency = sSymbol[1];
					//
				}
#endif
			}
			return con;
		}

		bool ib_source::CreateContract(feed_item* ib, Contract& con)
		{
			std::map<std::string, ib_contract_info*>::iterator it = m_contracts.find(ib->get_code());
			if (it != m_contracts.end())
			{
				con.secType = it->second->secType;
				con.exchange = it->second->exchange;
				con.currency = it->second->currency;
				con.localSymbol = it->second->localSymbol;
#if 0
				con.expiry = it->second->expire;
#else
				con.lastTradeDateOrContractMonth = it->second->expire;
				con.symbol = it->first;
				if (it->second->secType == "CASH")
				{
					con.localSymbol = "";
					//
					std::vector<std::string> sSymbol;
					boost::split(sSymbol, it->second->localSymbol, boost::is_any_of("-"));
					con.symbol   = sSymbol[0];
					con.currency = sSymbol[1];
					//
				}
#endif
			}
			else
			{
				return false;
			}
			return true;
		}
		bool ib_source::unsubscribe_item(feed_item* pItem)
		{
			if (pItem == nullptr)
				return false;
			m_pClient->cancelMktData(std::atoi(pItem->get_code().c_str()));
			return true;
		}
		void ib_source::process_msg(ib_feed_update* pMsg)
		{
			feed_item* aFeedItem = m_requestFeedItems[pMsg->m_tickId];
			if (aFeedItem == nullptr)
			{
				//loggerv2::info("ib_source: instrument %s not found", pMsg->m_tickId);				
				return;
			}
			process_msg(pMsg, aFeedItem);
			return post(aFeedItem);
		}
		//EWrapper
		void ib_source::nextValidId(OrderId orderId)
		{
			printf_ex("ib_source::nextValidId %d\n",orderId);
			//g_orderId = orderId;
		}
		void ib_source::connectAck()
		{
			printf_ex("ib_source::connectAck\n");
			if (!m_extraAuth && m_pClient->asyncEConnect())
				m_pClient->startApi();
		}
		void ib_source::error(const int id, const int errorCode, const std::string errorString)
		{
			/*
			In TWS (not IB Gateway) at the time of initial connection, if there is valid connection to the IB server there will also be callbacks to IBApi::EWrapper::error with errorCode=2104 or 2106 and messages of the type "Market Data Server is ok" to indicate there is an active connection to the IB market data server. 
			It is best to wait until this message has been received before sending messages to TWS, since they may be dropped if there is not an active connection.
			*/
			printf_ex("ib_source::error,%d,%s\n",errorCode,errorString.c_str());
		}
		void ib_source::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)
		{
			//logger::info("calling ib_source::tickPrice,TickerId %d, TickType %d, double %f", tickerId, field, price);

			ib_feed_update* ibfu = new ib_feed_update(tickerId, field, price, 0);
			get_queue()->CopyPush(ibfu);
		}

		void ib_source::tickSize(TickerId tickerId, TickType field, int size)
		{
			//logger::info("calling ib_source::tickSize,TickerId %d, TickType %d, int %d", tickerId, field, size);
			ib_feed_update* ibfu = new ib_feed_update(tickerId, field, 0, size);
			get_queue()->CopyPush(ibfu);

		}

		void ib_source::tickOptionComputation(TickerId tickerId, TickType tickType, double impliedVol, double delta, double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice)
		{
			//logger::info("calling ib_source::tickOptionComputation");
		}

		void ib_source::tickGeneric(TickerId tickerId, TickType tickType, double value) {
			//logger::info("calling ib_source::tickGeneric,TickerId %d, TickType %d, value %f", tickerId, tickType, value);
			ib_feed_update* ibfu = new ib_feed_update(tickerId, tickType, value, 0);
			get_queue()->CopyPush(ibfu);
		}

		void ib_source::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
			//logger::info("calling ib_source::tickString,TickerId %d, TickType %d, std::string& %s", tickerId, tickType, value.c_str());
		}

		void ib_source::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints, double totalDividends, int holdDays, const std::string& futureExpiry, double dividendImpact, double dividendsToExpiry) {
			//logger::info("calling ib_source::tickEFP");
		}

		void ib_source::openOrder(OrderId orderId, const Contract&, const Order&, const OrderState& ostate) {}
		void ib_source::openOrderEnd() {}
		void ib_source::winError(const std::string &str, int lastError) {}
		void ib_source::connectionClosed() {}
		void ib_source::updateAccountValue(const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName) {}
		void ib_source::updatePortfolio(const Contract& contract, int position, double marketPrice, double marketValue, double averageCost, double unrealizedPNL, double realizedPNL, const std::string& accountName){}
		void ib_source::updateAccountTime(const std::string& timeStamp) {}
		void ib_source::accountDownloadEnd(const std::string& accountName) {}
		void ib_source::contractDetails(int reqId, const ContractDetails& contractDetails) {}
		void ib_source::bondContractDetails(int reqId, const ContractDetails& contractDetails) {}
		void ib_source::contractDetailsEnd(int reqId) {}
		void ib_source::execDetails(int reqId, const Contract& contract, const Execution& execution)
		{
			//printf_ex("ib_source::execDetails orderId:%d\n", execution.orderId);
		}
		void ib_source::execDetailsEnd(int reqId) {}

		void ib_source::updateMktDepth(TickerId id, int position, int operation, int side, double price, int size)
		{
			//logger::info("calling ib_source::updateMktDepth..");
			//ib_feed_update* ibfu = new ib_feed_update(id,position,side,price,size);
			//get_queue()->Push(ibfu);
		}

		void ib_source::updateMktDepthL2(TickerId id, int position, std::string marketMaker, int operation, int side, double price, int size)
		{
			//logger::info("calling ib_source::updateMktDepthsL2");

		}
		void ib_source::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {}
		void ib_source::managedAccounts(const std::string& accountsList) {}
		void ib_source::receiveFA(faDataType pFaDataType, const std::string& cxml) {}
		void ib_source::historicalData(TickerId reqId, const std::string& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps) {}
		void ib_source::scannerParameters(const std::string &xml) {}
		void ib_source::scannerData(int reqId, int rank, const ContractDetails &contractDetails, const std::string &distance, const std::string &benchmark, const std::string &projection, const std::string &legsStr) {}
		void ib_source::scannerDataEnd(int reqId) {}
		void ib_source::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, long volume, double wap, int count) {}
		void ib_source::fundamentalData(TickerId reqId, const std::string& data) {}
		void ib_source::deltaNeutralValidation(int reqId, const UnderComp& underComp) {}
		void ib_source::tickSnapshotEnd(int reqId) {}
		void ib_source::marketDataType(TickerId reqId, int marketDataType) {}
		void ib_source::commissionReport(const CommissionReport& commissionReport) {}
		void ib_source::position(const std::string& account, const Contract& contract, int position, double avgCost) {}
		void ib_source::positionEnd() {}
		void ib_source::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency) {}
		void ib_source::accountSummaryEnd(int reqId) {}
		void ib_source::verifyMessageAPI(const std::string& apiData) {}
		void ib_source::verifyCompleted(bool isSuccessful, const std::string& errorText) {}
		void ib_source::displayGroupList(int reqId, const std::string& groups) {}
		void ib_source::displayGroupUpdated(int reqId, const std::string& contractInfo) {}
		//void ib_source::process()
		//{
		//	//int i = 0;
		//	while (true)
		//	{
		//		//if (get_queue()->m_queue.read_available() > 0)
		//		//{
		//		//	for (auto &func : get_queue()->m_handler)
		//		//	{
		//		//		func();
		//		//	}
		//		//}
		//		//while (get_queue()->Pop_Handle())
		//		//++i;
		//		get_queue()->Pops_Handle();
		//		//if (i >= 10)
		//		{
		//			//i = 0;
		//			sleep_by_milliseconds(std::chrono::microseconds(500));
		//		}
		//	}
		//}

		void ib_source::process()
		{
			get_queue()->Pops_Handle();
		}
		//int ib_source::process_out_bound_msg_handler()
		//{
		//	int i = 0;
		//	for (; i < 10 && get_queue()->m_queue.read_available()>0; ++i)
		//	{
		//		ib_feed_update* msg = get_queue()->Pop();
		//		this->process_msg(msg);
		//	}
		//	return i;
		//}
		void ib_source::process_msg(ib_feed_update* pMsg, feed_item * feed_item)
		{
			TickType ticktype = pMsg->m_ticktype;
			double price = pMsg->m_price;
			int qty = pMsg->m_qty;
			if (feed_item == nullptr)
				return;
			switch (ticktype)
			{
			case BID:
			{
				if (feed_item->get_implicit_pre_open() == false)
				{
					feed_item->set_bid_price(price);
				}
				else
				{
					feed_item->set_market_bid(0, price);
				}
			}
			break;
			case BID_SIZE:
			{
				if (feed_item->get_implicit_pre_open() == false)
				{
					feed_item->set_bid_quantity(qty);
				}
				else
				{
					feed_item->set_market_bid_qty(0, qty);
				}
			}
			break;
			case ASK:
			{
				if (feed_item->get_implicit_pre_open() == false)
				{
					feed_item->set_ask_price(price);
				}
				else
				{
					feed_item->set_market_ask(0, price);
				}

			}
			break;
			case ASK_SIZE:
			{
				if (feed_item->get_implicit_pre_open() == false)
				{
					feed_item->set_ask_quantity(qty);
				}
				else
				{
					feed_item->set_market_ask_qty(0, qty);
				}
			}
			break;
			case LAST:
			{
				feed_item->set_last_price(price);
			}
			break;
			case LAST_SIZE:
			{
				feed_item->set_last_quantity(qty);
			}
			break;
			case HIGH:
			{

			}
			break;
			case LOW:
			{

			}
			break;
			case VOLUME:
			{
				feed_item->set_daily_volume(qty);
			}
			break;
			case CLOSE:
			{
				double closePrice = !(price == NO_PRICE || price == 0) ? price : 0;
				feed_item->set_close_price(closePrice);
			}
			break;
			default:
				break;
			}
		}
	}
}

