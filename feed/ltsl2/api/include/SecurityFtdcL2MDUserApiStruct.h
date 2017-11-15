/////////////////////////////////////////////////////////////////////////
///@company shanghai liber information Technology Co.,Ltd
///@file SecurityFtdcL2MDUserApiStruct.h
///@brief ����ҵ�����ݽṹ
/////////////////////////////////////////////////////////////////////////

#if !defined(SECURITY_L2MD_FTDCSTRUCT_H)
#define SECURITY_L2MD_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SecurityFtdcL2MDUserApiDataType.h"

_LTS_NS_BEGIN_

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

///�û���¼��Ϣ
struct CSecurityFtdcUserLoginField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///����
	TSecurityFtdcPasswordType	Password;
	///�������ݵȼ�
	TSecurityFtdcDataLevelType	DataLevel;
};

///�û��ǳ���Ϣ
struct CSecurityFtdcUserLogoutField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
};

///ָ���ĺ�Լ
struct CSecurityFtdcSpecificInstrumentField
{
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
};

///Level2����
struct CSecurityFtdcL2MarketDataField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///ʱ���
	TSecurityFtdcTimeType	TimeStamp;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�����̼�
	TSecurityFtdcPriceType	PreClosePrice;
	///���̼�
	TSecurityFtdcPriceType	OpenPrice;
	///���̼�
	TSecurityFtdcPriceType	ClosePrice;
	///��ֵ��ֵ
	TSecurityFtdcPriceType	IOPV;
	///����������
	TSecurityFtdcRatioType	YieldToMaturity;
	///��̬�ο��۸�
	TSecurityFtdcPriceType	AuctionPrice;
	///���׽׶�
	TSecurityFtdcTradingPhaseType	TradingPhase;
	///��������
	TSecurityFtdcOpenRestrictionType	OpenRestriction;
	///��߼�
	TSecurityFtdcPriceType	HighPrice;
	///��ͼ�
	TSecurityFtdcPriceType	LowPrice;
	///���¼�
	TSecurityFtdcPriceType	LastPrice;
	///�ɽ�����
	TSecurityFtdcLargeVolumeType	TradeCount;
	///�ɽ�����
	TSecurityFtdcLargeVolumeType	TotalTradeVolume;
	///�ɽ��ܽ��
	TSecurityFtdcMoneyType	TotalTradeValue;
	///�ֲ���
	TSecurityFtdcLargeVolumeType	OpenInterest;
	///ί����������
	TSecurityFtdcLargeVolumeType	TotalBidVolume;
	///��Ȩƽ��ί���
	TSecurityFtdcPriceType	WeightedAvgBidPrice;
	///ծȯ��Ȩƽ��ί���
	TSecurityFtdcPriceType	AltWeightedAvgBidPrice;
	///ί����������
	TSecurityFtdcLargeVolumeType	TotalOfferVolume;
	///��Ȩƽ��ί����
	TSecurityFtdcPriceType	WeightedAvgOfferPrice;
	///ծȯ��Ȩƽ��ί���۸�
	TSecurityFtdcPriceType	AltWeightedAvgOfferPrice;
	///������
	TSecurityFtdcPriceLevelType	BidPriceLevel;
	///�������
	TSecurityFtdcPriceLevelType	OfferPriceLevel;
	///�����һ
	TSecurityFtdcPriceType	BidPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	BidVolume1;
	///ʵ������ί�б���һ
	TSecurityFtdcVolumeType	BidCount1;
	///����۶�
	TSecurityFtdcPriceType	BidPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume2;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount2;
	///�������
	TSecurityFtdcPriceType	BidPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume3;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount3;
	///�������
	TSecurityFtdcPriceType	BidPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume4;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount4;
	///�������
	TSecurityFtdcPriceType	BidPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume5;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount5;
	///�������
	TSecurityFtdcPriceType	BidPrice6;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume6;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount6;
	///�������
	TSecurityFtdcPriceType	BidPrice7;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume7;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount7;
	///����۰�
	TSecurityFtdcPriceType	BidPrice8;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume8;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount8;
	///����۾�
	TSecurityFtdcPriceType	BidPrice9;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume9;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount9;
	///�����ʮ
	TSecurityFtdcPriceType	BidPriceA;
	///������ʮ
	TSecurityFtdcLargeVolumeType	BidVolumeA;
	///ʵ������ί�б���ʮ
	TSecurityFtdcVolumeType	BidCountA;
	///������һ
	TSecurityFtdcPriceType	OfferPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	OfferVolume1;
	///ʵ������ί�б���һ
	TSecurityFtdcVolumeType	OfferCount1;
	///�����۶�
	TSecurityFtdcPriceType	OfferPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume2;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount2;
	///��������
	TSecurityFtdcPriceType	OfferPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume3;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount3;
	///��������
	TSecurityFtdcPriceType	OfferPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume4;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount4;
	///��������
	TSecurityFtdcPriceType	OfferPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume5;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount5;
	///��������
	TSecurityFtdcPriceType	OfferPrice6;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume6;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount6;
	///��������
	TSecurityFtdcPriceType	OfferPrice7;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume7;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount7;
	///�����۰�
	TSecurityFtdcPriceType	OfferPrice8;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume8;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount8;
	///�����۾�
	TSecurityFtdcPriceType	OfferPrice9;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume9;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount9;
	///������ʮ
	TSecurityFtdcPriceType	OfferPriceA;
	///������ʮ
	TSecurityFtdcLargeVolumeType	OfferVolumeA;
	///ʵ������ί�б���ʮ
	TSecurityFtdcVolumeType	OfferCountA;
	///�������
	TSecurityFtdcMDStreamIDType	MDStreamID;
	///��Լ״̬
	TSecurityFtdcInstrumentStatusType	InstrumentStatus;
	///��ֵ��ֵ
	TSecurityFtdcPriceType	PreIOPV;
	///��ӯ��һ
	TSecurityFtdcRatioType	PERatio1;
	///��ӯ�ʶ�
	TSecurityFtdcRatioType	PERatio2;
	///��ͣ��
	TSecurityFtdcPriceType	UpperLimitPrice;
	///��ͣ��
	TSecurityFtdcPriceType	LowerLimitPrice;
	///Ȩ֤�����
	TSecurityFtdcRatioType	WarrantPremiumRatio;
	///Ȩִ֤��������
	TSecurityFtdcLargeVolumeType	TotalWarrantExecQty;
	///����һ
	TSecurityFtdcPriceType	PriceDiff1;
	///������
	TSecurityFtdcPriceType	PriceDiff2;
	///ETF�깺����
	TSecurityFtdcLargeVolumeType	ETFBuyNumber;
	///ETF�깺����
	TSecurityFtdcLargeVolumeType	ETFBuyAmount;
	///ETF�깺���
	TSecurityFtdcMoneyType	ETFBuyMoney;
	///ETF��ر���
	TSecurityFtdcLargeVolumeType	ETFSellNumber;
	///ETF�������
	TSecurityFtdcLargeVolumeType	ETFSellAmount;
	///ETF��ؽ��
	TSecurityFtdcMoneyType	ETFSellMoney;
	///���볷������
	TSecurityFtdcLargeVolumeType	WithdrawBuyNumber;
	///���볷������
	TSecurityFtdcLargeVolumeType	WithdrawBuyAmount;
	///���볷�����
	TSecurityFtdcMoneyType	WithdrawBuyMoney;
	///�����ܱ���
	TSecurityFtdcLargeVolumeType	TotalBidNumber;
	///����ί�гɽ����ȴ�ʱ��
	TSecurityFtdcLargeVolumeType	BidTradeMaxDuration;
	///��ί�м�λ��
	TSecurityFtdcLargeVolumeType	NumBidOrders;
	///������������
	TSecurityFtdcLargeVolumeType	WithdrawSellNumber;
	///������������
	TSecurityFtdcLargeVolumeType	WithdrawSellAmount;
	///�����������
	TSecurityFtdcMoneyType	WithdrawSellMoney;
	///�����ܱ���
	TSecurityFtdcLargeVolumeType	TotalOfferNumber;
	///����ί�гɽ����ȴ�ʱ��
	TSecurityFtdcLargeVolumeType	OfferTradeMaxDuration;
	///����ί�м�λ��
	TSecurityFtdcLargeVolumeType	NumOfferOrders;
};

///Level2�������ʱ������
struct CSecurityFtdcL2UpdateTimeField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///ʱ���
	TSecurityFtdcTimeType	TimeStamp;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
};

///Level2���龲̬����
struct CSecurityFtdcL2StaticField
{
	///�����̼�
	TSecurityFtdcPriceType	PreClosePrice;
	///���̼�
	TSecurityFtdcPriceType	OpenPrice;
	///���̼�
	TSecurityFtdcPriceType	ClosePrice;
	///��ֵ��ֵ
	TSecurityFtdcPriceType	IOPV;
	///����������
	TSecurityFtdcRatioType	YieldToMaturity;
	///��̬�ο��۸�
	TSecurityFtdcPriceType	AuctionPrice;
	///���׽׶�
	TSecurityFtdcTradingPhaseType	TradingPhase;
	///��������
	TSecurityFtdcOpenRestrictionType	OpenRestriction;
};

///Level2����۸���������
struct CSecurityFtdcL2PriceIntervalField
{
	///��߼�
	TSecurityFtdcPriceType	HighPrice;
	///��ͼ�
	TSecurityFtdcPriceType	LowPrice;
};

///Level2���������Ϣ
struct CSecurityFtdcL2BaseField
{
	///���¼�
	TSecurityFtdcPriceType	LastPrice;
};

///Level2�ɽ���Ϣ
struct CSecurityFtdcL2TradedField
{
	///�ɽ�����
	TSecurityFtdcLargeVolumeType	TradeCount;
	///�ɽ�����
	TSecurityFtdcLargeVolumeType	TotalTradeVolume;
	///�ɽ��ܽ��
	TSecurityFtdcMoneyType	TotalTradeValue;
	///�ֲ���
	TSecurityFtdcLargeVolumeType	OpenInterest;
};

///Level2������������
struct CSecurityFtdcL2DataLevelField
{
	///�۸�
	TSecurityFtdcPriceType	Price;
	///����
	TSecurityFtdcLargeVolumeType	Volume;
	///ʵ����ί�б���
	TSecurityFtdcVolumeType	Count;
};

///Level2ί����Ϣ
struct CSecurityFtdcL2BidOrderField
{
	///ί����������
	TSecurityFtdcLargeVolumeType	TotalBidVolume;
	///��Ȩƽ��ί���
	TSecurityFtdcPriceType	WeightedAvgBidPrice;
	///ծȯ��Ȩƽ��ί���
	TSecurityFtdcPriceType	AltWeightedAvgBidPrice;
};

///Level2ί����Ϣ
struct CSecurityFtdcL2OfferOrderField
{
	///ί����������
	TSecurityFtdcLargeVolumeType	TotalOfferVolume;
	///��Ȩƽ��ί����
	TSecurityFtdcPriceType	WeightedAvgOfferPrice;
	///ծȯ��Ȩƽ��ί���۸�
	TSecurityFtdcPriceType	AltWeightedAvgOfferPrice;
};

///Level2�۸��������
struct CSecurityFtdcL2PriceLevelField
{
	///������
	TSecurityFtdcPriceLevelType	BidPriceLevel;
	///�������
	TSecurityFtdcPriceLevelType	OfferPriceLevel;
};

///Level2��������һ����
struct CSecurityFtdcL2Bid1Field
{
	///�����һ
	TSecurityFtdcPriceType	BidPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	BidVolume1;
	///ʵ������ί�б���һ
	TSecurityFtdcVolumeType	BidCount1;
};

///Level2��������һ����
struct CSecurityFtdcL2Offer1Field
{
	///������һ
	TSecurityFtdcPriceType	OfferPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	OfferVolume1;
	///ʵ������ί�б���һ
	TSecurityFtdcVolumeType	OfferCount1;
};

///Level2�������������
struct CSecurityFtdcL2Bid2Field
{
	///����۶�
	TSecurityFtdcPriceType	BidPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume2;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount2;
};

///Level2��������������
struct CSecurityFtdcL2Offer2Field
{
	///�����۶�
	TSecurityFtdcPriceType	OfferPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume2;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount2;
};

///Level2��������������
struct CSecurityFtdcL2Bid3Field
{
	///�������
	TSecurityFtdcPriceType	BidPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume3;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount3;
};

///Level2��������������
struct CSecurityFtdcL2Offer3Field
{
	///��������
	TSecurityFtdcPriceType	OfferPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume3;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount3;
};

///Level2��������������
struct CSecurityFtdcL2Bid4Field
{
	///�������
	TSecurityFtdcPriceType	BidPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume4;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount4;
};

///Level2��������������
struct CSecurityFtdcL2Offer4Field
{
	///��������
	TSecurityFtdcPriceType	OfferPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume4;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount4;
};

///Level2��������������
struct CSecurityFtdcL2Bid5Field
{
	///�������
	TSecurityFtdcPriceType	BidPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume5;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount5;
};

///Level2��������������
struct CSecurityFtdcL2Offer5Field
{
	///��������
	TSecurityFtdcPriceType	OfferPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume5;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount5;
};

///Level2��������������
struct CSecurityFtdcL2Bid6Field
{
	///�������
	TSecurityFtdcPriceType	BidPrice6;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume6;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount6;
};

///Level2��������������
struct CSecurityFtdcL2Offer6Field
{
	///��������
	TSecurityFtdcPriceType	OfferPrice6;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume6;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount6;
};

///Level2��������������
struct CSecurityFtdcL2Bid7Field
{
	///�������
	TSecurityFtdcPriceType	BidPrice7;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume7;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount7;
};

///Level2��������������
struct CSecurityFtdcL2Offer7Field
{
	///��������
	TSecurityFtdcPriceType	OfferPrice7;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume7;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount7;
};

///Level2�������������
struct CSecurityFtdcL2Bid8Field
{
	///����۰�
	TSecurityFtdcPriceType	BidPrice8;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume8;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount8;
};

///Level2��������������
struct CSecurityFtdcL2Offer8Field
{
	///�����۰�
	TSecurityFtdcPriceType	OfferPrice8;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume8;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount8;
};

///Level2�������������
struct CSecurityFtdcL2Bid9Field
{
	///����۾�
	TSecurityFtdcPriceType	BidPrice9;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume9;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount9;
};

///Level2��������������
struct CSecurityFtdcL2Offer9Field
{
	///�����۾�
	TSecurityFtdcPriceType	OfferPrice9;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume9;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount9;
};

///Level2��������ʮ����
struct CSecurityFtdcL2BidAField
{
	///�����ʮ
	TSecurityFtdcPriceType	BidPriceA;
	///������ʮ
	TSecurityFtdcLargeVolumeType	BidVolumeA;
	///ʵ������ί�б���ʮ
	TSecurityFtdcVolumeType	BidCountA;
};

///Level2��������ʮ����
struct CSecurityFtdcL2OfferAField
{
	///������ʮ
	TSecurityFtdcPriceType	OfferPriceA;
	///������ʮ
	TSecurityFtdcLargeVolumeType	OfferVolumeA;
	///ʵ������ί�б���ʮ
	TSecurityFtdcVolumeType	OfferCountA;
};

///Level2������������
struct CSecurityFtdcL2BidField
{
	///�����һ
	TSecurityFtdcPriceType	BidPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	BidVolume1;
	///ʵ������ί�б���һ
	TSecurityFtdcVolumeType	BidCount1;
	///����۶�
	TSecurityFtdcPriceType	BidPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume2;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount2;
	///�������
	TSecurityFtdcPriceType	BidPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume3;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount3;
	///�������
	TSecurityFtdcPriceType	BidPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume4;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount4;
	///�������
	TSecurityFtdcPriceType	BidPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume5;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount5;
	///�������
	TSecurityFtdcPriceType	BidPrice6;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume6;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount6;
	///�������
	TSecurityFtdcPriceType	BidPrice7;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume7;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount7;
	///����۰�
	TSecurityFtdcPriceType	BidPrice8;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume8;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount8;
	///����۾�
	TSecurityFtdcPriceType	BidPrice9;
	///��������
	TSecurityFtdcLargeVolumeType	BidVolume9;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	BidCount9;
	///�����ʮ
	TSecurityFtdcPriceType	BidPriceA;
	///������ʮ
	TSecurityFtdcLargeVolumeType	BidVolumeA;
	///ʵ������ί�б���ʮ
	TSecurityFtdcVolumeType	BidCountA;
};

///Level2������������
struct CSecurityFtdcL2OfferField
{
	///������һ
	TSecurityFtdcPriceType	OfferPrice1;
	///������һ
	TSecurityFtdcLargeVolumeType	OfferVolume1;
	///ʵ������ί�б���һ
	TSecurityFtdcVolumeType	OfferCount1;
	///�����۶�
	TSecurityFtdcPriceType	OfferPrice2;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume2;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount2;
	///��������
	TSecurityFtdcPriceType	OfferPrice3;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume3;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount3;
	///��������
	TSecurityFtdcPriceType	OfferPrice4;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume4;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount4;
	///��������
	TSecurityFtdcPriceType	OfferPrice5;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume5;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount5;
	///��������
	TSecurityFtdcPriceType	OfferPrice6;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume6;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount6;
	///��������
	TSecurityFtdcPriceType	OfferPrice7;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume7;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount7;
	///�����۰�
	TSecurityFtdcPriceType	OfferPrice8;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume8;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount8;
	///�����۾�
	TSecurityFtdcPriceType	OfferPrice9;
	///��������
	TSecurityFtdcLargeVolumeType	OfferVolume9;
	///ʵ������ί�б�����
	TSecurityFtdcVolumeType	OfferCount9;
	///������ʮ
	TSecurityFtdcPriceType	OfferPriceA;
	///������ʮ
	TSecurityFtdcLargeVolumeType	OfferVolumeA;
	///ʵ������ί�б���ʮ
	TSecurityFtdcVolumeType	OfferCountA;
};

///Level2ָ������
struct CSecurityFtdcL2IndexField
{
	///������
	TSecurityFtdcDateType	TradingDay;
	///����ʱ�䣨�룩
	TSecurityFtdcTimeType	TimeStamp;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///ָ������
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///ǰ����ָ��
	TSecurityFtdcIndexType	PreCloseIndex;
	///����ָ��
	TSecurityFtdcIndexType	OpenIndex;
	///��������ָ��
	TSecurityFtdcIndexType	CloseIndex;
	///���ָ��
	TSecurityFtdcIndexType	HighIndex;
	///���ָ��
	TSecurityFtdcIndexType	LowIndex;
	///����ָ��
	TSecurityFtdcIndexType	LastIndex;
	///���������Ӧָ���ĳɽ���Ԫ��
	TSecurityFtdcMoneyType	TurnOver;
	///���������Ӧָ���Ľ����������֣�
	TSecurityFtdcLargeVolumeType	TotalVolume;
	///�������
	TSecurityFtdcMDStreamIDType	MDStreamID;
};

///Level2�����û���Ϣ
struct CSecurityFtdcL2UserInfoField
{
	///���͹�˾����
	TSecurityFtdcBrokerIDType	BrokerID;
	///�û�����
	TSecurityFtdcUserIDType	UserID;
	///�û�����
	TSecurityFtdcUserNameType	UserName;
	///����
	TSecurityFtdcPasswordType	Password;
	///�������ݵȼ�
	TSecurityFtdcDataLevelType	DataLevel;
	///��Ʊ���������
	TSecurityFtdcVolumeType	MaxStockCount;
	///ָ�����������
	TSecurityFtdcVolumeType	MaxIndexCount;
};

///UDP�鲥����Ϣ
struct CSecurityFtdcMulticastGroupInfoField
{
	///�鲥��IP��ַ
	TSecurityFtdcIPAddressType	GroupIP;
	///�鲥��IP�˿�
	TSecurityFtdcIPPortType	GroupPort;
	///Դ��ַ
	TSecurityFtdcIPAddressType	SourceIP;
};

///Level2���ί��
struct CSecurityFtdcL2OrderField
{
	///ί����
	TSecurityFtdcGroupIDType	OrderGroupID;
	///ί�����
	TSecurityFtdcGroupNoType	OrderIndex;
	///ί��ʱ�䣨�룩
	TSecurityFtdcTimeType	OrderTime;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///ί�м۸�
	TSecurityFtdcPriceType	Price;
	///ί������
	TSecurityFtdcLargeVolumeType	Volume;
	///��������
	TSecurityFtdcOrderKindType	OrderKind;
	///������
	TSecurityFtdcFunctionCodeType	FunctionCode;
	///�������
	TSecurityFtdcMDStreamIDType	MDStreamID;
};

///Level2��ʳɽ�
struct CSecurityFtdcL2TradeField
{
	///�ɽ���
	TSecurityFtdcGroupIDType	TradeGroupID;
	///�ɽ����
	TSecurityFtdcGroupNoType	TradeIndex;
	///��ί�����
	TSecurityFtdcGroupNoType	BuyIndex;
	///����ί�����
	TSecurityFtdcGroupNoType	SellIndex;
	///�ɽ�ʱ�䣨�룩
	TSecurityFtdcTimeType	TradeTime;
	///����������
	TSecurityFtdcExchangeIDType	ExchangeID;
	///��Լ����
	TSecurityFtdcInstrumentIDType	InstrumentID;
	///�ɽ��۸�
	TSecurityFtdcPriceType	Price;
	///�ɽ�����
	TSecurityFtdcLargeVolumeType	Volume;
	///��������
	TSecurityFtdcOrderKindType	OrderKind;
	///������
	TSecurityFtdcFunctionCodeType	FunctionCode;
	///�����̱�־
	TSecurityFtdcOrderBSFlagType	OrderBSFlag;
	///�������
	TSecurityFtdcMDStreamIDType	MDStreamID;
};

///Level2����������̬����
struct CSecurityFtdcL2NewStaticField
{
	///�������
	TSecurityFtdcMDStreamIDType	MDStreamID;
	///��Լ״̬
	TSecurityFtdcInstrumentStatusType	InstrumentStatus;
	///��ֵ��ֵ
	TSecurityFtdcPriceType	PreIOPV;
	///��ӯ��һ
	TSecurityFtdcRatioType	PERatio1;
	///��ӯ�ʶ�
	TSecurityFtdcRatioType	PERatio2;
	///��ͣ��
	TSecurityFtdcPriceType	UpperLimitPrice;
	///��ͣ��
	TSecurityFtdcPriceType	LowerLimitPrice;
	///Ȩ֤�����
	TSecurityFtdcRatioType	WarrantPremiumRatio;
	///Ȩִ֤��������
	TSecurityFtdcLargeVolumeType	TotalWarrantExecQty;
};

///Level2����������Ϣ
struct CSecurityFtdcL2PriceDiffField
{
	///����һ
	TSecurityFtdcPriceType	PriceDiff1;
	///������
	TSecurityFtdcPriceType	PriceDiff2;
};

///Level2ETF�깺��Ϣ
struct CSecurityFtdcL2ETFBuyInfoField
{
	///ETF�깺����
	TSecurityFtdcLargeVolumeType	ETFBuyNumber;
	///ETF�깺����
	TSecurityFtdcLargeVolumeType	ETFBuyAmount;
	///ETF�깺���
	TSecurityFtdcMoneyType	ETFBuyMoney;
};

///Level2ETF�����Ϣ
struct CSecurityFtdcL2ETFSellInfoField
{
	///ETF��ر���
	TSecurityFtdcLargeVolumeType	ETFSellNumber;
	///ETF�������
	TSecurityFtdcLargeVolumeType	ETFSellAmount;
	///ETF��ؽ��
	TSecurityFtdcMoneyType	ETFSellMoney;
};

///Level2������Ϣ
struct CSecurityFtdcL2BuyInfoField
{
	///���볷������
	TSecurityFtdcLargeVolumeType	WithdrawBuyNumber;
	///���볷������
	TSecurityFtdcLargeVolumeType	WithdrawBuyAmount;
	///���볷�����
	TSecurityFtdcMoneyType	WithdrawBuyMoney;
	///�����ܱ���
	TSecurityFtdcLargeVolumeType	TotalBidNumber;
	///����ί�гɽ����ȴ�ʱ��
	TSecurityFtdcLargeVolumeType	BidTradeMaxDuration;
	///��ί�м�λ��
	TSecurityFtdcLargeVolumeType	NumBidOrders;
};

///Level2������Ϣ
struct CSecurityFtdcL2SellInfoField
{
	///������������
	TSecurityFtdcLargeVolumeType	WithdrawSellNumber;
	///������������
	TSecurityFtdcLargeVolumeType	WithdrawSellAmount;
	///�����������
	TSecurityFtdcMoneyType	WithdrawSellMoney;
	///�����ܱ���
	TSecurityFtdcLargeVolumeType	TotalOfferNumber;
	///����ί�гɽ����ȴ�ʱ��
	TSecurityFtdcLargeVolumeType	OfferTradeMaxDuration;
	///����ί�м�λ��
	TSecurityFtdcLargeVolumeType	NumOfferOrders;
};


#pragma pack(pop)
_LTS_NS_END_

#endif
