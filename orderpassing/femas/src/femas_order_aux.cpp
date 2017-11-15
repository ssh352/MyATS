#include "femas_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "femas_connection.h"
namespace femas
{  
	/*
	///���뱨��
	struct CUstpFtdcInputOrderField
	{
	///���͹�˾���
	TUstpFtdcBrokerIDType	BrokerID;
	///����������
	TUstpFtdcExchangeIDType	ExchangeID;
	///ϵͳ�������
	TUstpFtdcOrderSysIDType	OrderSysID;
	///Ͷ���߱��
	TUstpFtdcInvestorIDType	InvestorID;
	///�û�����
	TUstpFtdcUserIDType	UserID;
	///ָ��ͨ����ϯλ����µ�
	TUstpFtdcSeatNoType	SeatNo;
	///��Լ����/������Ϻ�Լ��
	TUstpFtdcInstrumentIDType	InstrumentID;
	///�û����ر�����
	TUstpFtdcUserOrderLocalIDType	UserOrderLocalID;
	///��������
	TUstpFtdcOrderPriceTypeType	OrderPriceType;
	///��������
	TUstpFtdcDirectionType	Direction;
	///��ƽ��־
	TUstpFtdcOffsetFlagType	OffsetFlag;
	///Ͷ���ױ���־
	TUstpFtdcHedgeFlagType	HedgeFlag;
	///�۸�
	TUstpFtdcPriceType	LimitPrice;
	///����
	TUstpFtdcVolumeType	Volume;
	///��Ч������
	TUstpFtdcTimeConditionType	TimeCondition;
	///GTD����
	TUstpFtdcDateType	GTDDate;
	///�ɽ�������
	TUstpFtdcVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TUstpFtdcVolumeType	MinVolume;
	///ֹ���ֹӮ��
	TUstpFtdcPriceType	StopPrice;
	///ǿƽԭ��
	TUstpFtdcForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TUstpFtdcBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TUstpFtdcBusinessUnitType	BusinessUnit;
	///�û��Զ�����
	TUstpFtdcCustomType	UserCustom;
	///����ҵ���ʶ
	TUstpFtdcBusinessLocalIDType	BusinessLocalID;
	///ҵ��������
	TUstpFtdcDateType	ActionDay;
	///�������
	TUstpFtdcArbiTypeType	ArbiType;
	};
	*/
   order * femas_order_aux::anchor(femas_connection* pConnection, CUstpFtdcInputOrderField* pOrder)
   {
      // instrument
	  std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();
	  tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
      if (instr == nullptr)
      {
         loggerv2::error("femas_order_aux::anchor - instrument [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
		 return nullptr;
      }      
	  // client field : orderId|tradingType|portfolio
	  int nAccount     = 0;
	  int nUserOrdId   = -1;
	  int nInternalRe  = -1;
	  int nPortfolio   = 0;
	  int nTradingType = 0;
	  char *ptr = pOrder->UserCustom;
	  int id = -1;	  
	  order* o = pConnection->get_order_from_pool();
	  // way
	  if (pOrder->Direction == USTP_FTDC_D_Buy)
		  o->set_way(AtsType::OrderWay::Buy);
	  else if (pOrder->Direction == USTP_FTDC_D_Sell)
		  o->set_way(AtsType::OrderWay::Sell);
	  else
	  {
		  loggerv2::error("femas_order_aux - anchor: unknown order_way[%c]", pOrder->Direction);
		  o->set_way(AtsType::OrderWay::Undef);
	  }
	  pConnection->get_user_info(pOrder->UserCustom, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
	  id = (o->get_way() == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
	  if (id < 1)
	  {		 
		  id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderSysID);
	  }	  
	  //
	  o->set_id(id);	
	  o->set_instrument(instr);
	  o->set_last_action(AtsType::OrderAction::Created);
	  o->set_status(AtsType::OrderStatus::WaitMarket);
	  o->set_quantity(pOrder->Volume);
      o->set_price(pOrder->LimitPrice);     
	  /*
	  /////////////////////////////////////////////////////////////////////////
	  ///TFtdcUstpOffsetFlagType��һ����ƽ��־����
	  /////////////////////////////////////////////////////////////////////////
	  ///����
	  #define USTP_FTDC_OF_Open '0'
	  ///ƽ��
	  #define USTP_FTDC_OF_Close '1'
	  ///ǿƽ
	  #define USTP_FTDC_OF_ForceClose '2'
	  ///ƽ��
	  #define USTP_FTDC_OF_CloseToday '3'
	  ///ƽ��
	  #define USTP_FTDC_OF_CloseYesterday '4'
	  */
	  if (pOrder->OffsetFlag == '0')
		  o->set_open_close(AtsType::OrderOpenClose::Open);
	  else if (pOrder->OffsetFlag == '3')
		  o->set_open_close(AtsType::OrderOpenClose::CloseToday);
	  else
		  o->set_open_close(AtsType::OrderOpenClose::Close);

	  //
	  if (pOrder->TimeCondition == USTP_FTDC_TC_IOC)
	  {
		  if (pOrder->VolumeCondition == USTP_FTDC_VC_CV)
		  {
			  o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		  }
		  else
		  {
			  o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		  }
	  }
	  else
	  {
		  o->set_restriction(AtsType::OrderRestriction::None);
	  }
	  //

	  o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
	  o->set_trading_type(nTradingType);
	  	  	  
	  o->set_user_orderid(nUserOrdId);
	  
	  femas_order_aux::set_user_ord_local_id(o, pOrder->UserOrderLocalID);
	  femas_order_aux::set_order_sys_id(o, pOrder->OrderSysID);
	  
	  //auto time = microsec_clock::local_time();
	  auto time = get_lwtp_now();
	  o->set_last_time(time);

	  loggerv2::info("femas_order_aux::anchor - successfully rebuild order [%d][%x]", id, o);
      return o;
   }

   order* femas_order_aux::anchor(femas_connection* pConnection, CUstpFtdcOrderField* pOrder)
   {      	  
	   std::string sInstrCode = std::string(pOrder->InstrumentID) + "@" + pConnection->getName();
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == nullptr)
	   {
		   loggerv2::error("femas_order_aux::anchor - instrument [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
		   return nullptr;
	   }
	   // client field : orderId|tradingType|portfolio
	   int nAccount = 0;
	   int nUserOrdId = -1;
	   int nInternalRe = -1;
	   int nPortfolio = 0;
	   int nTradingType = 0;
	   char *ptr = pOrder->UserCustom;
	   int id = -1;
	   order* o = pConnection->get_order_from_pool();
	   // way
	   if (pOrder->Direction == USTP_FTDC_D_Buy)
		   o->set_way(AtsType::OrderWay::Buy);
	   else if (pOrder->Direction == USTP_FTDC_D_Sell)
		   o->set_way(AtsType::OrderWay::Sell);
	   else
	   {
		   loggerv2::error("femas_order_aux - anchor: unknown order_way[%c]", pOrder->Direction);
		   o->set_way(AtsType::OrderWay::Undef);
	   }
	   pConnection->get_user_info(pOrder->UserCustom, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
	   id = (o->get_way() == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
	   if (id < 1)
	   {		
		   id = atoi(pOrder->BrokerID) * 100000 + atoi(pOrder->OrderSysID);
	   }	 
	   //
	   o->set_id(id);
	   o->set_instrument(instr);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);
	   o->set_quantity(pOrder->Volume);
	   o->set_price(pOrder->LimitPrice);	  
	   /*
	   /////////////////////////////////////////////////////////////////////////
	   ///TFtdcUstpOffsetFlagType��һ����ƽ��־����
	   /////////////////////////////////////////////////////////////////////////
	   ///����
	   #define USTP_FTDC_OF_Open '0'
	   ///ƽ��
	   #define USTP_FTDC_OF_Close '1'
	   ///ǿƽ
	   #define USTP_FTDC_OF_ForceClose '2'
	   ///ƽ��
	   #define USTP_FTDC_OF_CloseToday '3'
	   ///ƽ��
	   #define USTP_FTDC_OF_CloseYesterday '4'
	   */
	   if (pOrder->OffsetFlag == '0')
		   o->set_open_close(AtsType::OrderOpenClose::Open);
	   else if (pOrder->OffsetFlag == '3')
		   o->set_open_close(AtsType::OrderOpenClose::CloseToday);
	   else
		   o->set_open_close(AtsType::OrderOpenClose::Close);

	   //
	   if (pOrder->TimeCondition == USTP_FTDC_TC_IOC)
	   {
		   if (pOrder->VolumeCondition == USTP_FTDC_VC_CV)
		   {
			   o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		   }
		   else
		   {
			   o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		   }
	   }
	   else
	   {
		   o->set_restriction(AtsType::OrderRestriction::None);
	   }
	   //
	 
	   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
	   o->set_trading_type(nTradingType);	   
	   o->set_user_orderid(nUserOrdId);
	   // huatai id
	   femas_order_aux::set_user_ord_local_id(o, pOrder->UserOrderLocalID);
	   femas_order_aux::set_ord_local_id(o, pOrder->OrderLocalID);
	   femas_order_aux::set_order_sys_id(o, pOrder->OrderSysID);

	   auto time = get_lwtp_now();//microsec_clock::local_time();

	   if (strlen(pOrder->InsertTime) < 1)
	   {
			o->set_last_time(time);
	   }
	   else
	   {
		   auto tp = string_to_lwtp(from_undelimited_string(pOrder->TradingDay), (pOrder->InsertTime));
		   o->set_last_time(tp);
	   }

	   loggerv2::info("femas_order_aux::anchor - successfully rebuild order [%d][%x]", id, o);
	   return o;
   }

   order* femas_order_aux::anchor(femas_connection* pConnection, CUstpFtdcTradeField* pTrade)
   {
	   std::string sInstrCode = std::string(pTrade->InstrumentID) + "@" + pConnection->getName();
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == nullptr)
	   {
		   loggerv2::error("femas_order_aux::anchor - instrument [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
		   return nullptr;
	   }
	   // client field : orderId|tradingType|portfolio
	   int nAccount     = 0;
	   int nUserOrdId   = -1;
	   int nInternalRe  = -1;
	   int nPortfolio   = 0;
	   int nTradingType = 0;
	   //UserOrderLocalID
	   string UserID = pConnection->get_user_id(pTrade->UserOrderLocalID);
	   char *ptr = (char*)UserID.c_str();
	   int id = -1;
	   order* o = pConnection->get_order_from_pool();	
	   // way
	   if (pTrade->Direction == USTP_FTDC_D_Buy)
		   o->set_way(AtsType::OrderWay::Buy);
	   else if (pTrade->Direction == USTP_FTDC_D_Sell)
		   o->set_way(AtsType::OrderWay::Sell);
	   else
	   {
		   loggerv2::error("femas_order_aux - anchor: unknown order_way[%c]", pTrade->Direction);
		   o->set_way(AtsType::OrderWay::Undef);
	   }
	   pConnection->get_user_info((char*)UserID.c_str(), nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
	   id = (o->get_way() == OrderWay::Buy && nUserOrdId > 0) ? nUserOrdId : nInternalRe;
	   if (id < 1)
	   {		
		   id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderSysID);
	   }	  
	   //
	   o->set_id(id);
	   o->set_instrument(instr);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);  	
	   o->set_price(pTrade->TradePrice);
	   /*
	   /////////////////////////////////////////////////////////////////////////
	   ///TFtdcUstpOffsetFlagType��һ����ƽ��־����
	   /////////////////////////////////////////////////////////////////////////
	   ///����
	   #define USTP_FTDC_OF_Open '0'
	   ///ƽ��
	   #define USTP_FTDC_OF_Close '1'
	   ///ǿƽ
	   #define USTP_FTDC_OF_ForceClose '2'
	   ///ƽ��
	   #define USTP_FTDC_OF_CloseToday '3'
	   ///ƽ��
	   #define USTP_FTDC_OF_CloseYesterday '4'
	   */
	   if (pTrade->OffsetFlag == '0')
		   o->set_open_close(AtsType::OrderOpenClose::Open);
	   else if (pTrade->OffsetFlag == '3')
		   o->set_open_close(AtsType::OrderOpenClose::CloseToday);
	   else
		   o->set_open_close(AtsType::OrderOpenClose::Close);

	   if (ptr[1] == ' ')
	   {
		   o->set_unknown_order();
	   }
	   else
	   {
		   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		   o->set_trading_type(nTradingType);
	   }
	   o->set_user_orderid(nUserOrdId);
	   // huatai id
	   femas_order_aux::set_ord_local_id(o, pTrade->UserOrderLocalID);
	   femas_order_aux::set_order_sys_id(o, pTrade->OrderSysID);

	   auto time = get_lwtp_now();//microsec_clock::local_time();

	   if (strlen(pTrade->TradeTime) < 1)
	   {
			o->set_last_time(time);
	   }
	   else
	   {
		   auto tp = string_to_lwtp(from_undelimited_string(pTrade->TradingDay), (pTrade->TradeTime));
		   o->set_last_time(tp);
	   }

	   loggerv2::info("femas_order_aux::anchor - successfully rebuild order [%d][%x]", id, o);
	   return o;
   }
}

