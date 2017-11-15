/////////////////////////////////////////////////////////////////////////
///@company shanghai liber information Technology Co.,Ltd
///@file SecurityFtdcUserApiStruct.h
///@brief ����ҵ�����ݽṹ
/////////////////////////////////////////////////////////////////////////

#if !defined(SECURITY_FTDCSTRUCT_H)
#define SECURITY_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SecurityFtdcUserApiDataType.h"

#pragma pack(push)
#pragma pack(1)
///��Ӧ��Ϣ
struct CSecurityFtdcRspInfoField
{
	///�������
	TSecurityFtdcErrorIDType	ErrorID;
	///������Ϣ
	TSecurityFtdcErrorMsgType	ErrorMsg;
};

///������
struct CSecurityFtdcExchangeField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///����������
	TSecurityFtdcExchangeNameType	ExchangeName;
	///����������
	TSecurityFtdcExchangePropertyType	ExchangeProperty;
};

///��Ʒ
struct CSecurityFtdcProductField
{
	///��Ʒ����
	TSecurityFtdcInstrumentIDType	ProductID;
	///��Ʒ����
	TSecurityFtdcProductNameType	ProductName;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Ʒ����
	TSecurityFtdcProductClassType	ProductClass;
	///��Լ��������
	TSecurityFtdcVolumeMultipleType	VolumeMultiple;
	///��С�䶯��λ
	TSecurityFtdcPriceType	PriceTick;
	///�м۵�����µ���
	TSecurityFtdcVolumeType	MaxMarketOrderVolume;
	///�м۵���С�µ���
	TSecurityFtdcVolumeType	MinMarketOrderVolume;
	///�޼۵�����µ���
	TSecurityFtdcVolumeType	MaxLimitOrderVolume;
	///�޼۵���С�µ���
	TSecurityFtdcVolumeType	MinLimitOrderVolume;
	///�ֲ�����
	TSecurityFtdcPositionTypeType	PositionType;
	///�ֲ���������
	TSecurityFtdcPositionDateTypeType	PositionDateType;
	///ETF��С���׵�λ
	TSecurityFtdcVolumeType	EFTMinTradeVolume;
};

///��Լ
struct CSecurityFtdcInstrumentField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentNameType	InstrumentName;
	///��Լ�ڽ������Ĵ���
	TSecurityFtdcExchangeInstIDType	ExchangeInstID;
	///��Ʒ����
	TSecurityFtdcInstrumentIDType	ProductID;
	///��Ʒ����
	TSecurityFtdcProductClassType	ProductClass;
	///�������
	TSecurityFtdcYearType	DeliveryYear;
	///������
	TSecurityFtdcMonthType	DeliveryMonth;
	///�м۵�����µ���
	TSecurityFtdcVolumeType	MaxMarketOrderVolume;
	///�м۵���С�µ���
	TSecurityFtdcVolumeType	MinMarketOrderVolume;
	///�޼۵�����µ���
	TSecurityFtdcVolumeType	MaxLimitOrderVolume;
	///�޼۵���С�µ���
	TSecurityFtdcVolumeType	MinLimitOrderVolume;
	///��Լ��������
	TSecurityFtdcVolumeMultipleType	VolumeMultiple;
	///��С�䶯��λ
	TSecurityFtdcPriceType	PriceTick;
	///������
	TSecurityFtdcDateType	CreateDate;
	///������
	TSecurityFtdcDateType	OpenDate;
	///������
	TSecurityFtdcDateType	ExpireDate;
	///��ʼ������
	TSecurityFtdcDateType	StartDelivDate;
	///����������
	TSecurityFtdcDateType	EndDelivDate;
	///��Լ��������״̬
	TSecurityFtdcInstLifePhaseType	InstLifePhase;
	///��ǰ�Ƿ���
	TSecurityFtdcBoolType	IsTrading;
	///�ֲ�����
	TSecurityFtdcPositionTypeType	PositionType;
	///�����ܷ񳷵�
	TSecurityFtdcBoolType	OrderCanBeWithdraw;
	///��С���µ���λ
	TSecurityFtdcVolumeType	MinBuyVolume;
	///��С���µ���λ
	TSecurityFtdcVolumeType	MinSellVolume;
	///��ƱȨ��ģ�����
	TSecurityFtdcRightModelIDType	RightModelID;
	///�ֲֽ�������
	TSecurityFtdcPosTradeTypeType	PosTradeType;
	///�г�����
	TSecurityFtdcInstrumentIDType	MarketID;
	///��Ȩִ�м۸�
	TSecurityFtdcPriceType	ExecPrice;
	///��Ȩ���ֱ�֤��
	TSecurityFtdcMoneyType	UnitMargin;
	///��Լ����
	TSecurityFtdcInstrumentTypeType	InstrumentType;
	///��Ȩ��֤�����1
	TSecurityFtdcRatioType	OptionsMarginParam1;
	///��Ȩ��֤�����2
	TSecurityFtdcRatioType	OptionsMarginParam2;
};

///���͹�˾
struct CSecurityFtdcBrokerField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///���͹�˾���
	TSecurityFtdcBrokerAbbrType	BrokerAbbr;
	///���͹�˾����
	TSecurityFtdcBrokerNameType	BrokerName;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
};

///��Ա����;��͹�˾������ձ�
struct CSecurityFtdcPartBrokerField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
};

///Ͷ����
struct CSecurityFtdcInvestorField
{
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���߷������
	TSecurityFtdcInvestorIDType	InvestorGroupID;
	///Ͷ��������
	TSecurityFtdcPartyNameType	InvestorName;
	///֤������
	TSecurityFtdcIdCardTypeType	IdentifiedCardType;
	///֤������
	TSecurityFtdcIdentifiedCardNoType	IdentifiedCardNo;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
	///�Ϻ�Ӫҵ�����
	TSecurityFtdcTraderIDType	SHBranchID;
	///����Ӫҵ�����
	TSecurityFtdcTraderIDType	SZBranchID;
	///��������ϵͳ����
	TSecurityFtdcSettleSystemTypeType	SettleSystemType;
	///Ͷ������Ȩ���׵ȼ�
	TSecurityFtdcInvestorLevelType	InvestorLevel;
};

///���ױ���
struct CSecurityFtdcTradingCodeField
{
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�ͻ�����
	TSecurityFtdcClientIDType	ClientID;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
	///AccountID
	TSecurityFtdcAccountIDType	AccountID;
	///���׵�Ԫ��
	TSecurityFtdcTraderIDType	PBU;
	///ClientType
	TSecurityFtdcClientTypeType	ClientType;
};

///�����û�
struct CSecurityFtdcSuperUserField
{
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�û�����
	TSecurityFtdcUserNameType	UserName;
	///����
	TSecurityFtdcPasswordType	Password;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
};

///�����û�����Ȩ��
struct CSecurityFtdcSuperUserFunctionField
{
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///���ܴ���
	TSecurityFtdcFunctionCodeType	FunctionCode;
};

///���͹�˾�û�
struct CSecurityFtdcBrokerUserField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�û�����
	TSecurityFtdcUserNameType	UserName;
	///�û�����
	TSecurityFtdcUserTypeType	UserType;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
	///�Ƿ�ʹ������
	TSecurityFtdcBoolType	IsUsingOTP;
};

///���͹�˾�û�����Ȩ��
struct CSecurityFtdcBrokerUserFunctionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///���͹�˾���ܴ���
	TSecurityFtdcBrokerFunctionCodeType	BrokerFunctionCode;
};

///�ʽ��˻�
struct CSecurityFtdcTradingAccountField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ�����ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///�ϴ���Ѻ���
	TSecurityFtdcMoneyType	PreMortgage;
	///�ϴ����ö��
	TSecurityFtdcMoneyType	PreCredit;
	///�ϴδ���
	TSecurityFtdcMoneyType	PreDeposit;
	///�ϴν���׼����
	TSecurityFtdcMoneyType	PreBalance;
	///�ϴ�ռ�õı�֤��
	TSecurityFtdcMoneyType	PreMargin;
	///��Ϣ����
	TSecurityFtdcMoneyType	InterestBase;
	///��Ϣ����
	TSecurityFtdcMoneyType	Interest;
	///�����
	TSecurityFtdcMoneyType	Deposit;
	///������
	TSecurityFtdcMoneyType	Withdraw;
	///����ı�֤��
	TSecurityFtdcMoneyType	FrozenMargin;
	///������ʽ�
	TSecurityFtdcMoneyType	FrozenCash;
	///�����������
	TSecurityFtdcMoneyType	FrozenCommission;
	///��ǰ��֤���ܶ�
	TSecurityFtdcMoneyType	CurrMargin;
	///�ʽ���
	TSecurityFtdcMoneyType	CashIn;
	///������
	TSecurityFtdcMoneyType	Commission;
	///����׼����
	TSecurityFtdcMoneyType	Balance;
	///�ֽ�
	TSecurityFtdcMoneyType	Available;
	///��ȡ�ʽ�
	TSecurityFtdcMoneyType	WithdrawQuota;
	///����׼����
	TSecurityFtdcMoneyType	Reserve;
	///������
	TSecurityFtdcDateType	TradingDay;
	///��֤��������
	TSecurityFtdcMoneyType	Credit;
	///��Ѻ���
	TSecurityFtdcMoneyType	Mortgage;
	///��������֤��
	TSecurityFtdcMoneyType	ExchangeMargin;
	///Ͷ���߽��֤��
	TSecurityFtdcMoneyType	DeliveryMargin;
	///���������֤��
	TSecurityFtdcMoneyType	ExchangeDeliveryMargin;
	///����Ĺ�����
	TSecurityFtdcMoneyType	FrozenTransferFee;
	///�����ӡ��˰
	TSecurityFtdcMoneyType	FrozenStampTax;
	///������
	TSecurityFtdcMoneyType	TransferFee;
	///ӡ��˰
	TSecurityFtdcMoneyType	StampTax;
	///������
	TSecurityFtdcMoneyType	ConversionAmount;
	///���Ŷ��
	TSecurityFtdcMoneyType	CreditAmount;
	///֤ȯ�ܼ�ֵ
	TSecurityFtdcMoneyType	StockValue;
	///��ծ�ع�ռ���ʽ�
	TSecurityFtdcMoneyType	BondRepurchaseAmount;
	///��ծ��ع�ռ���ʽ�
	TSecurityFtdcMoneyType	ReverseRepurchaseAmount;
	///����
	TSecurityFtdcCurrencyCodeType	CurrencyCode;
	///�˻�����
	TSecurityFtdcAccountTypeType	AccountType;
	///����������
	TSecurityFtdcMoneyType	MarginTradeAmount;
	///��ȯ�������
	TSecurityFtdcMoneyType	ShortSellAmount;
	///���ʳֲ�ӯ��
	TSecurityFtdcMoneyType	MarginTradeProfit;
	///��ȯ�ֲ�ӯ��
	TSecurityFtdcMoneyType	ShortSellProfit;
	///��ȯ����ֵ
	TSecurityFtdcMoneyType	SSStockValue;
	///ά�ֵ�������
	TSecurityFtdcRatioType	CreditRatio;
	///��Ȩ�����ʽ�
	TSecurityFtdcMoneyType	FrozenExecCash;
	///��Ȩ�����ʽ�(SSE)
	TSecurityFtdcMoneyType	SSEOptionsBuyAmount;
	///��Ȩ���붳���ʽ�(SSE)
	TSecurityFtdcMoneyType	SSEOptionsBuyFrozenAmount;
	///���㱣֤���ܶ�
	TSecurityFtdcMoneyType	SettleMargin;
};

///��ֹ��¼�û�
struct CSecurityFtdcLoginForbiddenUserField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///�������
struct CSecurityFtdcDepthMarketDataField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���
	TSecurityFtdcExchangeInstIDType	ExchangeInstID;
	///���¼�
	TSecurityFtdcPriceType	LastPrice;
	///�ϴν����
	TSecurityFtdcPriceType	PreSettlementPrice;
	///������
	TSecurityFtdcPriceType	PreClosePrice;
	///��ֲ���
	TSecurityFtdcLargeVolumeType	PreOpenInterest;
	///����
	TSecurityFtdcPriceType	OpenPrice;
	///��߼�
	TSecurityFtdcPriceType	HighestPrice;
	///��ͼ�
	TSecurityFtdcPriceType	LowestPrice;
	///����
	TSecurityFtdcLargeVolumeType	Volume;
	///�ɽ����
	TSecurityFtdcMoneyType	Turnover;
	///�ֲ���
	TSecurityFtdcLargeVolumeType	OpenInterest;
	///������
	TSecurityFtdcPriceType	ClosePrice;
	///���ν����
	TSecurityFtdcPriceType	SettlementPrice;
	///��ͣ���
	TSecurityFtdcPriceType	UpperLimitPrice;
	///��ͣ���
	TSecurityFtdcPriceType	LowerLimitPrice;
	///����ʵ��
	TSecurityFtdcRatioType	PreDelta;
	///����ʵ��
	TSecurityFtdcRatioType	CurrDelta;
	///���ջ���ֵ
	TSecurityFtdcPriceType	PreIOPV;
	///����ֵ
	TSecurityFtdcPriceType	IOPV;
	///��̬�ο��۸�
	TSecurityFtdcPriceType	AuctionPrice;
	///����޸�ʱ��
	TSecurityFtdcTimeType	UpdateTime;
	///����޸ĺ���
	TSecurityFtdcMillisecType	UpdateMillisec;
	///�����һ
	TSecurityFtdcPriceType	BidPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	BidVolume1;
	///������һ
	TSecurityFtdcPriceType	AskPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	AskVolume1;
	///����۶�
	TSecurityFtdcPriceType	BidPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume2;
	///�����۶�
	TSecurityFtdcPriceType	AskPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume2;
	///�������
	TSecurityFtdcPriceType	BidPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume3;
	///��������
	TSecurityFtdcPriceType	AskPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume3;
	///�������
	TSecurityFtdcPriceType	BidPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume4;
	///��������
	TSecurityFtdcPriceType	AskPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume4;
	///�������
	TSecurityFtdcPriceType	BidPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume5;
	///��������
	TSecurityFtdcPriceType	AskPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume5;
	///���վ���
	TSecurityFtdcPriceType	AveragePrice;
	///ҵ������
	TSecurityFtdcDateType	ActionDay;
	///���׽׶�
	TSecurityFtdcTradingPhaseType	TradingPhase;
	///��������
	TSecurityFtdcOpenRestrictionType	OpenRestriction;
};

///Ͷ���ߺ�Լ����Ȩ��
struct CSecurityFtdcInstrumentTradingRightField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///Ͷ���߷�Χ
	TSecurityFtdcInvestorRangeType	InvestorRange;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����
	TSecurityFtdcDirectionType	Direction;
	///����Ȩ��
	TSecurityFtdcTradingRightType	TradingRight;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��ƱȨ�޷���
	TSecurityFtdcInstrumentRangeType	InstrumentRange;
};

///Ͷ���ֲ߳���ϸ
struct CSecurityFtdcInvestorPositionDetailField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///Ͷ���ױ���־
	TSecurityFtdcHedgeFlagType	HedgeFlag;
	///����
	TSecurityFtdcDirectionType	Direction;
	///��������
	TSecurityFtdcDateType	OpenDate;
	///�ɽ����
	TSecurityFtdcTradeIDType	TradeID;
	///����
	TSecurityFtdcLargeVolumeType	Volume;
	///���ּ�
	TSecurityFtdcPriceType	OpenPrice;
	///������
	TSecurityFtdcDateType	TradingDay;
	///�ɽ�����
	TSecurityFtdcTradeTypeType	TradeType;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///Ͷ���߱�֤��
	TSecurityFtdcMoneyType	Margin;
	///��������֤��
	TSecurityFtdcMoneyType	ExchMargin;
	///������
	TSecurityFtdcPriceType	LastSettlementPrice;
	///�����
	TSecurityFtdcPriceType	SettlementPrice;
	///ƽ����
	TSecurityFtdcLargeVolumeType	CloseVolume;
	///ƽ�ֽ��
	TSecurityFtdcMoneyType	CloseAmount;
	///������
	TSecurityFtdcMoneyType	TransferFee;
	///ӡ��˰
	TSecurityFtdcMoneyType	StampTax;
	///������
	TSecurityFtdcMoneyType	Commission;
	///AccountID
	TSecurityFtdcAccountIDType	AccountID;
	///��Ѻ�������
	TSecurityFtdcLargeVolumeType	PledgeInPosition;
	///��Ѻ��ⶳ������
	TSecurityFtdcLargeVolumeType	PledgeInFrozenPosition;
	///���ع�ʹ�õı�׼ȯ����
	TSecurityFtdcLargeVolumeType	RepurchasePosition;
	///������ȯ���
	TSecurityFtdcMoneyType	Amount;
	///��ĺ�Լ����
	TSecurityFtdcInstrumentIDType	UnderlyingInstrumentID;
};

///ծȯ��Ϣ
struct CSecurityFtdcBondInterestField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��Ϣ
	TSecurityFtdcInterestType	Interest;
};

///��ֵ������Ϣ
struct CSecurityFtdcMarketRationInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///����������
	TSecurityFtdcLargeVolumeType	RationVolume;
};

///��Լ��������
struct CSecurityFtdcInstrumentCommissionRateField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///Ͷ���߷�Χ
	TSecurityFtdcInvestorRangeType	InvestorRange;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��������
	TSecurityFtdcDirectionType	Direction;
	///ӡ��˰��
	TSecurityFtdcRatioType	StampTaxRateByMoney;
	///ӡ��˰��(������)
	TSecurityFtdcRatioType	StampTaxRateByVolume;
	///��������
	TSecurityFtdcRatioType	TransferFeeRateByMoney;
	///��������(������)
	TSecurityFtdcRatioType	TransferFeeRateByVolume;
	///���׷�
	TSecurityFtdcRatioType	TradeFeeByMoney;
	///���׷�(������)
	TSecurityFtdcRatioType	TradeFeeByVolume;
	///���׸��ӷ���
	TSecurityFtdcRatioType	MarginByMoney;
	///��С���׷�
	TSecurityFtdcPriceType	MinTradeFee;
	///��С������
	TSecurityFtdcPriceType	MinTransferFee;
};

///��ȯ��Ϣ
struct CSecurityFtdcExcessStockInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��ȯ����
	TSecurityFtdcLargeVolumeType	ExcessVolume;
	///��ȯ��������
	TSecurityFtdcLargeVolumeType	ExcessFrozenVolume;
};

///ETF��Լ
struct CSecurityFtdcETFInstrumentField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ETF֤ȯ����
	TSecurityFtdcInstrumentIDType	ETFInstrumentID;
	///ETF��Ӧ�������
	TSecurityFtdcInstrumentIDType	ETFPurRedInstrumentID;
	///��С�깺��ص�λ��Ӧ��ETF����
	TSecurityFtdcVolumeType	CreationRedemptionUnit;
	///����ֽ��������
	TSecurityFtdcRatioType	Maxcashratio;
	///�������깺���״̬
	TSecurityFtdcCreationredemptionStatusType	Creationredemption;
	///Ԥ�����
	TSecurityFtdcMoneyType	EstimateCashComponent;
	///�������굥λ��ֵ
	TSecurityFtdcMoneyType	ETFNetValue;
	///�������
	TSecurityFtdcFundClassType	FundClass;
};

///ETF��Ʊ��
struct CSecurityFtdcETFBasketField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ETF֤ȯ����
	TSecurityFtdcInstrumentIDType	ETFInstrumentID;
	///��Ʊ֤ȯ����
	TSecurityFtdcInstrumentIDType	StockInstrumentID;
	///��Ʊ֤ȯ����
	TSecurityFtdcInstrumentNameType	StockInstrumentName;
	///��Ʊ����
	TSecurityFtdcVolumeType	Volume;
	///�����־
	TSecurityFtdcETFCurrenceReplaceStatusType	CurrenceReplaceStatus;
	///��۱���
	TSecurityFtdcRatioType	Premium;
	///�ܽ��
	TSecurityFtdcMoneyType	Amount;
};

///OF��Լ
struct CSecurityFtdcOFInstrumentField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///OF�������
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�������깺���״̬
	TSecurityFtdcCreationredemptionStatusType	Creationredemption;
	///����ֵ
	TSecurityFtdcPriceType	NetPrice;
	///�������
	TSecurityFtdcFundClassType	FundClass;
};

///SF��Լ
struct CSecurityFtdcSFInstrumentField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�������
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///SF�������
	TSecurityFtdcInstrumentIDType	SFInstrumentID;
	///�������ֺϲ�״̬
	TSecurityFtdcSplitMergeStatusType	SplitMergeStatus;
	///��С�������
	TSecurityFtdcVolumeType	MinSplitVolume;
	///��С�ϲ�����
	TSecurityFtdcVolumeType	MinMergeVolume;
	///���/�ϲ�����
	TSecurityFtdcVolumeType	VolumeRatio;
	///����ֵ
	TSecurityFtdcPriceType	NetPrice;
};

///��Լ���ֱ�֤��
struct CSecurityFtdcInstrumentUnitMarginField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��Լ���ֱ�֤��
	TSecurityFtdcMoneyType	UnitMargin;
};

///��Ȩ�ʽ����Ʋ���
struct CSecurityFtdcOptionsFundLimitParamField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��������
	TSecurityFtdcMoneyType	MaxBuyAmount;
};

///Ͷ������Ȩ�ֲ�
struct CSecurityFtdcInvestorOptionsPositionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���߷�Χ
	TSecurityFtdcInvestorRangeType	InvestorRange;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///InstrumentID
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����ͷ��λ����
	TSecurityFtdcLargeVolumeType	MaxLongPositionLimit;
	///��󿪲�������
	TSecurityFtdcLargeVolumeType	MaxOpenVolLimit;
	///����ܲ�λ����
	TSecurityFtdcLargeVolumeType	MaxPositionLimit;
	///��ͷ�ֲ�
	TSecurityFtdcLargeVolumeType	LongPosition;
	///��ͷ�ֲ�
	TSecurityFtdcLargeVolumeType	ShortPosition;
	///���տ�����
	TSecurityFtdcLargeVolumeType	TodayOpenVolume;
	///���տ��ֶ�����
	TSecurityFtdcLargeVolumeType	TodayOpenFrozenVolume;
	///��ͷ����ֲ�
	TSecurityFtdcLargeVolumeType	LongFrozenPosition;
	///��ͷ����ֲ�
	TSecurityFtdcLargeVolumeType	ShortFrozenPosition;
};

///Ԥ������Ϣ
struct CSecurityFtdcPreDelivInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcDelivTypeType	DelivType;
	///��ĺ�Լ����
	TSecurityFtdcInstrumentIDType	UnderlyingInstrumentID;
	///��������
	TSecurityFtdcLargeVolumeType	DelivVolume;
	///������
	TSecurityFtdcMoneyType	DelivAmount;
	///��Ȩִ������
	TSecurityFtdcLargeVolumeType	ExecVolume;
	///��������
	TSecurityFtdcDirectionType	Direction;
};

///����ȯ������Ϣ
struct CSecurityFtdcCreditStockAssignInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��ȯ����
	TSecurityFtdcLargeVolumeType	LimitVolume;
	///������ȯ����
	TSecurityFtdcLargeVolumeType	YDVolume;
	///ʣ�����ȯ����
	TSecurityFtdcLargeVolumeType	LeftVolume;
	///������ȯ����
	TSecurityFtdcLargeVolumeType	FrozenVolume;
};

///�����ʷ�����Ϣ
struct CSecurityFtdcCreditCashAssignInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///�����޶�
	TSecurityFtdcMoneyType	LimitAmount;
	///�������ʽ��
	TSecurityFtdcMoneyType	YDAmount;
};

///֤ȯ������
struct CSecurityFtdcConversionRateField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�������
	TSecurityFtdcRatioType	ConversionRate;
	///��ǰ�Ƿ�֧�����ʽ���
	TSecurityFtdcBoolType	IsTradingForMargin;
	///��ǰ�Ƿ�֧����ȯ����
	TSecurityFtdcBoolType	IsTradingForShort;
};

///��ʷ���ø�ծ��Ϣ
struct CSecurityFtdcHisCreditDebtInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcDateType	OpenDate;
	///��ծ����
	TSecurityFtdcDirectionType	Direction;
	///���ּ�
	TSecurityFtdcPriceType	OpenPrice;
	///����
	TSecurityFtdcLargeVolumeType	Volume;
	///������ȯ���
	TSecurityFtdcMoneyType	Amount;
};

///���龲̬��Ϣ
struct CSecurityFtdcMarketDataStaticInfoField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��ͣ���
	TSecurityFtdcPriceType	UpperLimitPrice;
	///��ͣ���
	TSecurityFtdcPriceType	LowerLimitPrice;
	///�����
	TSecurityFtdcPriceType	PreSettlementPrice;
	///������
	TSecurityFtdcPriceType	PreClosePrice;
	///���ջ���ֵ
	TSecurityFtdcPriceType	PreIOPV;
	///�Ƿ�ǽ���ҵ��
	TSecurityFtdcBoolType	IsNotTrade;
};

///���ڻع���Ϣ
struct CSecurityFtdcExpireRepurchInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///�ʽ��˻�����
	TSecurityFtdcAccountIDType	AccountID;
	///��������
	TSecurityFtdcExpireTypeType	ExpireType;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����
	TSecurityFtdcLargeVolumeType	Volume;
	///���
	TSecurityFtdcMoneyType	Amount;
	///��Ϣ
	TSecurityFtdcMoneyType	Interest;
};

///ծȯ��ѺΪ��׼ȯ����
struct CSecurityFtdcBondPledgeRateField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�������
	TSecurityFtdcRatioType	Ratio;
};

///ծȯ��Ѻ������չ�ϵ
struct CSecurityFtdcPledgeBondField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��Ѻ����
	TSecurityFtdcInstrumentIDType	PledgeID;
};

///����������Ա���̻�
struct CSecurityFtdcTraderOfferField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///����
	TSecurityFtdcPasswordType	Password;
	///��������
	TSecurityFtdcOfferTypeType	OfferType;
	///��װ���
	TSecurityFtdcInstallIDType	InstallID;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
	///����������Ա����״̬
	TSecurityFtdcTraderConnectStatusType	TraderConnectStatus;
	///�����������������
	TSecurityFtdcDateType	ConnectRequestDate;
	///�������������ʱ��
	TSecurityFtdcTimeType	ConnectRequestTime;
	///�ϴα�������
	TSecurityFtdcDateType	LastReportDate;
	///�ϴα���ʱ��
	TSecurityFtdcTimeType	LastReportTime;
	///�����������
	TSecurityFtdcDateType	ConnectDate;
	///�������ʱ��
	TSecurityFtdcTimeType	ConnectTime;
	///��������
	TSecurityFtdcDateType	StartDate;
	///����ʱ��
	TSecurityFtdcTimeType	StartTime;
	///������
	TSecurityFtdcDateType	TradingDay;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
};

///���������鱨�̻�
struct CSecurityFtdcMDTraderOfferField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///����
	TSecurityFtdcPasswordType	Password;
	///��������
	TSecurityFtdcOfferTypeType	OfferType;
	///��װ���
	TSecurityFtdcInstallIDType	InstallID;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
	///����������Ա����״̬
	TSecurityFtdcTraderConnectStatusType	TraderConnectStatus;
	///�����������������
	TSecurityFtdcDateType	ConnectRequestDate;
	///�������������ʱ��
	TSecurityFtdcTimeType	ConnectRequestTime;
	///�ϴα�������
	TSecurityFtdcDateType	LastReportDate;
	///�ϴα���ʱ��
	TSecurityFtdcTimeType	LastReportTime;
	///�����������
	TSecurityFtdcDateType	ConnectDate;
	///�������ʱ��
	TSecurityFtdcTimeType	ConnectTime;
	///��������
	TSecurityFtdcDateType	StartDate;
	///����ʱ��
	TSecurityFtdcTimeType	StartTime;
	///������
	TSecurityFtdcDateType	TradingDay;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
};

///ǰ��״̬
struct CSecurityFtdcFrontStatusField
{
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�ϴα�������
	TSecurityFtdcDateType	LastReportDate;
	///�ϴα���ʱ��
	TSecurityFtdcTimeType	LastReportTime;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
};

///�û��Ự
struct CSecurityFtdcUserSessionField
{
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///��¼����
	TSecurityFtdcDateType	LoginDate;
	///��¼ʱ��
	TSecurityFtdcTimeType	LoginTime;
	///IP��ַ
	TSecurityFtdcIPAddressType	IPAddress;
	///�û��˲�Ʒ��Ϣ
	TSecurityFtdcProductInfoType	UserProductInfo;
	///�ӿڶ˲�Ʒ��Ϣ
	TSecurityFtdcProductInfoType	InterfaceProductInfo;
	///Э����Ϣ
	TSecurityFtdcProtocolInfoType	ProtocolInfo;
	///Mac��ַ
	TSecurityFtdcMacAddressType	MacAddress;
};

///����
struct CSecurityFtdcOrderField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�����۸�����
	TSecurityFtdcOrderPriceTypeType	OrderPriceType;
	///��������
	TSecurityFtdcDirectionType	Direction;
	///��Ͽ�ƽ��־
	TSecurityFtdcCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TSecurityFtdcCombHedgeFlagType	CombHedgeFlag;
	///�۸�
	TSecurityFtdcStockPriceType	LimitPrice;
	///����
	TSecurityFtdcVolumeType	VolumeTotalOriginal;
	///��Ч������
	TSecurityFtdcTimeConditionType	TimeCondition;
	///GTD����
	TSecurityFtdcDateType	GTDDate;
	///�ɽ�������
	TSecurityFtdcVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TSecurityFtdcVolumeType	MinVolume;
	///��������
	TSecurityFtdcContingentConditionType	ContingentCondition;
	///ֹ���
	TSecurityFtdcPriceType	StopPrice;
	///ǿƽԭ��
	TSecurityFtdcForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TSecurityFtdcBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TSecurityFtdcBusinessUnitType	BusinessUnit;
	///������
	TSecurityFtdcRequestIDType	RequestID;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSecurityFtdcClientIDType	ClientID;
	///��Լ�ڽ������Ĵ���
	TSecurityFtdcExchangeInstIDType	ExchangeInstID;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///��װ���
	TSecurityFtdcInstallIDType	InstallID;
	///�����ύ״̬
	TSecurityFtdcOrderSubmitStatusType	OrderSubmitStatus;
	///�˻�����
	TSecurityFtdcAccountIDType	AccountID;
	///������ʾ���
	TSecurityFtdcSequenceNoType	NotifySequence;
	///������
	TSecurityFtdcDateType	TradingDay;
	///�������
	TSecurityFtdcOrderSysIDType	OrderSysID;
	///������Դ
	TSecurityFtdcOrderSourceType	OrderSource;
	///����״̬
	TSecurityFtdcOrderStatusType	OrderStatus;
	///��������
	TSecurityFtdcOrderTypeType	OrderType;
	///��ɽ�����
	TSecurityFtdcVolumeType	VolumeTraded;
	///ʣ������
	TSecurityFtdcVolumeType	VolumeTotal;
	///��������
	TSecurityFtdcDateType	InsertDate;
	///ί��ʱ��
	TSecurityFtdcTimeType	InsertTime;
	///����ʱ��
	TSecurityFtdcTimeType	ActiveTime;
	///����ʱ��
	TSecurityFtdcTimeType	SuspendTime;
	///����޸�ʱ��
	TSecurityFtdcTimeType	UpdateTime;
	///����ʱ��
	TSecurityFtdcTimeType	CancelTime;
	///����޸Ľ���������Ա����
	TSecurityFtdcTraderIDType	ActiveTraderID;
	///�����Ա���
	TSecurityFtdcParticipantIDType	ClearingPartID;
	///���
	TSecurityFtdcSequenceNoType	SequenceNo;
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///�û��˲�Ʒ��Ϣ
	TSecurityFtdcProductInfoType	UserProductInfo;
	///״̬��Ϣ
	TSecurityFtdcErrorMsgType	StatusMsg;
	///�û�ǿ����־
	TSecurityFtdcBoolType	UserForceClose;
	///�����û�����
	TSecurityFtdcUserIDType	ActiveUserID;
	///���͹�˾�������
	TSecurityFtdcSequenceNoType	BrokerOrderSeq;
	///��ر���
	TSecurityFtdcOrderSysIDType	RelativeOrderSysID;
	///Ӫҵ�����
	TSecurityFtdcBranchIDType	BranchID;
	///�ɽ����
	TSecurityFtdcMoneyType	TradeAmount;
	///�Ƿ�ETF
	TSecurityFtdcBoolType	IsETF;
	///��Լ����
	TSecurityFtdcInstrumentTypeType	InstrumentType;
};

///��������
struct CSecurityFtdcOrderActionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///������������
	TSecurityFtdcOrderActionRefType	OrderActionRef;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///������
	TSecurityFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///������־
	TSecurityFtdcActionFlagType	ActionFlag;
	///�۸�
	TSecurityFtdcPriceType	LimitPrice;
	///�����仯
	TSecurityFtdcVolumeType	VolumeChange;
	///��������
	TSecurityFtdcDateType	ActionDate;
	///����ʱ��
	TSecurityFtdcTimeType	ActionTime;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///��װ���
	TSecurityFtdcInstallIDType	InstallID;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
	///�������ر��
	TSecurityFtdcOrderLocalIDType	ActionLocalID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSecurityFtdcClientIDType	ClientID;
	///ҵ��Ԫ
	TSecurityFtdcBusinessUnitType	BusinessUnit;
	///��������״̬
	TSecurityFtdcOrderActionStatusType	OrderActionStatus;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///Ӫҵ�����
	TSecurityFtdcBranchIDType	BranchID;
	///״̬��Ϣ
	TSecurityFtdcErrorMsgType	StatusMsg;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��Լ����
	TSecurityFtdcInstrumentTypeType	InstrumentType;
};

///���󱨵�
struct CSecurityFtdcErrOrderField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�����۸�����
	TSecurityFtdcOrderPriceTypeType	OrderPriceType;
	///��������
	TSecurityFtdcDirectionType	Direction;
	///��Ͽ�ƽ��־
	TSecurityFtdcCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TSecurityFtdcCombHedgeFlagType	CombHedgeFlag;
	///�۸�
	TSecurityFtdcStockPriceType	LimitPrice;
	///����
	TSecurityFtdcVolumeType	VolumeTotalOriginal;
	///��Ч������
	TSecurityFtdcTimeConditionType	TimeCondition;
	///GTD����
	TSecurityFtdcDateType	GTDDate;
	///�ɽ�������
	TSecurityFtdcVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TSecurityFtdcVolumeType	MinVolume;
	///��������
	TSecurityFtdcContingentConditionType	ContingentCondition;
	///ֹ���
	TSecurityFtdcPriceType	StopPrice;
	///ǿƽԭ��
	TSecurityFtdcForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TSecurityFtdcBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TSecurityFtdcBusinessUnitType	BusinessUnit;
	///������
	TSecurityFtdcRequestIDType	RequestID;
	///�û�ǿ����־
	TSecurityFtdcBoolType	UserForceClose;
	///�������
	TSecurityFtdcErrorIDType	ErrorID;
	///������Ϣ
	TSecurityFtdcErrorMsgType	ErrorMsg;
};

///���󱨵�����
struct CSecurityFtdcErrOrderActionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///������������
	TSecurityFtdcOrderActionRefType	OrderActionRef;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///������
	TSecurityFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///������־
	TSecurityFtdcActionFlagType	ActionFlag;
	///�۸�
	TSecurityFtdcPriceType	LimitPrice;
	///�����仯
	TSecurityFtdcVolumeType	VolumeChange;
	///��������
	TSecurityFtdcDateType	ActionDate;
	///����ʱ��
	TSecurityFtdcTimeType	ActionTime;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///��װ���
	TSecurityFtdcInstallIDType	InstallID;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
	///�������ر��
	TSecurityFtdcOrderLocalIDType	ActionLocalID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSecurityFtdcClientIDType	ClientID;
	///ҵ��Ԫ
	TSecurityFtdcBusinessUnitType	BusinessUnit;
	///��������״̬
	TSecurityFtdcOrderActionStatusType	OrderActionStatus;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///Ӫҵ�����
	TSecurityFtdcBranchIDType	BranchID;
	///״̬��Ϣ
	TSecurityFtdcErrorMsgType	StatusMsg;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�������
	TSecurityFtdcErrorIDType	ErrorID;
	///������Ϣ
	TSecurityFtdcErrorMsgType	ErrorMsg;
	///��Լ����
	TSecurityFtdcInstrumentTypeType	InstrumentType;
};

///�ɽ�
struct CSecurityFtdcTradeField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�ɽ����
	TSecurityFtdcTradeIDType	TradeID;
	///��������
	TSecurityFtdcDirectionType	Direction;
	///�������
	TSecurityFtdcOrderSysIDType	OrderSysID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///�ͻ�����
	TSecurityFtdcClientIDType	ClientID;
	///���׽�ɫ
	TSecurityFtdcTradingRoleType	TradingRole;
	///��Լ�ڽ������Ĵ���
	TSecurityFtdcExchangeInstIDType	ExchangeInstID;
	///��ƽ��־
	TSecurityFtdcOffsetFlagType	OffsetFlag;
	///Ͷ���ױ���־
	TSecurityFtdcHedgeFlagType	HedgeFlag;
	///�۸�
	TSecurityFtdcStockPriceType	Price;
	///����
	TSecurityFtdcVolumeType	Volume;
	///�ɽ�ʱ��
	TSecurityFtdcDateType	TradeDate;
	///�ɽ�ʱ��
	TSecurityFtdcTimeType	TradeTime;
	///�ɽ�����
	TSecurityFtdcTradeTypeType	TradeType;
	///�ɽ�����Դ
	TSecurityFtdcPriceSourceType	PriceSource;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
	///�����Ա���
	TSecurityFtdcParticipantIDType	ClearingPartID;
	///ҵ��Ԫ
	TSecurityFtdcBusinessUnitType	BusinessUnit;
	///���
	TSecurityFtdcSequenceNoType	SequenceNo;
	///�ɽ���Դ
	TSecurityFtdcTradeSourceType	TradeSource;
	///������
	TSecurityFtdcDateType	TradingDay;
	///���͹�˾�������
	TSecurityFtdcSequenceNoType	BrokerOrderSeq;
	///�ɽ����
	TSecurityFtdcMoneyType	TradeAmount;
	///�ɽ����
	TSecurityFtdcTradeIndexType	TradeIndex;
};

///Ͷ���ֲ߳�
struct CSecurityFtdcInvestorPositionField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///�ֲֶ�շ���
	TSecurityFtdcPosiDirectionType	PosiDirection;
	///Ͷ���ױ���־
	TSecurityFtdcHedgeFlagType	HedgeFlag;
	///�ֲ�����
	TSecurityFtdcPositionDateType	PositionDate;
	///���ճֲ�
	TSecurityFtdcLargeVolumeType	YdPosition;
	///�ֲܳ�
	TSecurityFtdcLargeVolumeType	Position;
	///��ͷ����
	TSecurityFtdcLargeVolumeType	LongFrozen;
	///��ͷ����
	TSecurityFtdcLargeVolumeType	ShortFrozen;
	///���ֶ�����
	TSecurityFtdcMoneyType	LongFrozenAmount;
	///���ֶ�����
	TSecurityFtdcMoneyType	ShortFrozenAmount;
	///������
	TSecurityFtdcLargeVolumeType	OpenVolume;
	///ƽ����
	TSecurityFtdcLargeVolumeType	CloseVolume;
	///���ֽ��
	TSecurityFtdcMoneyType	OpenAmount;
	///ƽ�ֽ��
	TSecurityFtdcMoneyType	CloseAmount;
	///�ֲֳɱ�
	TSecurityFtdcMoneyType	PositionCost;
	///������ʽ�
	TSecurityFtdcMoneyType	FrozenCash;
	///�ʽ���
	TSecurityFtdcMoneyType	CashIn;
	///������
	TSecurityFtdcMoneyType	Commission;
	///�ϴν����
	TSecurityFtdcPriceType	PreSettlementPrice;
	///���ν����
	TSecurityFtdcPriceType	SettlementPrice;
	///������
	TSecurityFtdcDateType	TradingDay;
	///���ֳɱ�
	TSecurityFtdcMoneyType	OpenCost;
	///��������֤��
	TSecurityFtdcMoneyType	ExchangeMargin;
	///���ճֲ�
	TSecurityFtdcLargeVolumeType	TodayPosition;
	///������
	TSecurityFtdcMoneyType	TransferFee;
	///ӡ��˰
	TSecurityFtdcMoneyType	StampTax;
	///�����깺�������
	TSecurityFtdcLargeVolumeType	TodayPurRedVolume;
	///������
	TSecurityFtdcRatioType	ConversionRate;
	///������
	TSecurityFtdcMoneyType	ConversionAmount;
	///֤ȯ��ֵ
	TSecurityFtdcMoneyType	StockValue;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///AccountID
	TSecurityFtdcAccountIDType	AccountID;
	///��Ѻ�������
	TSecurityFtdcLargeVolumeType	PledgeInPosition;
	///���ع�ʹ�õı�׼ȯ����
	TSecurityFtdcLargeVolumeType	RepurchasePosition;
	///ETF�����ͷ����
	TSecurityFtdcLargeVolumeType	PurRedShortFrozen;
	///������������
	TSecurityFtdcLargeVolumeType	MarginTradeVolume;
	///����������
	TSecurityFtdcMoneyType	MarginTradeAmount;
	///�������붳������
	TSecurityFtdcLargeVolumeType	MarginTradeFrozenVolume;
	///�������붳����
	TSecurityFtdcMoneyType	MarginTradeFrozenAmount;
	///��������ӯ��
	TSecurityFtdcMoneyType	MarginTradeConversionProfit;
	///��ȯ��������
	TSecurityFtdcLargeVolumeType	ShortSellVolume;
	///��ȯ�������
	TSecurityFtdcMoneyType	ShortSellAmount;
	///��ȯ������������
	TSecurityFtdcLargeVolumeType	ShortSellFrozenVolume;
	///��ȯ����������
	TSecurityFtdcMoneyType	ShortSellFrozenAmount;
	///��ȯ����ӯ��
	TSecurityFtdcMoneyType	ShortSellConversionProfit;
	///��ȯ����ֵ
	TSecurityFtdcMoneyType	SSStockValue;
	///�������ʳֲ�
	TSecurityFtdcLargeVolumeType	TodayMTPosition;
	///������ȯ�ֲ�
	TSecurityFtdcLargeVolumeType	TodaySSPosition;
	///��ʷ�ֲֿ��ֳɱ�
	TSecurityFtdcMoneyType	YdOpenCost;
	///������λ
	TSecurityFtdcLargeVolumeType	LockPosition;
	///���Ҳ�λ
	TSecurityFtdcLargeVolumeType	CoverPosition;
	///���������λ
	TSecurityFtdcLargeVolumeType	LockFrozenPosition;
	///���������λ
	TSecurityFtdcLargeVolumeType	UnlockFrozenPosition;
	///���Ҷ����λ
	TSecurityFtdcLargeVolumeType	CoverFrozenPosition;
	///��Ȩ�����λ
	TSecurityFtdcLargeVolumeType	ExecFrozenPosition;
	///���ձ��Ҳ�λ
	TSecurityFtdcLargeVolumeType	YDCoverPosition;
};

///�����ͬ��
struct CSecurityFtdcSyncDepositField
{
	///�������ˮ��
	TSecurityFtdcDepositSeqNoType	DepositSeqNo;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///�����
	TSecurityFtdcMoneyType	Deposit;
	///�Ƿ�ǿ�ƽ���
	TSecurityFtdcBoolType	IsForce;
	///�˻�����
	TSecurityFtdcAccountIDType	AccountID;
};

///Ͷ���ֲֵ߳���
struct CSecurityFtdcAdjustInvestorPositionField
{
	///�������
	TSecurityFtdcSequenceNoType	OperateID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///���ױ�������
	TSecurityFtdcClientTypeType	ClientType;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�ֲֶ�շ���
	TSecurityFtdcPosiDirectionType	PosiDirection;
	///��������
	TSecurityFtdcLargeVolumeType	AdjustVolume;
};

///��ѯ������
struct CSecurityFtdcQryExchangeField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///��ѯ��Ʒ
struct CSecurityFtdcQryProductField
{
	///��Ʒ����
	TSecurityFtdcInstrumentIDType	ProductID;
};

///��ѯ��Լ
struct CSecurityFtdcQryInstrumentField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ�ڽ������Ĵ���
	TSecurityFtdcExchangeInstIDType	ExchangeInstID;
	///��Ʒ����
	TSecurityFtdcInstrumentIDType	ProductID;
};

///��ѯ���͹�˾
struct CSecurityFtdcQryBrokerField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
};

///��ѯ���͹�˾��Ա����
struct CSecurityFtdcQryPartBrokerField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
};

///��ѯͶ����
struct CSecurityFtdcQryInvestorField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯ���ױ���
struct CSecurityFtdcQryTradingCodeField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�ͻ�����
	TSecurityFtdcClientIDType	ClientID;
};

///��ѯ�����û�
struct CSecurityFtdcQrySuperUserField
{
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��ѯ�����û�����Ȩ��
struct CSecurityFtdcQrySuperUserFunctionField
{
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��ѯ���͹�˾�û�
struct CSecurityFtdcQryBrokerUserField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��ѯ���͹�˾�û�Ȩ��
struct CSecurityFtdcQryBrokerUserFunctionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��ѯ�ʽ��˻�
struct CSecurityFtdcQryTradingAccountField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯ��ֹ��¼�û�
struct CSecurityFtdcQryLoginForbiddenUserField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��ѯ����
struct CSecurityFtdcQryDepthMarketDataField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ��Լ����Ȩ��
struct CSecurityFtdcQryInstrumentTradingRightField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯͶ���ֲ߳���ϸ
struct CSecurityFtdcQryInvestorPositionDetailField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯծȯ��Ϣ
struct CSecurityFtdcQryBondInterestField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ��ֵ������Ϣ
struct CSecurityFtdcQryMarketRationInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ�����ʺ�
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///��ѯ��Լ��������
struct CSecurityFtdcQryInstrumentCommissionRateField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcDirectionType	Direction;
	///��ƽ��־
	TSecurityFtdcOffsetFlagType	OffsetFlag;
};

///��ѯ��ȯ��Ϣ
struct CSecurityFtdcQryExcessStockInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯͶ�����ʻ���ϵ
struct CSecurityFtdcQryInvestorAccountField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯETF��Լ
struct CSecurityFtdcQryETFInstrumentField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ETF֤ȯ����
	TSecurityFtdcInstrumentIDType	ETFInstrumentID;
};

///��ѯETF��Ʊ��
struct CSecurityFtdcQryETFBasketField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ETF֤ȯ����
	TSecurityFtdcInstrumentIDType	ETFInstrumentID;
};

///��ѯOF��Լ
struct CSecurityFtdcQryOFInstrumentField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ETF֤ȯ����
	TSecurityFtdcInstrumentIDType	OFInstrumentID;
};

///��ѯSF��Լ
struct CSecurityFtdcQrySFInstrumentField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ETF֤ȯ����
	TSecurityFtdcInstrumentIDType	SFInstrumentID;
};

///��ѯ��Լ���ֱ�֤��
struct CSecurityFtdcQryInstrumentUnitMarginField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ��Ȩ�ʽ����Ʋ���
struct CSecurityFtdcQryOptionsFundLimitParamField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///��ѯͶ������Ȩ�ֲ�
struct CSecurityFtdcQryInvestorOptionsPositionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��ĺ�Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯԤ������Ϣ
struct CSecurityFtdcQryPreDelivInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ����ȯ������Ϣ
struct CSecurityFtdcQryCreditStockAssignInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ�����ʷ�����Ϣ
struct CSecurityFtdcQryCreditCashAssignInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯ֤ȯ������
struct CSecurityFtdcQryConversionRateField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ��ʷ���ø�ծ��Ϣ
struct CSecurityFtdcQryHisCreditDebtInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ���龲̬��Ϣ
struct CSecurityFtdcQryMarketDataStaticInfoField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ���ڻع���Ϣ
struct CSecurityFtdcQryExpireRepurchInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯծȯ��ѺΪ��׼ȯ����
struct CSecurityFtdcQryBondPledgeRateField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯծȯ��Ѻ������չ�ϵ
struct CSecurityFtdcQryPledgeBondField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ����Ա���̻�
struct CSecurityFtdcQryTraderOfferField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
};

///��ѯ���鱨�̻�
struct CSecurityFtdcQryMDTraderOfferField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Ա����
	TSecurityFtdcParticipantIDType	ParticipantID;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
};

///��ѯǰ��״̬
struct CSecurityFtdcQryFrontStatusField
{
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
};

///��ѯ�û��Ự
struct CSecurityFtdcQryUserSessionField
{
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��ѯ����
struct CSecurityFtdcQryOrderField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�������
	TSecurityFtdcOrderSysIDType	OrderSysID;
	///��ʼʱ��
	TSecurityFtdcTimeType	InsertTimeStart;
	///����ʱ��
	TSecurityFtdcTimeType	InsertTimeEnd;
};

///��ѯ��������
struct CSecurityFtdcQryOrderActionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///��ѯ���󱨵�
struct CSecurityFtdcQryErrOrderField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯ���󱨵�����
struct CSecurityFtdcQryErrOrderActionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ѯ�ɽ�
struct CSecurityFtdcQryTradeField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�ɽ����
	TSecurityFtdcTradeIDType	TradeID;
	///��ʼʱ��
	TSecurityFtdcTimeType	TradeTimeStart;
	///����ʱ��
	TSecurityFtdcTimeType	TradeTimeEnd;
};

///��ѯͶ���ֲ߳�
struct CSecurityFtdcQryInvestorPositionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///��ѯ�������ˮ
struct CSecurityFtdcQrySyncDepositField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�������ˮ��
	TSecurityFtdcDepositSeqNoType	DepositSeqNo;
};

///�û�������
struct CSecurityFtdcUserPasswordUpdateField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///ԭ���Ŀ���
	TSecurityFtdcPasswordType	OldPassword;
	///�µĿ���
	TSecurityFtdcPasswordType	NewPassword;
};

///�ʽ��˻���������
struct CSecurityFtdcTradingAccountPasswordUpdateField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ�����ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///ԭ���Ŀ���
	TSecurityFtdcPasswordType	OldPassword;
	///�µĿ���
	TSecurityFtdcPasswordType	NewPassword;
};

///�ֹ�ͬ���û���̬����
struct CSecurityFtdcManualSyncBrokerUserOTPField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///��̬��������
	TSecurityFtdcOTPTypeType	OTPType;
	///��һ����̬����
	TSecurityFtdcPasswordType	FirstOTP;
	///�ڶ�����̬����
	TSecurityFtdcPasswordType	SecondOTP;
};

///���͹�˾�û�����
struct CSecurityFtdcBrokerUserPasswordField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����
	TSecurityFtdcPasswordType	Password;
};

///�ʽ��˻�������
struct CSecurityFtdcTradingAccountPasswordField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ�����ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///����
	TSecurityFtdcPasswordType	Password;
};

///�û�Ȩ��
struct CSecurityFtdcUserRightField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�ͻ�Ȩ������
	TSecurityFtdcUserRightTypeType	UserRightType;
	///�Ƿ��ֹ
	TSecurityFtdcBoolType	IsForbidden;
};

///Ͷ�����˻�
struct CSecurityFtdcInvestorAccountField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///Ͷ�����ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///�Ƿ����˻�
	TSecurityFtdcBoolType	IsDefault;
	///�˻�����
	TSecurityFtdcAccountTypeType	AccountType;
	///�Ƿ��Ծ
	TSecurityFtdcBoolType	IsActive;
	///�Ͻ������׵�Ԫ��
	TSecurityFtdcTraderIDType	SHBranchPBU;
	///������׵�Ԫ��
	TSecurityFtdcTraderIDType	SZBranchPBU;
};

///�û�IP
struct CSecurityFtdcUserIPField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///IP��ַ
	TSecurityFtdcIPAddressType	IPAddress;
	///IP��ַ����
	TSecurityFtdcIPAddressType	IPMask;
	///Mac��ַ
	TSecurityFtdcMacAddressType	MacAddress;
};

///�û���̬���Ʋ���
struct CSecurityFtdcBrokerUserOTPParamField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///��̬�����ṩ��
	TSecurityFtdcOTPVendorsIDType	OTPVendorsID;
	///��̬�������к�
	TSecurityFtdcSerialNumberType	SerialNumber;
	///������Կ
	TSecurityFtdcAuthKeyType	AuthKey;
	///Ư��ֵ
	TSecurityFtdcLastDriftType	LastDrift;
	///�ɹ�ֵ
	TSecurityFtdcLastSuccessType	LastSuccess;
	///��̬��������
	TSecurityFtdcOTPTypeType	OTPType;
};

///�û���¼����
struct CSecurityFtdcReqUserLoginField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����
	TSecurityFtdcPasswordType	Password;
	///�û��˲�Ʒ��Ϣ
	TSecurityFtdcProductInfoType	UserProductInfo;
	///�ӿڶ˲�Ʒ��Ϣ
	TSecurityFtdcProductInfoType	InterfaceProductInfo;
	///Э����Ϣ
	TSecurityFtdcProtocolInfoType	ProtocolInfo;
	///Mac��ַ
	TSecurityFtdcMacAddressType	MacAddress;
	///��̬����
	TSecurityFtdcPasswordType	OneTimePassword;
	///�ն�IP��ַ
	TSecurityFtdcIPAddressType	ClientIPAddress;
	///�ͻ�����֤��
	TSecurityFtdcAuthCodeType	AuthCode;
	///�����
	TSecurityFtdcAuthCodeType	RandCode;
	///Ӳ�����к�
	TSecurityFtdcHDSerialNumberType	HDSerialNumber;
};

///�û���¼Ӧ��
struct CSecurityFtdcRspUserLoginField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///��¼�ɹ�ʱ��
	TSecurityFtdcTimeType	LoginTime;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����ϵͳ����
	TSecurityFtdcSystemNameType	SystemName;
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///��󱨵�����
	TSecurityFtdcOrderRefType	MaxOrderRef;
};

///�û��ǳ�����
struct CSecurityFtdcUserLogoutField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///ȫ���ǳ���Ϣ
struct CSecurityFtdcLogoutAllField
{
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///ϵͳ����
	TSecurityFtdcSystemNameType	SystemName;
};

///ǿ�ƽ���Ա�˳�
struct CSecurityFtdcForceUserLogoutField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///���͹�˾�û�����
struct CSecurityFtdcActivateBrokerUserField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///��֤�����
struct CSecurityFtdcAuthRandCodeField
{
	///�����
	TSecurityFtdcAuthCodeType	RandCode;
};

///���뱨��
struct CSecurityFtdcInputOrderField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///�����۸�����
	TSecurityFtdcOrderPriceTypeType	OrderPriceType;
	///��������
	TSecurityFtdcDirectionType	Direction;
	///��Ͽ�ƽ��־
	TSecurityFtdcCombOffsetFlagType	CombOffsetFlag;
	///���Ͷ���ױ���־
	TSecurityFtdcCombHedgeFlagType	CombHedgeFlag;
	///�۸�
	TSecurityFtdcStockPriceType	LimitPrice;
	///����
	TSecurityFtdcVolumeType	VolumeTotalOriginal;
	///��Ч������
	TSecurityFtdcTimeConditionType	TimeCondition;
	///GTD����
	TSecurityFtdcDateType	GTDDate;
	///�ɽ�������
	TSecurityFtdcVolumeConditionType	VolumeCondition;
	///��С�ɽ���
	TSecurityFtdcVolumeType	MinVolume;
	///��������
	TSecurityFtdcContingentConditionType	ContingentCondition;
	///ֹ���
	TSecurityFtdcPriceType	StopPrice;
	///ǿƽԭ��
	TSecurityFtdcForceCloseReasonType	ForceCloseReason;
	///�Զ������־
	TSecurityFtdcBoolType	IsAutoSuspend;
	///ҵ��Ԫ
	TSecurityFtdcBusinessUnitType	BusinessUnit;
	///������
	TSecurityFtdcRequestIDType	RequestID;
	///�û�ǿ����־
	TSecurityFtdcBoolType	UserForceClose;
};

///���뱨������
struct CSecurityFtdcInputOrderActionField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///������������
	TSecurityFtdcOrderActionRefType	OrderActionRef;
	///��������
	TSecurityFtdcOrderRefType	OrderRef;
	///������
	TSecurityFtdcRequestIDType	RequestID;
	///ǰ�ñ��
	TSecurityFtdcFrontIDType	FrontID;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///������־
	TSecurityFtdcActionFlagType	ActionFlag;
	///�۸�
	TSecurityFtdcPriceType	LimitPrice;
	///�����仯
	TSecurityFtdcVolumeType	VolumeChange;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������Ա����
	TSecurityFtdcTraderIDType	BranchPBU;
	///���ر������
	TSecurityFtdcOrderLocalIDType	OrderLocalID;
};

///ָ���ĺ�Լ
struct CSecurityFtdcSpecificInstrumentField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///ָ���Ľ�����
struct CSecurityFtdcSpecificExchangeField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///�����������
struct CSecurityFtdcMarketDataBaseField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///�ϴν����
	TSecurityFtdcPriceType	PreSettlementPrice;
	///������
	TSecurityFtdcPriceType	PreClosePrice;
	///��ֲ���
	TSecurityFtdcLargeVolumeType	PreOpenInterest;
	///����ʵ��
	TSecurityFtdcRatioType	PreDelta;
	///���ջ���ֵ
	TSecurityFtdcPriceType	PreIOPV;
};

///���龲̬����
struct CSecurityFtdcMarketDataStaticField
{
	///����
	TSecurityFtdcPriceType	OpenPrice;
	///��߼�
	TSecurityFtdcPriceType	HighestPrice;
	///��ͼ�
	TSecurityFtdcPriceType	LowestPrice;
	///������
	TSecurityFtdcPriceType	ClosePrice;
	///��ͣ���
	TSecurityFtdcPriceType	UpperLimitPrice;
	///��ͣ���
	TSecurityFtdcPriceType	LowerLimitPrice;
	///���ν����
	TSecurityFtdcPriceType	SettlementPrice;
	///����ʵ��
	TSecurityFtdcRatioType	CurrDelta;
	///����ֵ
	TSecurityFtdcPriceType	IOPV;
	///��̬�ο��۸�
	TSecurityFtdcPriceType	AuctionPrice;
};

///�������³ɽ�����
struct CSecurityFtdcMarketDataLastMatchField
{
	///���¼�
	TSecurityFtdcPriceType	LastPrice;
	///����
	TSecurityFtdcLargeVolumeType	Volume;
	///�ɽ����
	TSecurityFtdcMoneyType	Turnover;
	///�ֲ���
	TSecurityFtdcLargeVolumeType	OpenInterest;
};

///�������ż�����
struct CSecurityFtdcMarketDataBestPriceField
{
	///�����һ
	TSecurityFtdcPriceType	BidPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	BidVolume1;
	///������һ
	TSecurityFtdcPriceType	AskPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	AskVolume1;
};

///�����������������
struct CSecurityFtdcMarketDataBid23Field
{
	///����۶�
	TSecurityFtdcPriceType	BidPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume2;
	///�������
	TSecurityFtdcPriceType	BidPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume3;
};

///������������������
struct CSecurityFtdcMarketDataAsk23Field
{
	///�����۶�
	TSecurityFtdcPriceType	AskPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume2;
	///��������
	TSecurityFtdcPriceType	AskPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume3;
};

///���������ġ�������
struct CSecurityFtdcMarketDataBid45Field
{
	///�������
	TSecurityFtdcPriceType	BidPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume4;
	///�������
	TSecurityFtdcPriceType	BidPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume5;
};

///���������ġ�������
struct CSecurityFtdcMarketDataAsk45Field
{
	///��������
	TSecurityFtdcPriceType	AskPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume4;
	///��������
	TSecurityFtdcPriceType	AskPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	AskVolume5;
};

///�������ʱ������
struct CSecurityFtdcMarketDataUpdateTimeField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����޸�ʱ��
	TSecurityFtdcTimeType	UpdateTime;
	///����޸ĺ���
	TSecurityFtdcMillisecType	UpdateMillisec;
	///ҵ������
	TSecurityFtdcDateType	ActionDay;
	///���׽׶�
	TSecurityFtdcTradingPhaseType	TradingPhase;
	///��������
	TSecurityFtdcOpenRestrictionType	OpenRestriction;
};

///�ɽ�����
struct CSecurityFtdcMarketDataAveragePriceField
{
	///���վ���
	TSecurityFtdcPriceType	AveragePrice;
};

///���齻������������
struct CSecurityFtdcMarketDataExchangeField
{
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///��Ϣ�ַ�
struct CSecurityFtdcDisseminationField
{
	///����ϵ�к�
	TSecurityFtdcSequenceSeriesType	SequenceSeries;
	///���к�
	TSecurityFtdcSequenceNoType	SequenceNo;
};

///�ʽ�ת������
struct CSecurityFtdcInputFundTransferField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///Ͷ�����ʽ��ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///�ʽ��ʻ�����
	TSecurityFtdcPasswordType	Password;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///���׽��
	TSecurityFtdcTradeAmountType	TradeAmount;
	///ժҪ
	TSecurityFtdcDigestType	Digest;
	///�˻�����
	TSecurityFtdcAccountTypeType	AccountType;
};

///�ʽ�ת��
struct CSecurityFtdcFundTransferField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///Ͷ�����ʽ��ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///�ʽ��ʻ�����
	TSecurityFtdcPasswordType	Password;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///���׽��
	TSecurityFtdcTradeAmountType	TradeAmount;
	///ժҪ
	TSecurityFtdcDigestType	Digest;
	///�Ự���
	TSecurityFtdcSessionIDType	SessionID;
	///Liber������ˮ��
	TSecurityFtdcLiberSerialType	LiberSerial;
	///ת��ƽ̨��ˮ��
	TSecurityFtdcPlateSerialType	PlateSerial;
	///��������ˮ��
	TSecurityFtdcBankSerialType	TransferSerial;
	///������
	TSecurityFtdcDateType	TradingDay;
	///ת��ʱ��
	TSecurityFtdcTimeType	TradeTime;
	///�������
	TSecurityFtdcFundDirectionType	FundDirection;
	///�������
	TSecurityFtdcErrorIDType	ErrorID;
	///������Ϣ
	TSecurityFtdcErrorMsgType	ErrorMsg;
};

///�ʽ�ת�˲�ѯ����
struct CSecurityFtdcQryFundTransferSerialField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ�����ʽ��ʺ�
	TSecurityFtdcAccountIDType	AccountID;
	///�˻�����
	TSecurityFtdcAccountTypeType	AccountType;
};

///�ʽ���ת
struct CSecurityFtdcFundInterTransferField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�ʽ��˻�����
	TSecurityFtdcAccountIDType	AccountID;
	///�ʽ��˻�����
	TSecurityFtdcPasswordType	Password;
	///���
	TSecurityFtdcTradeAmountType	TradeAmount;
	///��ת����
	TSecurityFtdcFundInterTransferTypeType	TransferType;
	///�ʽ���ת���
	TSecurityFtdcLiberSerialType	SerialID;
};

///�ʽ���ת��ˮ
struct CSecurityFtdcFundInterTransferSerialField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�ʽ��˻�����
	TSecurityFtdcAccountIDType	AccountID;
	///�ʽ��˻�����
	TSecurityFtdcPasswordType	Password;
	///���
	TSecurityFtdcTradeAmountType	TradeAmount;
	///��ת����
	TSecurityFtdcFundInterTransferTypeType	TransferType;
	///�ʽ���ת���
	TSecurityFtdcLiberSerialType	SerialID;
	///ת��ʱ��
	TSecurityFtdcTimeType	TransferTime;
	///�������
	TSecurityFtdcErrorIDType	ErrorID;
	///������Ϣ
	TSecurityFtdcErrorMsgType	ErrorMsg;
};

///�ʽ���ת��ˮ��ѯ����
struct CSecurityFtdcQryFundInterTransferSerialField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///Ͷ���ߴ���
	TSecurityFtdcInvestorIDType	InvestorID;
};

///��ȡ���ݿ���Ϣ
struct CSecurityFtdcFetchDBInfoField
{
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����
	TSecurityFtdcPasswordType	Password;
	///���ݿ�����
	TSecurityFtdcUserNameType	DBIndex;
	///���ݿ�IP��ַ
	TSecurityFtdcIPAddressType	IPAddress;
	///���ݿ�IP�˿�
	TSecurityFtdcIPPortType	IPPort;
	///���ݿ�����
	TSecurityFtdcUserNameType	DBName;
	///���ݿ��û���
	TSecurityFtdcUserIDType	DBUserID;
	///���ݿ�����
	TSecurityFtdcPasswordType	DBPassword;
};

///MD�û���Ϣ
struct CSecurityFtdcMDUserInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�û�����
	TSecurityFtdcUserNameType	UserName;
	///����
	TSecurityFtdcPasswordType	Password;
	///����ϵͳ���
	TSecurityFtdcSequenceNoType	MDSysID;
	///��Ʊ���������
	TSecurityFtdcVolumeType	MaxStockCount;
	///��Ȩ���������
	TSecurityFtdcVolumeType	MaxOptionsCount;
};


#pragma pack(pop) 
#endif