#include "femas_connection.h"
#include "string_tokenizer.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <vector>
#include "portfolio_gh.h"
#include "portfolio.h"
#include "position.h"
using namespace terra::common;
namespace femas
{
   //
   // femas_connection
   //
   femas_connection::femas_connection(bool checkSecurities) : ctpbase_connection(checkSecurities)
   {
      m_sName     = "femas_connection";      
      m_pFemasApi = new femas_api(this);     
   }
   femas_connection::~femas_connection()
   {
      delete m_pFemasApi;
   }
   bool femas_connection::init_config(const string &name, const std::string &strConfigFile)
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
	   //boost::property_tree::ptree root;
	   //boost::property_tree::ini_parser::read_ini(strConfigFile, root);
	   //m_sUsername = root.get<string>(name + ".investor_id", "");
	   //
	   return true;
   }     
   void femas_connection::request_investor_position(terra::marketaccess::orderpassing::tradeitem* i)
   {       
       CUstpFtdcQryInvestorPositionField pRequest;
       memset(&pRequest, 0, sizeof(pRequest));
       strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
	   strcpy(pRequest.InvestorID, m_sUsername.c_str());
       strcpy(pRequest.InstrumentID, i->get_trading_code());
       m_pFemasApi->ReqQryInvestorPosition(&pRequest);
	   if (m_debug)
		   loggerv2::info("femas_connection::request_investor_position requesting investor position for instrument %s", i->get_trading_code());	          
   }
   //
   void femas_connection::req_RiskDegree()
   {
	   //request_trading_account();
   }
   //
   void femas_connection::request_investor_full_positions()
   {
	   CUstpFtdcQryInvestorPositionField pRequest;
	   memset(&pRequest, 0, sizeof(pRequest));
	   strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
	   strcpy(pRequest.InvestorID, m_sUsername.c_str());
	   strcpy(pRequest.UserID, get_login_id().c_str());	   
	   m_pFemasApi->ReqQryInvestorPosition(&pRequest);
	   if (m_debug)
		   loggerv2::info("femas_connection:: calling request_investor_full_positions");
   }
   void femas_connection::request_trading_account()
   {
	   if (m_debug)
		   loggerv2::info("femas_connection:: calling request_trading_account");   
	   CUstpFtdcQryInvestorAccountField pRequest;
	   memset(&pRequest, 0, sizeof(pRequest));
	   strcpy(pRequest.BrokerID, m_sBrokerId.c_str());
	   strcpy(pRequest.InvestorID, m_sUsername.c_str());
	   m_pFemasApi->ReqQryInvestorAccount(&pRequest);	      
   } 

   void femas_connection::init_connection()
   {      
      // femas API
      m_pFemasApi->init();
	  //init_process(io_service_type::trader, 10);
#ifdef Linux
	  init_epoll_eventfd();
#else
	  init_process(io_service_type::trader, 10);
#endif
	  m_bKey_with_exchange = false;
   }

#ifdef Linux
   void  femas_connection::init_epoll_eventfd()
   {
	   efd = eventfd(0, EFD_NONBLOCK);
	   if (-1 == efd)
	   {
		   cout << "x1 efd create fail" << endl;
		   exit(1);
	   }

	   add_fd_fun_to_io_service(io_service_type::trader, efd, std::bind(&femas_connection::process, this));
	   m_pFemasApi->get_input_queue()->set_fd(efd);
	   m_pFemasApi->get_order_queue()->set_fd(efd);
	   m_pFemasApi->get_trade_queue()->set_fd(efd);
	   m_pFemasApi->get_input_action_queue()->set_fd(efd);
	   m_pFemasApi->get_user_info_queue()->set_fd(efd);
	   m_outboundQueue.set_fd(efd);
   }
#endif

   void femas_connection::release()
   {
      // femas API
	  ctpbase_connection::release();
      m_pFemasApi->release();
   }
   void femas_connection::connect()
   {
	   if (m_status == AtsType::ConnectionStatus::Disconnected)
      {
         loggerv2::info("femas_connection::connect connecting to femas...");

		 on_status_changed(AtsType::ConnectionStatus::WaitConnect);

         m_pFemasApi->connect();
      }
   }
   void femas_connection::disconnect()
   {
	   if (m_status != AtsType::ConnectionStatus::Disconnected)
      {
         if (m_pFemasApi->disconnect() == false)
         {
			 on_status_changed(AtsType::ConnectionStatus::Disconnected, "femas_connection - ReqUserLogout failed");
         }
      }
   }
   void femas_connection::process()
   {
	   m_outboundQueue.Pops_Handle_Keep(10);
	   m_pFemasApi->Process();
   }
   /*
   ///输入报单
   struct CUstpFtdcInputOrderField
   {
   ///经纪公司编号
   TUstpFtdcBrokerIDType	BrokerID;
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///系统报单编号
   TUstpFtdcOrderSysIDType	OrderSysID;
   ///投资者编号
   TUstpFtdcInvestorIDType	InvestorID;
   ///用户代码
   TUstpFtdcUserIDType	UserID;
   ///指定通过此席位序号下单
   TUstpFtdcSeatNoType	SeatNo;
   ///合约代码/套利组合合约号
   TUstpFtdcInstrumentIDType	InstrumentID;
   ///用户本地报单号
   TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
   typedef char TUstpFtdcUserOrderLocalIDType[21];
   ///报单类型
   TUstpFtdcOrderPriceTypeType	OrderPriceType;
   ///买卖方向
   TUstpFtdcDirectionType	Direction;
   ///开平标志
   TUstpFtdcOffsetFlagType	OffsetFlag;
   ///投机套保标志
   TUstpFtdcHedgeFlagType	HedgeFlag;
   ///价格
   TUstpFtdcPriceType	LimitPrice;
   ///数量
   TUstpFtdcVolumeType	Volume;
   ///有效期类型
   TUstpFtdcTimeConditionType	TimeCondition;
   ///GTD日期
   TUstpFtdcDateType	GTDDate;
   ///成交量类型
   TUstpFtdcVolumeConditionType	VolumeCondition;
   ///最小成交量
   TUstpFtdcVolumeType	MinVolume;
   ///止损价止赢价
   TUstpFtdcPriceType	StopPrice;
   ///强平原因
   TUstpFtdcForceCloseReasonType	ForceCloseReason;
   ///自动挂起标志
   TUstpFtdcBoolType	IsAutoSuspend;
   ///业务单元
   TUstpFtdcBusinessUnitType	BusinessUnit;
   ///用户自定义域
   TUstpFtdcCustomType	UserCustom;
   ///本地业务标识
   TUstpFtdcBusinessLocalIDType	BusinessLocalID;
   ///业务发生日期
   TUstpFtdcDateType	ActionDay;
   ///策略类别
   TUstpFtdcArbiTypeType	ArbiType;
   };
   */
   int femas_connection::market_create_order_async(order* o, char* pszReason)
   {       
	  CUstpFtdcInputOrderField *request = femas_create_pool.get_mem();
      memset(request, 0, sizeof(request));

	  sprintf(request->UserOrderLocalID,"%015d", m_pFemasApi->get_next_OrderRef());	  
	  femas_order_aux::set_user_ord_local_id(o,request->UserOrderLocalID);

	  //printf_ex("femas_connection::market_create_order_async id:%d,UserOrderLocalID:%s\n", o->get_id(), request->UserOrderLocalID);

	  strcpy(request->BrokerID, m_sBrokerId.c_str());
	  strcpy(request->InvestorID, m_sUsername.c_str());
	  strcpy(request->UserID, get_login_id().c_str());
	  strcpy(request->InstrumentID,o->get_instrument()->get_trading_code());
	  strcpy(request->ExchangeID, o->get_exchange_id().c_str());

	  if (o->get_way() == AtsType::OrderWay::Buy)
		  request->Direction = USTP_FTDC_D_Buy;
	  else if (o->get_way() == AtsType::OrderWay::Sell)
		  request->Direction = USTP_FTDC_D_Sell;

	  request->LimitPrice = o->get_price();
	  request->Volume = o->get_quantity();
	  //
	  request->StopPrice = request->LimitPrice;
	  //
	  request->VolumeCondition = USTP_FTDC_VC_AV;
	  //
	  if (o->get_restriction() == AtsType::OrderRestriction::None)
		  request->TimeCondition = USTP_FTDC_TC_GFD; // or GFS ???
	  else if (o->get_restriction() == AtsType::OrderRestriction::ImmediateAndCancel)//FOK:立即全部成交否则全部自动撤销
	  {
		  request->TimeCondition   = USTP_FTDC_TC_IOC;
		  request->VolumeCondition = USTP_FTDC_VC_CV;
	  }
	  else if (o->get_restriction() == AtsType::OrderRestriction::FillAndKill)//FAK:立即成交,剩余部分自动撤销
	  {
		  request->TimeCondition   = USTP_FTDC_TC_IOC;
		  request->VolumeCondition = USTP_FTDC_VC_AV;
	  }
      else
      {
         snprintf(pszReason, REASON_MAXLENGTH, "restriction %d not supported", o->get_restriction());
		 femas_create_pool.free_mem(request);
         return 0;
      }
      
      TUstpFtdcOffsetFlagType oc = USTP_FTDC_OF_Open;
	  if (o->get_open_close() == OrderOpenClose::Undef)
	  {
		  o->set_open_close(compute_open_close(o, m_bCloseToday));
	  }
      switch(o->get_open_close())
      {
	  case AtsType::OrderOpenClose::Open:
		break;          
	  case  AtsType::OrderOpenClose::Close:
		oc = USTP_FTDC_OF_Close;
		break;
	  case AtsType::OrderOpenClose::CloseToday:
		oc = USTP_FTDC_OF_CloseToday;
		break;
      default:
        break;
      }	  

      request->OffsetFlag = oc;
      if (m_debug)
		loggerv2::info("femas_connection::market_create_order CombOffsetFlag is %c", oc);	 
	  
	  if (!compute_userId(o, request->UserCustom, sizeof(request->UserCustom)))
	  {
		  femas_create_pool.free_mem(request);
		  return 0;
	  }

	  switch (o->get_price_mode())
      {
	  case AtsType::OrderPriceMode::Limit:
			{
            request->OrderPriceType = USTP_FTDC_OPT_LimitPrice;
            request->LimitPrice = o->get_price();
			}
            break;
	  case AtsType::OrderPriceMode::Market:
			{
			request->OrderPriceType = USTP_FTDC_OPT_AnyPrice;
			}
            break;
      default:
			{
            snprintf(pszReason, REASON_MAXLENGTH, "undefined price mode.");
			o->set_status(AtsType::OrderStatus::Reject);
			femas_create_pool.free_mem(request);
            return 0;
			}
      }
	  request->HedgeFlag        = USTP_FTDC_CHF_Speculation;     
	  //request->VolumeCondition  = USTP_FTDC_VC_AV;
	  request->ForceCloseReason = USTP_FTDC_FCR_NotForceClose;
	  request->MinVolume     = 1;
	  request->IsAutoSuspend = 0;
      if (!m_pFemasApi->ReqOrderInsert(request))
      {
		  snprintf(pszReason, REASON_MAXLENGTH, "femas api reject!");
		  femas_create_pool.free_mem(request);
		  return 0;
      }      
	  femas_create_pool.free_mem(request);
      return 1;
   }
   /*
   ///报单操作
   struct CUstpFtdcOrderActionField
   {
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///报单编号
   TUstpFtdcOrderSysIDType	OrderSysID;
   ///经纪公司编号
   TUstpFtdcBrokerIDType	BrokerID;
   ///投资者编号
   TUstpFtdcInvestorIDType	InvestorID;
   ///用户代码
   TUstpFtdcUserIDType	UserID;
   ///本次撤单操作的本地编号
   TUstpFtdcUserOrderLocalIDType	UserOrderActionLocalID;
   ///被撤订单的本地报单编号
   TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
   ///报单操作标志
   TUstpFtdcActionFlagType	ActionFlag;
   ///价格
   TUstpFtdcPriceType	LimitPrice;
   ///数量变化
   TUstpFtdcVolumeType	VolumeChange;
   ///本地业务标识
   TUstpFtdcBusinessLocalIDType	BusinessLocalID;
   };
   */
   int femas_connection::market_cancel_order_async(order* o, char* pszReason)
   {
      if (m_debug)
		  loggerv2::info("+++ market_cancel_order_async : %d,UserOrderLocalID:%s,OrderSysID:%s", o->get_id(), femas_order_aux::get_user_ord_local_id(o).c_str(), femas_order_aux::get_order_sys_id(o).c_str());
	        
	  CUstpFtdcOrderActionField *request = femas_cancel_pool.get_mem();
      memset(request, 0, sizeof(request));
      
	  sprintf(request->UserOrderActionLocalID, "%015d", m_pFemasApi->get_next_OrderRef());

      strcpy(request->BrokerID, m_sBrokerId.c_str());
	  strcpy(request->InvestorID, m_sUsername.c_str());
	  strcpy(request->UserID, get_login_id().c_str());
      strcpy(request->ExchangeID, o->get_exchange_id().c_str());

	  strcpy(request->UserOrderLocalID,femas_order_aux::get_user_ord_local_id(o).c_str());	  
	  strcpy(request->OrderSysID,femas_order_aux::get_order_sys_id(o).c_str());

	  //
	  request->LimitPrice = o->get_price();
	  //
	  request->ActionFlag = USTP_FTDC_AF_Delete;

	  if (!m_pFemasApi->ReqOrderAction(request))
      {
		 femas_cancel_pool.free_mem(request);
         return 0;
      }
	  femas_cancel_pool.free_mem(request);
      return 1;      
   }

   
   /*
   ///输入报单
   struct CUstpFtdcInputOrderField
   {
   ///经纪公司编号
   TUstpFtdcBrokerIDType	BrokerID;
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///系统报单编号
   TUstpFtdcOrderSysIDType	OrderSysID;
   ///投资者编号
   TUstpFtdcInvestorIDType	InvestorID;
   ///用户代码
   TUstpFtdcUserIDType	UserID;
   ///合约代码
   TUstpFtdcInstrumentIDType	InstrumentID;
   ///用户本地报单号
   TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
   ///报单类型
   TUstpFtdcOrderPriceTypeType	OrderPriceType;
   ///买卖方向
   TUstpFtdcDirectionType	Direction;
   ///开平标志
   TUstpFtdcOffsetFlagType	OffsetFlag;
   ///投机套保标志
   TUstpFtdcHedgeFlagType	HedgeFlag;
   ///价格
   TUstpFtdcPriceType	LimitPrice;
   ///数量
   TUstpFtdcVolumeType	Volume;
   ///有效期类型
   TUstpFtdcTimeConditionType	TimeCondition;
   ///GTD日期
   TUstpFtdcDateType	GTDDate;
   ///成交量类型
   TUstpFtdcVolumeConditionType	VolumeCondition;
   ///最小成交量
   TUstpFtdcVolumeType	MinVolume;
   ///止损价
   TUstpFtdcPriceType	StopPrice;
   ///强平原因
   TUstpFtdcForceCloseReasonType	ForceCloseReason;
   ///自动挂起标志
   TUstpFtdcBoolType	IsAutoSuspend;
   ///业务单元
   TUstpFtdcBusinessUnitType	BusinessUnit;
   ///用户自定义域
   TUstpFtdcCustomType	UserCustom;
   };
   */
   void femas_connection::OnRspOrderInsertAsync(CUstpFtdcInputOrderField* pOrder)
   {   
	   int errorId = pOrder->IsAutoSuspend;

      // 0 - log
      if (m_debug)
		  loggerv2::info("femas_connection::OnRspOrderInsertAsync - OrderSysID[%s] UserCustom[%s] UserOrderLocalID[%s] InstrumentID[%s] UserCustom[%s] IsAutoSuspend[%d]", pOrder->OrderSysID, pOrder->UserCustom, pOrder->UserOrderLocalID, pOrder->InstrumentID, pOrder->UserCustom, pOrder->IsAutoSuspend);
	  	
      // 1 - retrieve order
	  OrderWay::type way = pOrder->Direction == USTP_FTDC_D_Buy ? OrderWay::Buy : OrderWay::Sell;
	  int orderId = get_order_id(pOrder->UserCustom, way);
      if (orderId < 1)
      {
		  //loggerv2::warn("femas_connection::OnRspOrderInsertAsync - cannot extract orderId from UserId[%*.*s]...", sizeof(pOrder->UserOrderLocalID), sizeof(pOrder->UserOrderLocalID), pOrder->UserOrderLocalID);
         //return;
		  orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderSysID);
      }
	  int ret;
	  order *o = get_order_from_map(orderId, ret);
	  switch (ret)
	  {
	  case 0:
		  break;
	  case 1:
		  loggerv2::info("femas_connection::OnRspOrderInsert - message received on dead order[%d]...", orderId);
		  break;
	  case 2:
		  o = femas_order_aux::anchor(this, pOrder);
		  if (o == nullptr)
		  {
			  loggerv2::error("femas_connection::femas_connection cannot anchor order");
			  return;
		  }
		  add_pending_order(o);
		  break;
	  default:
		  break;
	  }
	  if (o == nullptr) // should not happen
	  {
		  loggerv2::error("femas_connection::OnRspOrderInsertAsync - order recovered nullptr");
		  return;
	  }
	  // 2 - treat message
	  if (errorId != 0)
	  {
		  char szErrorMsg[32 + 1];
		  memset(szErrorMsg, 0, sizeof(szErrorMsg));
		  snprintf(szErrorMsg, sizeof(szErrorMsg), "error %d", errorId);
		  on_nack_from_market_cb(o, szErrorMsg);		
	  }
	  else
	  {
		  loggerv2::error("femas_connection::OnRspOrderInsertAsync - order[%d] errorId[%d]",orderId,errorId);
	  }	  
   }   
   void femas_connection::OnRspOrderActionAsync(int* nOrdId)
   {
	   if (m_debug)
		   loggerv2::info("femas_connection::OnRspOrderActionAsync,orderId:%d",*nOrdId);	   	   
	   int ret;
	   order *o = get_order_from_map(*nOrdId, ret);
	   switch (ret)
	   {
	   case 0:		   
		   break;
	   case 1:		   
		   loggerv2::info("femas_connection::OnRspOrderActionAsync - message received on dead order[%d]...", *nOrdId);
		   break;
	   case 2:
	   default:
		   break;
	   }
	   if (o == nullptr) // should not happen
	   {
		   loggerv2::error("femas_connection::OnRspOrderActionAsync - order recovered nullptr");
		   return;
	   }
	   update_instr_on_ack_from_market_cb(o);
	   on_nack_from_market_cb(o, nullptr);
   }
   /*
   ///报单
   struct CUstpFtdcOrderField
   {
   ///经纪公司编号
   TUstpFtdcBrokerIDType	BrokerID;
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///系统报单编号
   TUstpFtdcOrderSysIDType	OrderSysID;
   ///投资者编号
   TUstpFtdcInvestorIDType	InvestorID;
   ///用户代码
   TUstpFtdcUserIDType	UserID;
   ///合约代码
   TUstpFtdcInstrumentIDType	InstrumentID;
   ///用户本地报单号
   TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
   ///报单类型
   TUstpFtdcOrderPriceTypeType	OrderPriceType;
   ///买卖方向
   TUstpFtdcDirectionType	Direction;
   ///开平标志
   TUstpFtdcOffsetFlagType	OffsetFlag;
   ///投机套保标志
   TUstpFtdcHedgeFlagType	HedgeFlag;
   ///价格
   TUstpFtdcPriceType	LimitPrice;
   ///数量
   TUstpFtdcVolumeType	Volume;
   ///有效期类型
   TUstpFtdcTimeConditionType	TimeCondition;
   ///GTD日期
   TUstpFtdcDateType	GTDDate;
   ///成交量类型
   TUstpFtdcVolumeConditionType	VolumeCondition;
   ///最小成交量
   TUstpFtdcVolumeType	MinVolume;
   ///止损价
   TUstpFtdcPriceType	StopPrice;
   ///强平原因
   TUstpFtdcForceCloseReasonType	ForceCloseReason;
   ///自动挂起标志
   TUstpFtdcBoolType	IsAutoSuspend;
   ///业务单元
   TUstpFtdcBusinessUnitType	BusinessUnit;
   ///用户自定义域
   TUstpFtdcCustomType	UserCustom;
   ///交易日
   TUstpFtdcTradingDayType	TradingDay;
   ///会员编号
   TUstpFtdcParticipantIDType	ParticipantID;
   ///客户号
   TUstpFtdcClientIDType	ClientID;
   ///下单席位号
   TUstpFtdcSeatIDType	SeatID;
   ///插入时间
   TUstpFtdcTimeType	InsertTime;
   ///本地报单编号
   TUstpFtdcOrderLocalIDType	OrderLocalID;
   ///报单来源
   TUstpFtdcOrderSourceType	OrderSource;
   ///报单状态
   TUstpFtdcOrderStatusType	OrderStatus;
   ///撤销时间
   TUstpFtdcTimeType	CancelTime;
   ///撤单用户编号
   TUstpFtdcUserIDType	CancelUserID;
   ///今成交数量
   TUstpFtdcVolumeType	VolumeTraded;
   ///剩余数量
   TUstpFtdcVolumeType	VolumeRemain;
   };
   */
   void femas_connection::OnRtnOrderAsync(CUstpFtdcOrderField* pOrder)
   {
      // 0 - log
	   if (m_debug)
		   loggerv2::info("femas_connection::OnRtnOrderAsync - "
		   "BrokerID[%*.*s] "
		   "InvestorID[%*.*s] "
		   "UserID[%*.*s] "

		   "InstrumentID[%*.*s] "
		   "UserOrderLocalID[%*.*s] "
		   "OrderSysID[%*.*s] "
		   "OrderLocalID[%*.*s] "

		   "OrderPriceType[%c] "
		   "Direction[%c] "
		   "LimitPrice[%f] "
		   "Volume[%d] "
		   "TimeCondition[%c] "
		   "GTDDate[%*.*s] "
		   "VolumeCondition[%c] "
		   "MinVolume[%d] "
		   "StopPrice[%f] "
		   "ForceCloseReason[%c] "
		   "IsAutoSuspend[%d] "
		   "BusinessUnit[%*.*s] "
		   "ExchangeID[%*.*s] "
		   "ParticipantID[%*.*s] "
		   "ClientID[%*.*s] "
		   "TradingDay[%*.*s] "
		   "OrderSource[%c] "
		   "OrderStatus[%c] "
		   "VolumeTraded[%d] "
		   "InsertTime[%*.*s] "
		   "CancelTime[%*.*s] "
		   "UserCustom[%s] "
		   "OffsetFlag[%c] ",

		   sizeof(pOrder->BrokerID), sizeof(pOrder->BrokerID), pOrder->BrokerID,
		   sizeof(pOrder->InvestorID), sizeof(pOrder->InvestorID), pOrder->InvestorID,
		   sizeof(pOrder->UserID), sizeof(pOrder->UserID), pOrder->UserID,
		   
		   sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID,
		   sizeof(pOrder->UserOrderLocalID), sizeof(pOrder->UserOrderLocalID), pOrder->UserOrderLocalID,
		   sizeof(pOrder->OrderSysID), sizeof(pOrder->OrderSysID), pOrder->OrderSysID,
		   sizeof(pOrder->OrderLocalID), sizeof(pOrder->OrderLocalID), pOrder->OrderLocalID,
		   
		   pOrder->OrderPriceType,
		   pOrder->Direction,
		   pOrder->LimitPrice,
		   pOrder->Volume,
		   pOrder->TimeCondition,
		   sizeof(pOrder->GTDDate), sizeof(pOrder->GTDDate), pOrder->GTDDate,
		   pOrder->VolumeCondition,
		   pOrder->MinVolume,
		   pOrder->StopPrice,
		   pOrder->ForceCloseReason,
		   pOrder->IsAutoSuspend,
		   sizeof(pOrder->BusinessUnit), sizeof(pOrder->BusinessUnit), pOrder->BusinessUnit,

		   sizeof(pOrder->ExchangeID), sizeof(pOrder->ExchangeID), pOrder->ExchangeID,
		   sizeof(pOrder->ParticipantID), sizeof(pOrder->ParticipantID), pOrder->ParticipantID,
		   sizeof(pOrder->ClientID), sizeof(pOrder->ClientID), pOrder->ClientID,
		   sizeof(pOrder->TradingDay), sizeof(pOrder->TradingDay), pOrder->TradingDay,
		   pOrder->OrderSource,
		   pOrder->OrderStatus,
		   pOrder->VolumeTraded,
		   sizeof(pOrder->InsertTime), sizeof(pOrder->InsertTime), pOrder->InsertTime,
		   sizeof(pOrder->CancelTime), sizeof(pOrder->CancelTime), pOrder->CancelTime,
		   pOrder->UserCustom,
		   pOrder->OffsetFlag
		   );	      
	   
	  OrderWay::type way = pOrder->Direction == USTP_FTDC_D_Buy ? OrderWay::Buy : OrderWay::Sell;
	  int account, bidId, askId, portfolioId, ntradingType;
	  get_user_info(pOrder->UserCustom, account, bidId, askId, portfolioId, ntradingType);
	  int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;	  
      if (orderId < 1)
      {		
		  orderId = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderSysID);
      }	  	  
	  int ret;
	  order *o = get_order_from_map(orderId, ret);
	  switch (ret)
	  {
	  case 0:		  
		  break;
	  case 1:		  
		  loggerv2::info("femas_connection::OnRtnOrderAsync - message received on dead order[%d]...", orderId);
		  break;
	  case 2:
		  o = femas_order_aux::anchor(this, pOrder);
		  if (o == nullptr)
		  {
			  loggerv2::error("femas_connection::OnRtnOrderAsync cannot anchor order");
			  return;
		  }
		  add_pending_order(o);
		  break;
	  default:
		  break;
	  }
	  if (o == nullptr) // should not happen
	  {
		  loggerv2::error("femas_connection::OnRtnOrderAsync - order recovered NULL");
		  return;
	  }
      // 2 - treat message      
	  femas_order_aux::set_user_ord_local_id(o, pOrder->UserOrderLocalID);
	  femas_order_aux::set_ord_local_id(o, pOrder->OrderLocalID);
	  femas_order_aux::set_order_sys_id(o, pOrder->OrderSysID);

      if (o->get_quantity() != pOrder->Volume)
      {
          if (m_debug)
              loggerv2::debug("femas_connection::OnRtnOrderAsync resetting order quantity to %d", pOrder->Volume);
          o->set_quantity(pOrder->Volume);
      }
	  if (o->get_status() != OrderStatus::Exec && o->get_status() != OrderStatus::Cancel)
	  {
		  if (o->get_book_quantity() != o->get_quantity() - o->get_exec_quantity())
		  {
			  if (m_debug)
				  loggerv2::debug("femas_connection::OnRtnOrderAsync resetting order book quantity to %d", o->get_quantity() - o->get_exec_quantity());
			  o->set_book_quantity(o->get_quantity() - o->get_exec_quantity());
			  o->set_price(pOrder->LimitPrice);
		  }
	  }
	  /*
	  /////////////////////////////////////////////////////////////////////////
	  ///TFtdcUstpOrderStatusType是一个报单状态类型
	  /////////////////////////////////////////////////////////////////////////
	  ///全部成交
	  #define USTP_FTDC_OS_AllTraded '0'
	  ///部分成交还在队列中
	  #define USTP_FTDC_OS_PartTradedQueueing '1'
	  ///部分成交不在队列中
	  #define USTP_FTDC_OS_PartTradedNotQueueing '2'
	  ///未成交还在队列中
	  #define USTP_FTDC_OS_NoTradeQueueing '3'
	  ///未成交不在队列中
	  #define USTP_FTDC_OS_NoTradeNotQueueing '4'
	  ///撤单
	  #define USTP_FTDC_OS_Canceled '5'
	  ///订单已报入交易所未应答
	  #define USTP_FTDC_OS_AcceptedNoReply '6'
	  */
	  switch (pOrder->OrderStatus)
	  {
	  case USTP_FTDC_OS_AllTraded:
	  case USTP_FTDC_OS_AcceptedNoReply:      ///订单已报入交易所未应答
	  case USTP_FTDC_OS_PartTradedNotQueueing:///部分成交不在队列中
	  case USTP_FTDC_OS_PartTradedQueueing:   ///部分成交还在队列中
	  case USTP_FTDC_OS_NoTradeNotQueueing:   ///未成交不在队列中			
	  case USTP_FTDC_OS_NoTradeQueueing:      ///未成交还在队列中
			{
				if (o->get_status() == AtsType::OrderStatus::WaitMarket || o->get_status() == AtsType::OrderStatus::WaitServer)
				{
					update_instr_on_ack_from_market_cb(o);
					on_ack_from_market_cb(o);
				}
				break;
			}		  		
	  case USTP_FTDC_OS_Canceled:
			{			
				update_instr_on_cancel_from_market_cb(o);
				on_cancel_from_market_cb(o);
				break;
			}
	  default:
			{	
				loggerv2::info("femas_connection::OnRtnOrderAsync didn't do with the state:%c", pOrder->OrderStatus);
				break;
			}	  
	  }
   }
   /*
   ///成交
   struct CUstpFtdcTradeField
   {
   ///经纪公司编号
   TUstpFtdcBrokerIDType	BrokerID;
   ///交易所代码
   TUstpFtdcExchangeIDType	ExchangeID;
   ///交易日
   TUstpFtdcTradingDayType	TradingDay;
   ///会员编号
   TUstpFtdcParticipantIDType	ParticipantID;
   ///下单席位号
   TUstpFtdcSeatIDType	SeatID;
   ///投资者编号
   TUstpFtdcInvestorIDType	InvestorID;
   ///客户号
   TUstpFtdcClientIDType	ClientID;
   ///用户编号
   TUstpFtdcUserIDType	UserID;
   ///下单用户编号
   TUstpFtdcUserIDType	OrderUserID;
   ///成交编号
   TUstpFtdcTradeIDType	TradeID;
   ///报单编号
   TUstpFtdcOrderSysIDType	OrderSysID;
   ///本地报单编号
   TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
   ///合约代码
   TUstpFtdcInstrumentIDType	InstrumentID;
   ///买卖方向
   TUstpFtdcDirectionType	Direction;
   ///开平标志
   TUstpFtdcOffsetFlagType	OffsetFlag;
   ///投机套保标志
   TUstpFtdcHedgeFlagType	HedgeFlag;
   ///成交价格
   TUstpFtdcPriceType	TradePrice;
   ///成交数量
   TUstpFtdcVolumeType	TradeVolume;
   ///成交时间
   TUstpFtdcTimeType	TradeTime;
   ///清算会员编号
   TUstpFtdcParticipantIDType	ClearingPartID;
   ///本次成交手续费
   TUstpFtdcMoneyType	UsedFee;
   ///本次成交占用保证金
   TUstpFtdcMoneyType	UsedMargin;
   ///本次成交占用权利金
   TUstpFtdcMoneyType	Premium;
   ///持仓表今持仓量
   TUstpFtdcVolumeType	Position;
   ///持仓表今日持仓成本
   TUstpFtdcPriceType	PositionCost;
   ///资金表可用资金
   TUstpFtdcMoneyType	Available;
   ///资金表占用保证金
   TUstpFtdcMoneyType	Margin;
   ///资金表冻结的保证金
   TUstpFtdcMoneyType	FrozenMargin;
   ///本地业务标识
   TUstpFtdcBusinessLocalIDType	BusinessLocalID;
   ///业务发生日期
   TUstpFtdcDateType	ActionDay;
   ///策略类别
   TUstpFtdcArbiTypeType	ArbiType;
   ///合约代码
   TUstpFtdcInstrumentIDType	ArbiInstrumentID;
   };
   */
   void femas_connection::OnRtnTradeAsync(CUstpFtdcTradeField* pTrade)
   {
      // 0 - log
	   if (m_debug)
	   {
		   loggerv2::info("femas_connection::OnRtnTradeAsync "
			   "TradeID:%s,"
			   "OrderSysID:%s,"
			   "UserOrderLocalID:%s,"
			   "InstrumentID:%s,"
			   "Direction:%c,"
			   "OffsetFlag:%c,"
			   "TradePrice:%f,"
			   "TradeVolume:%d,"
			   "TradeTime:%s,"
			   "TradingDay:%s",

			   pTrade->TradeID,
			   pTrade->OrderSysID,
			   pTrade->UserOrderLocalID,
			   pTrade->InstrumentID,
			   pTrade->Direction,
			   pTrade->OffsetFlag,
			   pTrade->TradePrice,
			   pTrade->TradeVolume,
			   pTrade->TradeTime,
			   pTrade->TradingDay
			   );
	   }
	  // 1 - retrieve order
	  OrderWay::type way = pTrade->Direction == USTP_FTDC_D_Buy ? OrderWay::Buy : OrderWay::Sell;
	  int account, bidId, askId, portfolioId, ntradingType;
	  //UserOrderLocalID
	  string UserID = m_pFemasApi->get_user_id(pTrade->UserOrderLocalID);
	  get_user_info(UserID.c_str(), account, bidId, askId, portfolioId, ntradingType);
	  int orderId = (way == OrderWay::Buy && bidId > 0) ? bidId : askId;            	  
      if (orderId < 1)
      {
		  orderId = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderSysID);
      }          
	  int ret;
	  order *o = get_order_from_map(orderId, ret);
	  switch (ret)
	  {
	  case 0:		  
		  break;
	  case 1:		  
		  loggerv2::info("femas_connection::OnRtnTradeAsync - message received on dead order[%d]...", orderId);
		  break;
	  case 2:
		  o = femas_order_aux::anchor(this, pTrade);
		  if (o == nullptr)
		  {
			  loggerv2::error("femas_connection::OnRtnTradeAsync cannot anchor order");
			  return;
		  }
		  add_pending_order(o);
		  break;
	  default:
		  break;
	  }
	  if (o == nullptr) // should not happen
	  {
		  loggerv2::error("femas_connection::OnRtnTradeAsync - order recovered nullptr");
		  return;
	  }
      // 2 - treat message
	  int execQty = pTrade->TradeVolume;
      double execPrc = pTrade->TradePrice;
	  const char* pszExecRef = pTrade->TradeID;
	  const char* pszTime = pTrade->TradeTime;	  
	  exec* e = new exec(o, pszExecRef, execQty, execPrc, pszTime);
	  on_exec_from_market_cb(o, e);


	  lwtp tp = string_to_lwtp(from_undelimited_string(pTrade->TradingDay), pTrade->TradeTime);
	  int hour = get_hour_from_lwtp(tp);
	  tp = tp + std::chrono::seconds(2);//允许2s的误差

	  bool onlyUpdatePending = false;
	  if (account == m_account_num)//这个回报对应的account是对的，按orderID来判断这个回报是否为历史回报
	  {
		  if (orderId > m_pFemasApi->get_begin_id())//当前id大于beginID，这个回报不是历史回包
			  onlyUpdatePending = false;
		  else
			  onlyUpdatePending = true;
		  loggerv2::info("femas_connection::OnRtnTradeAsync onlyUpdatePending:%d,orderId:%d,beginid:%d", onlyUpdatePending, orderId, m_pFemasApi->get_begin_id());
	  }
	  else
	  {
		  if (m_bTsession && (o->get_instrument()->get_last_sychro_timepoint() > tp || hour < 9 || hour>16))
			  onlyUpdatePending = true;
		  if (!m_bTsession && o->get_instrument()->get_last_sychro_timepoint() > tp)
			  onlyUpdatePending = true;
		  if (onlyUpdatePending)
		  {
			  loggerv2::info("femas_connection::OnRtnTradeAsync will only update tradeitem pending close quantity because the trade time is older than tradeitem resychro time. tradeTime %s", lwtp_to_simple_time_string(tp).c_str());
		  }
	  }	
	  update_instr_on_exec_from_market_cb(o,e,onlyUpdatePending);  
   }   
   std::string femas_connection::getMaturity(std::string& sMat)
   {
	   std::string newMat;
	   newMat = sMat.substr(0, 4);
	   newMat += "-";
	   newMat += sMat.substr(4, 2);
	   newMat += "-";
	   newMat += sMat.substr(6, 2);
	   return newMat.c_str();
   }
   void femas_connection::OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
   {
	   if (pRspInstrument != nullptr)
	   {
		   loggerv2::info("femas_api::OnRspQryInstrument "
			   "ExchangeID：%s,"
			   "ProductID：%s,"
			   "ProductName:%s,"
			   "InstrumentID：%s,"
			   "InstrumentName:%s"
			   "bIsLast:%d",
			   pRspInstrument->ExchangeID,
			   pRspInstrument->ProductID,
			   pRspInstrument->ProductName,
			   pRspInstrument->InstrumentID,
			   pRspInstrument->InstrumentName,
			   bIsLast);

		   m_database->open_database();

		   string sInstr = pRspInstrument->InstrumentID;
		   std::string sUnderlying = pRspInstrument->UnderlyingInstrID; //
		   std::string strExecDate = pRspInstrument->ExpireDate;
		   std::string sMat = getMaturity(strExecDate); //pInstrument->ExpireDate;
		   std::string sCmd = "";
		   std::string sExcge = pRspInstrument->ExchangeID;
		   if (pRspInstrument->OptionsType != USTP_FTDC_OT_NotOptions)
		   {
			   std::string sSearch = "select * from Options where Code= '" + sInstr + "'";
			   //char *zErrMsg = 0;			   
			   std::string sCP = "C";  //"CallPut"
			   switch (pRspInstrument->OptionsType)
			   {
			   case USTP_FTDC_OT_CallOptions:
				   sCP = "C";
				   break;
			   case USTP_FTDC_OT_PutOptions:
			   default:
				   sCP = "P";
				   break;
			   }
			   std::string sInstClass = "O_" + string(pRspInstrument->ProductID);

			   std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

			   if (pTree->size() == 0) //tradeitem doesn't exist
			   {
				   sCmd = "INSERT INTO Options VALUES (";
				   sCmd += "'" + sInstr + "',";
				   sCmd += "'" + sExcge + "',";
				   sCmd += "'" + sInstr + "',";
				   sCmd += "' ',";
				   sCmd += "'" + std::string(pRspInstrument->InstrumentID) + "@" + get_type() + "',";
				   sCmd += "'" + std::string(pRspInstrument->InstrumentID) + "@" + get_type() + "',";
				   sCmd += "'" + sUnderlying + "',";
				   sCmd += "'" + sMat + "',";
				   sCmd += "'" + std::to_string(pRspInstrument->StrikePrice) + "',";
				   sCmd += "'" + std::to_string(pRspInstrument->VolumeMultiple) + "',";
				   sCmd += "'" + sCP + "',";
				   sCmd += "'" + sInstClass + "')";

				   int rc = m_database->executeNonQuery(sCmd.c_str());

				   if (rc == 0)
				   {
					   //loggerv2::info("failed to insert into database, ret is %d",rc);
				   }
			   }
			   else //exists
			   {
				   std::string sConnectionCodes = std::string(pRspInstrument->InstrumentID) + "@" + get_type();
				   sCmd = "UPDATE Options SET ";
				   sCmd += "Code = '" + sInstr + "',";
				   sCmd += "Exchange = '" + sExcge + "',";
				   sCmd += "ISIN = '" + sInstr + "',";
				   sCmd += "Maturity = '" + sMat + "',";
				   sCmd += "Strike = '" + std::to_string(pRspInstrument->StrikePrice) + "',";
				   sCmd += "PointValue ='" + std::to_string(pRspInstrument->VolumeMultiple) + "',";
				   sCmd += "FeedCodes='" + sConnectionCodes + "',";
				   sCmd += "ConnectionCodes='" + sConnectionCodes + "'";
				   sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

				   int rc = m_database->executeNonQuery(sCmd.c_str());

				   if (rc == 0)
				   {
					   //loggerv2::info("failed to update the database,error is %d",rc);					
				   }
			   }
		   }
		   else
		   {
			   std::string sSearch = "select * from Futures where Code= '" + sInstr + "'";
			   std::string sInstClass = "F_" + string(pRspInstrument->ProductID);

			   std::vector<boost::property_tree::ptree>* pTree = m_database->get_table(sSearch.c_str());

			   if (pTree->size() == 0) //tradeitem doesn't exist
			   {
				   sCmd = "INSERT INTO Futures VALUES (";
				   sCmd += "'" + sInstr + "',";
				   sCmd += "'" + sExcge + "',";
				   sCmd += "'" + sInstr + "',";
				   sCmd += "' ',";
				   sCmd += "'" + sInstr + "@" + get_type() + "|" + sInstr + "@CTP" + "',";
				   sCmd += "'" + sInstr + "@" + get_type() + "|" + sInstr + "@CTP" + "',";
				   sCmd += "'" + sUnderlying + "',";
				   sCmd += "'" + sMat + "',";
				   sCmd += "'" + sInstClass + "')";

				   int rc = m_database->executeNonQuery(sCmd.c_str());

				   if (rc == 0)
				   {
					   loggerv2::info("femas_connection::OnRspQryInstrument:failed to insert into database, ret is %d,cmd:%s", rc, sCmd.c_str());
				   }
				   else
				   {
					   loggerv2::info("femas_connection::OnRspQryInstrument cmd:%s\n", sCmd.c_str());
				   }
			   }
			   else //exists
			   {
				   std::string sConnectionCodes = sInstr + "@" + get_type();
				   sCmd = "UPDATE Futures SET ";
				   sCmd += "Code = '" + sInstr + "',";
				   sCmd += "Exchange = '" + sExcge + "',";
				   sCmd += "ISIN = '" + sInstr + "',";
				   sCmd += "Maturity = '" + sMat + "',";
				   sCmd += "FeedCodes='" + sConnectionCodes + "|" + sInstr + "@CTP" + "',";
				   sCmd += "ConnectionCodes='" + sConnectionCodes + "|" + sInstr + "@CTP" + "',";
				   sCmd += "Underlying='" + sUnderlying + "'";
				   sCmd += " where ConnectionCodes like '" + sConnectionCodes + "%';";

				   int rc = m_database->executeNonQuery(sCmd.c_str());

				   if (rc == 0)
				   {
					   loggerv2::info("femas_connection::OnRspQryInstrument:failed to update the database,error is %d,cmd:%s", rc, sCmd.c_str());
				   }
				   else
				   {
					   loggerv2::info("femas_connection::OnRspQryInstrument update to the cmd:%s\n", sCmd.c_str());
				   }
			   }
		   }	   
		   m_database->close_databse();
		   if (bIsLast && get_is_last() == false)
		   {
			   set_is_last(true);
		   }
	   }
   }
}

