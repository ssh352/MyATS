/**
* ��Ȩ����(C)2012-2016, �����ɴ���Ϣ�������޹�˾
* �ļ����ƣ�DFITCApiStruct.h
* �ļ�˵��������ӿ���������ݽӿ�
* ��ǰ�汾��1.0.15.5
* ���ߣ�XSpeed��Ŀ��
* �������ڣ�2016��3��18��
*/

#ifndef DFITCAPISTRUCT_H_
#define DFITCAPISTRUCT_H_

#include "DFITCApiDataType.h"

#define APISTRUCT

///������
struct APISTRUCT DFITCTimeOutField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
};


///���󱨵���������(��������)
struct APISTRUCT DFITCInsertOrderField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�, ��APIʹ����ά������ͬһ���Ự�в����ظ�
    DFITCInstrumentIDType               instrumentID;                 //��Լ����, ֧��Ŀǰ�����ڻ������������к�Լ������������/֣������������Լ
    DFITCPriceType                      insertPrice;                  //�����۸�, ����������Ϊ�м�ʱ�����ֶβ�������
    DFITCAmountType                     orderAmount;                  //��������
    DFITCBuySellTypeType                buySellType;                  //������־
    DFITCOpenCloseTypeType              openCloseType;                //��ƽ��־
    DFITCSpeculatorType                 speculator;                   //Ͷ������, ֧��Ͷ�����������ױ�
    DFITCInsertType                     insertType;                   //�Զ������(Ĭ��Ϊ��ͨ����)
    DFITCOrderTypeType                  orderType;                    //��������, ֧���޼� ���мۣ���������Լ��֧���м�
    DFITCOrderPropertyType              orderProperty;                //������������, ֧��None��FAK��FOK
    DFITCInstrumentTypeType             instrumentType;               //��Լ����, ��ѡֵ���ڻ�����Ȩ
    DFITCAmountType                     minMatchAmount;               //��С�ɽ���
    DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCPriceType                      profitLossPrice;              //ֹӯֹ��۸�        
};


///������������
struct APISTRUCT DFITCCancelOrderField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCSessionIDType                  sessionID;                    //�ỰID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCOrderSysIDType                 OrderSysID;                   //�������������
};


///ί����Ӧ����
struct APISTRUCT DFITCOrderRspDataRtnField
{
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCPriceType                      fee;                          //������,���ֶν����µ�ʱʹ��
    DFITCPriceType                      margin;                       //���ᱣ֤��,���ֶν����µ�ʱʹ��
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ���� 
    DFITCSessionIDType                  sessionID;                    //�ỰID  
    DFITCExchangeIDType                 exchangeID;                   //������  
    DFITCBuySellTypeType                buySellType;                  //����    
    DFITCOpenCloseTypeType              openCloseType;                //��ƽ
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSpeculatorType                 speculator;                   //Ͷ�����
    DFITCPriceType                      insertPrice;                  //ί�м�
    DFITCPriceType                      profitLossPrice;              //ֹӯֹ��۸�
    DFITCAmountType                     minMatchAmount;               //��С�ɽ���
    DFITCAmountType                     orderAmount;                  //ί������
    DFITCInsertType                     insertType;                   //�Զ������
    DFITCOrderTypeType                  orderType;                    //��������
    DFITCOrderPropertyType              orderProperty;                //��������
    DFITCClientIDType                   clientID;                     //���ױ���
};



///��ѯ�ʽ���������
struct APISTRUCT DFITCCapitalField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCCurrencyType                   currencyID;                   //���ִ���
};


///��ѯ�ֲ���������
struct APISTRUCT DFITCPositionField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCIsReturnRealizedPNLType        retRealizedPNL;               //�Ƿ񷵻�δ�ֲֺ�Լ��ƽ��ӯ��
};


///��������Լ
struct APISTRUCT DFITCExchangeInstrumentField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
};


///�û���¼��������
struct APISTRUCT DFITCUserLoginField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCPasswdType                     passwd;                       //����
    DFITCCompanyIDType                  companyID;                    //����ID
};


///�û��˳�����
struct APISTRUCT DFITCUserLogoutField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCSessionIDType                  sessionID;                    //�ỰID
};


///ί�лر�
struct APISTRUCT DFITCOrderRtnField
{
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
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
    DFITCSPDOrderIDType                 extSpdOrderID;                //���������
    DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCOrderPropertyType              orderProperty;                //��������
    DFITCAmountType                     minMatchAmount;               //��С�ɽ���
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCErrorMsgInfoType               statusMsg;                    //״̬��Ϣ
    DFITCExtOrderType                   extOrderType;                 //����������
};


///�ɽ��ر�
struct APISTRUCT DFITCMatchRtnField
{
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
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
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCProfitLossType                 dateCloseProfitLoss;          //����ƽ��ӯ��
    DFITCAmountType                     remainingAmount;              //ʣ������
};


///�����ر�
struct APISTRUCT DFITCOrderCanceledRtnField
{
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCOrderSysIDType                 OrderSysID;                   //�������  
    DFITCInstrumentIDType               instrumentID;                 //��Լ����     
    DFITCPriceType                      insertPrice;                  //�����۸�  
    DFITCBuySellTypeType                buySellType;                  //��������
    DFITCOpenCloseTypeType              openCloseType;                //��ƽ��־
    DFITCAmountType                     cancelAmount;                 //��������
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCSpeculatorType                 speculator;                   //Ͷ��
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCDateType                       canceledTime;                 //����ʱ��
    DFITCSessionIDType                  sessionID;                    //�Ự��ʶ
    DFITCOrderAnswerStatusType          orderStatus;                  //�걨���
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCAmountType                     orderAmount;                  //ί������   
    DFITCPriceType                      margin;                       //��֤��
    DFITCPriceType                      fee;                          //������
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCPriceType                      profitLossPrice;              //ֹӯֹ��۸�
    DFITCAmountType                     minMatchAmount;               //��С�ɽ���
    DFITCInsertType                     insertType;                   //�Զ������
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCErrorMsgInfoType               statusMsg;                    //״̬��Ϣ
    DFITCOrderPropertyType              orderProperty;                //������������
};


///������Ϣ
struct APISTRUCT DFITCErrorRtnField
{
    DFITCRequestIDType                  requestID;                    //����ID
    DFITCSessionIDType                  sessionID;                    //�Ự��ʶ
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCErrorIDType                    nErrorID;                     //����ID
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCErrorMsgInfoType               errorMsg;                     //������Ϣ
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
};


///�����ʽ���Ϣ
struct APISTRUCT DFITCCapitalInfoRtnField
{
    DFITCRequestIDType                  requestID;                    //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�
    DFITCEquityType                     preEquity;                    //����Ȩ��
    DFITCEquityType                     todayEquity;                  //���տͻ�Ȩ��
    DFITCProfitLossType                 closeProfitLoss;              //ƽ��ӯ��
    DFITCProfitLossType                 positionProfitLoss;           //�ֲ�ӯ��
    DFITCProfitLossType                 frozenMargin;                 //�����ʽ�
    DFITCProfitLossType                 margin;                       //�ֱֲ�֤��
    DFITCProfitLossType                 fee;                          //����������
    DFITCProfitLossType                 available;                    //�����ʽ�
    DFITCProfitLossType                 withdraw;                     //��ȡ�ʽ�
    DFITCRiskDegreeType                 riskDegree;                   //���ն�
    DFITCPremiumType                    todayPremiumIncome;           //����Ȩ��������
    DFITCPremiumType                    todayPremiumPay;              //����Ȩ���𸶳�
    DFITCPremiumType                    yesterdayPremium;             //��Ȩ�����ո�
    DFITCMarketValueType                optMarketValue;               //��Ȩ��ֵ
    DFITCProfitLossType                 floatProfitLoss;              //����ӯ��
    DFITCProfitLossType                 totFundOut;                   //�ܳ���
    DFITCProfitLossType                 totFundIn;                    //�����
    DFITCCurrencyType                   currencyID;                   //���ִ���
    DFITCProfitLossType                 mortgage;                     //��Ѻ���
    DFITCProfitLossType                 fundMortgageIn;               //����������
    DFITCProfitLossType                 fundMortgageOut;              //�����ʳ����
    DFITCProfitLossType                 fundMortgageAvailable;        //������Ѻ���
};


///���سֲ���Ϣ
struct APISTRUCT DFITCPositionInfoRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentIDType               instrumentID;                 //��Լ��
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCPriceType                      openAvgPrice;                 //���־���
    DFITCPriceType                      positionAvgPrice;             //�ֲ־���
    DFITCAmountType                     positionAmount;               //�ֲ���
    DFITCAmountType                     totalAvaiAmount;              //�ܿ���
    DFITCAmountType                     todayAvaiAmount;              //�����
    DFITCAmountType                     lastAvaiAmount;               //�����
    DFITCAmountType                     todayAmount;                  //���
    DFITCAmountType                     lastAmount;                   //���
    DFITCAmountType                     tradingAmount;                //ƽ��ҵ���
    DFITCProfitLossType                 datePositionProfitLoss;       //���гֲ�ӯ��
    DFITCProfitLossType                 dateCloseProfitLoss;          //(����)ƽ��ӯ��
    DFITCProfitLossType                 dPremium;                     //Ȩ����
    DFITCProfitLossType                 floatProfitLoss;              //����ӯ��
    DFITCProfitLossType                 dMargin;                      //ռ�ñ�֤��
    DFITCSpeculatorType                 speculator;                   //Ͷ�����
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCPriceType                      preSettlementPrice;           //������
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCAmountType                     yesterdayTradingAmount;       //ƽ��ҵ���
    DFITCProfitLossType                 optionValue;                  //��Ȩ��ֵ
};


///�û���¼������Ϣ
struct APISTRUCT DFITCUserLoginInfoRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCAccountLoginResultType         loginResult;                  //��¼���
    DFITCLocalOrderIDType               initLocalOrderID;             //��ʼ����ί�к�
    DFITCSessionIDType                  sessionID;                    //sessionID(�ỰID)
    DFITCErrorIDType                    nErrorID;                     //����ID
    DFITCErrorMsgInfoType               errorMsg;                     //������Ϣ
    DFITCDateType                       tradingDay;                   //������ yyyy.mm.dd
    DFITCTimeType                       DCEtime;                      //������ʱ��
    DFITCTimeType                       SHFETime;                     //������ʱ��
    DFITCTimeType                       CFFEXTime;                    //�н���ʱ��
    DFITCTimeType                       CZCETime;                     //֣����ʱ��
    DFITCTimeType                       INETime;                      //������ʱ��
};


///�û��˳�������Ϣ
struct APISTRUCT DFITCUserLogoutInfoRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCAccountLogoutResultType        logoutResult;                 //�˳����
    DFITCErrorIDType                    nErrorID;                     //����ID
    DFITCErrorMsgInfoType               errorMsg;                     //������Ϣ
};


///������Լ��ѯ
struct  APISTRUCT DFITCAbiInstrumentField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCExchangeIDType                 exchangeID;                   //����������
};


///������Լ������Ϣ
struct APISTRUCT DFITCAbiInstrumentRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentIDType               InstrumentID;                 //��Լ����
    DFITCInstrumentNameType             instrumentName;               //��Լ����
    DFITCPriceType                      upperLimitPrice;              //��ͣ���
    DFITCPriceType                      lowerLimitPrice;              //��ͣ���        
    DFITCPriceType                      priceTick;                    //��С�䶯��λ
};


///ָ���ĺ�Լ
struct APISTRUCT DFITCSpecificInstrumentField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               InstrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSpeculatorType                 speculator;                   //Ͷ������
};


//****************��Ȩ��չ�����Լ�ر�����*************************
///ָ���ĺ�Լ��Ϣ
struct APISTRUCT DFITCSpecificInstrumentFieldEX
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCFunctionIDType                 FunctionID;                   //����ID
    DFITCInstrumentIDType               InstrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
};
//*****************************************

///���鶩�ķ�����Ϣ
struct APISTRUCT DFITCActiveContractField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCActiveContractType             activeContract;               //��Ч��Լ
};


///��������Լ������Ϣ
struct APISTRUCT DFITCExchangeInstrumentRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCVarietyNameType                VarietyName;                  //Ʒ������
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCAmountType                     orderTopLimit;                //�޼�ί������
    DFITCAmountType                     mktOrderTopLimit;             //�м�ί������
    DFITCPriceType                      contractMultiplier;           //��Լ����
    DFITCPriceType                      minPriceFluctuation;          //��С�䶯��λ
    DFITCInstrumentMaturityType         instrumentMaturity;           //��Լ�������
    DFITCPriceType                      upperLimitPrice;              //��ͣ���
    DFITCPriceType                      lowerLimitPrice;              //��ͣ���
    DFITCPriceType                      preClosePrice;                //������
    DFITCPriceType                      preSettlementPrice;           //������
    DFITCPriceType                      settlementPrice;              //�����
    DFITCAmountType                     preOpenInterest;              //��ֲ���
    DFITCInstrumentPrefixType           instrumentPrefix;             //��Լǰ׺
    DFITCInstrumenExpirationDateType    instrumentExpiration;         //��Լ������
    DFITCInstrumentIDType               underlying;                   //��Ȩ��Ӧ�ı�ĺ�Լ����
    DFITCOptionTypeType                 optionType;                   //��Ȩ����
    DFITCPriceType                      strikePrice;                  //ִ�м۸�
    DFITCRiskDegreeType                 exchangeRiskDegree;           //��������Ȩ��ͱ��Ϸ���ϵ��
    DFITCPriceType                      minMargin;                    //��λ���֣���Ȩ��Լ��С��֤��
    DFITCAmountType                     tradeSize;                    //��Ȩ���ֵ�λ
};


///ί�в�ѯ���ݽṹ
struct APISTRUCT DFITCOrderField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬(�ݲ�֧��)
    DFITCOrderTypeType                  orderType;                    //��������(�ݲ�֧��)
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCSessionIDType                  sessionID;                    //�ỰID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
};


///�ɽ���ѯ���ݽṹ
struct APISTRUCT DFITCMatchField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCOrderTypeType                  orderType;                    //��������(�ݲ�֧��)
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
};


///ί�в�ѯ��Ӧ���ݽṹ
struct APISTRUCT DFITCOrderCommRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCOpenCloseTypeType              openClose;                    //��ƽ��־
    DFITCPriceType                      insertPrice;                  //ί�м�
    DFITCAmountType                     orderAmount;                  //ί������
    DFITCPriceType                      matchedPrice;                 //�ɽ��۸�
    DFITCAmountType                     matchedAmount;                //�ɽ�����
    DFITCAmountType                     cancelAmount;                 //��������
    DFITCInsertType                     insertType;                   //�Զ������
    DFITCSpeculatorType                 speculator;                   //Ͷ��
    DFITCDateType                       commTime;                     //ί��ʱ��
    DFITCDateType                       submitTime;                   //�걨ʱ��
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCFrontAddrType                  operStation;                  //ί�е�ַ
    DFITCAccountIDType                  accountID;                    //�ͻ���
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSessionIDType                  sessionId;                    //�ỰID
    DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
    DFITCOrderSysIDType                 OrderSysID;                   //�������
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCPriceType                      margin;                       //��֤��
    DFITCPriceType                      fee;                          //������
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCPriceType                      profitLossPrice;              //ֹ��ֹӯ��
    DFITCOrderTypeType                  orderType;                    //�������
    DFITCOrderPropertyType              orderProperty;                //��������
};


///�ɽ���ѯ������Ӧ
struct APISTRUCT DFITCMatchedRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCOpenCloseTypeType              openClose;                    //��ƽ
    DFITCPriceType                      matchedPrice;                 //�ɽ��۸�
    DFITCAmountType                     matchedAmount;                //�ɽ�����
    DFITCPriceType                      matchedMort;                  //�ɽ����
    DFITCSpeculatorType                 speculator;                   //Ͷ�����
    DFITCDateType                       matchedTime;                  //�ɽ�ʱ��
    DFITCMatchIDType                    matchedID;                    //�ɽ����
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCMatchType                      matchType;                    //�ɽ�����
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSessionIDType                  sessionId;                    //�ỰID
    DFITCReservedType                   reservedType2;                //Ԥ���ֶ�2
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCPriceType                      fee;                          //������
    DFITCOrderTypeType                  orderType;                    //��������
    DFITCOrderSysIDType                 OrderSysID;                   //�������
};


///���غ�Լ��Ϣ���ݽṹ
struct APISTRUCT DFITCInstrumentRtnField
{
    DFITCRequestIDType                  lRequestID;                   //������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCRatioType                      longMarginRatio;              //��ͷ��֤����
    DFITCRatioType                      shortMarginRatio;             //��ͷ��֤����
    DFITCPriceType                      longMarginRatioByVolume;      //��ͷ��֤���(����)
    DFITCPriceType                      shortMarginRatioByVolume;     //��ͷ��֤���(����)
    DFITCRatioType                      openFeeVolRatio;              //���������� ����������
    DFITCRatioType                      closeFeeVolRatio;             //ƽ�������� ����������
    DFITCRatioType                      closeTodayFeeVolRatio;        //ƽ�������� ����������
    DFITCRatioType                      openFeeAmtRatio;              //������������ ��������
    DFITCRatioType                      closeFeeAmtRatio;             //ƽ���������� ��������
    DFITCRatioType                      closeTodayFeeAmtRatio;        //ƽ���������� ��������
    DFITCAmountType                     orderTopLimit;                //�޼�ί������
    DFITCPriceType                      contractMultiplier;           //��Լ����
    DFITCPriceType                      minimumPriceChange;           //��С�䶯��λ
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCInstrumentMaturityType         instrumentMaturity;           //��Լ�������
    DFITCComputeModeType                computeMode;                  //���㷽ʽ             
    DFITCPriceType                      atMoneyNorm;                  //ƽֵ������
    DFITCPriceType                      upperLimitPrice;              //��ͣ���
    DFITCPriceType                      lowerLimitPrice;              //��ͣ���
    DFITCPriceType                      preClosePrice;                //������
    DFITCPriceType                      preSettlementPrice;           //������
    DFITCPriceType                      settlementPrice;              //�����
    DFITCAmountType                     preOpenInterest;              //��ֲ���
    DFITCRatioType                      optExecRatio;                 //��Ȩ����Ȩ������ �ڻ����������
    DFITCRatioType                      optExecRatioPerVol;           //��Ȩ����Ȩ������ �ڻ����������
};


///�������
struct APISTRUCT DFITCDepthMarketDataField
{
    DFITCDateType                       tradingDay;                   //������ yyyy.mm.dd
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentIDType               exchangeInstID;               //��Լ�ڽ������Ĵ���
    DFITCPriceType                      lastPrice;                    //���¼�
    DFITCPriceType                      preSettlementPrice;           //�ϴν����
    DFITCPriceType                      preClosePrice;                //������
    DFITCAmountType                     preOpenInterest;              //��ֲ���
    DFITCPriceType                      openPrice;                    //����
    DFITCPriceType                      highestPrice;                 //��߼�
    DFITCPriceType                      lowestPrice;                  //��ͼ�
    DFITCAmountType                     Volume;                       //�ɽ�����
    DFITCPriceType                      turnover;                     //�ɽ����
    DFITCAmountType                     openInterest;                 //�ֲ���
    DFITCPriceType                      closePrice;                   //������
    DFITCPriceType                      settlementPrice;              //���ν����
    DFITCPriceType                      upperLimitPrice;              //��ͣ���
    DFITCPriceType                      lowerLimitPrice;              //��ͣ���
    DFITCDeltaType                      preDelta;                     //����ʵ��
    DFITCDeltaType                      currDelta;                    //����ʵ��
    DFITCDateType                       UpdateTime;                   //����޸�ʱ��
    DFITCMilliSecType                   UpdateMillisec;               //����޸ĺ���
    DFITCPriceType                      BidPrice1;                    //�����һ
    DFITCVolumeType                     BidVolume1;                   //������һ
    DFITCPriceType                      AskPrice1;                    //������һ
    DFITCVolumeType                     AskVolume1;                   //������һ
    DFITCPriceType                      BidPrice2;                    //����۶�
    DFITCVolumeType                     BidVolume2;                   //��������
    DFITCPriceType                      AskPrice2;                    //�����۶�
    DFITCVolumeType                     AskVolume2;                   //��������
    DFITCPriceType                      BidPrice3;                    //�������
    DFITCVolumeType                     BidVolume3;                   //��������
    DFITCPriceType                      AskPrice3;                    //��������
    DFITCVolumeType                     AskVolume3;                   //��������
    DFITCPriceType                      BidPrice4;                    //�������
    DFITCVolumeType                     BidVolume4;                   //��������
    DFITCPriceType                      AskPrice4;                    //��������
    DFITCVolumeType                     AskVolume4;                   //��������
    DFITCPriceType                      BidPrice5;                    //�������
    DFITCVolumeType                     BidVolume5;                   //��������
    DFITCPriceType                      AskPrice5;                    //��������
    DFITCVolumeType                     AskVolume5;                   //��������
    DFITCPriceType                      AveragePrice;                 //���վ���
    DFITCDateType                       XSpeedTime;                   //��̨ϵͳʱ��
};

///********************************��Ȩ��չ����************************************


struct APISTRUCT DFITCMarketDataFieldEx
{
    DFITCFunctionIDType                FunctionID;                    //���ܺ�   
    DFITCDateType                      tradingDay;                    //���� 2014.04.01   
    DFITCDateType                      UpdateTime;                    //ʱ�� 11:01:29
    DFITCMilliSecType                  UpdateMillisec;                //���� 000 
    DFITCInstrumentIDType              instrumentID;                  //��Ȩ��Լ���
    DFITCExtMarketDataType             ExtMarketData;                 //��չ��������
};
///********************************************************************************


//�Զ����������
struct APISTRUCT DFITCCustomMarketDataField
{
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCVolumeType                     bidVolume1;                   //��һ��
    DFITCPriceType                      bidPrice1;                    //��һ��(�Ҽۼ۲�)
    DFITCVolumeType                     askVolume1;                   //��һ��
    DFITCPriceType                      askPrice1;                    //��һ��(�Լۼ۲�)
    DFITCPriceType                      lastPrice;                    //���¼ۼ۲�
};

///��ѯ�ֲ���ϸ
struct APISTRUCT DFITCPositionDetailField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
};


///��ѯ�ֲ���ϸ��Ӧ
struct APISTRUCT DFITCPositionDetailRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentIDType               instrumentID;                 //��Լ��
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCPriceType                      openPrice;                    //���ּ�
    DFITCAmountType                     volume;                       //����
    DFITCMatchIDType                    matchID;                      //�ɽ����
    DFITCDateType                       matchedDate;                  //�ɽ�����
    DFITCProfitLossType                 datePositionProfitLoss;       //���гֲ�ӯ��
    DFITCProfitLossType                 dateCloseProfitLoss;          //����ƽ��ӯ��
    DFITCProfitLossType                 floatProfitLoss;              //����ӯ��
    DFITCProfitLossType                 dMargin;                      //ռ�ñ�֤��
    DFITCSpeculatorType                 speculator;                   //Ͷ�����
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCPriceType                      preSettlementPrice;           //������
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCAmountType                     closeOrderVol;                //ƽ��ί������
    DFITCAmountType                     closeMatchVol;                //ƽ�ֳɽ�����
    DFITCPositionDateType               positionDateType;             //�ֲ���������
};

///��ѯ�����¼�֪ͨ
struct APISTRUCT DFITCQryTradingNoticeField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCRequestIDType                  lRequestID;                   //����ID
};

///��ѯ�����¼���Ӧ
struct APISTRUCT DFITCTradingNoticeField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCTimeType                       sendTime;                     //����ʱ��
    DFITCContentType                    fieldContent;                 //��Ϣ����
    DFITCNoticeType                     noticeType;                   //��Ϣ����
    DFITCRequestIDType                  lRequestID;                   //����ID
};

///�û��¼�֪ͨ��Ϣ
struct APISTRUCT DFITCTradingNoticeInfoField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCTimeType                       sendTime;                     //����ʱ��
    DFITCContentType                    fieldContent;                 //��Ϣ����
    DFITCNoticeType                     noticeType;                   //��Ϣ����
};


///��Լ����״̬֪ͨ��Ϣ
struct APISTRUCT DFITCInstrumentStatusField
{
    DFITCExchangeIDType                 ExchangeID;                   //����������
    DFITCInstrumentIDType               InstrumentID;                 //��Լ����
    DFITCInstrumentStatusType           InstrumentStatus;             //��Լ����״̬
    DFITCTradingSegmentSNType           TradingSegmentSN;             //���׽׶α��
    DFITCTimeType                       EnterTime;                    //���뱾״̬ʱ��
    DFITCInstStatusEnterReasonType      EnterReason;                  //���뱾״̬ԭ��
};


///�û������޸�
struct APISTRUCT DFITCResetPwdField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCPasswdType                     oldpasswd;                    //������
    DFITCPasswdType                     newpasswd;                    //������
};


///�û������޸ķ�����Ϣ
struct APISTRUCT DFITCResetPwdRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCExecStateType                  execState;                    //״̬��־
};


///�˵�ȷ��
struct APISTRUCT DFITCBillConfirmField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCDateType                       date;                         //ȷ������  ��ʽ��yyyy.mm.dd
    DFITCConfirmMarkType                confirmFlag;                  //ȷ�ϱ�־
};


///�˵�ȷ����Ӧ
struct APISTRUCT DFITCBillConfirmRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCExecStateType                  execState;                    //״̬��־
};


///���ױ����ѯ
struct APISTRUCT DFITCQryTradeCodeField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
};


///���ױ����ѯ��Ӧ
struct APISTRUCT DFITCQryTradeCodeRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
    DFITCExchangeIDType                 exchangeCode;                 //����������
    DFITCClientIDType                   clientID;                     //���ױ��� 
    DFITCClientStatusType               clientStatus;                 //���ױ���״̬
    DFITCSpeculatorType                 clientIDType;                 //���ױ�������
};


///��ӯ�����Ƿ���㵽Ȩ����
struct APISTRUCT DFITCEquityComputModeRtnField
{
    DFITCCapControlModeType             capConMode;                   //�ʽ���Ʒ�ʽ,�÷�ʽ��Ҫ�û���λ�ж�,����Ϊ�������
    DFITCPriceNoteType                  priceNote;                    //����ڻ��۸�˵��
};


///��ѯ�˵�
struct APISTRUCT DFITCQryBillField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
    DFITCDateType                       date;                         //��ѯ����   ��ʽ��yyyy.mm.dd
};


///��ѯ�˵���Ӧ
struct APISTRUCT DFITCQryBillRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
    DFITCMsgInfoType                    message;                      //������Ϣ
};


///��ѯ�����˵���Ϣȷ������
struct APISTRUCT DFITCQryBillConfirmField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
};

///��ѯ�����˵����ȷ����Ϣ��Ӧ
struct APISTRUCT DFITCQryBillConfirmRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
    DFITCDateType                       confirmDate;                  //ȷ������
    DFITCDateType                       confirmTime;                  //ȷ��ʱ��
};

///����IDȷ������
struct APISTRUCT DFITCProductField
{
    DFITCProductIDType                  productID;                    //��Ʒ���
    DFITCSoftwareVendorIDType           vendorID;                     //�����Ӧ�̱�� 
};


///����IDȷ����Ӧ
struct APISTRUCT DFITCProductRtnField
{
    DFITCProductIDType                  productID;                    //��Ʒ���
    DFITCSoftwareVendorIDType           vendorID;                     //�����Ӧ��
    DFITCProductOnlineCountType         productOnlineCount;           //��Ʒ��������
    DFITCBrokerInfoType                 brokerInfoName;               //�ڻ���˾����
    DFITCFrontIDType                    frontID;                      //ǰ�û�ID
};


///��ѯ����������
struct APISTRUCT DFITCTradingDayField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
};


///������������Ӧ
struct APISTRUCT DFITCTradingDayRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCDateType                       date;                         //������ yyyy.mm.dd
};


///ѯ��֪ͨ��������
struct APISTRUCT DFITCQuoteSubscribeField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCExchangeIDType                 exchangeID;                   //������
};

///ѯ��֪ͨ������Ӧ
struct APISTRUCT DFITCQuoteSubscribeRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCExecStateType                  subscribeFlag;                //����״̬
    DFITCExchangeIDType                 exchangeID;                   //������
};

///ѯ��֪ͨ�˶�����
struct APISTRUCT DFITCQuoteUnSubscribeField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCExchangeIDType                 exchangeID;                   //������
};

///ѯ��֪ͨ�˶���Ӧ
struct APISTRUCT DFITCQuoteUnSubscribeRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCExecStateType                  subscribeFlag;                //�˶�״̬
    DFITCExchangeIDType                 exchangeID;                   //������
};

///ѯ��֪ͨ���Ļر�
struct APISTRUCT DFITCQuoteSubscribeRtnField
{
    DFITCQuoteIDType                    quoteID;                      //ѯ�۱��
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCSourceType                     source;                       //��Դ
    DFITCDateType                       quoteTime;                    //ѯ��ʱ��
};

///ѯ��֪ͨ��ѯ����
struct APISTRUCT DFITCQryQuoteNoticeField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
};

///ѯ��֪ͨ��ѯ��Ӧ
struct APISTRUCT DFITCQryQuoteNoticeRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCQuoteIDType                    quoteID;                      //ѯ�۱��
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCSourceType                     source;                       //��Դ
    DFITCDateType                       quoteTime;                    //ѯ��ʱ��
};

///�����̱�������
struct APISTRUCT DFITCQuoteInsertField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCInsertType                     insertType;                   //�Զ������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCQuoteIDType                    quoteID;                      //ѯ�۱��
    DFITCInstrumentTypeType             instrumentType;               //��Լ���� 
    DFITCAmountType                     bOrderAmount;                 //������������
    DFITCAmountType                     sOrderAmount;                 //��������������
    DFITCPriceType                      bInsertPrice;                 //ί�м۸���         
    DFITCPriceType                      sInsertPrice;                 //ί�м۸�����          
    DFITCOpenCloseTypeType              bOpenCloseType;               //��ƽ��־����  
    DFITCOpenCloseTypeType              sOpenCloseType;               //��ƽ��־������
    DFITCSpeculatorType                 bSpeculator;                  //Ͷ�������       
    DFITCSpeculatorType                 sSpeculator;                  //Ͷ���������          
    DFITCStayTimeType                   stayTime;                     //ͣ��ʱ�䣬��֧��֣�ݡ��������������Ϊ0 
    DFITCCustomCategoryType             customCategory;               //�Զ������
};



///�����̱�����Ӧ
struct APISTRUCT DFITCQuoteRspField
{
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�              
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�                
    DFITCRequestIDType                  lRequestID;                   //����ID                    
    DFITCPriceType                      fee;                          //������(������ʹ��)            
    DFITCPriceType                      margin;                       //��֤��(������ʹ��)
    DFITCDateType                       orderTime;                    //ί��ʱ��(������ʹ��)  
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCQuoteIDType                    quoteID;                      //ѯ�۱��
    DFITCSessionIDType                  sessionID;                    //�ỰID 
    DFITCClientIDType                   clientID;                     //���ױ���
};


///�����̱����ر�
struct APISTRUCT DFITCQuoteRtnField
{
    DFITCExchangeIDType                 exchangeID;                    //������                   
    DFITCClientIDType                   clientID;                      //���ױ���  
    DFITCOrderSysIDType                 orderSysID;                    //�������
    DFITCInstrumentIDType               instrumentID;                  //��Լ����                   
    DFITCLocalOrderIDType               localOrderID;                  //����ί�к�                
    DFITCSeatCodeType                   seatCode;                      //ϯλ����                  
    DFITCOpenCloseTypeType              bOpenCloseType;                //��ƽ��־����          
    DFITCOpenCloseTypeType              sOpenCloseType;                //��ƽ��־������             
    DFITCSpeculatorType                 speculator;                    //Ͷ�����                       
    DFITCAmountType                     bOrderAmount;                  //ί����������            
    DFITCAmountType                     sOrderAmount;                  //ί������������           
    DFITCPriceType                      bInsertPrice;                  //ί�м�  ����               
    DFITCPriceType                      sInsertPrice;                  //ί�м�  ������               
    DFITCSPDOrderIDType                 spdOrderID;                    //��̨ί�к�                               
    DFITCAccountIDType                  accountID;                     //�ʽ��˺�                     
    DFITCInstrumentTypeType             instrumentType;                //��Լ����                  
    DFITCDateType                       suspendTime;                   //�ҵ�ʱ��                               
    DFITCEntrusTellerType               entrusTeller;                  //ί�й�Ա  
    DFITCOrderAnswerStatusType          orderStatus;                   //ί��״̬ 
    DFITCSessionIDType                  sessionID;                     //�ỰID
    DFITCQuoteIDType                    quoteID;                       //ѯ�۱��
    DFITCErrorMsgInfoType               errorMsg;                      //������Ϣ
    DFITCCustomCategoryType             customCategory;                //�Զ������
};



///�����̳����ر�
struct APISTRUCT DFITCQuoteCanceledRtnField
{
    DFITCExchangeIDType                 exchangeID;                    //������                   
    DFITCClientIDType                   clientID;                      //���ױ���  
    DFITCOrderSysIDType                 orderSysID;                    //�������
    DFITCInstrumentIDType               instrumentID;                  //��Լ����                   
    DFITCLocalOrderIDType               localOrderID;                  //����ί�к�                
    DFITCSeatCodeType                   seatCode;                      //ϯλ����                  
    DFITCOpenCloseTypeType              bOpenCloseType;                //��ƽ��־����          
    DFITCOpenCloseTypeType              sOpenCloseType;                //��ƽ��־������             
    DFITCSpeculatorType                 speculator;                    //Ͷ�����                       
    DFITCSPDOrderIDType                 spdOrderID;                    //��̨ί�к�                               
    DFITCAccountIDType                  accountID;                     //�ʽ��˺�                     
    DFITCEntrusTellerType               entrusTeller;                  //ί�й�Ա       
    DFITCOrderAnswerStatusType          orderStatus;                   //ί��״̬ 
    DFITCAmountType                     cancelAmount;                  //��������                              
    DFITCPriceType                      fee;                           //�ⶳ������                  
    DFITCPriceType                      margin;                        //�ⶳ��֤�� 
    DFITCSessionIDType                  sessionID;                     //�ỰID
    DFITCBuySellTypeType                buySellType;                   //������־
    DFITCQuoteIDType                    quoteID;                       //ѯ�۱��
    DFITCDateType                       canceledTime;                  //����ʱ��
    DFITCCustomCategoryType             customCategory;                //�Զ������
};



///�����̳ɽ��ر�
struct APISTRUCT DFITCQuoteMatchRtnField
{
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCClientIDType                   clientID;                     //���ױ���
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCSeatCodeType                   seatCode;                     //ϯλ����
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCOpenCloseTypeType              openCloseType;                //��ƽ��־
    DFITCSpeculatorType                 speculator;                   //Ͷ�����  
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCOrderSysIDType                 OrderSysID;                   //�������(�������������)
    DFITCMatchIDType                    matchID;                      //�ɽ����
    DFITCAmountType                     matchedAmount;                //�ɽ�����
    DFITCPriceType                      matchedPrice;                 //�ɽ��۸�
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCPriceType                      turnover;                     //�ɽ����
    DFITCEntrusTellerType               entrusTeller;                 //ί�й�Ա
    DFITCDateType                       matchedTime;                  //�ɽ�ʱ��
    DFITCFeeType                        fee;                          //������
    DFITCPriceType                      insertPrice;                  //ί�м۸�
    DFITCAmountType                     orderAmount;                  //ί������
    DFITCOrderAnswerStatusType          orderStatus;                  //�걨���
    DFITCPriceType                      margin;                       //����Ϊ��֤��,ƽ��Ϊ�ⶳ��֤��
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCAmountType                     closeTodayAmount;             //ƽ������
    DFITCPriceType                      closePrice;                   //ƽ�ֽ��
    DFITCPriceType                      closeTodayPrice;              //ƽ����
    DFITCAdjustmentInfoType             adjustmentInfo;               //��ϻ�����ı�֤�������Ϣ,��ʽ:[��Լ����,������־,Ͷ�����,�������;] 
    DFITCPriceType                      frozenCapita;                 //�ɽ��ⶳί�ж�����ʽ�
    DFITCProfitLossType                 dateCloseProfitLoss;          //����ƽ��ӯ��
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSessionIDType                  sessionID;                    //�Ự��ʶ
    DFITCLargeMarginDirectType          largeMarginDirect;            //��߱�֤����
    DFITCQuoteIDType                    quoteID;                      //ѯ�۱��
    DFITCCustomCategoryType             customCategory;               //�Զ������
};


///������������
struct APISTRUCT DFITCCancelAllOrderField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCExchangeIDType                 exchangeID;                   //����������(Ŀǰֻ֧�ִ�����)
};

///����������Ӧ
struct APISTRUCT DFITCCancelAllOrderRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
};

///ѯ������
struct APISTRUCT DFITCForQuoteField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
};

///ѯ��������Ӧ
struct APISTRUCT DFITCForQuoteRspField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCDateType                       commTime;                     //ί��ʱ��
};

///ѯ�ۻر�
struct APISTRUCT DFITCForQuoteRtnField
{
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCSessionIDType                  sessionID;                    //�ỰID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
};

///�����̱���ί�в�ѯ
struct APISTRUCT DFITCQuoteOrderField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
};

///�����̱��۲�ѯ��Ӧ
struct APISTRUCT DFITCQuoteOrderRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCPriceType                      margin;                       //��֤��
    DFITCPriceType                      fee;                          //������
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCAccountIDType                  accountID;                    //�ͻ���
    DFITCDateType                       commTime;                     //ί��ʱ��
    DFITCDateType                       submitTime;                   //�걨ʱ��
    DFITCExchangeIDType                 exchangeID;                   //������ID
    DFITCAmountType                     bOrderAmount;                 //ί����������
    DFITCAmountType                     bMatchedAmount;               //�ɽ���������
    DFITCAmountType                     bCancelAmount;                //������������
    DFITCPriceType                      bInsertPrice;                 //ί�м۸���
    DFITCPriceType                      bMatchedPrice;                //�ɽ��۸���
    DFITCOpenCloseTypeType              bOpenCloseType;               //��ƽ��־����
    DFITCAmountType                     sOrderAmount;                 //ί������������
    DFITCAmountType                     sMatchedAmount;               //�ɽ�����������
    DFITCAmountType                     sCancelAmount;                //��������������
    DFITCPriceType                      sInsertPrice;                 //ί�м۸�����
    DFITCPriceType                      sMatchedPrice;                //�ɽ��۸�����
    DFITCOpenCloseTypeType              sOpenCloseType;               //��ƽ��־������
    DFITCFrontAddrType                  operStation;                  //����վ��
    DFITCSessionIDType                  sessionID;                    //�ỰID
    DFITCQuoteIDType                    quoteID;                      //ѯ�۱��
    DFITCCustomCategoryType             customCategory;               //�Զ������
};

///��ѯת������
struct APISTRUCT DFITCQryTransferBankField
{
    DFITCAccountIDType                  accountID;                     //�ʽ��˺�
    DFITCBankIDType                     bankID;                        //���д���
    DFITCRequestIDType                  lRequestID;                    //����ID
};

///ת��������Ӧ
struct APISTRUCT DFITCTransferBankRspField
{
    DFITCAccountIDType                  accountID;                     //�ͻ���
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCCurrencyType                   currency;                      //����
    DFITCDateType                       registDate;                    //�Ǽ�����
    DFITCRequestIDType                  lRequestID;                    //����ID
};

///��ѯת����ˮ
struct APISTRUCT DFITCQryTransferSerialField
{
    DFITCAccountIDType                  accountID;                     //�ʽ��˺�
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCRequestIDType                  lRequestID;                    //����ID
};

///ת����ˮ��Ӧ
struct APISTRUCT DFITCTransferSerialRspField
{
    DFITCAccountIDType                  accountID;                     //�ʽ��˺�
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCCurrencyType                   currency;                      //���ִ���
    DFITCApplyNumberType                applyNum;                      //�����
    DFITCTransferType                   type;                          //ת��ҵ�����
    DFITCPriceType                      tradeAmount;                   //ת�˽��
    DFITCPriceType                      curFutAccountFund;             //�����ʽ����
    DFITCSerialType                     bankSerialNum;                 //������ˮ��
    DFITCTimeType                       reqTransferTime;               //����ת��ʱ��
    DFITCTimeType                       dealTransferTime;              //ת�˳ɹ�ʱ��
    DFITCProcResultType                 procResult;                    //ת�˴�����
    DFITCRequestIDType                  lRequestID;                    //����ID
};

///�ʽ�ת��������Ϣ
struct APISTRUCT DFITCReqTransferField
{
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCPasswdType                     bankPassword;                  //��������
    DFITCAccountIDType                  accountID;                     //Ͷ�����˺�
    DFITCPasswdType                     password;                      //�ڻ�����
    DFITCCurrencyType                   currency;                      //���ִ���
    DFITCPriceType                      tradeAmount;                   //ת�˽��
    DFITCRequestIDType                  lRequestID;                    //����ID
};

///�ʽ�ת����Ӧ��Ϣ
struct APISTRUCT DFITCTransferRspField
{
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCAccountIDType                  accountID;                     //Ͷ�����˺�
    DFITCPriceType                      tradeAmount;                   //ת�˽��
    DFITCApplyNumberType                applyNumber;                   //ת�������
    DFITCRequestIDType                  lRequestID;                    //����ID
};

///�ʽ�ת��֪ͨ��Ϣ
struct APISTRUCT DFITCTransferRtnField
{
    DFITCAccountIDType                  accountID;                     //Ͷ�����˺�
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCTransferType                   type;                          //ת�����
    DFITCPriceType                      tradeAmount;                   //ת�˽��
    DFITCSerialType                     bankSerialNum;                 //������ˮ��
    DFITCApplyNumberType                applyNumber;                   //ת�������
    DFITCSessionIDType                  sessionID;                     //�ỰID
};

///���л���ϯ����������֪ͨ
struct APISTRUCT DFITCRepealRtnField
{
    DFITCAccountIDType                  accountID;                     //Ͷ�����˺�
    DFITCBankIDType                     bankID;                        //���д���
    DFITCBankAccountType                bankAccount;                   //�����˺�
    DFITCTransferType                   type;                          //ת�����
    DFITCPriceType                      tradeAmount;                   //ת�˽��
    DFITCSerialType                     bankSerialNum;                 //������ˮ��
    DFITCSerialType                     repealSerial;                  //��������ˮ��
};

///����״̬��ѯ����
struct APISTRUCT DFITCQryExchangeStatusField
{
    DFITCRequestIDType                  lRequestID;                    //����ID
    DFITCExchangeIDType                 exchangeID;                    //����������
};

///������״̬��ѯ��Ӧ
struct APISTRUCT DFITCExchangeStatusRspField
{
    DFITCRequestIDType                  lRequestID;                    //����ID
    DFITCExchangeStatusType             exchangeStatus;                //������״̬
    DFITCExchangeIDType                 exchangeID;                    //����������
};

///������״̬֪ͨ
struct APISTRUCT DFITCExchangeStatusRtnField
{
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeStatusType             exchangeStatus;               //������״̬
    DFITCTimeType                       enterTime;                    //���뱾״̬ʱ��
    DFITCInstStatusEnterReasonType      enterReason;                  //���뱾״̬ԭ��
};

///�����ѯ����
struct DFITCQryDepthMarketDataField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������
};

///��ѯѯ������
struct APISTRUCT DFITCQryForQuoteField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������
};

///��ѯѯ����Ӧ
struct APISTRUCT DFITCQryForQuoteRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCExchangeIDType                 exchangeID;                   //������
    DFITCDateType                       SuspendTime;                  //����ʱ��
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
};

///��ѯ����
struct APISTRUCT DFITCQryExchangeRateField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCCurrencyType                   fromCurrencyID;               //Դ����
    DFITCCurrencyType                   toCurrencyID;                 //Ŀ�����
};

///��ѯ������Ӧ
struct APISTRUCT DFITCExchangeRateField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCCurrencyType                   fromCurrencyID;               //Դ����
    DFITCCurrencyUnitType               fromCurrencyUnit;             //Դ���ֵ�λ����
    DFITCCurrencyType                   toCurrencyID;                 //Ŀ�����
    DFITCExchangeRateType               exchangeRate;                 //����
};

///���鴥������
struct APISTRUCT DFITCPricesTriggerField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCExtOrderPriceTypeType      extOrderPriceType;  //��������������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCCompareFlagType            compareFlag;        //�Ƚϱ�־(�۸�)
    DFITCPriceType                  comparePrice;       //�����۸�(��������Ϊ����۸���ڻ�С�ڵ��ڴ����۸�)
    DFITCPriceReferenceType         priceReference;     //�۸����
    DFITCBreakDownTimesType         breakDownTimes;     //��������
    DFITCDateType                   validate;           //��Ч����(yyyy.mm.dd,�ݲ�֧��)
    DFITCFrozenTypeType             frozenType;         //�Ƿ񶳽��ʽ�����
    DFITCInstrumentTypeType         instrumentType;     //��Լ����
    DFITCAmountType                 limitAmount;        //��������
    DFITCCompareFlagType            qtyCmpFlag;         //�Ƚϱ�־(����)
    DFITCTriggerType                triggerType;        //��������
    DFITCBreakDownTypeType          breakDownType;      //��������
};

///���鴥�����޸�����
struct APISTRUCT DFITCPricesTriggerModifyField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCPriceType                  comparePrice;       //�����۸�
    DFITCBreakDownTimesType         breakDownTimes;     //��������
    DFITCDateType                   validate;           //��Ч����(yyyy.mm.dd)
};

///�ɽ���������
struct APISTRUCT DFITCMatchTriggerField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCInstrumentIDType           trigInstrumentID;   //������Լ����
    DFITCPriceType                  trigInsertPrice;    //ί�м۸�
    DFITCAmountType                 trigOrderAmount;    //ί������
    DFITCSpeculatorType             trigSpeculator;     //Ͷ������
    DFITCBuySellTypeType            trigBuySellType;    //������־
    DFITCOpenCloseTypeType          trigOpenCloseType;  //��ƽ��־
    DFITCFrozenTypeType             frozentype;         //��������
};

///�ɽ������޸�
struct APISTRUCT DFITCMatchTriggerModifyField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCInstrumentIDType           trigInstrumentID;   //������Լ����
    DFITCPriceType                  trigInsertPrice;    //ί�м۸�
    DFITCAmountType                 trigOrderAmount;    //ί������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
};

///ֹ��ֹӯ������
struct APISTRUCT DFITCProfitLossField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCPriceType                  comparePriceFall;   //�µ������۲�
    DFITCPriceType                  insertPriceFall;    //�µ�ί�м۲�
    DFITCAddPriceTimesType          addPriceTimesFall;  //�µ�׷�۴���
    DFITCOrderTypeType              orderTypeFall;      //�µ���������
    DFITCPriceType                  comparePriceRise;   //���Ǵ����۲�
    DFITCPriceType                  insertPriceRise;    //����ί�м۲�
    DFITCAddPriceTimesType          addPriceTimesRise;  //����׷�۴���
    DFITCOrderTypeType              orderTypeRise;      //���Ǳ�������
    DFITCDateType                   validate;           //��Ч����
};

///ֹ��ֹӯ�޸�����
struct APISTRUCT DFITCProfitLossModifyField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCPriceType                  comparePriceFall;   //�µ������۲�
    DFITCPriceType                  insertPriceFall;    //�µ�ί�м۲�
    DFITCAddPriceTimesType          addPriceTimesFall;  //�µ�׷�۴���
    DFITCPriceType                  comparePriceRise;   //���Ǵ����۲�
    DFITCPriceType                  insertPriceRise;    //����ί�м۲�
    DFITCAddPriceTimesType          addPriceTimesRise;  //����׷�۴���
};

///��ϴ�����������
struct APISTRUCT DFITCArbitrageField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����1
    DFITCInstrumentIDType           instrumentID2;      //��Լ����2
    DFITCDateType                   validate;           //��Ч����
    DFITCPriceType                  insertPrice;        //ƫ����
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCPriceType                  insertPrice2;       //ƫ����2
    DFITCAmountType                 orderAmount2;       //ί������2
    DFITCSpeculatorType             speculator2;        //Ͷ������2
    DFITCBuySellTypeType            buySellType2;       //������־2
    DFITCOpenCloseTypeType          openCloseType2;     //��ƽ��־2
    DFITCPriceType                  priceMargin;        //�۲�
    DFITCMinEntryType               minEntry;           //��С��������
    DFITCSplitContinueType          splitContinue;      //�����Ƿ����
    DFITCPriceType                  deviatePrice;       //ƫ����
    DFITCArbitrageTypeType          arbitrageType;      //��������
};

///��ϴ������޸�����
struct APISTRUCT DFITCArbitrageModifyField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCPriceType                  insertPrice;        //ƫ����1
    DFITCPriceType                  insertPrice2;       //ƫ����2
    DFITCPriceType                  priceMargin;        //�۲�
    DFITCMinEntryType               minEntry;           //��С��������
    DFITCSplitContinueType          splitContinue;      //�����Ƿ����
    DFITCPriceType                  deviatePrice;       //ƫ����
};

///��������ѯ����
struct APISTRUCT DFITCQryExtOrderField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID
    DFITCSPDOrderIDType             extSpdOrderID;      //���������(�ݲ�֧��)
    DFITCExtOrderType               extOrderType;       //����������
};

///��������������
struct APISTRUCT DFITCCancelExtOrderField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCExtOrderType               extOrderType;       //����������
};


///���鴥�����ر�
struct APISTRUCT DFITCPricesTriggerRtnField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCExtOrderPriceTypeType      extOrderPriceType;  //��������������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCCompareFlagType            compareFlag;        //�Ƚϱ�־
    DFITCPriceType                  comparePrice;       //�����۸�
    DFITCPriceReferenceType         priceReference;     //�۸����
    DFITCBreakDownTimesType         breakDownTimes;     //��������
    DFITCDateType                   modifiedtime;       //�޸�ʱ��
    DFITCDateType                   commTime;           //ί��ʱ��
    DFITCSPDOrderIDType             spdOrderID;         //��̨ί�к�
    DFITCDateType                   canceledTime;       //����ʱ��
    DFITCFrozenTypeType             frozentype;         //��������
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCProfitLossType             frozenMargin;       //�����ʽ�
    DFITCAmountType                 frozenAmount;       //��������
    DFITCOrderAnswerStatusType      orderStatus;        //ί��״̬
    DFITCSessionIDType              sessionID;          //�ỰID
    DFITCErrorMsgInfoType           statusMsg;          //״̬��Ϣ
    DFITCAmountType                 limitAmount;        //��������
    DFITCCompareFlagType            qtyCmpFlag;         //�Ƚϱ�־(����)
    DFITCTriggerType                triggerType;        //��������
    DFITCBreakDownTypeType          breakDownType;      //��������
    DFITCExtOrderTriggerStatusType  orderTriggerStatus; //����������״̬
};


///��ѯ���鴥��ί��
struct APISTRUCT DFITCQryPricesTriggerField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCOrderTypeType              orderType;          //��������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCCompareFlagType            compareFlag;        //�Ƚϱ�־
    DFITCPriceType                  comparePrice;       //�����۸�
    DFITCPriceReferenceType         priceReference;     //�۸����
    DFITCBreakDownTimesType         breakDownTimes;     //��������
    DFITCDateType                   validate;           //��Ч����
    DFITCDateType                   modifiedtime;       //�޸�ʱ��
    DFITCDateType                   commTime;           //ί��ʱ��
    DFITCDateType                   commdate;           //ί������
    DFITCSPDOrderIDType             spdOrderID;         //��̨ί�к�
    DFITCDateType                   canceledTime;       //����ʱ��
    DFITCFrozenTypeType             frozentype;         //��������
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCExtOrderType               extOrderType;       //����������
    DFITCProfitLossType             frozenMargin;       //�����ʽ�
    DFITCAmountType                 frozenAmount;       //��������
    DFITCOrderAnswerStatusType      orderStatus;        //ί��״̬
    DFITCExtOrderTriggerStatusType  orderTriggerStatus; //����������״̬
    DFITCAmountType                 limitAmount;        //��������
    DFITCCompareFlagType            qtyCmpFlag;         //�Ƚϱ�־(����)
    DFITCTriggerType                triggerType;        //��������
    DFITCBreakDownTypeType          breakDownType;      //��������

};



///��ѯ�ɽ�����ί��
struct APISTRUCT DFITCQryMatchTriggerField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCAmountType                 matchedAmount;      //�ɽ�����
    DFITCAmountType                 cancelAmount;       //��������
    DFITCSPDOrderIDType             spdOrderID;         //��̨ί�к�
    DFITCInstrumentIDType           trigInstrumentID;   //��Լ����
    DFITCBuySellTypeType            trigBuySellType;    //������־
    DFITCOpenCloseTypeType          trigOpenCloseType;  //��ƽ��־
    DFITCSpeculatorType             trigSpeculator;     //Ͷ������
    DFITCPriceType                  trigInsertPrice;    //ί�м۸�
    DFITCAmountType                 trigOrderAmount;    //ί������ 
    DFITCSPDOrderIDType             trigSpdOrderID;     //��̨ί�к�
    DFITCDateType                   commdate;           //ί������
    DFITCDateType                   commTime;           //ί��ʱ��
    DFITCDateType                   canceledTime;       //����ʱ��
    DFITCDateType                   modifiedtime;       //�޸�ʱ��
    DFITCOrderTypeType              orderType;          //��������
    DFITCProfitLossType             frozenMargin;       //�����ʽ�
    DFITCAmountType                 frozenAmount;       //��������
    DFITCCompareFlagType            compareFlag;        //�Ƚϱ�־
    DFITCPriceType                  comparePrice;       //�����۸�
    DFITCPriceReferenceType         priceReference;     //�۸����
    DFITCBreakDownTimesType         breakDownTimes;     //��������
    DFITCDateType                   validate;           //��Ч����
    DFITCSPDOrderIDType             trigExtSpdOrderID;  //���������
    DFITCExtOrderType               extOrderType;       //����������
    DFITCOrderAnswerStatusType      orderStatus;        //ί��״̬
};

///��ѯֹ��ֹӯ����ί��
struct APISTRUCT DFITCProfitLossRspField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             spdOrderID;         //��̨ί�к�
    DFITCSPDOrderIDType             trigSpdOrderID;     //��̨ί�к�
    DFITCSPDOrderIDType             trigExtSpdOrderID;  //���������
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCPriceType                  insertPrice;        //ί�м۸�
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCPriceType                  comparePriceFall;   //�µ������۲�
    DFITCPriceType                  insertPriceFall;    //�µ�ί�м۲�
    DFITCAddPriceTimesType          addPriceTimesFall;  //�µ�׷�۴���
    DFITCOrderTypeType              orderTypeFall;      //�µ���������
    DFITCPriceType                  comparePriceRise;   //���Ǵ����۲�
    DFITCPriceType                  insertPriceRise;    //����ί�м۲�
    DFITCAddPriceTimesType          addPriceTimesRise;  //����׷�۴���
    DFITCOrderTypeType              orderTypeRise;      //���Ǳ�������
    DFITCDateType                   validate;           //��Ч����
    DFITCDateType                   commTime;           //ί��ʱ��
    DFITCDateType                   canceledTime;       //����ʱ��
    DFITCDateType                   modifiedtime;       //�޸�ʱ��
    DFITCOrderAnswerStatusType      orderStatus;        //ί��״̬
};

///��ѯ������������ί��
struct APISTRUCT DFITCArbitrageRspField
{
    DFITCAccountIDType              accountID;          //�ʽ��˻�
    DFITCRequestIDType              lRequestID;         //����ID 
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCInstrumentIDType           instrumentID;       //��Լ����
    DFITCInstrumentIDType           instrumentID2;      //��Լ����
    DFITCInstrumentIDType           instrumentID3;      //��Լ����
    DFITCPriceType                  insertPrice;        //ƫ����
    DFITCAmountType                 orderAmount;        //ί������
    DFITCSpeculatorType             speculator;         //Ͷ������
    DFITCBuySellTypeType            buySellType;        //������־
    DFITCOpenCloseTypeType          openCloseType;      //��ƽ��־
    DFITCPriceType                  insertPrice2;       //ƫ����2
    DFITCAmountType                 orderAmount2;       //ί������2
    DFITCSpeculatorType             speculator2;        //Ͷ������2
    DFITCBuySellTypeType            buySellType2;       //������־2
    DFITCOpenCloseTypeType          openCloseType2;     //��ƽ��־2
    DFITCDateType                   modifiedtime;       //�޸�ʱ��
    DFITCDateType                   commTime;           //ί��ʱ��
    DFITCDateType                   commdate;           //ί������
    DFITCPriceType                  priceMargin;        //�۲�
    DFITCArbitrageTypeType          arbitrageType;      //��������
    DFITCMinEntryType               minEntry;           //��С��������
    DFITCSplitContinueType          splitContinue;      //�����Ƿ����
    DFITCPriceType                  deviatePrice;       //ƫ����
    DFITCOrderAnswerStatusType      orderStatus;        //ί��״̬
};


///������ί����Ӧ����
struct APISTRUCT DFITCExtOrderRspDataField
{
    DFITCLocalOrderIDType           localOrderID;       //����ί�к�
    DFITCSPDOrderIDType             spdOrderID;         //��̨ί�к�
    DFITCSPDOrderIDType             extSpdOrderID;      //���������
    DFITCOrderAnswerStatusType      orderStatus;        //ί��״̬
    DFITCRequestIDType              lRequestID;         //����ID
    DFITCPriceType                  frozenMargin;       //�����ʽ�(���µ�ʹ��)
    DFITCExtOrderType               extOrderType;       //����������
    DFITCAccountIDType              accountID;          //�ʽ��˻�
};

///������������������ر�
struct APISTRUCT DFITCOrderCancelErrField
{
    DFITCAccountIDType                  accountID;                    //�ʽ��˺�
    DFITCSessionIDType                  sessionID;                    //�ỰID
    DFITCLocalOrderIDType               localOrderID;                 //����ί�к�
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCPriceType                      insertPrice;                  //ί�м�
    DFITCAmountType                     orderAmount;                  //ί������
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCOpenCloseTypeType              openCloseType;                //��ƽ
    DFITCSpeculatorType                 speculator;                   //Ͷ�����
    DFITCInsertType                     insertType;                   //�Զ������
    DFITCOrderTypeType                  orderType;                    //��������
    DFITCOrderPropertyType              orderProperty;                //��������
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCAmountType                     minMatchAmount;               //��С�ɽ���
    DFITCCustomCategoryType             customCategory;               //�Զ������
    DFITCPriceType                      profitLossPrice;              //ֹӯֹ��۸�
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCOrderAnswerStatusType          orderStatus;                  //ί��״̬
    DFITCExchangeIDType                 exchangeID;                   //������ 
    DFITCOrderSysIDType                 OrderSysID;                   //�������
    DFITCErrorMsgInfoType               errorMsg;                     //������Ϣ
    DFITCReservedType                   reservedType;                 //Ԥ���ֶ�
};

///�����̳�������ر�
struct APISTRUCT DFITCQuoteCancelErrField
{
    DFITCAccountIDType                  accountID;                     //�ʽ��˺�
    DFITCSessionIDType                  sessionID;                     //�ỰID
    DFITCLocalOrderIDType               localOrderID;                  //����ί�к�
    DFITCInstrumentIDType               instrumentID;                  //��Լ����
    DFITCInstrumentTypeType             instrumentType;                //��Լ����
    DFITCSPDOrderIDType                 spdOrderID;                    //��̨ί�к�
    DFITCOrderAnswerStatusType          orderStatus;                   //ί��״̬
    DFITCCustomCategoryType             customCategory;                //�Զ������
    DFITCQuoteIDType                    quoteID;                       //ѯ�۱��
    DFITCExchangeIDType                 exchangeID;                    //������  
    DFITCOrderSysIDType                 OrderSysID;                    //�������
    DFITCErrorMsgInfoType               errorMsg;                      //������Ϣ
    DFITCReservedType                   reservedType;                  //Ԥ���ֶ�
};

///��ѯ��ϳֲ���ϸ
struct APISTRUCT DFITCArbitrageCombineDetailField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��˻�ID
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
};


///��ѯ��ϳֲ���Ӧ
struct APISTRUCT DFITCArbitrageCombineDetailRtnField
{
    DFITCRequestIDType                  lRequestID;                   //����ID
    DFITCAccountIDType                  accountID;                    //�ʽ��ʺ�ID
    DFITCExchangeIDType                 exchangeID;                   //����������
    DFITCInstrumentIDType               instrumentID;                 //��Լ����
    DFITCBuySellTypeType                buySellType;                  //����
    DFITCPriceType                      openPrice;                    //���ּ�
    DFITCInstrumentIDType               comTradeID;                   //��ϱ��
    DFITCDateType                       openDate;                     //��������
    DFITCMatchIDType                    matchID;                      //�ɽ����/������
    DFITCAmountType                     volume;                       //�ֲ�����
    DFITCAmountType                     tradingAmount;                //�ҵ�����
    DFITCProfitLossType                 margin;                       //��֤��
    DFITCSpeculatorType                 speculator;                   //Ͷ�����
    DFITCInstrumentTypeType             instrumentType;               //��Լ����
    DFITCSPDOrderIDType                 spdOrderID;                   //��̨ί�к�
    DFITCCustomCategoryType             customCategory;               //�Զ������
};
#endif//DFITCAPISTRUCT_H_
