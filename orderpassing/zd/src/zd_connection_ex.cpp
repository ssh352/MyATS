#include "zd_connection.h"
#include "ShZdFutureUserApiStruct.h"
#include <string>
#include "string_tokenizer.h"
#include <boost/property_tree/ini_parser.hpp>
using namespace terra::common;
namespace zd
{

	zd_connection::zd_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
	{
		m_sName = "zd_connection_ex";
		m_pZdApi = new zd_api(this);
		m_bKey_with_exchange = false;
	}
	zd_connection::~zd_connection()
	{
		delete m_pZdApi;
	}	
	bool zd_connection::init_config(const string &name, const std::string &strConfigFile)
	{
		if (!ctpbase_connection::init_config(name, strConfigFile))
			return false;
		lwtp tp = get_lwtp_now();
		int hour = get_hour_from_lwtp(tp);
		if (hour < 16 && hour > 8)
			m_bTsession = true;
		else
			m_bTsession = false;
		boost::filesystem::path p(strConfigFile);
		if (!boost::filesystem::exists(p))
		{
			printf_ex("es_connection::load config_file:%s not exist!\n", strConfigFile.c_str());
			return false;
		}
		boost::property_tree::ptree root;
		boost::property_tree::ini_parser::read_ini(strConfigFile, root);
		m_strAuthCode = root.get<string>(name + ".auth_code", "");
		//
		m_strFutures = root.get<string>(name + ".Futures", "");
		m_strInsertTimeStart = root.get<string>(name + ".InsertTimeStart", "");
		//
		return true;
	}
	void zd_connection::request_instruments()
	{
		m_pZdApi->request_instruments();
	}
	void zd_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
	{
#if 0
		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		strcpy(pRequest.InstrumentID, i->get_trading_code());
		m_pZdApi->ReqQryInvestorPosition(&pRequest);
		if (m_debug)
			loggerv2::info("zd_connection::request_investor_position requesting investor position for tradeitem %s", i->getCode().c_str());
#endif
	}
	void zd_connection::request_investor_full_positions()
	{
#if 0
		CThostFtdcQryInvestorPositionField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, m_sUsername.c_str());
		m_pZdApi->ReqQryInvestorPosition(&pRequest);
		if (m_debug)
			loggerv2::info("zd_connection:: calling OnRspQryInvestorPosition ");
#else
		/*
		int ReqQryInvestorPosition(CTShZdQryInvestorPositionField *pQryInvestorPosition, int nRequestID)
		CTShZdQryInvestorPositionField pQryInvestorPosition;
		memset(&pQryInvestorPosition,0,sizeof(CTShZdQryInvestorPositionField));
		memcpy(pQryInvestorPosition.UserID,"MN000301",13);
		memcpy(pQryInvestorPosition.InvestorID,"MN00000903",18);
		apiTrade->ReqQryInvestorPosition(&pQryInvestorPosition,12);
		*/	
		m_pZdApi->ReqQryInvestorPosition();
#endif
	}
	//
	void zd_connection::req_RiskDegree()
	{
		request_trading_account();
	}
	//
	void zd_connection::request_trading_account()
	{
//#if 1
		//if (m_debug)
			//loggerv2::info("zd_connection:: calling ReqQryTradingAccount ");
		/*
		///ֱ���ѯ�ʽ��˻�
		struct CTShZdQryTradingAccountField
		{
		///���͹�˾����
		TShZdBrokerIDType	BrokerID;
		///Ͷ���ߴ���  �ʽ��˺�
		TShZdInvestorIDType	InvestorID;
		///�û�����
		TShZdUserIDType	UserID;
		};
		*/
		CTShZdQryTradingAccountField pRequest;
		memset(&pRequest, 0, sizeof(pRequest));
		strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
		strcpy(pRequest.InvestorID, get_investor_id().c_str());
		strcpy(pRequest.UserID, get_user_id().c_str());
		m_pZdApi->ReqQryTradingAccount(&pRequest);
//#endif
	}
	void zd_connection::init_connection()
	{
		loggerv2::info("zd_connection::init_connection create trader api..");
		m_pZdApi->init();

#ifdef Linux
		init_epoll_eventfd();
#else
		init_process(io_service_type::trader, 10);
#endif		
		m_bKey_with_exchange = false;
	}
#ifdef Linux
	void  zd_connection::init_epoll_eventfd()
	{
		efd = eventfd(0, EFD_NONBLOCK);
		if (-1 == efd)
		{
			cout << "x1 efd create fail" << endl;
			exit(1);
		}

		add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&zd_connection::process, this));

		m_pZdApi->get_input_queue() ->set_fd(efd);
		//m_pZdApi->get_input_quote_queue()->set_fd(efd);

		m_pZdApi->get_order_queue()->set_fd(efd);
		//m_pZdApi->get_order_quote_queue()->set_fd(efd);

		m_pZdApi->get_trade_queue()->set_fd(efd);
		m_pZdApi->get_input_action_queue()->set_fd(efd);
		//m_pZdApi->get_input_action_quote_queue()->set_fd(efd);

		m_outboundQueue.set_fd(efd);
		//m_outquoteboundQueue.set_fd(efd);
	}
#endif
	void zd_connection::release()
	{				
		ctpbase_connection::release();
		m_pZdApi->release();
	}
	void zd_connection::connect()
	{
		if (m_status == AtsType::ConnectionStatus::Disconnected)
		{
			loggerv2::info("zd_connection::connect connecting to zd...");
			on_status_changed(AtsType::ConnectionStatus::WaitConnect);
			m_pZdApi->connect();
		}
	}
	void zd_connection::disconnect()
	{
		if (m_status != AtsType::ConnectionStatus::Disconnected)
		{
			if (m_pZdApi->disconnect() == false)
			{
				on_status_changed(AtsType::ConnectionStatus::Disconnected, "zd_connection_ex - ReqUserLogout failed");
			}
		}
	}
	void zd_connection::process()
	{
		m_outboundQueue.Pops_Handle_Keep(10);
		m_outquoteboundQueue.Pops_Handle_Keep(10);
		m_pZdApi->Process();
	}
	/*
	CTShZdInputOrderField pOrder;
	memset(&pOrder,0,sizeof(CTShZdInputOrderField));

	memcpy(pOrder.OrderLocalID,"20161128141700",14);
	memcpy(pOrder.ExchangeID,"CME",11);//������
	memcpy(pOrder.InvestorID,"MN00000903",13);//�ʽ��ʺ�

	memcpy(pOrder.InstrumentID,"CL1703",31);
	memcpy(pOrder.UserID,"MN000301",16);
	pOrder.Direction='1';
	pOrder.VolumeTotalOriginal = 3;
	pOrder.LimitPrice = atof("45");
	pOrder.OrderPriceType='2';
	pOrder.OrderType ='P';
	apiTrade->ReqOrderInsert(&pOrder,0);
	*/
	int zd_connection::market_create_order_async(order* o, char* pszReason)
	{		
		if (o->get_way() != AtsType::OrderWay::Exercise)
		{
			CTShZdInputOrderField *request = zd_create_pool.get_mem();
			memset(request, 0, sizeof(CTShZdInputOrderField));

			strcpy(request->InvestorID, this->get_investor_id().c_str());
			
			strcpy(request->UserID, this->get_user_id().c_str());

			strcpy(request->InstrumentID, o->get_instrument()->get_trading_code());

			strcpy(request->ExchangeID, o->get_exchange_id().c_str());

			//
			request->OrderType = TSHZD_ORDT_Api;
			//

			if (o->get_way() == AtsType::OrderWay::Buy)
				request->Direction = TSHZD_D_Buy;
			else if (o->get_way() == AtsType::OrderWay::Sell)
				request->Direction = TSHZD_D_Sell;

			request->LimitPrice = o->get_price();
			request->VolumeTotalOriginal = o->get_quantity();
			/*
			/////////////////////////////////////////////////////////////////////////
			///��Ч����������
			/////////////////////////////////////////////////////////////////////////
			///������Ч  ֱ��
			#define TSHZD_TC_DAY '1'
			///������Ч��GTC��  ֱ��
			#define TSHZD_TC_GTC '2'
			///OPG
			#define TSHZD_TC_OPG '3'
			///IOC
			#define TSHZD_TC_IOC '4'
			///FOK
			#define TSHZD_TC_FOK '5'
			///���Ͼ�����Ч
			#define TSHZD_TC_GFA '6'

			typedef char TShZdTimeConditionType;
			*/
			/*
			/////////////////////////////////////////////////////////////////////////
			///�ɽ�����������
			/////////////////////////////////////////////////////////////////////////
			///�κ�����
			#define TSHZD_VC_AV '1'
			///��С����
			#define TSHZD_VC_MV '2'
			///ȫ������
			#define TSHZD_VC_CV '3'

			typedef char TShZdVolumeConditionType;
			*/
			if (o->get_restriction() == AtsType::OrderRestriction::None)
				request->TimeCondition = TSHZD_TC_DAY;
			else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)
				request->TimeCondition = TSHZD_TC_IOC;
			else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:�����ɽ�,ʣ�ಿ���Զ�����
			{
				request->TimeCondition   = TSHZD_TC_IOC;
				request->VolumeCondition = TSHZD_VC_AV;
			}
			else
			{
				snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
				zd_create_pool.free_mem(request);
				return 0;
			}
			/*
			/////////////////////////////////////////////////////////////////////////
			///��ƽ��־����
			/////////////////////////////////////////////////////////////////////////
			///����
			#define TSHZD_OF_Open '0'
			///ƽ��
			#define TSHZD_OF_Close '1'
			///ǿƽ
			#define TSHZD_OF_ForceClose '2'
			///ƽ��
			#define TSHZD_OF_CloseToday '3'
			///ƽ��
			#define TSHZD_OF_CloseYesterday '4'
			///ǿ��
			#define TSHZD_OF_ForceOff '5'
			///����ǿƽ
			#define TSHZD_OF_LocalForceClose '6'

			typedef char TShZdOffsetFlagType;
			*/
			TShZdOffsetFlagType oc = TSHZD_OF_Open;
			if (o->get_open_close() == OrderOpenClose::Undef)
			{
				o->set_open_close(compute_open_close(o, m_bCloseToday));
			}
			switch (o->get_open_close())
			{
			case AtsType::OrderOpenClose::Open:
				break;

			case AtsType::OrderOpenClose::Close:
				oc = TSHZD_OF_Close;
				break;
			case AtsType::OrderOpenClose::CloseToday:
				oc = TSHZD_OF_CloseToday;
				break;

			default:

				break;
			}
			request->CombOffsetFlag[0] = oc;
			if (m_debug)
				loggerv2::info("zd_connection::market_create_order CombOffsetFlag is %c", oc);			

			/*
			/////////////////////////////////////////////////////////////////////////
			///�����۸���������
			/////////////////////////////////////////////////////////////////////////
			///�޼۵�
			#define TSHZD_OPT_LimitPrice '1'
			///�м۵�
			#define TSHZD_OPT_AnyPrice '2'
			///�޼�ֹ��
			#define TSHZD_OPT_BestPrice '3'
			///ֹ��
			#define TSHZD_OPT_LastPrice '4'

			typedef char TShZdOrderPriceTypeType;
			*/
			switch (o->get_price_mode())
			{
				case AtsType::OrderPriceMode::Limit:
				{
					request->OrderPriceType = TSHZD_OPT_LimitPrice;
					//strcpy(request->LimitPrice, std::to_string(o->get_price()).c_str());
					request->LimitPrice = o->get_price();
				}
				break;
				case AtsType::OrderPriceMode::Market:
				{
					request->OrderPriceType = TSHZD_OPT_AnyPrice;
				}

				break;
				default:
				{
					snprintf(pszReason, REASON_MAXLENGTH, "undefined price mode.\n");
					o->set_status(AtsType::OrderStatus::Reject);
					zd_create_pool.free_mem(request);
					return 0;
				}
			}
			/*
			/////////////////////////////////////////////////////////////////////////
			///Ͷ���ױ���־����
			/////////////////////////////////////////////////////////////////////////
			///Ͷ��
			#define TSHZD_HF_Speculation '1'
			///����
			#define TSHZD_HF_Arbitrage '2'
			///�ױ�
			#define TSHZD_HF_Hedge '3'

			typedef char TShZdHedgeFlagType;
			*/
			request->CombHedgeFlag[0] = TSHZD_HF_Speculation;
			//request->TimeCondition = THOST_FTDC_TC_GFD;
			request->VolumeCondition = TSHZD_VC_AV;
			if (o->get_restriction() == AtsType::OrderRestriction::None)
			{
				request->TimeCondition = TSHZD_TC_DAY; // or GFS ???
				strcpy(request->GTDDate, "");
			}
			else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:����ȫ���ɽ�����ȫ���Զ�����
			{
				request->TimeCondition   = TSHZD_TC_IOC;
				request->VolumeCondition = TSHZD_VC_CV;
			}
			else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:�����ɽ�,ʣ�ಿ���Զ�����
			{
				request->TimeCondition  = TSHZD_TC_IOC;
				request->VolumeCondition = TSHZD_VC_AV;
			}
			else
			{
				snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported\n", o->get_restriction());
				zd_create_pool.free_mem(request);
				return 0;
			}
			//request->VolumeCondition = THOST_FTDC_VC_AV;
			request->ContingentCondition = TSHZD_CC_Immediately;
			request->ForceCloseReason    = TSHZD_FCC_NotForceClose;

			request->MinVolume = 1;
			//request->IsAutoSuspend = 0;
			//request->UserForceClose = 0;
//#if 1			
			sprintf(request->OrderLocalID, "%d", o->get_id());		
			char user_id[256];
			memset(user_id, 0, sizeof(user_id));
			compute_userId(o, user_id, sizeof(user_id));
			this->create_user_info(user_id, request->OrderLocalID);
//#endif
			if (!m_pZdApi->ReqOrderInsert(request))
			{				
				snprintf(pszReason, REASON_MAXLENGTH, "zd api reject!\n");
				zd_create_pool.free_mem(request);
				loggerv2::error("zd_connection::market_create_order_async fail to ReqOrderInsert,id:%d",o->get_id());
				return 0;
			}
			zd_create_pool.free_mem(request);
		}
		return 1;
	}	
	/*
	//����
	CTShZdOrderActionField pCancel;
	memset(&pCancel,0,sizeof(CTShZdInputOrderActionField));

	printf("��������ϵͳ�ţ�������(�ո�ֿ�)\n");
	scanf("%s%s",pCancel.OrderRef,pCancel.OrderSysID);
	pCancel.ActionFlag = '0';//0�ǳ���
	pCancel.OrderType ='P';
	apiTrade->ReqOrderAction(&pCancel,0);
	*/
	int zd_connection::market_cancel_order_async(order* o, char* pszReason)
	{
		if (m_debug)
			loggerv2::info("+++ market_cancel_order_async : %d", o->get_id());			
		if (o->get_way() != AtsType::OrderWay::Exercise)
		{
			/*
			///ֱ�ﱨ������  ���� ���ĵ� ����
			struct CTShZdOrderActionField
			{
			///�������
			TShZdOrderRefType	OrderRef;
			///ϵͳ���
			TShZdOrderSysIDType	OrderSysID;
			///������־
			TShZdActionFlagType	ActionFlag;
			///�޸ĵļ۸� ���ĵ���д��
			TShZdPriceType	LimitPrice;
			///�����仯(�ĵ���д)
			TShZdVolumeType	VolumeChange;
			///�û�����
			TShZdUserIDType	UserID;
			///�����ͻ�������  API���û�ֻ����дC ����  P
			TShZdOrderTypeType OrderType;
			};
			*/
			CTShZdOrderActionField *request = zd_cancel_pool.get_mem();
			memset(request, 0, sizeof(CTShZdOrderActionField));

			strcpy(request->UserID, this->get_user_id().c_str());

			strcpy(request->OrderSysID, zd_order_aux::get_order_sys_id(o).c_str());
			sprintf(request->OrderRef,"%d", zd_order_aux::get_ord_ref(o));

			//
			if (zd_order_aux::get_ord_ref(o) < 1)
			{
				zd_cancel_pool.free_mem(request);
				loggerv2::info("zd_connection::market_cancel_order_async should not be called for the orderRef:%d is invalid", zd_order_aux::get_ord_ref(o));
				return -1;
			}
			//

			request->ActionFlag = TSHZD_AF_Delete;

			if (!m_pZdApi->ReqOrderAction(request))
			{
				zd_cancel_pool.free_mem(request);
				loggerv2::error("zd_connection::market_cancel_order_async fail to call the ReqOrderAction,id:%d",o->get_id());
				return 0;
			}
			m_pZdApi->getOrdInputActiondRefMap().insert(std::pair<string, int>(request->OrderRef, o->get_id()));
			loggerv2::info("zd_connection::market_cancel_order_async update the OrdInputActiondRefMap by the %s:%d",request->OrderRef, o->get_id());
			//
			zd_cancel_pool.free_mem(request);
			return 1;
		}
	}
	//
	//  zd callbacks
	//
	/*
	///ֱ�����뱨��
	struct CTShZdInputOrderField
	{
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///ֱ����ʽ��˺�  ֱ��
	TShZdInvestorIDType	InvestorID;
	///��Լ���� ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///ϵͳ���  ֱ��
	TShZdOrderSysIDType	OrderSysID;
	///���ر������  ֱ��
	TShZdOrderLocalIDType	OrderLocalID;
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
	///�����۸�����   1�޼۵� 2�м۵� 3�޼�ֹ��stop to limit����4ֹ��stop to market�� ֱ��
	TShZdOrderPriceTypeType	OrderPriceType;
	///��������   1�� 2��  ֱ��
	TShZdDirectionType	Direction;
	///��Ͽ�ƽ��־
	TShZdCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TShZdCombHedgeFlagType	CombHedgeFlag;
	///�۸�  ֱ��
	TShZdPriceType	LimitPrice;
	///����  ֱ��
	TShZdVolumeType	VolumeTotalOriginal;
	///��Ч������  1=������Ч, 2=������Ч��GTC�� ֱ��
	TShZdTimeConditionType	TimeCondition;
	///ǿƽ���  ֱ��
	TShZdDateType	GTDDate;
	///�ɽ�������  1=regular 2=FOK 3=IOC
	TShZdVolumeConditionType	VolumeCondition;
	///��С�ɽ���  ����С�ڵ���ί��������Ч��=4ʱ��ShowVolume>=1С��ί����ʱ��FOK������ί����ʱ��FAK  ֱ��
	TShZdVolumeType	MinVolume;
	///��������
	TShZdContingentConditionType	ContingentCondition;
	///ֹ���  ������  ֱ��
	TShZdPriceType	StopPrice;
	///ǿƽԭ��
	TShZdForceCloseReasonType	ForceCloseReason;
	/// ����Ǳ�ɽ����ShowVolume��ֵ1��orderNumber�����Ǳ�ɽ����ShowVolume��ֵΪ0  ֱ��
	TShZdVolumeType	ShowVolume;
	///�����ͻ�������  API���û�ֻ����дC ����  P
	TShZdOrderTypeType OrderType;
	};
	*/
	void zd_connection::OnRspOrderInsertAsync(CTShZdInputOrderField* pOrder)
	{
		//
		// used only for rejects.
		//		
		// 0 - log
		if (m_debug)
		{
			//loggerv2::info("zd_connection::OnRspOrderInsert - orderRef[%s] userId[%s] RequestID[%d] errorId[%d]", pOrder->OrderRef, pOrder->UserID, pOrder->RequestID, errorId);
			loggerv2::info("zd_connection::OnRspOrderInsertAsync "
				"InstrumentID:%s,"
				"OrderSysID:%s,"
				"OrderLocalID:%s,"
				"InvestorID:%s,"
				"UserID:%s,"
				"OrderPriceType:%c,"
				"Direction:%c,"
				"LimitPrice:%f,"
				"VolumeTotalOriginal:%d,"
				"ForceCloseReason:%c,",
				pOrder->InstrumentID, 
				pOrder->OrderSysID, 
				pOrder->OrderLocalID, 
				pOrder->InvestorID, 
				pOrder->UserID, 
				pOrder->OrderPriceType, 
				pOrder->Direction, 
				pOrder->LimitPrice,
				pOrder->VolumeTotalOriginal,pOrder->ForceCloseReason);
		}
		// 1 - retrieve order
		OrderWay::type way = pOrder->Direction == TSHZD_D_Buy ? OrderWay::Buy : OrderWay::Sell;
		int orderId = get_order_id(get_user_id_ex(pOrder->OrderLocalID).c_str(), way);		
		if (orderId < 1)
		{
			orderId = FAKE_ID_MIN + atoi(pOrder->OrderLocalID);
		}
		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("zd_connection::OnRspOrderInsert - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = zd_order_aux::anchor(this, pOrder);
			if (o == NULL)
			{
				loggerv2::error("zd_connection::OnRspOrderInsertAsync cannot anchor order");
				return;
			}
			add_pending_order(o);
			break;
		default:
			break;
		}		
		if (o == NULL) // should not happen
		{
			loggerv2::error("zd_connection::OnRspOrderInsertAsync - order recovered NULL");
			return;
		}		
		// 2 - treat message
		if (pOrder->ForceCloseReason == 'E')
		{
			char szErrorMsg[32 + 1];
			snprintf(szErrorMsg, sizeof(szErrorMsg), "error(%d)",pOrder->MinVolume);

			on_nack_from_market_cb(o, szErrorMsg);
			//bug fix. should not call update_instr_on_nack_from_market_cb, because we haven't ack the order before!
			//
			//update_instr_on_nack_from_market_cb(o);
		}
		else
		{
			//loggerv2::error("zd_connection::OnRspOrderInsertAsync - order[%d] errorId[0] ???", orderId);
		}
	}		
	void zd_connection::OnRspOrderActionAsync(int* nOrdId)
	{
		if (m_debug)
			loggerv2::info("zd_connection::OnRspOrderActionAsync,order[%d]", *nOrdId);
		int ret;
		order *o = get_order_from_map(*nOrdId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("zd_connection::OnRspOrderActionAsync - message received on dead order[%d]...", *nOrdId);
			break;
		case 2:
		default:
			break;
		}
		if (o == NULL) // should not happen
		{
			loggerv2::error("zd_connection::OnRspOrderActionAsync - order recovered NULL");
			return;
		}
		//
		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("zd_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}
		//
		update_instr_on_cancel_from_market_cb(o);
		on_cancel_from_market_cb(o);		
	}

/*
///ֱ�ﱨ��
struct CTShZdOrderField
{	
///���͹�˾����  ֱ��
TShZdBrokerIDType	BrokerID;
///�ʽ��˺� ֱ��
TShZdInvestorIDType	InvestorID;
///��Լ����  ֱ��
TShZdInstrumentIDType	InstrumentID;
///������  ֱ��
TShZdOrderRefType	OrderRef;
///�û�����   ֱ��
TShZdUserIDType	UserID;
///�����۸�����   1�޼۵� 2�м۵� 3�޼�ֹ��stop to limit����4ֹ��stop to market�� ֱ��
TShZdOrderPriceTypeType	OrderPriceType;
///��Ч������  ��1=������Ч, 2=������Ч��GTC����3=OPG��4=IOC��5=FOK��6=GTD��7=ATC��8=FAK�� ֱ��
TShZdTimeConditionType	TimeCondition;
///��������  ֱ��
TShZdDirectionType	Direction;
///��Ͽ�ƽ��־  
TShZdCombOffsetFlagType	CombOffsetFlag;
///���Ͷ���ױ���־
TShZdCombHedgeFlagType	CombHedgeFlag;
///�۸�  ֱ��
TShZdPriceType	LimitPrice;
///����   ֱ��
TShZdVolumeType	VolumeTotalOriginal;	
///��С�ɽ���  ֱ��
TShZdVolumeType	MinVolume;	
///ֹ��ۡ�������  ֱ��
TShZdPriceType	StopPrice;	
///������  ֱ��
TShZdRequestIDType	RequestID;
///���ر��  ֱ��
TShZdOrderLocalIDType	OrderLocalID;
///����������   ֱ��
TShZdExchangeIDType	ExchangeID;	
///��Լ�ڽ������Ĵ���  ֱ��
TShZdExchangeInstIDType	ExchangeInstID;	
///����״̬ 1�������� 2�����Ŷ� 3�����ֳɽ� 4����ȫ�ɽ� 5���ѳ��൥ 6���ѳ��� 7��ָ��ʧ��  ֱ��
TShZdOrderSubmitStatusType	OrderSubmitStatus;
/// ����Ǳ�ɽ����ShowVolume��ֵ1��orderNumber�����Ǳ�ɽ����ShowVolume��ֵΪ0  ֱ��
TShZdVolumeType	ShowVolume;	
///������  ֱ��
TShZdDateType	TradingDay;	
///ϵͳ��  ֱ��
TShZdOrderSysIDType	OrderSysID;	
///��������  �µ������ C�ͻ��µ�  D��dealor�µ� R ��ǿƽ����أ�F������ O��3���������  ֱ��
TShZdOrderTypeType	OrderType;
///��ɽ�����  ֱ��
TShZdVolumeType	VolumeTraded;
///�ɽ��۸�  ֱ��
TShZdPriceType  PriceTraded;	
///��������  ֱ��
TShZdDateType	InsertDate;
///ί��ʱ��  ֱ��
TShZdTimeType	InsertTime;	
///�������� ֱ��
TShZdDateType  CancelDate;
///����ʱ��    ֱ��
TShZdTimeType	CancelTime;	
///�û�ǿ����־  ֱ��
TShZdBoolType	UserForceClose;	
///������  ֱ��
TShZdOrderSysIDType	RelativeOrderSysID;
};
*/
void zd_connection::OnRtnOrderAsync(CTShZdOrderField* pOrder)
	{
		if (m_debug)
		{
			loggerv2::info("zd_connection::OnRtnOrderAsync,"
				"BrokerID:%s,"
				"InvestorID:%s,"
				"InstrumentID:%s,"
				"OrderRef:%s,"
				"UserID:%s,"
				"Direction:%c,"
				"LimitPrice:%f,"
				"VolumeTotalOriginal:%d,"
				"OrderLocalID:%s,"
				"ExchangeID:%s,"
				"OrderSubmitStatus:%c,"
				"OrderSysID:%s,"
				"VolumeTraded:%d,"
				"PriceTraded:%f,"
				"InsertDate:%s,"
				"InsertTime:%s,"
				"CancelDate:%s,"
				"CancelTime:%s,"
				"TradingDay:%s,", 
				pOrder->BrokerID, 
				pOrder->InvestorID, 
				pOrder->InstrumentID, 
				pOrder->OrderRef, 
				pOrder->UserID, 
				pOrder->Direction,
				pOrder->LimitPrice,
				pOrder->VolumeTotalOriginal,
				pOrder->OrderLocalID,
				pOrder->ExchangeID,
				pOrder->OrderSubmitStatus,
				pOrder->OrderSysID,
				pOrder->VolumeTraded, 
				pOrder->PriceTraded, pOrder->InsertDate, pOrder->InsertTime, pOrder->CancelDate, pOrder->CancelTime, pOrder->TradingDay);
		}
		// 1 - retrieve order
		OrderWay::type way = pOrder->Direction == TSHZD_D_Buy ? OrderWay::Buy : OrderWay::Sell;
			
		int account, bidId, askId, portfolioId, ntradingType;
		
		get_user_info(get_user_id_ex(pOrder->OrderLocalID).c_str(), account, bidId, askId, portfolioId, ntradingType);

		int orderId = (way==OrderWay::Buy && bidId > 0) ? bidId : askId;
			
		if (orderId < 1)
		{			
			orderId = FAKE_ID_MIN + atoi(pOrder->OrderLocalID);
		}

		int ret;
		order *o = get_order_from_map(orderId, ret);
		switch (ret)
		{
		case 0:
			//o = reinterpret_cast<cffex_order*>(ord);
			break;
		case 1:
			//o = reinterpret_cast<cffex_order*>(ord);
			loggerv2::info("zd_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
			break;
		case 2:
			o = zd_order_aux::anchor(this, pOrder);
			if (o == nullptr)
			{
				loggerv2::error("zd_connection::OnRtnOrderAsync cannot anchor order");
				return;
			}
			add_pending_order(o);
			break;
		default:
			break;
		}		
		if (o == nullptr) // should not happen
		{
			loggerv2::error("zd_connection::OnRtnOrderAsync - order recovered NULL");
			return;
		}
		//
		zd_order_aux::set_order_sys_id(o, pOrder->OrderSysID);
		zd_order_aux::set_ord_ref(o, atoi(pOrder->OrderRef));
		//
		if (o->get_quantity() != pOrder->VolumeTotalOriginal)
		{
			if (m_debug)
				loggerv2::debug("zd_connection::OnRtnOrderAsync resetting order quantity to %d", pOrder->VolumeTotalOriginal);
			o->set_quantity(pOrder->VolumeTotalOriginal);
		}

		if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
		{
			if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
			{
				if (m_debug)
					loggerv2::debug("zd_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
				o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			}
		}
		/*
		/////////////////////////////////////////////////////////////////////////
		///�����ύ״̬����
		///����״̬��1��������2�����Ŷӣ�3�����ֳɽ���4����ȫ�ɽ���5���ѳ��൥��6���ѳ�����7��ָ��ʧ�ܣ�
		/////////////////////////////////////////////////////////////////////////
		///������
		#define TSHZD_OSS_InsertSubmitted '1'
		///���Ŷ�
		#define TSHZD_OSS_Accepted '2'
		///���ֳɽ�
		#define TSHZD_OSS_PartTraded '3'
		///��ȫ�ɽ�
		#define TSHZD_OSS_AllTraded '4'
		///�ѳ��൥
		#define TSHZD_OSS_CancelSub '5'
		///�ѳ���
		#define TSHZD_OSS_CancelAll '6'
		///ָ��ʧ��
		#define TSHZD_OSS_Rejected '7'

		typedef char TShZdOrderSubmitStatusType;
		*/	
		switch (pOrder->OrderSubmitStatus)
		{
		case TSHZD_OSS_InsertSubmitted:
		case TSHZD_OSS_Accepted://ok
		case TSHZD_OSS_PartTraded:
		case TSHZD_OSS_AllTraded:
			{
				string ts = pOrder->InsertDate;
#if 0
				ts.insert(10, " ");
#else
				char buffer[64];
				memset(buffer, 0, sizeof(buffer));
				if (strlen(pOrder->InsertDate) > 0)
				{
					sprintf(buffer, "%s %s", pOrder->InsertDate, pOrder->InsertTime);
				}
				ts = buffer;
#endif
				auto tp = string_to_lwtp(ts.c_str());
				o->set_last_time(tp);
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{

					update_instr_on_ack_from_market_cb(o);
					on_ack_from_market_cb(o);
				}
			}
			break;
		case TSHZD_OSS_CancelAll:
		case TSHZD_OSS_CancelSub:
			{
				on_cancel_from_market_cb(o);
			}
			break;		
		case TSHZD_OSS_Rejected:
			update_instr_on_nack_from_market_cb(o);
			on_nack_from_market_cb(o, "");
			break;
		default:
			//loggerv2::info("zd_connection::OnRtnOrderAsync didn't do with the status:%d",pOrder->OrderSubmitStatus);
			string ts = pOrder->InsertDate;
#if 0
			ts.insert(10, " ");
#else
			char buffer[64];
			memset(buffer, 0, sizeof(buffer));
			if (strlen(pOrder->InsertDate) > 0)
			{
				sprintf(buffer, "%s %s", pOrder->InsertDate, pOrder->InsertTime);
			}
			ts = buffer;
#endif
			auto tp = string_to_lwtp(ts.c_str());
			o->set_last_time(tp);
			if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
			{

				update_instr_on_ack_from_market_cb(o);
				on_ack_from_market_cb(o);
			}
			break;
		}
	}

	/*
	///ֱ��ɽ�
	struct CTShZdTradeField
	{
	///�ʽ��˺�  ֱ��
	TShZdInvestorIDType	InvestorID;
	///��Լ����  ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///�������  ֱ��
	TShZdOrderRefType	OrderRef;
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///�ɽ����   ֱ��
	TShZdTradeIDType	TradeID;
	///��������  ֱ��
	TShZdDirectionType	Direction;
	///ϵͳ���  ֱ��
	TShZdOrderSysIDType	OrderSysID;
	///��ƽ��־  ֱ��
	TShZdOffsetFlagType	OffsetFlag;
	///Ͷ���ױ���־
	TShZdHedgeFlagType	HedgeFlag;
	///�۸�  ֱ��
	TShZdPriceType	Price;
	///����  ֱ��
	TShZdVolumeType	Volume;
	///�ɽ�ʱ��  ֱ��
	TShZdDateType	TradeDate;
	///�ɽ�ʱ��   ֱ��
	TShZdTimeType	TradeTime;
	///�ɽ�����
	TShZdTradeTypeType	TradeType;
	///���ر������   ֱ��
	TShZdOrderLocalIDType	OrderLocalID;
	///���ں�Ľ�������
	TShZdDateType	ChangeTradingDay;
	///�ɽ�������
	TShZdPriceType	PriceFree;
	};
	*/
void zd_connection::OnRtnTradeAsync(CTShZdTradeField* pTrade)
{
	// 0 - log
	//loggerv2::info();
	if (m_debug)
		loggerv2::info("zd_connection::OnRtnTradeAsync,"
		"InvestorID[%*.*s] "
		"InstrumentID[%*.*s] "
		"OrderRef[%*.*s] "
		"UserID[%*.*s] "

		"OffsetFlag[%c] "
		"HedgeFlag[%c] "
		"Price[%f] "
		"Volume[%d] "
		"TradeTime[%*.*s] "
		"TradeType[%c] "
		"OrderLocalID:%s,"
		"OrderSysID:%s,"
		"Direction:%c,"
		"TradeID:%s,"
		"TradeDate:%s",

		sizeof(pTrade->InvestorID), sizeof(pTrade->InvestorID), pTrade->InvestorID,
		sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID,
		sizeof(pTrade->OrderRef), sizeof(pTrade->OrderRef), pTrade->OrderRef,
		sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID,

		pTrade->OffsetFlag,
		pTrade->HedgeFlag,
		pTrade->Price,

		pTrade->Volume,
		sizeof(pTrade->TradeTime), sizeof(pTrade->TradeTime), pTrade->TradeTime,
		pTrade->TradeType,
		pTrade->OrderLocalID, pTrade->OrderSysID, pTrade->Direction, pTrade->TradeID,pTrade->TradeDate
		);

	OrderWay::type way = pTrade->Direction == TSHZD_D_Buy ? OrderWay::Buy : OrderWay::Sell;
	bool onlyUpdatePending = false;

	int account, bidId, askId, portfolioId, ntradingType;
	get_user_info(get_user_id_ex(pTrade->OrderLocalID).c_str(), account, bidId, askId, portfolioId, ntradingType);

	int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;	
	if (orderId < 1)
	{
		orderId = FAKE_ID_MIN + atoi(pTrade->OrderLocalID);
	}
	int ret;
	order *o = get_order_from_map(orderId, ret);
	switch (ret)
	{
	case 0:
		//o = reinterpret_cast<cffex_order*>(ord);
		break;
	case 1:
		//o = reinterpret_cast<cffex_order*>(ord);
		loggerv2::info("zd_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
		break;
	case 2:
		o = zd_order_aux::anchor(this, pTrade);
		if (o == NULL)
		{
			loggerv2::error("zd_connection::OnRtnTradeAsync cannot anchor order");
			return;
		}
		add_pending_order(o);
		break;
	default:
		break;
	}
	if (o == NULL) // should not happen
	{
		loggerv2::error("zd_connection::OnRtnTradeAsync - order recovered NULL");
		return;
	}
	// 2 - treat message
	int execQty = pTrade->Volume;
	double execPrc = pTrade->Price;
	const char* pszExecRef = pTrade->TradeID;
	const char* pszTime = pTrade->TradeTime;
	exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);
	on_exec_from_market_cb(o, e);
	if (account == m_account_num)//����ر���Ӧ��account�ǶԵģ���orderID���ж�����ر��Ƿ�Ϊ��ʷ�ر�
	{
		if (orderId > m_pZdApi->m_begin_Id)//��ǰid����beginID������ر�������ʷ�ذ�
			onlyUpdatePending = false;
		else
			onlyUpdatePending = true;
	}
	else	
	{		
		string ts = pTrade->TradeDate;
#if 0
		ts.insert(10, " ");
#else
		char buffer[64];
		memset(buffer, 0, sizeof(buffer));
		if (strlen(pTrade->TradeDate) > 0)
		{
			sprintf(buffer, "%s %s", pTrade->TradeDate, pTrade->TradeTime);
		}
		ts = buffer;
#endif
		ptime t(time_from_string(ts));
		lwtp tp = ptime_to_lwtp(t);

		//int hour = get_hour_from_lwtp(tp);
		tp = tp + std::chrono::seconds(2);//����2s�����

		//if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
		//	onlyUpdatePending = true;
		//if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
		//	onlyUpdatePending = true;

		if (o->get_instrument()->get_last_sychro_timepoint() > tp)
			onlyUpdatePending = true;

		if (onlyUpdatePending)
		{
			loggerv2::info("zd_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time. tradeTime %s", pTrade->TradeTime);
		}		
	}
	update_instr_on_exec_from_market_cb(o, e, onlyUpdatePending);
}

std::string zd_connection::getMaturity(std::string& sMat)
{
	std::string newMat;
	newMat = sMat.substr(0, 4);
	newMat += "-";
	newMat += sMat.substr(4, 2);
	newMat += "-";
	newMat += sMat.substr(6, 2);
	return newMat.c_str();
}

string get_instrument_class(string code)
	{
		char buffer[32];
		memset(buffer, 0, sizeof(buffer));
		strcat(buffer, code.c_str());
		string str;
		for (int i = 0; i < strlen(buffer); i++)
		{
			if (isdigit(buffer[i]) == 0)
			{
				str.push_back(buffer[i]);
			}
			else
				break;
		}		
		transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(std::toupper));
		return str;
	}
/*
OnRspQryInstrument,InstrumentID:GC1801,ExchangeID:CME,InstrumentName:Ŧ�ڽ�1801,ExchangeInstID:1801,ProductID:GC,ProductName:Ŧ�ڽ�,ProductClass:F,bIsLast:0,
*/
void zd_connection::OnRspQryInstrument_Future(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
	{		
//#if 1
		std::string sInstr = std::string(pInstrument->InstrumentID);
		boost::trim(sInstr);			
		std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";			
		char *zErrMsg = 0;
		std::string sUnderlying = pInstrument->ProductID; //
		if (sUnderlying == "")
		{
			printf_ex("zd_connection::OnRspQryInstrument_Future sInstr��%s,sUnderlyiny:%s,instrument_class:%s\n", sInstr.c_str(), sUnderlying.c_str(), get_instrument_class(sInstr).c_str());
			sUnderlying = get_instrument_class(sInstr);
		}
		std::string sInstClass = "F_" + sUnderlying;
		std::string strExecDate = pInstrument->ExpireDate;
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = pInstrument->ExchangeID;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Futures VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future:failed to insert into database, ret is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future cmd:%s\n", sCmd.c_str());
				printf_ex("zd_connection::OnRspQryInstrument_Future cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@" + get_type();
			sCmd = "UPDATE Futures SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "',";			
			sCmd += "ConnectionCodes='" + sConnectionCodes + "',";			
			sCmd += "Underlying='" + sUnderlying + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future:failed to update the database,error is %d,cmd:%s", rc, sCmd.c_str());
				//sqlite3_free(zErrMsg);
			}
			else
			{
				loggerv2::info("zd_connection::OnRspQryInstrument_Future update to the cmd:%s\n", sCmd.c_str());
				printf_ex("zd_connection::OnRspQryInstrument_Future update to the cmd:%s,rc:%d\n", sCmd.c_str(), rc);
			}
		}
		m_database->close_databse();	
		if (bIsLast && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
//#endif
	}
/*
///ֱ���Լ
struct CTShZdInstrumentField
{
///��Լ����  ֱ��
TShZdInstrumentIDType	InstrumentID;
///����������  ֱ��
TShZdExchangeIDType	ExchangeID;
///��Լ����  ֱ��
TShZdInstrumentNameType	InstrumentName;
///��Լ�ڽ������Ĵ���  ֱ��
TShZdExchangeInstIDType	ExchangeInstID;
///����������  ֱ��
TShZdExchangeNameType ExchangeName;
///��Ʒ����  ֱ��
TShZdInstrumentIDType	ProductID;
///��Ʒ����  ֱ��
TShZdInstrumentNameType	ProductName;
///��Ʒ����  F�ڻ� O��Ȩ  ֱ��
TShZdProductClassType	ProductClass;
///��Լ���Ҵ���  ֱ��
TShZdCurrencyNoType  CurrencyNo;
///��������  ֱ��
TShZdCurrencyNameType  CurrencyName;	
///����С��Ϊ�� ֱ��
TShZdVolumeType	MarketDot;
///������׵�λ 10���� 32����  64���Ƶ� ֱ��
TShZdVolumeType	MarketUnit;
///����Сʱ��λ��  ֱ��
TShZdVolumeType	ChangeMarketDot;
///��Լ��������  ��ֵ��һ����С����ļ�ֵ�� ֱ��
TShZdPriceType	VolumeMultiple;
///������С�䶯��λ  ֱ��
TShZdPriceType	ChangeMultiple;
///��С�䶯��λ  ֱ��
TShZdPriceType	PriceTick;	
///��������  ֱ��
TShZdDateType	StartDelivDate;
///��������  ֱ��
TShZdDateType	LastUpdateDate;
///�״�֪ͨ�� ֱ��
TShZdDateType	ExpireDate;
///�������  ֱ��
TShZdDateType	EndTradeDate;	
///��ǰ�Ƿ���
TShZdBoolType	IsTrading;
///��Ȩ����
TShZdOptionTypeType	OptionType;
///��Ȩ����  ֱ��
TShZdDateType	OptionDate;
///��֤����  ֱ��
TShZdRatioType	MarginRatio;
///�̶���֤��  ֱ��
TShZdRatioType	MarginValue;
///��������  ֱ��
TShZdRatioType	FreeRatio;
///�̶�������  ֱ��
TShZdRatioType	FreeValue;
///�ֻ���Ʒ������  ֱ��
TShZdPriceType  SpotYesSetPrice;
///�ֻ���Ʒ��ֵ  ֱ��
TShZdPriceType  SpotMultiple;
///�ֻ���Ʒ��С�䶯��λ  ֱ��
TShZdPriceType	SpotTick;
///��Ȩ�ٽ�۸�  ֱ��
TShZdPriceType	OptionTickPrice;
///��Ȩ�ٽ�۸�������С����  ֱ��
TShZdPriceType	OptionTick;
///��Ȩִ�м�  ֱ��
TShZdPriceType	OptionPrice;
///��Ȩ��Ӧ�ڻ�����Ʒ���� ֱ��
TShZdInstrumentIDType OptionCommodityNo;
///��Ȩ��Ӧ�ڻ��ĺ�Լ���� ֱ��
TShZdInstrumentIDType OptionContractNo;
};
*/
void zd_connection::OnRspQryInstrument(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInstrument == nullptr)
	{
		loggerv2::error("zd_connection::OnRspQryInstrument error,nullptr");
	}
	else
	{
		if (m_debug)
		{
			loggerv2::info("zd_connection::OnRspQryInstrument,"
				"InstrumentID:%s,"
				"ExchangeID:%s,"
				"InstrumentName:%s,"
				"ExchangeInstID:%s,"
				"ProductID:%s,"
				"ProductName:%s,"
				"ProductClass:%c,"
				"ExpireDate:%s,"
				"EndTradeDate:%s,"
				"bIsLast:%d,",
				pInstrument->InstrumentID,
				pInstrument->ExchangeID,
				pInstrument->InstrumentName,
				pInstrument->ExchangeInstID,
				pInstrument->ProductID,
				pInstrument->ProductName,
				pInstrument->ProductClass,
				pInstrument->ExpireDate,
				pInstrument->EndTradeDate,
				bIsLast
				);
		}	
		/*
		/////////////////////////////////////////////////////////////////////////
		///��Ʒ��������
		/////////////////////////////////////////////////////////////////////////
		///�ڻ�
		#define TSHZD_PC_Futures 'F'
		///��Ȩ
		#define TSHZD_PC_Options 'O'
		///���
		#define TSHZD_PC_Combination 'C'
		///����
		#define TSHZD_PC_Spot 'S'
		///��ת��
		#define TSHZD_PC_EFP 'T'

		typedef char TShZdProductClassType;
		*/
		if ((pInstrument->ProductClass != TSHZD_PC_Options && pInstrument->ProductClass != TSHZD_PC_Futures) || strlen(pInstrument->ExpireDate) < 1 /*|| strlen(pInstrument->ExchangeInstID) < 10*/)
		{
			if (bIsLast && this->get_is_last() == false)
			{
				this->set_is_last(true);
			}
			return;
		}
			
		if (pInstrument->ProductClass == TSHZD_PC_Futures)
			return OnRspQryInstrument_Future(pInstrument,pRspInfo,nRequestID,bIsLast);
#if 0
		std::string sInstr = std::string(pInstrument->ExchangeInstID);
		boost::trim(sInstr);
		printf_ex("zd_connection::OnRspQryInstrument sInstr��%s,bIsLast:%d,pInstrument->ProductClass:%d\n", sInstr.c_str(), bIsLast, pInstrument->ProductClass);

		std::string sSearch = "select * from Options where Code= '" + sInstr + "'";
		//const char* data = "Callback function called";
		char *zErrMsg = 0;

		std::string sUnderlying = pInstrument->UnderlyingInstrID; //
		std::string sCP = "C";  //"CallPut"
		switch (pInstrument->OptionsType)
		{
		case THOST_FTDC_CP_CallOptions:
			sCP = "C";
			break;
		case THOST_FTDC_CP_PutOptions:
		default:
			sCP = "P";
			break;
		}

		std::string sInstClass = "O_" + get_instrument_class(sInstr);
		std::string strExecDate = pInstrument->ExpireDate;
		std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		std::string sCmd = "";
		std::string sExcge = pInstrument->ExchangeID;

		m_database->open_database();
		std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

		if (pTree->size() == 0) //tradeitem doesn't exist
		{
			sCmd = "INSERT INTO Options VALUES (";
			sCmd += "'" + sInstr + "',";
			sCmd += "'" + sExcge + "',";
			sCmd += "'" + sInstr + "',";
			sCmd += "' ',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + std::string(pInstrument->InstrumentID) + "@" + get_type() + "',";
			sCmd += "'" + sUnderlying + "',";
			sCmd += "'" + sMat + "',";
			sCmd += "'" + std::to_string(pInstrument->StrikePrice) + "',";
			sCmd += "'" + std::to_string(pInstrument->VolumeMultiple) + "',";
			sCmd += "'" + sCP + "',";
			sCmd += "'" + sInstClass + "')";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				//loggerv2::info("failed to insert into database, ret is %d",rc);
				//sqlite3_free(zErrMsg);
			}
		}
		else //exists
		{
			std::string sConnectionCodes = std::string(pInstrument->InstrumentID) + "@" + get_type();
			sCmd = "UPDATE Options SET ";
			sCmd += "Code = '" + sInstr + "',";
			sCmd += "Exchange = '" + sExcge + "',";
			sCmd += "ISIN = '" + sInstr + "',";
			sCmd += "Maturity = '" + sMat + "',";
			sCmd += "Strike = '" + std::to_string(pInstrument->StrikePrice) + "',";
			sCmd += "PointValue ='" + std::to_string(pInstrument->VolumeMultiple) + "',";
			sCmd += "FeedCodes='" + sConnectionCodes + "',";
			sCmd += "ConnectionCodes='" + sConnectionCodes + "'";
			sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

			int rc = m_database->executeNonQuery(sCmd.c_str());

			if (rc == 0)
			{
				//loggerv2::info("failed to update the database,error is %d",rc);
				//sqlite3_free(zErrMsg);
			}
		}
		m_database->close_databse();
		if (bIsLast && this->get_is_last() == false)
		{
			this->set_is_last(true);
		}
#endif
	}
}
void zd_connection::init_user_info(char * user_info_file)
{
	if (user_info_file == nullptr)
		return;
	boost::filesystem::path p;
	p.clear();
	p.append(user_info_file);
	p.append("user_info.csv");
	m_user_info_file_name = p.string();
	printf_ex("es_connection::init_user_info filename:%s\n", m_user_info_file_name.c_str());
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
		info->OrderLocalID = tokenizer[0];
		info->UserID       = tokenizer[1];
		m_user_info_map.emplace(info->OrderLocalID, info);
	}
	stream.close();
}
void zd_connection::append(user_info * info)
{
	if (info == nullptr)
		return;
	boost::filesystem::ofstream stream;
	stream.open(m_user_info_file_name.c_str(), ios::app);
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s,%s\n", info->OrderLocalID.c_str(), info->UserID.c_str());
	stream << buffer;
	stream.close();
}
void zd_connection::OnUserInfoAsync(user_info* pInfo)
{
	this->append(pInfo);
}
void zd_connection::create_user_info(string UserID,string orderLocalId)
{
	if (m_user_info_map.find(orderLocalId) == m_user_info_map.end())
	{
		user_info * info = new user_info();
		info->OrderLocalID = orderLocalId;
		info->UserID       = UserID;

		m_user_info_map.emplace(info->OrderLocalID, info);

		//to do ... append the file every day
		m_userInfoQueue.CopyPush(info);			
	}
	else
	{
		//printf_ex("warn:sl_connection::create_user_info already include the OrderNo:%s\n", pField->OrderNo);
	}
}
string zd_connection::get_user_id_ex(string orderLocalId)
{
	if (m_user_info_map.find(orderLocalId) != m_user_info_map.end())
	{
		user_info * info = m_user_info_map[orderLocalId];
		return info->UserID;
	}
	return "";
}

}

