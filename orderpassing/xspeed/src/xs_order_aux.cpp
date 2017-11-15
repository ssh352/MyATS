#include "xs_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "xs_connection.h"


namespace xs
{
	//xs_order::xs_order(xs_connection* pConnection) : order(pConnection)
	//{
	// m_nSpdId = 0;
	//}

	order* xs_order_aux::anchor(xs_connection* pConnection, DFITCSOPEntrustOrderRtnField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		//DFITCSECLocalOrderIDType             localOrderID;             //����ί�к�
		//DFITCSECAccountIDType                accountID;                //�ͻ���
		//DFITCSECSessionIDType                sessionID;                //�Ự���
		//DFITCSECBranchIDType                 branchID;                 //��������
		//DFITCSECShareholderIDType            shareholderID;            //�ɶ���
		//DFITCSECExchangeIDType               exchangeID;               //������
		//DFITCSECSecurityIDType               securityID;               //֤������
		//DFITCSECWithdrawFlagType             withdrawFlag;             //������־
		//DFITCSECCurrencyType                 currency;                 //����
		//DFITCSECSpdOrderIDType               spdOrderID;               //��̨ί�к�
		//DFITCSECEntrustDirectionType         entrustDirection;         //ί�����
		//DFITCSECOpenCloseFlagType            openCloseFlag;            //��ƽ��־
		//DFITCSECPriceType                    entrustPrice;             //ί�м۸�
		//DFITCSECQuantityType                 entrustQty;               //ί������
		//DFITCSECTimeType                     entrustTime;              //ί��ʱ��(Ԥ���ֶ�)
		//DFITCSECCoveredFlagType              coveredFlag;              //��������(Ԥ���ֶ�)
		//DFITCSECOrderTypeType                orderType;                //��������(Ԥ���ֶ�)
		//DFITCSECOrderExpiryDateType          orderExpiryDate;          //����ʱЧ����(Ԥ���ֶ�)
		//DFITCSECOrderCategoryType            orderCategory;            //ί�е����(Ԥ���ֶ�)
		//DFITCSECDeclareResultType            declareResult;            //�걨���
		//DFITCSECTDevIDType                   devID;                    //����ID(N),�µ�ʱ������ֶΣ��Ż᷵��
		//DFITCSECTDevDecInfoType              devDecInfo;               //�û��Զ����ֶ�(N)���µ�ʱ������ֶΣ��Ż᷵��
		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->securityID), std::string(pOrder->exchangeID));
		/*if (pConnection->m_bKey_with_exchange)
			sInstrCode = std::string(pOrder->securityID) + "." + pOrder->exchangeID + "@" + pConnection->getName();
		else
			sInstrCode = std::string(pOrder->securityID) + "@" + pConnection->getName();*/


		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("xs_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->securityID), sizeof(pOrder->securityID), pOrder->securityID);
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
		char *ptr = pOrder->devDecInfo;
		int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			loggerv2::error("xs_order::anchor - cannot parse UserID field[%-*.*s]. Order may from another application", sizeof(pOrder->devDecInfo), sizeof(pOrder->devDecInfo), pOrder->devDecInfo);
			return nullptr;
		}
		else
		{
			pConnection->get_user_info(pOrder->devDecInfo, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}



		// ok
		//xs_order* o = new xs_order(pConnection);
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->entrustQty);

		//o->set_price(pOrder->LimitPrice);
		o->set_price(pOrder->entrustPrice);

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->openCloseFlag)
		{
		case DFITCSEC_OCF_Close:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case DFITCSEC_OCF_Open:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);

		//
		if (pOrder->orderExpiryDate == DFITCSEC_OE_FOK)
		{			
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);						
		}
		else if (pOrder->orderExpiryDate == DFITCSEC_OE_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//

		switch (pOrder->entrustDirection)
		{
		case DFITCSEC_ED_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case DFITCSEC_ED_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}
		if (ptr[1] == ' ')
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
		xs_order_aux::set_spdId(o, pOrder->spdOrderID);
		//xs order info
		//date_time ordTime = time(NULL);
		auto time = get_lwtp_now();
		o->set_last_time(time);

		//o->set_account(o->get_portfolio());
		// save original quantity / price
		//o->save_previous_values();

		//date_time now = date_time(time(NULL));
		o->set_rebuild_time(time);

		//std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
		//o->set_rebuild_time_point(microsec_clock::local_time());

		//loggerv2::info("xs_order::anchor - successfully rebuild order [%d][%x]", id, o);
		o->set_status(AtsType::OrderStatus::Ack);
		xs_order_aux::set_locId(o, abs(localID));
		return o;

	}

	order* xs_order_aux::anchor(xs_connection* pConnection, DFITCSOPTradeRtnField* pTrade)
	{
		order* o = pConnection->get_order_from_pool();

		//DFITCSECLocalOrderIDType             localOrderID;             //����ί�к�
		//DFITCSECAccountIDType                accountID;                //�ͻ���
		//DFITCSECSessionIDType                sessionID;                //�Ự���
		//DFITCSECShareholderIDType            shareholderID;            //�ɶ���
		//DFITCSECExchangeIDType               exchangeID;               //������
		//DFITCSECSecurityIDType               securityID;               //֤������
		//DFITCSECWithdrawFlagType             withdrawFlag;             //������־
		//DFITCSECCurrencyType                 currency;                 //����
		//DFITCSECSpdOrderIDType               spdOrderID;               //��̨ί�к�
		//DFITCSECEntrustDirectionType         entrustDirection;         //ί�����
		//DFITCSECOpenCloseFlagType            openCloseFlag;            //��ƽ��־
		//DFITCSECCoveredFlagType              coveredFlag;              //���ұ�־(Ԥ���ֶ�)
		//DFITCSECOrderCategoryType            orderCategory;            //ί�е����
		//DFITCSECFundsType                    tradePrice;               //�ɽ��۸�
		//DFITCSECQuantityType                 tradeQty;                 //�ɽ�����
		//DFITCSECTradeIDType                  tradeID;                  //�ɽ����
		//DFITCSECSerialIDType                 rtnSerialID;              //�ر����
		//DFITCSECDeclareOrderIDType           declareOrderID;           //�걨ί�к�(Ԥ���ֶ�)
		//DFITCSECDeclareResultType            declareResult;            //�걨���
		//DFITCSECTDevIDType                   devID;                    //����ID(N),�µ�ʱ������ֶΣ��Ż᷵��
		//DFITCSECTDevDecInfoType              devDecInfo;               //�û��Զ����ֶ�(N),�µ�ʱ������ֶΣ��Ż᷵��
		//DFITCSECTimeType                     tradeTime;                //�ɽ�ʱ��(Ԥ���ֶ�)

		std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->securityID), std::string(pTrade->exchangeID));
		/*std::string sInstrCode;
		if (pConnection->m_bKey_with_exchange)
			sInstrCode = std::string(pTrade->securityID) +"." +pTrade->exchangeID + "@" + pConnection->getName();
		else
			sInstrCode = std::string(pTrade->securityID) +"@" + pConnection->getName();*/

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("xs_order_aux::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->securityID), sizeof(pTrade->securityID), pTrade->securityID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pTrade->localOrderID;
		// client field : orderId|tradingType|portfolio
		char *ptr = pTrade->devDecInfo;
		int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			//loggerv2::error("xs_order::anchor - cannot parse UserID field[%-*.*s]. Trade may from another application", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			return NULL;
			//id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pTrade->devDecInfo, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}


		// ok
		//xs_order* o = new xs_order(pConnection);
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->tradeQty);
		//o->set_price(pTrade->Price);
		o->set_price(pTrade->tradePrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pTrade->openCloseFlag)
		{
		case DFITCSEC_OCF_Open:
			oc = AtsType::OrderOpenClose::Open;
			break;
		case DFITCSEC_OCF_Close:
			oc = AtsType::OrderOpenClose::Close;
			break;

		default:
			break;
		}
		o->set_open_close(oc);


		switch (pTrade->entrustDirection)
		{
		case DFITCSEC_ED_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case DFITCSEC_ED_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("xs_order - anchor: unknown order_way[%c]", pTrade->entrustDirection);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}


		if (ptr[1] == ' ')
		{
			o->set_unknown_order();
		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		xs_order_aux::set_spdId(o, pTrade->spdOrderID);


		auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->tradeTime));

		o->set_last_time(tp);

		//o->set_account(pConnection->getAccountName(nAccount).c_str());
		o->set_user_orderid(nUserOrdId);

		//o->save_previous_values();

		//date_time now = date_time(time(NULL));
		auto now = get_lwtp_now();
		o->set_rebuild_time(now);

		//std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
		//o->set_rebuild_time_point(microsec_clock::local_time());
		xs_order_aux::set_locId(o, abs(localID));
		return o;
	}

	order* xs_order_aux::anchor(xs_connection* pConnection, DFITCSOPWithdrawOrderRtnField* pCancel)
	{
		order* o = pConnection->get_order_from_pool();

		//DFITCSECLocalOrderIDType             localOrderID;             //����ί�к�
		//DFITCSECAccountIDType                accountID;                //�ͻ���
		//DFITCSECSessionIDType                sessionID;                //�Ự���
		//DFITCSECShareholderIDType            shareholderID;            //�ɶ���
		//DFITCSECExchangeIDType               exchangeID;               //����������
		//DFITCSECSecurityIDType               securityID;               //֤������
		//DFITCSECWithdrawFlagType             withdrawFlag;             //������־
		//DFITCSECCurrencyType                 currency;                 //����
		//DFITCSECSpdOrderIDType               spdOrderID;               //��̨ί�к�
		//DFITCSECEntrustDirectionType         entrustDirection;         //ί�����
		//DFITCSECOpenCloseFlagType            openCloseFlag;            //��ƽ��־
		//DFITCSECQuantityType                 withdrawQty;              //��������
		//DFITCSECQuantityType                 tradeQty;                 //�ɽ�����
		//DFITCSECDeclareResultType            declareResult;            //�걨���
		//DFITCSECMessageType                  noteMsg;                  //���˵��
		//DFITCSECFundsType                    wdUnFreezeFunds;          //�����ⶳ�ʽ�
		//DFITCSECTDevIDType                   devID;                    //����ID(N),�µ�ʱ������ֶΣ��Ż᷵��
		//DFITCSECTDevDecInfoType              devDecInfo;               //�û��Զ����ֶ�(N),�µ�ʱ������ֶΣ��Ż᷵��

		std::string sInstrCode = pConnection->compute_second_key(std::string(pCancel->securityID), std::string(pCancel->exchangeID));
		/*std::string sInstrCode;
		if (pConnection->m_bKey_with_exchange)
			sInstrCode = std::string(pCancel->securityID) + "."+ pCancel->exchangeID + "@" + pConnection->getName();
		else
			sInstrCode = std::string(pCancel->securityID) + "@" + pConnection->getName();*/

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("xs_order_aux::anchor - tradeitem [%-*.*s] not found", sizeof(pCancel->securityID), sizeof(pCancel->securityID), pCancel->securityID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pCancel->localOrderID;
		// client field : orderId|tradingType|portfolio
		char *ptr = pCancel->devDecInfo;
		int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ')
		{
			//loggerv2::error("xs_order::anchor - cannot parse UserID field[%-*.*s]. Trade may from another application", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			return NULL;
			//id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pCancel->devDecInfo, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}


		// ok
		//xs_order* o = new xs_order(pConnection);
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pCancel->tradeQty + pCancel->withdrawQty);
		o->set_exec_quantity(pCancel->tradeQty);
		//o->set_price(pTrade->Price);
		//o->set_price(pCancel->tradePrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pCancel->openCloseFlag)
		{
		case DFITCSEC_OCF_Open:
			oc = AtsType::OrderOpenClose::Open;
			break;
		case DFITCSEC_OCF_Close:
			oc = AtsType::OrderOpenClose::Close;
			break;

		default:
			break;
		}
		o->set_open_close(oc);


		switch (pCancel->entrustDirection)
		{
		case DFITCSEC_ED_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case DFITCSEC_ED_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("xs_order - anchor: unknown order_way[%c]", pCancel->entrustDirection);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}


		if (ptr[1] == ' ')
		{
			o->set_unknown_order();
		}
		else
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_trading_type(nTradingType);
		}

		xs_order_aux::set_spdId(o, pCancel->spdOrderID);

		o->set_user_orderid(nUserOrdId);

		//o->save_previous_values();

		//date_time now = date_time(time(NULL));
		auto now = get_lwtp_now();
		o->set_rebuild_time(now);

		//std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
		//o->set_rebuild_time_point(microsec_clock::local_time());
		xs_order_aux::set_locId(o, abs(localID));
		return o;
	}

	order* xs_order_aux::anchor(xs_connection* pConnection, DFITCStockEntrustOrderRtnField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		//DFITCSECLocalOrderIDType             localOrderID;             //����ί�к�
		//DFITCSECAccountIDType                accountID;                //�ͻ���
		//DFITCSECSessionIDType                sessionID;                //�Ự���
		//DFITCSECShareholderIDType            shareholderID;            //�ɶ���
		//DFITCSECExchangeIDType               exchangeID;               //����������
		//DFITCSECCurrencyType                 currency;                 //����
		//DFITCSECSecurityIDType               securityID;               //֤������
		//DFITCSECSecurityTypeType             securityType;             //֤ȯ���
		//DFITCSECQuantityType                 withdrawQty;              //��������
		//DFITCSECWithdrawFlagType             withdrawFlag;             //������־
		//DFITCSECFundsType                    freezeFunds;              //�����ʽ�
		//DFITCSECSpdOrderIDType               spdOrderID;               //��̨ί�к�
		//DFITCSECEntrustDirectionType         entrustDirection;         //ί�����
		//DFITCSECDeclareResultType            declareResult;            //�걨���
		//DFITCSECMessageType                  noteMsg;                  //���˵��
		//DFITCSECQuantityType                 entrustQty;               //ί������
		//DFITCSECOrderConfirmFlagType         orderConfirmFlag;         //ί��ȷ�ϱ�־
		//DFITCSECTimeType                     entrustTime;              //ί��ʱ��
		//DFITCSECPriceType                    entrustPrice;             //ί�м۸�(Ԥ���ֶ�)
		//DFITCSECOrderTypeType                orderType;                //��������(Ԥ���ֶ�)

		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->securityID), std::string(pOrder->exchangeID));


		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("xs_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->securityID), sizeof(pOrder->securityID), pOrder->securityID);
			return nullptr;
		}

		//int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pOrder->localOrderID;
		// client field : orderId|tradingType|portfolio
		//����ʱ���������͵�ack������devDecInfoΪ��
		int id = pOrder->localOrderID;


		// ok
		//xs_order* o = new xs_order(pConnection);
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->entrustQty);

		//o->set_price(pOrder->LimitPrice);
		o->set_price(pOrder->entrustPrice);

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		o->set_open_close(oc);


		switch (pOrder->entrustDirection)
		{
		case DFITCSEC_ED_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case DFITCSEC_ED_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

	  {
		  o->set_unknown_order();

	  }
	  /*else
	  {
	  o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
	  o->set_trading_type((trading_type)nTradingType);
	  }*/
	  //o->set_account(pConnection->getAccountName(nAccount).c_str());
	  o->set_user_orderid(nUserOrdId);
	  xs_order_aux::set_spdId(o, pOrder->spdOrderID);
	  //xs order info
	  //date_time ordTime = time(NULL);
	  auto time = get_lwtp_now();
	  o->set_last_time(time);
	  o->set_rebuild_time(time);

	  //std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
	  //o->set_rebuild_time_point(microsec_clock::local_time());

	  //loggerv2::info("xs_order::anchor - successfully rebuild order [%d][%x]", id, o);
	  o->set_status(AtsType::OrderStatus::Ack);
	  xs_order_aux::set_locId(o, abs(id));
	  return o;

	}

	order* xs_order_aux::anchor(xs_connection* pConnection, DFITCStockTradeRtnField* pTrade)
	{
		order* o = pConnection->get_order_from_pool();
		//DFITCSECLocalOrderIDType             localOrderID;             //����ί�к�
		//DFITCSECAccountIDType                accountID;                //�ͻ���
		//DFITCSECSessionIDType                sessionID;                //�Ự���
		//DFITCSECShareholderIDType            shareholderID;            //�ɶ���
		//DFITCSECExchangeIDType               exchangeID;               //����������
		//DFITCSECCurrencyType                 currency;                 //����
		//DFITCSECSecurityIDType               securityID;               //֤������
		//DFITCSECSecurityTypeType             securityType;             //֤ȯ���
		//DFITCSECWithdrawFlagType             withdrawFlag;             //������־
		//DFITCSECTradeIDType                  tradeID;                  //�ɽ����
		//DFITCSECTimeType                     tradeTime;                //�ɽ�ʱ��
		//DFITCSECQuantityType                 withdrawQty;              //��������
		//DFITCSECSpdOrderIDType               spdOrderID;               //��̨ί�к�
		//DFITCSECEntrustDirectionType         entrustDirection;         //ί�����
		//DFITCSECFundsType                    clearFunds;               //�����ʽ�
		//DFITCSECQuantityType                 totalTradeQty;            //ί���ܳɽ�����
		//DFITCSECFundsType                    totalTurnover;            //ί���ܳɽ����
		//DFITCSECQuantityType                 tradeQty;                 //���γɽ�����
		//DFITCSECPriceType                    tradePrice;               //���γɽ��۸�
		//DFITCSECFundsType                    turnover;                 //���γɽ����
		//DFITCSECQuantityType                 entrustQty;               //ί������
		
		std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->securityID), std::string(pTrade->exchangeID));

		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("xs_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->securityID), sizeof(pTrade->securityID), pTrade->securityID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pTrade->localOrderID;
		int id = -1;

		auto it = pConnection->m_localId2Portfolio.find(localID);
		if (it != pConnection->m_localId2Portfolio.end())
		{
			nAccount = it->second.accId;
			id = it->second.orderId;
			nTradingType = it->second.nTradingType;
			if (nAccount != pConnection->getAccountNum())
			{
				if (id > 0)
					id = id + 1000000 * nAccount;
			}
			o->set_portfolio(it->second.portfolio.c_str());
		}
		else
			id = -localID;

		o->set_id(id);
		o->set_trading_type(nTradingType);
		if (localID > 0)
			xs_order_aux::set_locId(o, localID);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->tradeQty);
		o->set_price(pTrade->tradePrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		o->set_open_close(oc);

		switch (pTrade->entrustDirection)
		{
		case DFITCSEC_ED_Buy:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case DFITCSEC_ED_Sell:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("xs_order - anchor: unknown order_way[%c]", pTrade->entrustDirection);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}

	   xs_order_aux::set_spdId(o, pTrade->spdOrderID);

	   auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->tradeTime));
	   o->set_last_time(tp);

	   o->set_user_orderid(nUserOrdId);

	   auto now = get_lwtp_now();;
	   o->set_rebuild_time(now);

	   return o;
	}


}

