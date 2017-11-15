/////////////////////////////////////////////////////////////////////////
///�Ϻ�ֱ����һ��APIϵͳ
///@company �Ϻ��ڻ���Ϣ�������޹�˾
///@file ShZdFutureUserApiStruct.h
///@brief �����˿ͻ��˽ӿ�ʹ�õ�ҵ�����ݽṹ
///@history 
///20161106	smithxiang		�������ļ�
/////////////////////////////////////////////////////////////////////////

#if !defined(TSHZD_TRADESTRUCT_H)
#define TSHZD_TRADESTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ShZdFutureUserApiDataType.h"
#pragma pack(push, 1)
///ֱ����Ϣ�ַ�
struct CTShZdDisseminationField
{
	///����ϵ�к�	
	TShZdSequenceSeriesType	SequenceSeries;
	///���к�
	TShZdSequenceNoType	SequenceNo;
};

///ֱ���û���¼����
struct CTShZdReqUserLoginField
{
	///������
	TShZdDateType	TradingDay;
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///�û�����  ֱ�������д
	TShZdUserIDType	UserID;
	///����  ֱ�������д
	TShZdPasswordType	Password;
	///�û��˲�Ʒ��Ϣ
	TShZdProductInfoType	UserProductInfo;
	///�ӿڶ˲�Ʒ��Ϣ
	TShZdProductInfoType	InterfaceProductInfo;
	///Э����Ϣ
	TShZdProtocolInfoType	ProtocolInfo;
	///Mac��ַ
	TShZdMacAddressType	MacAddress;
	///��̬����
	TShZdPasswordType	OneTimePassword;
	///�ն�IP��ַ  ֱ�������д
	TShZdIPAddressType	ClientIPAddress;
};

///ֱ���û���¼Ӧ��
struct CTShZdRspUserLoginField
{
	///������ ֱ��
	TShZdDateType	TradingDay;
	///��¼�ɹ�ʱ��
	TShZdTimeType	LoginTime;	
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
	///����ϵͳ����  ֱ��
	TShZdSystemNameType	SystemName;	
	///Ͷ�����ʺ�  �ʽ��˺�  ֱ��
	TShZdAccountIDType	AccountID;
	///���֣��˺ŵı���  ֱ��
	TShZdCurrencyNoType CurrencyNo;
	///�û����� ֱ��
	TShZdUserNameType UserName;	
};

///ֱ���û��ǳ�����
struct CTShZdUserLogoutField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
};

///ֱ����Ӧ��Ϣ
struct CTShZdRspInfoField
{
	///�������  ֱ��
	TShZdErrorIDType	ErrorID;
	///������Ϣ  ֱ��
	TShZdErrorMsgType	ErrorMsg;
};

///ֱ�ｻ����
struct CTShZdExchangeField
{
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///����������  ֱ��
	TShZdExchangeNameType	ExchangeName;
	///����������
	TShZdExchangePropertyType	ExchangeProperty;
};

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

///ֱ���ʽ��˻�
struct CTShZdTradingAccountField
{
	///�û�����  ֱ��
	TShZdUserIDType	UserID;	
	///�ʽ��˺�  ֱ��
	TShZdAccountIDType	AccountID;
	///�����  ֱ��
	TShZdMoneyType	PreMortgage;
	///��Ȩ�� ֱ��
	TShZdMoneyType	PreCredit;
	///���� ֱ��
	TShZdMoneyType	PreDeposit;
	///��Ȩ��  ֱ��
	TShZdMoneyType	CurrBalance;
	///����� ֱ��
	TShZdMoneyType	CurrUse;
	///���� ֱ��
	TShZdMoneyType	CurrDeposit;	
	///�����   ֱ��
	TShZdMoneyType	Deposit;
	///������   ֱ��
	TShZdMoneyType	Withdraw;
	///����ı�֤��  ֱ��
	TShZdMoneyType	FrozenMargin;	
	///��ǰ��֤���ܶ�  ֱ��
	TShZdMoneyType	CurrMargin;	
	///������  ֱ��
	TShZdMoneyType	Commission;
	///ƽ��ӯ��  ֱ��
	TShZdMoneyType	CloseProfit;
	///��ӯ������ӯ���� ֱ��
	TShZdMoneyType	NetProfit;
	///δ����ƽӯ   ֱ��
	TShZdMoneyType	UnCloseProfit;
	///δ����ƽӯ  ֱ��
	TShZdMoneyType	UnFrozenCloseProfit;	
	///������
	TShZdDateType	TradingDay;	
	///���ö��  ֱ��
	TShZdMoneyType	Credit;
	///�����ʽ�  ֱ��
	TShZdMoneyType	Mortgage;
	///ά�ֱ�֤��  ֱ��
	TShZdMoneyType	KeepMargin;
	///��Ȩ����  ֱ��
	TShZdMoneyType	RoyaltyMargin;
	///��ʼ�ʽ�  ֱ��
	TShZdMoneyType	FirstInitMargin;
	///ӯ����  ֱ��
	TShZdMoneyType	ProfitRatio;
	///������  ֱ��
	TShZdMoneyType	RiskRatio;
	///���֣��˺ŵı���  ֱ��
	TShZdCurrencyNoType CurrencyNo;
	///��������ҵĻ���  ֱ��
	TShZdMoneyType	CurrencyRatio;
};

///ֱ��Ͷ���ֲ߳�
struct CTShZdInvestorPositionField
{
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///��Լ����  ֱ��
	TShZdInstrumentIDType	InstrumentID;	
	///�ʽ����  ֱ��
	TShZdInvestorIDType	InvestorID;	
	///������  ֱ��
	TShZdVolumeType	HoldBuyVolume;
	///���򿪾���  ֱ��
	TShZdMoneyType	HoldBuyOpenPrice;
	///������� ֱ��
	TShZdMoneyType	HoldBuyPrice;
	///������  ֱ��
	TShZdVolumeType	HoldSaleVolume;
	//����������  ֱ��
	TShZdMoneyType	HoldSaleOpenPrice;
	///��������  ֱ��
	TShZdMoneyType	HoldSalePrice;
	///����֤��  ֱ��
	TShZdMoneyType	HoldBuyAmount;
	///������֤��  ֱ��
	TShZdMoneyType	HoldSaleAmount;
	///������  ֱ��
	TShZdVolumeType	OpenVolume;
	///�ɽ���  ֱ��
	TShZdVolumeType	FilledVolume;	
	///�ɽ�����  ֱ��
	TShZdMoneyType	FilledAmount;	
	///������  ֱ��
	TShZdMoneyType	Commission;	
	///�ֲ�ӯ��  ֱ��
	TShZdMoneyType	PositionProfit;	
	///������  ֱ��
	TShZdDateType	TradingDay;	
};

///�Ϻ�ֱ������ɽ�����  
struct CTShZdFilledDataField
{
	///������  ֱ��  
	TShZdDateType	TradingDay;
	///��Լ����   ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���  ֱ��
	TShZdExchangeInstIDType	ExchangeInstID;
	///�ɽ���  ֱ��
	TShZdPriceType	LastPrice;
	///�ɽ�����  ֱ��
	TShZdVolumeType	Volume;
	///�ɽ�������  ֱ��
	TShZdVolumeType	FilledVolume;
	///����޸�ʱ��  ֱ��
	TShZdTimeType	UpdateTime;
	///����޸ĺ���  ֱ��
	TShZdMillisecType	UpdateMillisec;	
};

///�������
struct CTShZdDepthMarketDataField
{
	///������  ֱ��
	TShZdDateType	TradingDay;
	///��Լ����  ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///����������   ֱ��
	TShZdExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���  
	TShZdExchangeInstIDType	ExchangeInstID;
	///���¼�  ֱ��
	TShZdPriceType	LastPrice;
	///�ϴν����  ֱ��
	TShZdPriceType	PreSettlementPrice;
	///������  ֱ��
	TShZdPriceType	PreClosePrice;
	///��ֲ��� ֱ��
	TShZdLargeVolumeType	PreOpenInterest;
	///����  ֱ��
	TShZdPriceType	OpenPrice;
	///��߼�  ֱ��
	TShZdPriceType	HighestPrice;
	///��ͼ�  ֱ��
	TShZdPriceType	LowestPrice;
	///����  ֱ��
	TShZdVolumeType	Volume;
	///�ɽ����
	TShZdMoneyType	Turnover;
	///�ֲ���  ֱ��
	TShZdLargeVolumeType	OpenInterest;
	///������  ֱ��
	TShZdPriceType	ClosePrice;
	///���ν����
	TShZdPriceType	SettlementPrice;
	///��ͣ��� �������
	TShZdPriceType	UpperLimitPrice;
	///��ͣ��� ��������
	TShZdPriceType	LowerLimitPrice;
	///����ʵ��  ��������
	TShZdRatioType	PreDelta;
	///����ʵ��  ��������
	TShZdRatioType	CurrDelta;
	///����޸�ʱ��  ֱ��
	TShZdTimeType	UpdateTime;
	///����޸ĺ���  ֱ��
	TShZdMillisecType	UpdateMillisec;
	///�����һ  ֱ��
	TShZdPriceType	BidPrice1;
	///������һ  ֱ��
	TShZdVolumeType	BidVolume1;
	///������һ  ֱ��
	TShZdPriceType	AskPrice1;
	///������һ  ֱ��
	TShZdVolumeType	AskVolume1;
	///����۶�  ֱ��
	TShZdPriceType	BidPrice2;
	///��������  ֱ��
	TShZdVolumeType	BidVolume2;
	///�����۶�  ֱ��
	TShZdPriceType	AskPrice2;
	///��������  ֱ��
	TShZdVolumeType	AskVolume2;
	///�������  ֱ��
	TShZdPriceType	BidPrice3;
	///��������  ֱ��
	TShZdVolumeType	BidVolume3;
	///��������  ֱ��
	TShZdPriceType	AskPrice3;
	///��������  ֱ��
	TShZdVolumeType	AskVolume3;
	///�������  ֱ��
	TShZdPriceType	BidPrice4;
	///��������  ֱ��
	TShZdVolumeType	BidVolume4;
	///��������  ֱ��
	TShZdPriceType	AskPrice4;
	///��������  ֱ��
	TShZdVolumeType	AskVolume4;
	///�������  ֱ��
	TShZdPriceType	BidPrice5;
	///��������  ֱ��
	TShZdVolumeType	BidVolume5;
	///��������  ֱ��
	TShZdPriceType	AskPrice5;
	///��������  ֱ��
	TShZdVolumeType	AskVolume5;
	///���վ���  ֱ��
	TShZdPriceType	AveragePrice;
	///�ɽ�������  ֱ��
	TShZdVolumeType	TotalVolume;
};

///ֱ���û�������
struct CTShZdUserPasswordUpdateField
{	
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
	///ԭ���Ŀ���  ֱ��
	TShZdPasswordType	OldPassword;
	///�µĿ���  ֱ��
	TShZdPasswordType	NewPassword;
};

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

///ֱ�ﱨ���������������ĵ����ر�
struct CTShZdInputOrderActionField
{	
	///�ʽ��˺�  ֱ��
	TShZdInvestorIDType	InvestorID;
	///������������  ֱ��
	TShZdOrderActionRefType	OrderActionRef;
	///������ ֱ��
	TShZdOrderRefType	OrderRef;
	///������  ֱ��
	TShZdRequestIDType	RequestID;	
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///ϵͳ��  ֱ��
	TShZdOrderSysIDType	OrderSysID;
	///������־  0 ���� 3 �ĵ�  ֱ��
	TShZdActionFlagType	ActionFlag;
	///�۸�仯  �ĵ���ļ۸�  ֱ��
	TShZdPriceType	LimitPrice;
	///�����仯  �ĵ�������� ��������  ֱ��
	TShZdVolumeType	VolumeChange;
	///�ѳɽ�����
	TShZdVolumeType    VolumeFilled;
	///�����۸�
	TShZdPriceType  OrderPrice;
	///��������   ����ֱ��
	TShZdVolumeType  OrderVolume; 
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
	///��Լ����  ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///��Ч������  ��1=������Ч, 2=������Ч��GTC����3=OPG��4=IOC��5=FOK��6=GTD��7=ATC��8=FAK��
	TShZdTimeConditionType	TimeCondition;
	///��������   1�� 2��  ֱ��
	TShZdDirectionType	Direction;
	///�����۸�����   1�޼۵� 2�м۵� 3�޼�ֹ��stop to limit����4ֹ��stop to market��
	TShZdOrderPriceTypeType	OrderPriceType;	
	///�ĵ������۸�
	TShZdPriceType  ModifyTriggerPrice;
	///��������(�ĵ����ڡ���������)
	TShZdDateType	ActionDate;
	///����ʱ��(�ĵ�ʱ�䡢����ʱ��)
	TShZdTimeType	ActionTime;
};

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

///ֱ���ѯ����
struct CTShZdQryOrderField
{
	///�û�����  ֱ��
	TShZdUserIDType	UserID;	
	///�ʽ���� ֱ��
	TShZdInvestorIDType	InvestorID;
	///��Լ����  ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///ϵͳ��  ֱ��
	TShZdOrderSysIDType	OrderSysID;
	///��ʼʱ��  ֱ��
	TShZdTimeType	InsertTimeStart;
	///����ʱ��  ֱ��
	TShZdTimeType	InsertTimeEnd;	
};

///ֱ���ѯ�ɽ�
struct CTShZdQryTradeField
{
	///�û�����   ֱ��
	TShZdUserIDType	UserID;	
	///�ʽ��˺�   ֱ��
	TShZdInvestorIDType	InvestorID;
	///��Լ����   ֱ��
	TShZdInstrumentIDType	InstrumentID;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///ϵͳ��   ֱ��
	TShZdOrderSysIDType	OrderSysID;
	///�ɽ����  ֱ��
	TShZdTradeIDType	TradeID;
	///��ʼʱ��  ֱ��
	TShZdTimeType	TradeTimeStart;
	///����ʱ��  ֱ��
	TShZdTimeType	TradeTimeEnd;
};

///ֱ���ѯͶ���ֲ߳�
struct CTShZdQryInvestorPositionField
{
	///�û�����
	TShZdUserIDType	UserID;
	///�ͻ��ʽ��˺� 
	TShZdInvestorIDType	InvestorID;
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
};

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

///ֱ���ѯ������
struct CTShZdQryExchangeField
{
	///����������
	TShZdExchangeIDType	ExchangeID;
};

///ֱ���ѯ��Լ
struct CTShZdQryInstrumentField
{
	///��Լ���롣��ѯ������Լ
	TShZdInstrumentIDType	InstrumentID;
	///���������룬�����дֵ����ѯһ���������ĺ�Լ
	TShZdExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���
	TShZdExchangeInstIDType	ExchangeInstID;
	///��Ʒ���� �������дֵ����ѯһ����Ʒ�ĺ�Լ
	TShZdInstrumentIDType	ProductID;
	///��ʼʱ��,�����д�������ʱ���Ժ�������
	TShZdTimeType	InsertTimeStart;
	///��ѯ������,ÿ�η��ص�����
	TShZdVolumeType	Index;
	///��ѯ��Լ�����  ֱ��
	TShZdProductClassType ProductType;
};

///ָ���ĺ�Լ  
struct CTShZdSpecificInstrumentField
{
	///��Լ���� ֱ��
	TShZdInstrumentIDType	InstrumentID;
};

///��Լ״̬ δʹ��
struct CTShZdInstrumentStatusField
{
	///����������
	TShZdExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���
	TShZdExchangeInstIDType	ExchangeInstID;
	///���������
	TShZdSettlementGroupIDType	SettlementGroupID;
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
	///��Լ����״̬
	TShZdInstrumentStatusType	InstrumentStatus;
	///���׽׶α��
	TShZdTradingSegmentSNType	TradingSegmentSN;
	///���뱾״̬ʱ��
	TShZdTimeType	EnterTime;
	///���뱾״̬ԭ��
	TShZdInstStatusEnterReasonType	EnterReason;
};

///ֱ���ѯͶ���ֲ߳���ϸ
struct CTShZdQryInvestorPositionDetailField
{
	///�û�����  ֱ��
	TShZdUserIDType	UserID;	
	///�ʽ��˺�  ֱ��
	TShZdInvestorIDType	InvestorID;
	///��Լ����  ֱ��
	TShZdInstrumentIDType	InstrumentID;
};

///ֱ��Ͷ���ֲ߳���ϸ
struct CTShZdInvestorPositionDetailField
{
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
	///��Լ����   ֱ��
	TShZdInstrumentIDType	InstrumentID;	
	///�ʽ��˺�  ֱ��
	TShZdInvestorIDType	InvestorID;
	///Ͷ���ױ���־  ֱ��
	TShZdHedgeFlagType	HedgeFlag;
	///����   ֱ��
	TShZdDirectionType	Direction;
	///������Ľ�������(yyyy-MM-dd)  ֱ��
	TShZdDateType	OpenDate;
	///�ɽ����  ֱ��
	TShZdTradeIDType	TradeID;
	///�ֲ�����  ֱ��
	TShZdVolumeType	Volume;
	///�ֲּ�  ֱ��
	TShZdPriceType	OpenPrice;
	///������   ֱ��
	TShZdDateType	TradingDay;	
	///����������  ֱ�� 
	TShZdExchangeIDType	ExchangeID;	
	///�����  ֱ��
	TShZdPriceType	SettlementPrice;	
	///�ֲ�״̬  ֱ��
	TShZdHoldSatusType HoldStatus;
	///���ұ���  ֱ��
	TShZdCurrencyNoType  CurrencyNo;
	///��Ʒ����  ֱ��
	TShZdInstrumentIDType	CommonID;
};

///ֱ��Ԥ��
struct CTShZdParkedOrderField
{
	///���͹�˾����  ֹ��ֹӯ���
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ��� �û��ʽ��˺�
	TShZdInvestorIDType	InvestorID;
	///��Լ���� 
	TShZdInstrumentIDType	InstrumentID;
	///��������  ���ر��
	TShZdOrderRefType	OrderRef;
	///�û�����
	TShZdUserIDType	UserID;
	///�����۸�����
	TShZdOrderPriceTypeType	OrderPriceType;
	///��������
	TShZdDirectionType	Direction;
	///��Ͽ�ƽ��־
	TShZdCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TShZdCombHedgeFlagType	CombHedgeFlag;
	///�۸�  ֹ��ֹӯ������
	TShZdPriceType	LimitPrice;
	///����
	TShZdVolumeType	VolumeTotalOriginal;
	///��Ч������
	TShZdTimeConditionType	TimeCondition;
	///GTD����
	TShZdDateType	GTDDate;
	///�ɽ�������
	TShZdVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TShZdVolumeType	MinVolume;
	///�������� 1�������¼۴�����2������۴�����3�������۴���
	TShZdContingentConditionType	ContingentCondition;
	///ֹ���   ֹ��ֹӯ������
	TShZdPriceType	StopPrice;
	///ǿƽԭ��
	TShZdForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TShZdBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TShZdBusinessUnitType	BusinessUnit;
	///������
	TShZdRequestIDType	RequestID;
	///�û�ǿ����־
	TShZdBoolType	UserForceClose;
	///����������
	TShZdExchangeIDType	ExchangeID;
	///Ԥ�񱨵����
	TShZdParkedOrderIDType	ParkedOrderID;
	///�û�����
	TShZdUserTypeType	UserType;
	///Ԥ��״̬
	TShZdParkedOrderStatusType	Status;
	///�������
	TShZdErrorIDType	ErrorID;
	///������Ϣ
	TShZdErrorMsgType	ErrorMsg;
	///��������־
	TShZdBoolType	IsSwapOrder;
};

///ֱ������Ԥ�񵥲���
struct CTShZdParkedOrderActionField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TShZdInvestorIDType	InvestorID;
	///������������
	TShZdOrderActionRefType	OrderActionRef;
	///�������
	TShZdOrderRefType	OrderRef;
	///������
	TShZdRequestIDType	RequestID;
	///ǰ�ñ��
	TShZdFrontIDType	FrontID;
	///�Ự���
	TShZdSessionIDType	SessionID;
	///����������
	TShZdExchangeIDType	ExchangeID;
	///ϵͳ���
	TShZdOrderSysIDType	OrderSysID;
	///������־
	TShZdActionFlagType	ActionFlag;
	///�۸�
	TShZdPriceType	LimitPrice;
	///�����仯
	TShZdVolumeType	VolumeChange;
	///�û�����
	TShZdUserIDType	UserID;
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
	///Ԥ�񳷵������
	TShZdParkedOrderActionIDType	ParkedOrderActionID;
	///�û�����
	TShZdUserTypeType	UserType;
	///Ԥ�񳷵�״̬
	TShZdParkedOrderStatusType	Status;
	///�������
	TShZdErrorIDType	ErrorID;
	///������Ϣ
	TShZdErrorMsgType	ErrorMsg;
};

///ֱ���ѯԤ��
struct CTShZdQryParkedOrderField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TShZdInvestorIDType	InvestorID;
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
	///����������
	TShZdExchangeIDType	ExchangeID;
};

///ֱ���ѯԤ�񳷵�
struct CTShZdQryParkedOrderActionField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TShZdInvestorIDType	InvestorID;
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
	///����������
	TShZdExchangeIDType	ExchangeID;
};

///ֱ��ɾ��Ԥ��
struct CTShZdRemoveParkedOrderField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TShZdInvestorIDType	InvestorID;
	///Ԥ�񱨵����
	TShZdParkedOrderIDType	ParkedOrderID;
};

///ֱ��ɾ��Ԥ�񳷵� 
struct CTShZdRemoveParkedOrderActionField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TShZdInvestorIDType	InvestorID;
	///Ԥ�񳷵����
	TShZdParkedOrderActionIDType	ParkedOrderActionID;
};

///��ѯ���󱨵�����  δʹ��
struct CTShZdErrorConditionalOrderField
{
	///���͹�˾����
	TShZdBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TShZdInvestorIDType	InvestorID;
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
	///�������
	TShZdOrderRefType	OrderRef;
	///�û�����
	TShZdUserIDType	UserID;
	///�����۸�����
	TShZdOrderPriceTypeType	OrderPriceType;
	///��������
	TShZdDirectionType	Direction;
	///��Ͽ�ƽ��־
	TShZdCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TShZdCombHedgeFlagType	CombHedgeFlag;
	///�۸�
	TShZdPriceType	LimitPrice;
	///����
	TShZdVolumeType	VolumeTotalOriginal;
	///��Ч������
	TShZdTimeConditionType	TimeCondition;
	///GTD����
	TShZdDateType	GTDDate;
	///�ɽ�������
	TShZdVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TShZdVolumeType	MinVolume;
	///��������
	TShZdContingentConditionType	ContingentCondition;
	///ֹ���
	TShZdPriceType	StopPrice;
	///ǿƽԭ��
	TShZdForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TShZdBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TShZdBusinessUnitType	BusinessUnit;
	///������
	TShZdRequestIDType	RequestID;
	///���ر������
	TShZdOrderLocalIDType	OrderLocalID;
	///����������
	TShZdExchangeIDType	ExchangeID;
	///��Ա����
	TShZdParticipantIDType	ParticipantID;
	///�ͻ�����
	TShZdClientIDType	ClientID;
	///��Լ�ڽ������Ĵ���
	TShZdExchangeInstIDType	ExchangeInstID;
	///����������Ա����
	TShZdTraderIDType	TraderID;
	///��װ���
	TShZdInstallIDType	InstallID;
	///�����ύ״̬
	TShZdOrderSubmitStatusType	OrderSubmitStatus;
	///������ʾ���
	TShZdSequenceNoType	NotifySequence;
	///������
	TShZdDateType	TradingDay;
	///������
	TShZdSettlementIDType	SettlementID;
	///ϵͳ���
	TShZdOrderSysIDType	OrderSysID;
	///������Դ
	TShZdOrderSourceType	OrderSource;
	///����״̬
	TShZdOrderStatusType	OrderStatus;
	///��������
	TShZdOrderTypeType	OrderType;
	///��ɽ�����
	TShZdVolumeType	VolumeTraded;
	///ʣ������
	TShZdVolumeType	VolumeTotal;
	///��������
	TShZdDateType	InsertDate;
	///ί��ʱ��
	TShZdTimeType	InsertTime;
	///����ʱ��
	TShZdTimeType	ActiveTime;
	///����ʱ��
	TShZdTimeType	SuspendTime;
	///����޸�ʱ��
	TShZdTimeType	UpdateTime;
	///����ʱ��
	TShZdTimeType	CancelTime;
	///����޸Ľ���������Ա����
	TShZdTraderIDType	ActiveTraderID;
	///�����Ա���
	TShZdParticipantIDType	ClearingPartID;
	///���
	TShZdSequenceNoType	SequenceNo;
	///ǰ�ñ��
	TShZdFrontIDType	FrontID;
	///�Ự���
	TShZdSessionIDType	SessionID;
	///�û��˲�Ʒ��Ϣ
	TShZdProductInfoType	UserProductInfo;
	///״̬��Ϣ
	TShZdErrorMsgType	StatusMsg;
	///�û�ǿ����־
	TShZdBoolType	UserForceClose;
	///�����û�����
	TShZdUserIDType	ActiveUserID;
	///���͹�˾�������
	TShZdSequenceNoType	BrokerOrderSeq;
	///��ر���
	TShZdOrderSysIDType	RelativeOrderSysID;
	///֣�����ɽ�����
	TShZdVolumeType	ZCETotalTradedVolume;
	///�������
	TShZdErrorIDType	ErrorID;
	///������Ϣ
	TShZdErrorMsgType	ErrorMsg;
	///��������־
	TShZdBoolType	IsSwapOrder;
};

///ֱ������
struct CTShZdTransferSerialField
{		
	///��������  ֱ��
	TShZdDateType	TradingDay;
	///����ʱ��  ֱ��
	TShZdTradeTimeType	TradeTime;	
	///�����������  ֱ��
	TShZdBankNameType BankInName;
	///��������ʺ�  ֱ��
	TShZdBankAccountType	BankInAccount;
	///������������ ֱ��
	TShZdBankNameType BankOutName;
	///���������ʺ� ֱ��
	TShZdBankAccountType	BankOutAccount;
	///������ˮ��  ֱ��
	TShZdBankSerialType	BankSerial;	
	///���ִ���  ֱ��
	TShZdCurrencyIDType	CurrencyID;	
	///���������״̬  ֱ��
	TShZdInOutMoneyStatusType	InOutStatus;
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
	///��������� ֱ��
	TShZdInOutMoneyType InOutMoneyType;
	///�����ʽ ֱ��
	TShZdInOutMoneyMedthType  InOutMedth;
	///�ʽ���; ֱ��
	TShZdInOutMoneyUsingType  InOutMoneyUsing;
	///���  ֱ��
	TShZdTradeAmountType	Amount;
	///��ע  ֱ��
	TShZdDescribingType  Describing;
};

///����ֱ������
struct CTShZdQryTransferSerialField
{	
	///�ʽ��˺�  ֱ��
	TShZdAccountIDType	AccountID;	
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
	///���������  ֱ��
	TShZdInOutMoneyType InOutMoneyType;
	///�����ʽ  ֱ��
	TShZdInOutMoneyMedthType  InOutMedth;
	///�ʽ���; ֱ��
	TShZdInOutMoneyUsingType  InOutMoneyUsing;
	///���ִ���  ֱ��
	TShZdCurrencyIDType	CurrencyID;
	///���  ֱ��
	TShZdTradeAmountType	Amount;
	///��ע  ֱ��
	TShZdDescribingType  Describing;
};

//ֱ���г�������ʱ������
struct CTShZdReqOpenCloseTimeField
{
	///�û�����  ֱ��
	TShZdUserIDType	UserID;
};

///ֱ�ｻ�׿���ʱ��ͱ���ʱ��
struct CTShZdOpenCloseTimeField
{
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
	///��������  ֱ��
	TShZdTradeDateType TradeDate;
	///����ʱ��  ֱ��
	TShZdTradeTimeType OpenTime;
	///����ʱ��  ֱ��
	TShZdTradeTimeType CloseTime;
};

///��ѯֱ��֧�ֵ��г�������ʱ��
struct CTShZdReqMarketOpenCloseTimeField
{
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;	
};

///ֱ���г��Ŀ�����ʱ��
struct CTShZdMarketOpenCloseTimeField
{
	///���  ֱ��  
	TShZdTradeTimeType CurrYear;
	///���ʼʱ��  ֱ��
	TShZdTradeTimeType SummeyBeginTime;
	///���ʼʱ��  ֱ��
	TShZdTradeTimeType WinterBeginTime;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;		
	///����������  ֱ��
	TShZdExchangeNameType ExchangeName;
};

///��ѯֱ���Ʒ�Ŀ�����ʱ��
struct CTShZdReqCommonOpenCloseTimeField
{
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///��Ʒ����  ֱ��
	TShZdInstrumentIDType	ProductID;
};

///ֱ���Ʒ������ʱ��ķ���
struct CTShZdCommonOpenCloseTimeField
{
	///��Ʒ�Ƿ���ս���  ֱ��
	TShZdTradeCrossDayType  TradeDay;
	///״̬  ֱ��
	TShZdCommonStageType  Stage;
	///��ͨʱ�䡢������ʱ  ֱ��
	TShZdTNswTimeType  NomalSummerWinter;
	///����ʱ��  ֱ��
	TShZdTradeTimeType OpenTime;
	///����ʱ��  ֱ��
	TShZdTradeTimeType CloseTime;
	///��Ʒ����  ֱ��
	TShZdInstrumentIDType	ProductID;
	///��Ʒ����  ֱ��
	TShZdProductNameType  ProductName;
	///����������  ֱ��
	TShZdExchangeIDType	ExchangeID;
	///����������  ֱ��
	TShZdExchangeNameType ExchangeName;		
};

///ֱ�ｻ���ʽ�仯
struct CTShZdTradeMoneyField
{
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
	///�ʽ��˺�  ֱ��
	TShZdAccountIDType	AccountID;
	///���ִ���  ֱ��
	TShZdCurrencyIDType	CurrencyID;
	///����� ֱ��
	TShZdMoneyType	PreUse;
	///���� ֱ��
	TShZdMoneyType	CurrDeposit;
	///ƽ��ӯ��  ֱ��
	TShZdMoneyType	CloseProfit;
	///������ʽ�  ֱ��
	TShZdMoneyType	FrozenDeposit;	
	///������  ֱ��
	TShZdMoneyType	Commission;
	///��ǰ��֤���ܶ�  ֱ��
	TShZdMoneyType	CurrMargin;	
	///ά�ֱ�֤��  ֱ��
	TShZdMoneyType	KeepMargin;		
	///�����   ֱ��
	TShZdMoneyType	Deposit;
	///������   ֱ��
	TShZdMoneyType	Withdraw;
	///δ����ƽӯ   ֱ��
	TShZdMoneyType	UnCloseProfit;
	///δ����ƽӯ  ֱ��
	TShZdMoneyType	UnFrozenCloseProfit;
	///��Ȩ����  ֱ��
	TShZdMoneyType	RoyaltyMargin;	
};

///ֱ����ʲ�ѯ
struct CTShZdReqMoneyRatioField
{
	///�û���ʶ  ֱ��
	TShZdUserIDType	UserID;
};

///ֱ����ʷ���
struct CTShZdMoneyRatioField
{
	///���ִ���  ֱ��
	TShZdCurrencyIDType	CurrencyID;
	///�Ƿ�������� 
	TShZdBoolType IsBaseCurrency;
	///����
	TShZdMoneyType Ratio;
};

///ֱ��ɽ��󣬸ú�Լ�ĳֲ�״̬����
struct CTShZdFilledHoldStatus
{
	///��Լ����
	TShZdInstrumentIDType	InstrumentID;
	///�������� 
	TShZdDirectionType	Direction;
	///�ֲ���
	TShZdVolumeType	Volume;
	///�ֲּ�  
	TShZdPriceType	Price;
};
#pragma pack(pop)
#endif
