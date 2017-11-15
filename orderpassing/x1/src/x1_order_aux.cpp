#include "x1_order_aux.h"
#include "terra_logger.h"
#include "tradeItem_gh.h"
#include "order_reference_provider.h"
#include "string_tokenizer.h"
#include "x1_connection.h"

namespace x1
{
	//x1_order::x1_order(x1_connection* pConnection) : order(pConnection)
	//{
	// m_nSpdId = 0;
	//}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspPriOrderField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		/*
		 TX1FtdcRequestIDType                  RequestID;                    ///< ����ID
		 TX1FtdcX1OrderIDType                  X1OrderID;                    ///< ��̨ί�к�
		 TX1FtdcOrderAnswerStatusType          OrderStatus;                  ///< ί��״̬
		 TX1FtdcInstrumentIDType               InstrumentID;                 ///< ��Լ����
		 TX1FtdcBuySellTypeType                BuySellType;                  ///< ����
		 TX1FtdcOpenCloseTypeType              OpenClose;                    ///< ��ƽ��־
		 TX1FtdcPriceType                      InsertPrice;                  ///< ί�м�
		 TX1FtdcAmountType                     OrderAmount;                  ///< ί������
		 TX1FtdcPriceType                      MatchedPrice;                 ///< �ɽ��۸�
		 TX1FtdcAmountType                     MatchedAmount;                ///< �ɽ�����
		 TX1FtdcAmountType                     CancelAmount;                 ///< ��������
		 TX1FtdcInsertType                     InsertType;                   ///< �Զ������
		 TX1FtdcSpeculatorType                 Speculator;                   ///< Ͷ��
		 TX1FtdcDateType                       CommTime;                     ///< ί��ʱ��
		 TX1FtdcDateType                       SubmitTime;                   ///< �걨ʱ��
		 TX1FtdcClientIDType                   ClientID;                     ///< ���ױ���
		 TX1FtdcExchangeIDType                 ExchangeID;                   ///< ������ID
		 TX1FtdcFrontAddrType                  OperStation;                  ///< ί�е�ַ
		 TX1FtdcAccountIDType                  AccountID;                    ///< �ͻ���
		 TX1FtdcInstrumentTypeType             InstrumentType;               ///< ��Լ����
		 TX1FtdcSessionIDType                  SessionId;                    ///< �ỰID(�˰汾��֧��)
		 TX1FtdcReservedType                   ReservedType2;                ///< Ԥ���ֶ�2
		 TX1FtdcOrderSysIDType                 OrderSysID;                   ///< �������
		 TX1FtdcCustomCategoryType             CustomCategory;               ///< �Զ������
		 TX1FtdcPriceType                      Margin;                       ///< ��֤��
		 TX1FtdcPriceType                      Fee;                          ///< ������
		 TX1FtdcLocalOrderIDType               LocalOrderID;                 ///< ����ί�к� ���µ�ʱ�õ��ӵı���ί�кţ����ͬһ�˺ŴӶ���ͻ����µ������ѯ���ص�LocalOrderID�������ظ���
		 TX1FtdcPriceType                      ProfitLossPrice;              ///< ֹ��ֹӯ��
		 TX1FtdcOrderTypeType                  OrderType;                    ///< �������
		 TX1FtdcOrderPropertyType              OrderProperty;                ///< ��������
		 */

		//std::string sInstrCode = std::string(pOrder->InstrumentID) + "." + pOrder->ExchangeID + "@" + pConnection->getName();
		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->InstrumentID), std::string(pOrder->ExchangeID));

		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = -1;
		int nTradingType = 0;
		int localID = pOrder->LocalOrderID;

		char *ptr = pOrder->CustomCategory;
		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pOrder->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_localId2Portfolio.find(pOrder->LocalOrderID);
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
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}
		
		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));

		pConnection->insert_spId2order(pOrder->X1OrderID, o);
		pConnection->insert_localId2order(localID, o);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->OrderAmount);
		o->set_price(pOrder->InsertPrice);

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->OpenCloseType)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;
		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);

		//
		if (pOrder->OrderProperty == X1_FTDC_SP_FOK)
		{
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		}
		else if (pOrder->OrderProperty == X1_FTDC_SP_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//

		switch (pOrder->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

		auto time = get_lwtp_now();
		o->set_last_time(time);

		o->set_rebuild_time(time);

		o->set_status(AtsType::OrderStatus::Ack);

		return o;

	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspPriCancelOrderField* pOrder)
	{
		order* o = pConnection->get_order_from_pool();
		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->InstrumentID), std::string(pOrder->ExchangeID));

		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = -1;
		int nTradingType = 0;
		int localID = pOrder->LocalOrderID;

		char *ptr = pOrder->CustomCategory;
		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pOrder->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_x1Id2Portfolio.find(pOrder->X1OrderID);
			if (it != pConnection->m_x1Id2Portfolio.end())
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
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}




		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));

		pConnection->insert_spId2order(pOrder->X1OrderID, o);
		pConnection->insert_localId2order(localID, o);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->OrderAmount);
		o->set_price(pOrder->InsertPrice);

		// way & open close

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->OpenCloseType)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;
		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);


		switch (pOrder->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

		auto time = get_lwtp_now();
		o->set_last_time(time);

		o->set_rebuild_time(time);

		o->set_status(AtsType::OrderStatus::Ack);

		return o;
	
	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspPriMatchInfoField* pTrade)
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
		DFITCCustomCategoryType             CustomCategory;               //�Զ������
		DFITCPriceType                      turnover;                     //�ɽ����
		DFITCOrderTypeType                  orderType;                    //��������
		DFITCInsertType                     insertType;                   //�Զ������
		DFITCClientIDType                   clientID;                     //���ױ��� */


		//std::string sInstrCode = std::string(pTrade->InstrumentID) + "." + pTrade->ExchangeID + "@" + pConnection->getName();
		std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->InstrumentID), std::string(pTrade->ExchangeID));
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pTrade->LocalOrderID;
		// client field : orderId|tradingType|portfolio
		char *ptr = pTrade->CustomCategory;
		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pTrade->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_x1Id2Portfolio.find(pTrade->X1OrderID);
			if (it != pConnection->m_x1Id2Portfolio.end())
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
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}
			
		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pTrade->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));

		pConnection->insert_spId2order(pTrade->X1OrderID, o);
		pConnection->insert_localId2order(localID, o);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->OrderAmount);
		o->set_exec_quantity(pTrade->MatchedAmount);
		//o->set_price(pTrade->Price);
		o->set_price(pTrade->InsertPrice);
		o->set_exec_price(pTrade->MatchedPrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pTrade->OpenCloseType)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);


		switch (pTrade->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("x1_order - anchor: unknown order_way[%c]", pTrade->BuySellType);
			o->set_way(AtsType::OrderWay::Undef);
			break;

		}

		auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->MatchedTime));
		o->set_last_time(tp);
		
		o->set_last_time(tp);
		o->set_rebuild_time(tp);


		o->set_status(AtsType::OrderStatus::Ack);

		return o;
	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspOrderField* pOrder)
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
		//DFITCCustomCategoryType             CustomCategory;               //�Զ������
		//DFITCPriceType                      margin;                       //��֤��
		//DFITCPriceType                      fee;                          //������
		//DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
		//DFITCPriceType                      profitLossPrice;              //ֹ��ֹӯ��
		//DFITCOrderTypeType                  orderType;                    //�������
		//DFITCOrderPropertyType              orderProperty;                //��������

		std::string sInstrCode = pConnection->compute_second_key(std::string(pOrder->InstrumentID), std::string(pOrder->ExchangeID));
		//std::string sInstrCode = std::string(pOrder->InstrumentID) + "." + pOrder->ExchangeID + "@" + pConnection->getName();

		// tradeitem
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == nullptr)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pOrder->InstrumentID), sizeof(pOrder->InstrumentID), pOrder->InstrumentID);
			return nullptr;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pOrder->LocalOrderID;
		//����ʱ���������͵�ack������devDecInfoΪ��
		//int id = pOrder->localOrderID;
		char *ptr = pOrder->CustomCategory;

		int id = -1;
		bool findbylocal = false;

		pConnection->get_user_info(pOrder->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
		id = nInternalRe;
		if (id <= 0)
		{
			findbylocal = true;
			auto it = pConnection->m_x1Id2Portfolio.find(pOrder->X1OrderID);
			if (it != pConnection->m_x1Id2Portfolio.end())
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
		}

		if (!findbylocal)
		{
			o->set_portfolio(pConnection->getPortfolioName(nPortfolio).c_str());
			o->set_user_orderid(nUserOrdId);
		}

		o->set_id(id);
		o->set_trading_type(nTradingType);
		x1_order_aux::set_spdId(o, pOrder->X1OrderID);
		if (localID > 0)
			x1_order_aux::set_locId(o, localID);
		else
			x1_order_aux::set_locId(o, abs(id));
		pConnection->insert_spId2order(pOrder->X1OrderID, o);
		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pOrder->OrderAmount);
		o->set_price(pOrder->InsertPrice);


		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pOrder->OpenClose)
		{

		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;
		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);

		//
		if (pOrder->OrderProperty == X1_FTDC_SP_FOK)
		{
			o->set_restriction(AtsType::OrderRestriction::ImmediateAndCancel);
		}
		else if (pOrder->OrderProperty == X1_FTDC_SP_FAK)
		{
			o->set_restriction(AtsType::OrderRestriction::FillAndKill);
		}
		else
		{
			o->set_restriction(AtsType::OrderRestriction::None);
		}
		//

		switch (pOrder->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;
		}

		auto time = get_lwtp_now();
		o->set_last_time(time);

		o->set_rebuild_time(time);

		o->set_status(AtsType::OrderStatus::Ack);

		return o;
	}

	order* x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcRspMatchField* pTrade)
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
		//DFITCCustomCategoryType             CustomCategory;               //�Զ������
		//DFITCPriceType                      fee;                          //������
		//DFITCOrderTypeType                  orderType;                    //��������
		//DFITCOrderSysIDType                 OrderSysID;                   //�������

		std::string sInstrCode = pConnection->compute_second_key(std::string(pTrade->InstrumentID), std::string(pTrade->ExchangeID));
		//std::string sInstrCode = std::string(pTrade->InstrumentID) + "." + pTrade->ExchangeID + "@" + pConnection->getName();
		tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(sInstrCode.c_str());
		if (instr == NULL)
		{
			loggerv2::error("x1_order::anchor - tradeitem [%-*.*s] not found", sizeof(pTrade->InstrumentID), sizeof(pTrade->InstrumentID), pTrade->InstrumentID);
			return NULL;
		}

		int nAccount = 0;
		int nUserOrdId = -1;
		int nInternalRe = -1;
		int nPortfolio = 0;
		int nTradingType = 0;
		int localID = pTrade->LocalOrderID;
		// client field : orderId|tradingType|portfolio
		char *ptr = pTrade->CustomCategory;
		int id = -1;
		//if (tokenizer.size() !=2 )
		if (ptr[1] == ' ' || ptr[14] == ' ')
		{
			//loggerv2::error("x1_order::anchor - cannot parse UserID field[%-*.*s]. Trade may from another application", sizeof(pTrade->UserID), sizeof(pTrade->UserID), pTrade->UserID);
			return NULL;
			//id = atoi(pTrade->BrokerID) * 100000 + atoi(pTrade->OrderRef);
		}
		else
		{
			pConnection->get_user_info(pTrade->CustomCategory, nAccount, nUserOrdId, nInternalRe, nPortfolio, nTradingType);
			id = nInternalRe;
		}


		// ok
		//x1_order* o = new x1_order(pConnection);
		o->set_id(id);

		o->set_instrument(instr);
		o->set_last_action(AtsType::OrderAction::Created);
		o->set_status(AtsType::OrderStatus::WaitMarket);

		o->set_quantity(pTrade->MatchedAmount);

		o->set_exec_quantity(pTrade->MatchedAmount);
		//o->set_price(pTrade->Price);
		//o->set_price(pTrade->InsertPrice);
		o->set_exec_price(pTrade->MatchedPrice);

		AtsType::OrderOpenClose::type oc = AtsType::OrderOpenClose::Undef;
		switch (pTrade->OpenClose)
		{
		case X1_FTDC_SPD_CLOSE:
			oc = AtsType::OrderOpenClose::Close;
			break;
		case X1_FTDC_SPD_CLOSETODAY:
			oc = AtsType::OrderOpenClose::CloseToday;
			break;

		case X1_FTDC_SPD_OPEN:
		default:
			oc = AtsType::OrderOpenClose::Open;
			break;
		}
		o->set_open_close(oc);


		switch (pTrade->BuySellType)
		{
		case X1_FTDC_SPD_BUY:
			o->set_way(AtsType::OrderWay::Buy);
			break;
		case X1_FTDC_SPD_SELL:
			o->set_way(AtsType::OrderWay::Sell);
			break;

		default:
			loggerv2::error("x1_order - anchor: unknown order_way[%c]", pTrade->BuySellType);
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

		x1_order_aux::set_spdId(o, pTrade->X1OrderID);

		auto tp = string_to_lwtp(day_clock::local_day(), (pTrade->MatchedTime));
		o->set_last_time(tp);

		o->set_user_orderid(nUserOrdId);

		auto now = get_lwtp_now();
		o->set_rebuild_time(now);

		/* std::chrono::system_clock::time_point tpnow = std::chrono::system_clock::now();
		 o->set_rebuild_time_point(tpnow);*/
		return o;
	}

	quote*  x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcQuoteRtnField* pQuote)
	{
		return nullptr;
	}

	quote*  x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcQuoteMatchRtnField* pQuote)
	{
		return nullptr;
	}

	quote*  x1_order_aux::anchor(x1_connection* pConnection, CX1FtdcQuoteCanceledRtnField* pQuote)
	{
		return nullptr;

	}

}

