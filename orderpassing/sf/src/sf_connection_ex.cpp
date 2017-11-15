#include "sf_connection.h"
#include "string_tokenizer.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>
#include "tradeItem_gh.h"
#include "terra_logger.h"
#include "boost/shared_ptr.hpp"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp> 
#include <thrift/TProcessor.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace ::apache::thrift::concurrency;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace terra::common;
namespace sf
{
	zmq::context_t   g_zmq_context(4);
	//
	// sf_connection
	//
	sf_connection::sf_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "sf_connection";
		m_connectionStatus = false;
		m_isAlive = true;
		m_nRequestId = 0;
		m_nCurrentOrderRef = 0;
		m_bKey_with_exchange = false;
		//
		m_zmq_receiver = new zmq_client(g_zmq_context, boost::bind(&sf_connection::process_data, this, _1, _2), "OrderTradeRtnMsg");
		//
	}	
	bool sf_connection::init_config(const string &name, const std::string &strConfigFile)
	{
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 4)
			m_bTsession = true;
		else
			m_bTsession = false;
		//
		boost::filesystem::path p(strConfigFile);
		if (!boost::filesystem::exists(p))
		{
			printf_ex("sf_connection::load config_file:%s not exist!\n", strConfigFile.c_str());
			return false;
		}
		boost::property_tree::ptree root;
		boost::property_tree::ini_parser::read_ini(strConfigFile, root);
		m_zmq_server = root.get<string>(name + ".zmq_server", "");
		m_zmq_port = root.get<string>(name + ".zmq_port", "");
		//
		return true;
	}
	void sf_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{
		if (m_debug)
			loggerv2::info("sf_connection::request_investor_position requesting investor position for tradeitem %s", i->getCode().data());
		return;
	}
	void sf_connection::request_investor_full_positions()
	{
		if (m_debug)
			loggerv2::info("sf_connection:: calling OnRspQryInvestorPosition ");

		return;
	}	
	void sf_connection::request_trading_account()
	{
		if (m_debug)
			loggerv2::info("sf_connection:: calling ReqQryTradingAccount ");			
		return;
	}
	void sf_connection::init_connection()
	{			
		loggerv2::info("sf_connection::init_connection create trader api..");
#if 0	
		m_pfsApi->init();
#else
		this->init_api();
#endif

#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif
	}

#ifdef Linux
	void  sf_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&sf_connection::process, this));
		m_inputQueue.set_fd(efd);
		m_orderQueue.set_fd(efd);
		m_tradeQueue.set_fd(efd);
		m_inputActionQueue.set_fd(efd);
		m_outboundQueue.set_fd(efd);
	}
#endif

	void sf_connection::release()
	{
		ctpbase_connection::release();
#if 0		
		m_pfsApi->release();
#else
		this->release_api();
#endif
	}

	void sf_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("sf_connection::connect connecting to fs...");

			on_status_changed(AtsType::ConnectionStatus::WaitConnect);

#if 0
			m_pfsApi->connect();
#else
			this->connect_api();
#endif 
		}
	}

	void sf_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
#if 0
			if (m_pfsApi->disconnect() == false)
#else
			if (this->disconnect_api() == false)
#endif
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "sf_connection - ReqUserLogout failed");
			}
		}
	}	
	void sf_connection::process()
	{		
		m_outboundQueue.Pops_Handle_Keep(10);
#if 0
		m_pfsApi->Process();
#else
		this->Process_api();
#endif
	}

	int sf_connection::market_create_order_async(order* o, char* pszReason)
	{
		if (m_debug)
		{
			loggerv2::info("sf_connection::market_create_order_async,id:%d",o->get_id());
		}
	    //to do ...
		if (m_pUserApi)
		{
			/*
			void SimulationOperationConcurrentClient::CreateOrder(OrderRtnMsg& _return, const std::string& InstrCode, const  ::AtsType::OrderWay::type way, const int32_t quantity, const double price, const  ::AtsType::OrderOpenClose::type openclose, const int32_t orderId, const int32_t tradingtype, const  ::AtsType::OrderRestriction::type orderrestriction)
			*/
			o->dump_info();
			OrderRtnMsg rtnMsg;
			m_pUserApi->CreateOrder(rtnMsg, o->get_instrument()->get_trading_code(),o->get_way(),o->get_quantity(),o->get_price(),o->get_open_close(),o->get_id(),o->get_trading_type(),o->get_restriction());						
			int ret=-1;
			order* o = get_order_from_map(rtnMsg.orderId, ret);
			printf_ex("sf_connection::market_create_order_async the order:%d,IsSuccess:%d,ret:%d\n", rtnMsg.orderId, rtnMsg.IsSuccess, ret);
			if (o == nullptr)
			{
				printf_ex("sf_connection::market_create_order_async didn't find the order:%d,ret:%d\n",rtnMsg.orderId,ret);
				return 1;
			}
			if (rtnMsg.IsSuccess == true)//to ack			
			{						
				if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
				{
					if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
					{
						if (m_debug)
							loggerv2::debug("es_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
						o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
					}
				}
				//
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{
					update_instr_on_ack_from_market_cb(o);
					on_ack_from_market_cb(o);
				}
				//		
			}
			else
			{
				update_instr_on_nack_from_market_cb(o);
				on_nack_from_market_cb(o, rtnMsg.reason.c_str());
			}
		}
		return 1;
	}	
	int sf_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());
		//to do ...
		if (m_pUserApi)
		{
			OrderCancelRtnMsg _return;
			m_pUserApi->CancelOrder(_return,o->get_id());
			int ret=-1;
			order* o = get_order_from_map(_return.orderId, ret);
			printf_ex("sf_connection::market_cancel_order_async the order:%d,IsSuccess:%d,ret:%d,reason:%s\n",_return.orderId,_return.IsSuccess,ret,_return.reason.c_str());
			if (o == nullptr)
			{
				printf_ex("sf_connection::market_cancel_order_async didn't find the order:%d,ret:%d\n",_return.orderId, ret);
				return 1;
			}		
			if (_return.IsSuccess == true)
			{
				update_instr_on_cancel_from_market_cb(o);
				on_cancel_from_market_cb(o);
			}
			else
			{

			}
		}
		//
		return 1;		
	}
	bool sf_connection::setUDPSockect(const char * pBroadcastIP, int nBroadcastPort)
	{
		if (m_zmq_receiver == nullptr)
			return false;		
		//to do ...ip+port
		////zmq_publisher_rep=tcp://192.168.1.26:6665
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s:%d", pBroadcastIP, nBroadcastPort);
		printf_ex("sf_connection::setUDPSockect %s\n",buffer);
		return m_zmq_receiver->init(buffer);
	}
	void sf_connection::start_receiver()
	{
		m_zmq_receiver->process_msg();
	}
	void sf_connection::process_data(uint8_t* buffer, size_t len)
	{
		printf_ex("sf_connection::process_data len:%d\n", len);
		boost::shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer(buffer, len));
		boost::shared_ptr<TCompactProtocol> bin_proto(new TCompactProtocol(mem_buf));

		OrderTradeRtnMsg *pMsg=new OrderTradeRtnMsg();
		pMsg->read(bin_proto.get());
		m_trade_rtn_queue.Push(pMsg);
	}
	void sf_connection::OnRtnTradeAsync(OrderTradeRtnMsg* pTrade)
	{
		if (m_debug)
		{
			loggerv2::info("sf_connection:: OnRtnTradeAsync "
				"orderId:%d"
				"ExeQuantity:%d"
				"price:%f"
				"tradertime:%s",
				pTrade->orderId,pTrade->ExeQuantity,pTrade->price,pTrade->tradertime.c_str());
		}
		int ret=-1;
		int orderId = pTrade->orderId;
		order* o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:			
			break;
		case 1:			
			loggerv2::warn("sf_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:			
			break;
		default:
		{
			printf_ex("sf_connection::OnRtnTradeAsync ret:%d\n", ret);
			break;
		}
		}
		if (o == nullptr) // should not happen
		{
			loggerv2::error("sf_connection::OnRtnTradeAsync - order recovered nullptr");
			return;
		}
		int execQty = pTrade->ExeQuantity;
		double execPrc = pTrade->price;
		const char* pszExecRef = (char*)pTrade->MatchID.c_str();
		char* pszTime = (char*)pTrade->tradertime.c_str();
		exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);
		on_exec_from_market_cb(o, e);		
	}
	void sf_connection::OnRspOrderActionAsync(int* nOrdId)
	{
		if (m_debug)
			loggerv2::info("sf_connection::OnRspOrderActionAsync");
		//std::map<int, sf_order*>::iterator it = m_cancelOrdMap.find(nOrdRef);
		//
		//if (it != m_cancelOrdMap.end())
		//{
		// sf_order* o = reinterpret_cast<sf_order*>(it->second); // or dynamic_cast??
		// if (o != nullptr)
		// {
		//  on_nack_from_market_cb(o,NULL);
		// }

		// m_cancelOrdMap.erase(it);
		//}
		//sf_order* o = NULL;

		int ret;
		order* o = get_order_from_map(*nOrdId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<sf_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<sf_order*>(ord);
			loggerv2::warn("sf_connection::OnRspOrderActionAsync - message received on dead order[%d]...", nOrdId);
			break;

		case 2:
		default:
			break;
		}


		if (o == NULL) // should not happen
		{
			loggerv2::error("sf_connection::OnRspOrderActionAsync - order recovered NULL");
			return;
		}

		on_nack_from_market_cb(o, NULL);
		//update_instr_on_nack_from_market_cb(o);
	}
	int sf_connection::get_order_id(const char*pszOrderRef)
	{
		if (pszOrderRef != nullptr)
		{
			if (m_user_info_map.find(atoi(pszOrderRef)) != m_user_info_map.end())
			{
				user_info * info = m_user_info_map[atoi(pszOrderRef)];
#if 0
				return info->InternalRef;
#else

				return ctpbase_connection::get_order_id(info->UserID.c_str());
#endif
			}
		}
		return 0;
	}
	/*根据资金账号生成userID*/
	bool sf_connection::get_userId(const char* pszUserName, char* userID, int n)
	{
		memset(userID, 0, n);
		strcpy(userID, pszUserName);
		return true;
	}
	/*根据order.internalRef简历internalRef<->orderRef的映射关系*/
	void sf_connection::create_user_info(order * o, int orderRef, string userID)
	{
		if (o == nullptr)
			return;
		//
		sf_order_aux::set_ord_ref(o, orderRef);
		//
		//
		//if (m_order_id_map.find(o->get_id()) == m_order_id_map.end())
		//{
		//m_order_id_map.emplace(o->get_id(), orderRef);
		//}
		//
		if (m_user_info_map.find(orderRef) == m_user_info_map.end())
		{
			user_info * info = new user_info();
			info->OrderRef = orderRef;
#if 0
			info->UserID = userID;
			info->InternalRef = o->get_id();
			info->AccountNum = getAccountNum(o->get_account().c_str());
			info->UserOrderID = o->get_user_orderid();
			info->TradeType = getPortfolioNum(o->get_portfolio().c_str());
			info->Portfolio = o->get_trading_type();
#else
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			ctpbase_connection::compute_userId(o, buffer, sizeof(buffer));
			info->UserID = buffer;
#endif
			m_user_info_map.emplace(orderRef, info);

			//to do ... append the file every day
			this->append(info);

			//printf("sf_connection::create_user_info orderRef:%d,size:%d\n", orderRef, m_user_info_map.size());
		}
		else
		{
			printf("warn:sf_connection::create_user_info already include the userid:%s,orderRef:%d\n", userID.c_str(), orderRef);
		}
	}
	void sf_connection::get_user_info(const char* pszOrderRef, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType)
	{
		if (pszOrderRef != nullptr)
		{
			if (m_user_info_map.find(atoi(pszOrderRef)) != m_user_info_map.end())
			{
				user_info * info = m_user_info_map[atoi(pszOrderRef)];
#if 0
				internalRef = info->InternalRef;
				nAccount = info->AccountNum;
				userOrderId = info->UserOrderID;
				nPortfolio = info->Portfolio;
				nTradeType = info->TradeType;
#else
				ctpbase_connection::get_user_info(info->UserID.c_str(), nAccount, userOrderId, internalRef, nPortfolio, nTradeType);
#endif
				loggerv2::info("sf_connection::get_user_info internalRef:%d,orderRef:%d,size:%d\n", internalRef, atoi(pszOrderRef), m_user_info_map.size());
			}
			else
			{
				loggerv2::warn("sf_connection::get_user_info didn't find the orderRef:%s\n", pszOrderRef);
			}
		}
		else
		{
			internalRef = -1;
		}
	}
	void sf_connection::init_user_info(char * user_info_file)
	{
		if (user_info_file == nullptr)
			return;
		boost::filesystem::path p;
		p.clear();
		p.append(user_info_file);
		p.append("user_info.csv");
		m_user_info_file_name = p.string();
		printf("sf_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
			int    OrderRef;
			string UserID;
			#if 0
			int    InternalRef;
			int    AccountNum;
			int    UserOrderID;
			int    TradeType;
			int    Portfolio;
			#endif
			};
			*/
			user_info * info = new user_info();
			info->OrderRef = atoi(tokenizer[0]);
			info->UserID = tokenizer[1];
#if 0
			info->InternalRef = atoi(tokenizer[2]);
			info->AccountNum = atoi(tokenizer[3]);
			info->UserOrderID = atoi(tokenizer[4]);
			info->TradeType = atoi(tokenizer[5]);
			info->Portfolio = atoi(tokenizer[6]);
#endif
			m_user_info_map.emplace(info->OrderRef, info);

			//if (m_order_id_map.find(info->InternalRef) == m_order_id_map.end())
			//{
			//m_order_id_map.emplace(info->InternalRef,info->OrderRef);
			//}
		}
		stream.close();
	}
	void sf_connection::append(user_info * info)
	{
		if (info == nullptr)
			return;
		boost::filesystem::ofstream stream;
		stream.open(m_user_info_file_name.c_str(), ios::app);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
#if 0
		sprintf(buffer, "%s,%d,%d,%d,%d,%d,%d\n", info->UserID.c_str(), info->OrderRef, info->InternalRef, info->AccountNum, info->UserOrderID, info->TradeType, info->Portfolio);
#else
		sprintf(buffer, "%d,%s\n", info->OrderRef, info->UserID.c_str());
#endif
		stream << buffer;
		stream.close();
	}
	void sf_connection::init_api()
	{
		boost::shared_ptr<TSocket> socket(new TSocket(this->m_sHostname.c_str(), atoi(this->m_sService.c_str())));
		boost::shared_ptr<TBufferedTransport> transport(new TBufferedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));        
		//
		m_trade_rtn_queue.setHandler(boost::bind(&sf_connection::OnRtnTradeAsync, this, _1));
		//
	}
	void sf_connection::release_api()
	{
#if 0
		m_pUserApi->Release();
#endif
	}
	void sf_connection::Process_api()
	{
		m_trade_rtn_queue.Pops_Handle(0);
	}
	bool sf_connection::connect_api()
	{
		loggerv2::info("calling sf_connection::connect,%s:%s",this->m_sHostname.c_str(),this->m_sService.c_str());
		printf_ex("calling sf_connection::connect,%s:%s\n", this->m_sHostname.c_str(), this->m_sService.c_str());
		// For first connection, we are disconnected so we need to connect API first (RequestLogin will be done on API UP).
		// For later connections (disconnect / reconnect), API is already up so we just need to relogin.
		//
		if (m_connectionStatus == false)
		{
#if 0
			char addr[1024 + 1];
			snprintf(addr, 1024, "%s:%s", this->m_sHostname.c_str(), this->m_sService.c_str());

			m_pUserApi->RegisterSpi(this);

			m_pUserApi->RegisterFront(addr);
			loggerv2::info("sf_connection::connect connecting to %s", addr);
			switch (this->getResynchronizationMode())
			{
			case ResynchronizationMode::None:
				m_pUserApi->SubscribePrivateTopic(THOST_TERT_QUICK);
				break;

			case ResynchronizationMode::Last:
				m_pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);
				break;

			default:
			case ResynchronizationMode::Full:
				m_pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);
				break;
			}

			loggerv2::info("sf_connection::connect initializing api");
			m_pUserApi->Init();
#else
			boost::shared_ptr<TSocket> socket(new TSocket(this->m_sHostname.c_str(), atoi(this->m_sService.c_str())));
			boost::shared_ptr<TBufferedTransport> transport(new TBufferedTransport(socket));
			boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			transport->open();
			if (transport->isOpen()==true)
			{
				if (m_pUserApi == nullptr)
				{
					m_pUserApi = new SimulationOperationClient(protocol);
				}
#if 0				
				m_connectionStatus = true;
				this->on_status_changed(AtsType::ConnectionStatus::Connected, "connected ok");
				loggerv2::info("sf_connection::connect api intialized and connected!,%s:%s", this->m_sHostname.c_str(), this->m_sService.c_str());
				printf_ex("sf_connection::connect api intialized and connected!,%s:%s\n", this->m_sHostname.c_str(), this->m_sService.c_str());
#else
				if (setUDPSockect(m_zmq_server.c_str(), atoi(m_zmq_port.c_str())))
				{							
					m_connectionStatus = true;
					this->on_status_changed(AtsType::ConnectionStatus::Connected, "connected ok");
					loggerv2::info("sf_connection::connect api intialized and connected!,%s:%s", this->m_sHostname.c_str(), this->m_sService.c_str());
					printf_ex("sf_connection::connect api intialized and connected!,%s:%s\n", this->m_sHostname.c_str(), this->m_sService.c_str());

					m_receiver_thread = std::thread(&sf_connection::start_receiver, this);

				}				
#endif				
				return true;
			}
			else
			{
				loggerv2::info("sf_connection::connect fail!,%s:%s", this->m_sHostname.c_str(), this->m_sService.c_str());
				printf_ex("sf_connection::connect fail!,%s:%s\n", this->m_sHostname.c_str(), this->m_sService.c_str());
				return false;
			}
#endif			
		}
		else
		{
			request_login();
		}
				
		return true;
	}

	bool sf_connection::disconnect_api()
	{
		return true;
	}

	void sf_connection::request_login()
	{
		loggerv2::info("sf_connection::request_login");
	}
	
	void sf_connection::OnFrontConnected()
	{
		loggerv2::info("sf_api is UP");

		m_connectionStatus = true;

		if (this->getStatus() == AtsType::ConnectionStatus::WaitConnect)
		{
			request_login();
		}
		else
		{
			loggerv2::info("sf_api not asking for reconnect...");
		}
	}

	void sf_connection::OnFrontDisconnected(int nReason)
	{
		//loggerv2::info("sf_api is DOWN");

		m_connectionStatus = false;

		char* pszMessage;
		switch (nReason)
		{
			// normal disconnection
		case 0:
			pszMessage = "";
			break;

			// error
		case 1:
		case 2:
		case 3:
		case 4:
			//pszMessage = "ERROR MSG TO TRANSLATE [" + nReason + "];
			pszMessage = "ERROR MSG TO TRANSLATE";
			break;


		default:
			//pszMessage = "unknown error [" + nReason + "];
			pszMessage = "unknown error";
			break;
		}

		this->on_status_changed(AtsType::ConnectionStatus::Disconnected, pszMessage);
	}

	void sf_connection::OnHeartBeatWarning(int nTimeLapse)
	{
		loggerv2::info("sf_api - heartbeat warning");
	}
}


