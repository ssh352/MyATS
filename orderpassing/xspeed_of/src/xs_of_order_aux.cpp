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
	   /*DFITCLocalOrderIDType               localOrderID;                 //本地委托号
	   DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
	   DFITCOrderSysIDType                 OrderSysID;                   //报单编号
	   DFITCOrderAnswerStatusType          orderStatus;                  //委托状态
	   DFITCSessionIDType                  sessionID;                    //会话ID
	   DFITCDateType                       SuspendTime;                  //挂起时间
	   DFITCInstrumentIDType               instrumentID;                 //合约代码
	   DFITCExchangeIDType                 exchangeID;                   //交易所
	   DFITCBuySellTypeType                buySellType;                  //买卖
	   DFITCOpenCloseTypeType              openCloseType;                //开平
	   DFITCInstrumentTypeType             instrumentType;               //合约类型
	   DFITCSpeculatorType                 speculator;                   //投资类别
	   DFITCPriceType                      insertPrice;                  //委托价
	   DFITCPriceType                      profitLossPrice;              //止盈止损价格
	   DFITCAccountIDType                  accountID;                    //资金账号
	   DFITCAmountType                     cancelAmount;                 //撤单数量
	   DFITCAmountType                     orderAmount;                  //委托数量
	   DFITCInsertType                     insertType;                   //自动单类别
	   DFITCOrderTypeType                  orderType;                    //报单类型
	   DFITCSPDOrderIDType                 extSpdOrderID;                //算法单编号
	   DFITCReservedType                   reservedType2;                //预留字段2
	   DFITCCustomCategoryType             customCategory;               //自定义类别
	   DFITCOrderPropertyType              orderProperty;                //订单属性
	   DFITCAmountType                     minMatchAmount;               //最小成交量
	   DFITCClientIDType                   clientID;                     //交易编码
	   DFITCErrorMsgInfoType               statusMsg;                    //状态信息*/

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
	   //重连时交易所发送的ack报文中devDecInfo为空
	   //int id = pOrder->localOrderID;//注意，这是个负数

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

	   /*DFITCLocalOrderIDType               localOrderID;                 //本地委托号
	   DFITCOrderSysIDType                 OrderSysID;                   //报单编号(交易所报单编号)
	   DFITCMatchIDType                    matchID;                      //成交编号
	   DFITCInstrumentIDType               instrumentID;                 //合约代码
	   DFITCBuySellTypeType                buySellType;                  //买卖
	   DFITCOpenCloseTypeType              openCloseType;                //开平标志
	   DFITCPriceType                      matchedPrice;                 //成交价格
	   DFITCAmountType                     orderAmount;                  //委托数量
	   DFITCAmountType                     matchedAmount;                //成交数量
	   DFITCDateType                       matchedTime;                  //成交时间
	   DFITCPriceType                      insertPrice;                  //报价
	   DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
	   DFITCMatchType                      matchType;                    //成交类型
	   DFITCSpeculatorType                 speculator;                   //投保
	   DFITCExchangeIDType                 exchangeID;                   //交易所ID
	   DFITCFeeType                        fee;                          //手续费
	   DFITCSessionIDType                  sessionID;                    //会话标识
	   DFITCInstrumentTypeType             instrumentType;               //合约类型
	   DFITCAccountIDType                  accountID;                    //资金账号
	   DFITCOrderAnswerStatusType          orderStatus;                  //申报结果
	   DFITCPriceType                      margin;                       //开仓为保证金,平仓为解冻保证金
	   DFITCPriceType                      frozenCapita;                 //成交解冻委托冻结的资金
	   DFITCAdjustmentInfoType             adjustmentInfo;               //组合或对锁的保证金调整信息,格式:[合约代码,买卖标志,投资类别,调整金额;] 
	   DFITCCustomCategoryType             customCategory;               //自定义类别
	   DFITCPriceType                      turnover;                     //成交金额
	   DFITCOrderTypeType                  orderType;                    //报单类型
	   DFITCInsertType                     insertType;                   //自动单类别
	   DFITCClientIDType                   clientID;                     //交易编码 */


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
	   //DFITCRequestIDType                  lRequestID;                   //请求ID
	   //DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
	   //DFITCOrderAnswerStatusType          orderStatus;                  //委托状态
	   //DFITCInstrumentIDType               instrumentID;                 //合约代码
	   //DFITCBuySellTypeType                buySellType;                  //买卖
	   //DFITCOpenCloseTypeType              openClose;                    //开平标志
	   //DFITCPriceType                      insertPrice;                  //委托价
	   //DFITCAmountType                     orderAmount;                  //委托数量
	   //DFITCPriceType                      matchedPrice;                 //成交价格
	   //DFITCAmountType                     matchedAmount;                //成交数量
	   //DFITCAmountType                     cancelAmount;                 //撤单数量
	   //DFITCInsertType                     insertType;                   //自动单类别
	   //DFITCSpeculatorType                 speculator;                   //投保
	   //DFITCDateType                       commTime;                     //委托时间
	   //DFITCDateType                       submitTime;                   //申报时间
	   //DFITCClientIDType                   clientID;                     //交易编码
	   //DFITCExchangeIDType                 exchangeID;                   //交易所ID
	   //DFITCFrontAddrType                  operStation;                  //委托地址
	   //DFITCAccountIDType                  accountID;                    //客户号
	   //DFITCInstrumentTypeType             instrumentType;               //合约类型
	   //DFITCSessionIDType                  sessionId;                    //会话ID
	   //DFITCReservedType                   reservedType2;                //预留字段2
	   //DFITCOrderSysIDType                 OrderSysID;                   //报单编号
	   //DFITCCustomCategoryType             customCategory;               //自定义类别
	   //DFITCPriceType                      margin;                       //保证金
	   //DFITCPriceType                      fee;                          //手续费
	   //DFITCLocalOrderIDType               localOrderID;                 //本地委托号
	   //DFITCPriceType                      profitLossPrice;              //止损止盈价
	   //DFITCOrderTypeType                  orderType;                    //报单类别
	   //DFITCOrderPropertyType              orderProperty;                //订单属性

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
	   //重连时交易所发送的ack报文中devDecInfo为空
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

	   //DFITCRequestIDType                  lRequestID;                   //请求ID
	   //DFITCSPDOrderIDType                 spdOrderID;                   //柜台委托号
	   //DFITCAccountIDType                  accountID;                    //资金账号
	   //DFITCExchangeIDType                 exchangeID;                   //交易所ID
	   //DFITCInstrumentIDType               instrumentID;                 //合约代码
	   //DFITCBuySellTypeType                buySellType;                  //买卖
	   //DFITCOpenCloseTypeType              openClose;                    //开平
	   //DFITCPriceType                      matchedPrice;                 //成交价格
	   //DFITCAmountType                     matchedAmount;                //成交数量
	   //DFITCPriceType                      matchedMort;                  //成交金额
	   //DFITCSpeculatorType                 speculator;                   //投保类别
	   //DFITCDateType                       matchedTime;                  //成交时间
	   //DFITCMatchIDType                    matchedID;                    //成交编号
	   //DFITCLocalOrderIDType               localOrderID;                 //本地委托号
	   //DFITCClientIDType                   clientID;                     //交易编码
	   //DFITCMatchType                      matchType;                    //成交类型
	   //DFITCInstrumentTypeType             instrumentType;               //合约类型
	   //DFITCSessionIDType                  sessionId;                    //会话ID
	   //DFITCReservedType                   reservedType2;                //预留字段2
	   //DFITCCustomCategoryType             customCategory;               //自定义类别
	   //DFITCPriceType                      fee;                          //手续费
	   //DFITCOrderTypeType                  orderType;                    //报单类型
	   //DFITCOrderSysIDType                 OrderSysID;                   //报单编号

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

