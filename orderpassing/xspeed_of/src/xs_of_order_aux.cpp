#include "xs_of_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "xs_of_connection.h"

namespace xs_of
{
   //xs_of_order::xs_of_order(xs_of_connection* pConnection) : order(pConnection)
   //{
	  // m_nSpdId = 0;
   //}

	order* xs_of_order_aux::anchor(xs_of_connection* pConnection, DFITCOrderRtnField* pOrder)
   {
	   order* o = pConnection->get_order_from_pool();
	   /*DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
	   DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
	   DFITCOrderSysIDType                 OrderSysID;                   //�������
	   DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
	   DFITCSessionIDType                  sessionID;                    //�ỰID
	   DFITCDateType                       SuspendTime;                  //����ʱ��
	   DFITCInstrumentIDType               instrumentID;                 //��Լ����
	   DFITCExchangeIDType                 exchangeID;                   //������
	   DFITCBuySellTypeType                buySellType;                  //����
	   DFITCOpenCloseTypeType              openCloseType;                //��ƽ
	   DFITCInstrumentTypeType             instrumentType;               //��Լ����
	   DFITCSpeculatorType                 speculator;                   //Ͷ�����
	   DFITCPriceType                      insertPrice;                  //ί�м�
	   DFITCPriceType                      profitLossPrice;              //ֹӯֹ��۸�
	   DFITCAccountIDType                  accountID;                    //�ʽ��˺�
	   DFITCAmountType                     cancelAmount;                 //��������
	   DFITCAmountType                     orderAmount;                  //ί������
	   DFITCInsertType                     insertType;                   //�Զ������
	   DFITCOrderTypeType                  orderType;                    //��������
	   DFITCSPDOrderIDType                 extSpdOrderID;                //�㷨�����
	   DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
	   DFITCCustomCategoryType             customCategory;               //�Զ������
	   DFITCOrderPropertyType              orderProperty;                //��������
	   DFITCAmountType                     minMatchAmount;               //��С�ɽ���
	   DFITCClientIDType                   clientID;                     //���ױ���
	   DFITCErrorMsgInfoType               statusMsg;                    //״̬��Ϣ*/

	   //std::string sInstrCode = std::string(pOrder->instrumentID) + "." + pOrder->exchangeID + "@" + pConnection->getName();
	   std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->instrumentID), std::string(pOrder->exchangeID));

	   // tradeitem
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == nullptr)
	   {
		   loggerv2::error("xs_of_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->instrumentID), sizeof(pOrder->instrumentID), pOrder->instrumentID);
		   return nullptr;
	   }

	   int nAccount = 0;
	   int nUserOrdId = -1;
	   int nInternalRe = -1;
	   int nPortfolio = 0;
	   int nTradingType = 0;
	   int localID = pOrder->localOrderID;

	   // client field : orderId|tradingType|portfolio
	   //����ʱ���������͵�ack������devDecInfoΪ��
	   //int id = pOrder->localOrderID;//ע�⣬���Ǹ�����

	   // client field : orderId|tradingType|portfolio
	   //string_tokenizer<1024> tokenizer;
	   //tokenizer.break_line(pOrder->customCategory, '-');
	   char *ptr = pOrder->customCategory;
	   int id = -1;
	   //if (tokenizer.size() !=2 )
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   /*loggerv2::error("xs_of_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->customCategory), sizeof(pOrder->customCategory), pOrder->customCategory);
		   return nullptr;*/
		   id = -1 * localID;
	   }
	   else
	   {
		   pConnection->get_user_info(pOrder->customCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		   id = nInternalRe;
	   }



	   // ok
	   //xs_of_order* o = new xs_of_order(pConnection);
	   o->set_id(id);

	   o->set_instrument(instr);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);

	   o->set_quantity(pOrder->orderAmount);

	   //o->set_price(pOrder->LimitPrice);
	   o->set_price(pOrder->insertPrice);

	   // way & open close

	   AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
	   switch (pOrder->openCloseType)
	   {
	   case DFITC_SPD_CLOSE:
	   case DFITC_SPD_FORCECLOSE:
		   oc = AtsType::OrderOpenClose::Close; 
		   break;
	   case DFITC_SPD_FORCECLOSETODAY:
	   case DFITC_SPD_CLOSETODAY:
		   oc = AtsType::OrderOpenClose::CloseToday;
		   break;
	   case DFITC_SPD_OPEN:
	   default:
		   oc = AtsType::OrderOpenClose::Open; 
		   break;
	   }
	   o->set_open_close(oc);

	   //
	   if (pOrder->orderProperty == DFITC_SP_FOK)
	   {
		   o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
	   }
	   else if (pOrder->orderProperty == DFITC_SP_FAK)
	   {
		   o->set_restriction(AtsType::OrderRestriction::FillAndKill);
	   }
	   else
	   {
		   o->set_restriction(AtsType::OrderRestriction::None);
	   }
	   //

	   switch (pOrder->buySellType)
	   {
	   case DFITC_SPD_BUY:
		   o->set_way(AtsType::OrderWay::Buy); 
		   break;
	   case DFITC_SPD_SELL:
		   o->set_way(AtsType::OrderWay::Sell); 
		   break;
	   }
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   o->set_unknown_order();

	   }
	   else
	   {
		   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		   o->set_trading_type(nTradingType);
	   }
	  //o->set_account(pConnection->getAccountName(nAccount).c_str());
	  o->set_user_orderid(nUserOrdId);
	  xs_of_order_aux::set_spdId(o,pOrder->spdOrderID);
	  //lts order info
	  //date_time ordTime = time(NULL);
	  auto time = get_lwtp_now();
	  o->set_last_time(time);

	  o->set_rebuild_time(time);

	/*  std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
	  o->set_rebuild_time_point(tpnow);*/

	  //loggerv2::info("xs_of_order::anchor - successfully rebuild order [%d][%x]", id, o);
	  o->set_status(AtsType::OrderStatus::Ack);
	  xs_of_order_aux::set_locId(o,abs(localID));
	  return o;

   }

   order* xs_of_order_aux::anchor(xs_of_connection* pConnection, DFITCMatchRtnField* pTrade)
   {
	   order* o = pConnection->get_order_from_pool();

	   /*DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
	   DFITCOrderSysIDType                 OrderSysID;                   //�������(�������������)
	   DFITCMatchIDType                    matchID;                      //�ɽ����
	   DFITCInstrumentIDType               instrumentID;                 //��Լ����
	   DFITCBuySellTypeType                buySellType;                  //����
	   DFITCOpenCloseTypeType              openCloseType;                //��ƽ��־
	   DFITCPriceType                      matchedPrice;                 //�ɽ��۸�
	   DFITCAmountType                     orderAmount;                  //ί������
	   DFITCAmountType                     matchedAmount;                //�ɽ�����
	   DFITCDateType                       matchedTime;                  //�ɽ�ʱ��
	   DFITCPriceType                      insertPrice;                  //����
	   DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
	   DFITCMatchType                      matchType;                    //�ɽ�����
	   DFITCSpeculatorType                 speculator;                   //Ͷ��
	   DFITCExchangeIDType                 exchangeID;                   //������ID
	   DFITCFeeType                        fee;                          //������
	   DFITCSessionIDType                  sessionID;                    //�Ự��ʶ
	   DFITCInstrumentTypeType             instrumentType;               //��Լ����
	   DFITCAccountIDType                  accountID;                    //�ʽ��˺�
	   DFITCOrderAnswerStatusType          orderStatus;                  //�걨���
	   DFITCPriceType                      margin;                       //����Ϊ��֤��,ƽ��Ϊ�ⶳ��֤��
	   DFITCPriceType                      frozenCapita;                 //�ɽ��ⶳί�ж�����ʽ�
	   DFITCAdjustmentInfoType             adjustmentInfo;               //��ϻ�����ı�֤�������Ϣ,��ʽ:[��Լ����,������־,Ͷ�����,�������;] 
	   DFITCCustomCategoryType             customCategory;               //�Զ������
	   DFITCPriceType                      turnover;                     //�ɽ����
	   DFITCOrderTypeType                  orderType;                    //��������
	   DFITCInsertType                     insertType;                   //�Զ������
	   DFITCClientIDType                   clientID;                     //���ױ��� */


	   //std::string sInstrCode = std::string(pTrade->instrumentID) + "." + pTrade->exchangeID + "@" + pConnection->getName();
	   std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->instrumentID), std::string(pTrade->exchangeID));
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == NULL)
	   {
		   loggerv2::error("xs_of_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->instrumentID), sizeof(pTrade->instrumentID), pTrade->instrumentID);
		   return NULL;
	   }

	   int nAccount = 0;
	   int nUserOrdId = -1;
	   int nInternalRe = -1;
	   int nPortfolio = 0;
	   int nTradingType = 0;

	   // client field : orderId|tradingType|portfolio
	   char *ptr = pTrade->customCategory;
	   int id = -1;
	   //if (tokenizer.size() !=2 )
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   /*loggerv2::error("xs_of_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->customCategory), sizeof(pOrder->customCategory), pOrder->customCategory);
		   return nullptr;*/
		   id = -1 * pTrade->localOrderID;
	   }
	   else
	   {
		   pConnection->get_user_info(pTrade->customCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		   id = nInternalRe;
	   }


	   // ok
	   //xs_of_order* o = new xs_of_order(pConnection);
	   o->set_id(id);

	   o->set_instrument(instr);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);

	   o->set_quantity(pTrade->orderAmount);
	   o->set_exec_quantity(pTrade->matchedAmount);
	   //o->set_price(pTrade->Price);
	   o->set_price(pTrade->insertPrice);
	   o->set_exec_price(pTrade->matchedPrice);

	   AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
	   switch (pTrade->openCloseType)
	   {
	   case DFITC_SPD_CLOSE:
	   case DFITC_SPD_FORCECLOSE:
		   oc = AtsType::OrderOpenClose::Close;
		   break;
	   case DFITC_SPD_FORCECLOSETODAY:
	   case DFITC_SPD_CLOSETODAY:
		   oc = AtsType::OrderOpenClose::CloseToday;
		   break;
	   case DFITC_SPD_OPEN:
	   default:
		   oc = AtsType::OrderOpenClose::Open;
		   break;
	   }
	   o->set_open_close(oc);


	   switch (pTrade->buySellType)
	   {
	   case DFITC_SPD_BUY:
		   o->set_way(AtsType::OrderWay::Buy);
		   break;
	   case DFITC_SPD_SELL:
		   o->set_way(AtsType::OrderWay::Sell);
		   break;

	   default:
		   loggerv2::error("xs_of_order - anchor: unknown order_way[%c]", pTrade->buySellType);
		   o->set_way(AtsType::OrderWay::Undef);
		   break;

	   }


	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   o->set_unknown_order();
	   }
	   else
	   {
		   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		   o->set_trading_type(nTradingType);
	   }

	   xs_of_order_aux::set_spdId(o,pTrade->spdOrderID);
	   /*date_time ordTime;
	   ordTime.set_time(pTrade->matchedTime);
	   */

	   auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->matchedTime));
	   o->set_last_time(tp);

	   //o->set_account(pConnection->getAccountName(nAccount).c_str());
	   o->set_user_orderid(nUserOrdId);

	   //o->save_previous_values();

	   //date_time now = date_time(time(NULL));
	   auto now = get_lwtp_now();
	   o->set_rebuild_time(now);

	  /* std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
	   o->set_rebuild_time_point(tpnow);*/
	   return o;
   }

   order* xs_of_order_aux::anchor(xs_of_connection* pConnection, DFITCOrderCommRtnField* pOrder)
   {
	   order* o = pConnection->get_order_from_pool();
	   //DFITCRequestIDType                  lRequestID;                   //����ID
	   //DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
	   //DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
	   //DFITCInstrumentIDType               instrumentID;                 //��Լ����
	   //DFITCBuySellTypeType                buySellType;                  //����
	   //DFITCOpenCloseTypeType              openClose;                    //��ƽ��־
	   //DFITCPriceType                      insertPrice;                  //ί�м�
	   //DFITCAmountType                     orderAmount;                  //ί������
	   //DFITCPriceType                      matchedPrice;                 //�ɽ��۸�
	   //DFITCAmountType                     matchedAmount;                //�ɽ�����
	   //DFITCAmountType                     cancelAmount;                 //��������
	   //DFITCInsertType                     insertType;                   //�Զ������
	   //DFITCSpeculatorType                 speculator;                   //Ͷ��
	   //DFITCDateType                       commTime;                     //ί��ʱ��
	   //DFITCDateType                       submitTime;                   //�걨ʱ��
	   //DFITCClientIDType                   clientID;                     //���ױ���
	   //DFITCExchangeIDType                 exchangeID;                   //������ID
	   //DFITCFrontAddrType                  operStation;                  //ί�е�ַ
	   //DFITCAccountIDType                  accountID;                    //�ͻ���
	   //DFITCInstrumentTypeType             instrumentType;               //��Լ����
	   //DFITCSessionIDType                  sessionId;                    //�ỰID
	   //DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
	   //DFITCOrderSysIDType                 OrderSysID;                   //�������
	   //DFITCCustomCategoryType             customCategory;               //�Զ������
	   //DFITCPriceType                      margin;                       //��֤��
	   //DFITCPriceType                      fee;                          //������
	   //DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
	   //DFITCPriceType                      profitLossPrice;              //ֹ��ֹӯ��
	   //DFITCOrderTypeType                  orderType;                    //�������
	   //DFITCOrderPropertyType              orderProperty;                //��������

	   std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->instrumentID), std::string(pOrder->exchangeID));
	   //std::string sInstrCode = std::string(pOrder->instrumentID) + "." + pOrder->exchangeID + "@" + pConnection->getName();

	   // tradeitem
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == nullptr)
	   {
		   loggerv2::error("xs_of_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->instrumentID), sizeof(pOrder->instrumentID), pOrder->instrumentID);
		   return nullptr;
	   }

	   int nAccount = 0;
	   int nUserOrdId = -1;
	   int nInternalRe = -1;
	   int nPortfolio = 0;
	   int nTradingType = 0;
	   int localID = pOrder->localOrderID;
	   // client field : orderId|tradingType|portfolio
	   //����ʱ���������͵�ack������devDecInfoΪ��
	   //int id = pOrder->localOrderID;
	   char *ptr = pOrder->customCategory;
	   int id = -1;
	   //if (tokenizer.size() !=2 )
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   /*loggerv2::error("xs_of_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->customCategory), sizeof(pOrder->customCategory), pOrder->customCategory);
		   return nullptr;*/
		   id = -1 * localID;
	   }
	   else
	   {
		   pConnection->get_user_info(pOrder->customCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		   id = nInternalRe;
	   }


	   // ok
	   //xs_of_order* o = new xs_of_order(pConnection);
	   o->set_id(id);

	   o->set_instrument(instr);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);

	   o->set_quantity(pOrder->orderAmount);

	   //o->set_price(pOrder->LimitPrice);
	   o->set_price(pOrder->insertPrice);

	   // way & open close

	   AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
	   switch (pOrder->openClose)
	   {
	   case DFITC_SPD_CLOSE:
	   case DFITC_SPD_FORCECLOSE:
		   oc = AtsType::OrderOpenClose::Close;
		   break;
	   case DFITC_SPD_FORCECLOSETODAY:
	   case DFITC_SPD_CLOSETODAY:
		   oc = AtsType::OrderOpenClose::CloseToday;
		   break;
	   case DFITC_SPD_OPEN:
	   default:
		   oc = AtsType::OrderOpenClose::Open;
		   break;
	   }
	   o->set_open_close(oc);

	   //
	   if (pOrder->orderProperty == DFITC_SP_FOK)
	   {
		   o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
	   }
	   else if (pOrder->orderProperty == DFITC_SP_FAK)
	   {
		   o->set_restriction(AtsType::OrderRestriction::FillAndKill);
	   }
	   else
	   {
		   o->set_restriction(AtsType::OrderRestriction::None);
	   }
	   //

	   switch (pOrder->buySellType)
	   {
	   case DFITC_SPD_BUY:
		   o->set_way(AtsType::OrderWay::Buy);
		   break;
	   case DFITC_SPD_SELL:
		   o->set_way(AtsType::OrderWay::Sell);
		   break;
	   }
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   o->set_unknown_order();

	   }
	   else
	   {
		   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		   o->set_trading_type(nTradingType);
	   }
	   //o->set_account(pConnection->getAccountName(nAccount).c_str());
	   o->set_user_orderid(nUserOrdId);
	   xs_of_order_aux::set_spdId(o,pOrder->spdOrderID);
	   //lts order info
	   //date_time ordTime = time(NULL);
	   auto time = get_lwtp_now();
	   o->set_last_time(time);

	   o->set_rebuild_time(time);

	 /*  std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
	   o->set_rebuild_time_point(tpnow);*/

	   //loggerv2::info("xs_of_order::anchor - successfully rebuild order [%d][%x]", id, o);
	   o->set_status(AtsType::OrderStatus::Ack);
	   xs_of_order_aux::set_locId(o,abs(id));
	   return o;
   }

   order* xs_of_order_aux::anchor(xs_of_connection* pConnection, DFITCMatchedRtnField* pTrade)
   {
	   order* o = pConnection->get_order_from_pool();

	   //DFITCRequestIDType                  lRequestID;                   //����ID
	   //DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
	   //DFITCAccountIDType                  accountID;                    //�ʽ��˺�
	   //DFITCExchangeIDType                 exchangeID;                   //������ID
	   //DFITCInstrumentIDType               instrumentID;                 //��Լ����
	   //DFITCBuySellTypeType                buySellType;                  //����
	   //DFITCOpenCloseTypeType              openClose;                    //��ƽ
	   //DFITCPriceType                      matchedPrice;                 //�ɽ��۸�
	   //DFITCAmountType                     matchedAmount;                //�ɽ�����
	   //DFITCPriceType                      matchedMort;                  //�ɽ����
	   //DFITCSpeculatorType                 speculator;                   //Ͷ�����
	   //DFITCDateType                       matchedTime;                  //�ɽ�ʱ��
	   //DFITCMatchIDType                    matchedID;                    //�ɽ����
	   //DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
	   //DFITCClientIDType                   clientID;                     //���ױ���
	   //DFITCMatchType                      matchType;                    //�ɽ�����
	   //DFITCInstrumentTypeType             instrumentType;               //��Լ����
	   //DFITCSessionIDType                  sessionId;                    //�ỰID
	   //DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
	   //DFITCCustomCategoryType             customCategory;               //�Զ������
	   //DFITCPriceType                      fee;                          //������
	   //DFITCOrderTypeType                  orderType;                    //��������
	   //DFITCOrderSysIDType                 OrderSysID;                   //�������

	   std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->instrumentID), std::string(pTrade->exchangeID));
	   //std::string sInstrCode = std::string(pTrade->instrumentID) + "." + pTrade->exchangeID + "@" + pConnection->getName();
	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == NULL)
	   {
		   loggerv2::error("xs_of_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->instrumentID), sizeof(pTrade->instrumentID), pTrade->instrumentID);
		   return NULL;
	   }

	   int nAccount = 0;
	   int nUserOrdId = -1;
	   int nInternalRe = -1;
	   int nPortfolio = 0;
	   int nTradingType = 0;
	   int localID = pTrade->localOrderID;
	   // client field : orderId|tradingType|portfolio
	   char *ptr = pTrade->customCategory;
	   int id = -1;
	   //if (tokenizer.size() !=2 )
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   /*loggerv2::error("xs_of_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->customCategory), sizeof(pOrder->customCategory), pOrder->customCategory);
		   return nullptr;*/
		   id = -1 * localID;
	   }
	   else
	   {
		   pConnection->get_user_info(pTrade->customCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		   id = nInternalRe;
	   }


	   // ok
	   //xs_of_order* o = new xs_of_order(pConnection);
	   o->set_id(id);

	   o->set_instrument(instr);
	   o->set_last_action(AtsType::OrderAction::Created);
	   o->set_status(AtsType::OrderStatus::WaitMarket);

	   o->set_quantity(pTrade->matchedAmount);
	   
	   o->set_exec_quantity(pTrade->matchedAmount);
	   //o->set_price(pTrade->Price);
	   //o->set_price(pTrade->insertPrice);
	   o->set_exec_price(pTrade->matchedPrice);

	   AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
	   switch (pTrade->openClose)
	   {
	   case DFITC_SPD_CLOSE:
	   case DFITC_SPD_FORCECLOSE:
		   oc = AtsType::OrderOpenClose::Close;
		   break;
	   case DFITC_SPD_FORCECLOSETODAY:
	   case DFITC_SPD_CLOSETODAY:
		   oc = AtsType::OrderOpenClose::CloseToday;
		   break;
	   case DFITC_SPD_OPEN:
	   default:
		   oc = AtsType::OrderOpenClose::Open;
		   break;
	   }
	   o->set_open_close(oc);


	   switch (pTrade->buySellType)
	   {
	   case DFITC_SPD_BUY:
		   o->set_way(AtsType::OrderWay::Buy);
		   break;
	   case DFITC_SPD_SELL:
		   o->set_way(AtsType::OrderWay::Sell);
		   break;

	   default:
		   loggerv2::error("xs_of_order - anchor: unknown order_way[%c]", pTrade->buySellType);
		   o->set_way(AtsType::OrderWay::Undef); 
		   break;

	   }

	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   o->set_unknown_order();

	   }
	   else
	   {
		   o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		   o->set_trading_type(nTradingType);
	   }

	   xs_of_order_aux::set_spdId(o,pTrade->spdOrderID);
	   /*date_time ordTime;
	   ordTime.set_time(pTrade->matchedTime);*/

	   auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->matchedTime));
	   o->set_last_time(tp);


	   //o->set_account(pConnection->getAccountName(nAccount));
	   o->set_user_orderid(nUserOrdId);

	   //o->save_previous_values();

	   //date_time now = date_time(time(NULL));
	   auto now = get_lwtp_now();
	   o->set_rebuild_time(now);

	   /* std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
		o->set_rebuild_time_point(tpnow);*/
	   return o;
   }

   quote* xs_of_order_aux::anchor(xs_of_connection* pConnection, DFITCQuoteRtnField* pQuote)
   {
	   std::string sInstrCode = std::string(pQuote->instrumentID) + "@" + pConnection->getName();

	   tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
	   if (instr == NULL)
	   {
		   loggerv2::error("cffex_order::anchor - tradeitem [%-*.*s] not found", sizeof(pQuote->instrumentID), sizeof(pQuote->instrumentID), pQuote->instrumentID);
		   return NULL;
	   }

	   int nAccount = 0;
	   int bidId = 0;
	   int askId = 0;
	   int nPortfolio = 0;
	   int nTradingType = 0;

	   ;
	   char *ptr = pQuote->customCategory;
	   //int id = -1;
	   //if (tokenizer.size() !=2 )
	   if (ptr[1] == ' ' || ptr[14] == ' ')
	   {
		   loggerv2::error("cffex_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pQuote->customCategory), sizeof(pQuote->customCategory), pQuote->customCategory);
		   //return NULL;
		   bidId = -1*pQuote->localOrderID;
	   }

	   else
	   {
		   pConnection->get_user_info(pQuote->customCategory, nAccount, bidId, askId, nPortfolio, nTradingType);
		   //id = nInternalRe;
	   }

	   quote* q = pConnection->get_quote_from_pool();
	   //q = pConnection->create_quote(instr, pQuote->BidVolume, pQuote->BidPrice, pQuote->AskVolume, pQuote->AskPrice, pQuote->ForQuoteSysID);
	   q->set_id(bidId);

	   q->set_instrument(instr);
	   int ret;
	   order* bid_order = pConnection->get_order_from_map(bidId, ret);
	   if (ret == 2)
	   {
		   bid_order = pConnection->create_order(instr, OrderWay::Buy, pQuote->bOrderAmount, pQuote->bInsertPrice);
		   auto time = get_lwtp_now();
		   bid_order->set_last_time(time);
		   //to do ...
		   bid_order->set_id(bidId);
		   pConnection->add_pending_order(bid_order);
	   }

	   order* ask_order = pConnection->get_order_from_map(askId, ret);
	   if (ret == 2)
	   {
		   ask_order = pConnection->create_order(instr, OrderWay::Sell, pQuote->sOrderAmount, pQuote->sInsertPrice);
		   auto time = get_lwtp_now();
		   ask_order->set_last_time(time);
		   //to do ...
		   ask_order->set_id(askId);
		   pConnection->add_pending_order(ask_order);
	   }
	   q->set_bid_order(bid_order);
	   q->set_ask_order(ask_order);

	   q->get_bid_order()->set_id(bidId);
	   q->get_ask_order()->set_id(askId);

	   q->set_last_action(AtsType::OrderAction::Created);
	   q->set_status(AtsType::OrderStatus::WaitMarket);




	   if (pQuote->sOpenCloseType == DFITC_SPD_OPEN)
		   q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::Open);
	   else if (pQuote->sOpenCloseType == DFITC_SPD_CLOSETODAY)
		   q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::CloseToday);
	   else
		   q->get_ask_order()->set_open_close(AtsType::OrderOpenClose::Close);


	   if (pQuote->bOpenCloseType == DFITC_SPD_OPEN)
		   q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::Open);
	   else if (pQuote->bOpenCloseType == DFITC_SPD_CLOSETODAY)
		   q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::CloseToday);
	   else
		   q->get_bid_order()->set_open_close(AtsType::OrderOpenClose::Close);



	   if (ptr[1] == ' ')
	   {
		   q->set_unknown_quote();

	   }
	   else
	   {
		   q->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
		   q->set_trading_type(nTradingType);
	   }

	   auto time = get_lwtp_now();
	   q->set_last_time(time);

	   loggerv2::info("cffex_order::anchor - successfully rebuild quote [%d][%x]", bidId, q);
	   return q;
   }

}

