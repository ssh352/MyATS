#ifndef _FASTMD2_H_
#define _FASTMD2_H_
//#define WIN32

#include <stdlib.h>
#include <string.h>

#ifdef Linux
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#pragma pack(1)
struct CFAST_MD
{
	double		LastPrice;			// ���¼�
	double		OpenPrice;			// ���̼�
	double		ClosePrice;			// ���̼�
	double		HighPrice;			// ��߼�
	double		LowPrice;			// ��ͼ�
	double		TradeVolume;		// �ɽ�����
	double		TradeValue;			// �ɽ����
	double		OpenInterest;		// �ֲ���
	double		IOPV;				// ����ֵ
	double		AuctionPrice;		// ��̬�ο��۸�
	double		BuyPrice1;			// �����һ
	double		BuyVolume1;			// ������һ
	double		SellPrice1;			// ������һ
	double		SellVolume1;		// ������һ
	double		BuyPrice2;			// ����۶�
	double		BuyVolume2;			// ��������
	double		SellPrice2;			// �����۶�
	double		SellVolume2;		// ��������
	double		BuyPrice3;			// �������
	double		BuyVolume3;			// ��������
	double		SellPrice3;			// ��������
	double		SellVolume3;		// ��������
	double		BuyPrice4;			// �������
	double		BuyVolume4;			// ��������
	double		SellPrice4;			// ��������
	double		SellVolume4;		// ��������
	double		BuyPrice5;			// �������
	double		BuyVolume5;			// ��������
	double		SellPrice5;			// ��������
	double		SellVolume5;		// ��������
	char		InstrumentID[9];	// ֤ȯ����
	char		TimeStamp[13];		// ʱ���
	char		TradingPhase;		// ���׽׶�
	char		OpenRestriction;	// ��������
};
#pragma pack()


#pragma pack(1)
struct CL2FAST_MD
{
	double		LastPrice;					// ���¼�
	double		OpenPrice;					// ���̼�
	double		ClosePrice;					// ���̼�
	double		HighPrice;					// ��߼�
	double		LowPrice;					// ��ͼ�
	double		TotalTradeVolume;			// �ɽ�����
	double		TotalTradeValue;			// �ɽ����
	double		TradeCount;					// �ɽ�����
	double		OpenInterest;				// �ֲ���
	double		IOPV;						// ����ֵ
	double		YieldToMaturity;			// ����������
	double		AuctionPrice;				// ��̬�ο��۸�
	double		TotalBidVolume;				// ��������
	double		WeightedAvgBidPrice;		// �����Ȩ����
	double		AltWeightedAvgBidPrice;		// ծȯ�����Ȩ����
	double		TotalOfferVolume;			// ��������
	double		WeightedAvgOfferPrice;		// ������Ȩ����
	double		AltWeightedAvgOfferPrice;	// ծȯ������Ȩ����
	int			BidPriceLevel;				// ������
	int			OfferPriceLevel;			// �������
	double		BidPrice1;					// �����һ
	double		BidVolume1;					// ������һ
	double		BidCount1;					// �������һ
	double		OfferPrice1;				// ������һ
	double		OfferVolume1;				// ������һ
	double		OfferCount1;				// ��������һ
	double		BidPrice2;					// ����۶�
	double		BidVolume2;					// ��������
	double		BidCount2;					// ���������
	double		OfferPrice2;				// �����۶�
	double		OfferVolume2;				// ��������
	double		OfferCount2;				// ����������
	double		BidPrice3;					// �������
	double		BidVolume3;					// ��������
	double		BidCount3;					// ���������
	double		OfferPrice3;				// ��������
	double		OfferVolume3;				// ��������
	double		OfferCount3;				// ����������
	double		BidPrice4;					// �������
	double		BidVolume4;					// ��������
	double		BidCount4;					// ���������
	double		OfferPrice4;				// ��������
	double		OfferVolume4;				// ��������
	double		OfferCount4;				// ����������
	double		BidPrice5;					// �������
	double		BidVolume5;					// ��������
	double		BidCount5;					// ���������
	double		OfferPrice5;				// ��������
	double		OfferVolume5;				// ��������
	double		OfferCount5;				// ����������
	double		BidPrice6;					// �������
	double		BidVolume6;					// ��������
	double		BidCount6;					// ���������
	double		OfferPrice6;				// ��������
	double		OfferVolume6;				// ��������
	double		OfferCount6;				// ����������
	double		BidPrice7;					// �������
	double		BidVolume7;					// ��������
	double		BidCount7;					// ���������
	double		OfferPrice7;				// ��������
	double		OfferVolume7;				// ��������
	double		OfferCount7;				// ����������
	double		BidPrice8;					// ����۰�
	double		BidVolume8;					// ��������
	double		BidCount8;					// ���������
	double		OfferPrice8;				// �����۰�
	double		OfferVolume8;				// ��������
	double		OfferCount8;				// ����������
	double		BidPrice9;					// ����۾�
	double		BidVolume9;					// ��������
	double		BidCount9;					// ���������
	double		OfferPrice9;				// �����۾�
	double		OfferVolume9;				// ��������
	double		OfferCount9;				// ����������
	double		BidPriceA;					// �����ʮ
	double		BidVolumeA;					// ������ʮ
	double		BidCountA;					// �������ʮ
	double		OfferPriceA;				// ������ʮ
	double		OfferVolumeA;				// ������ʮ
	double		OfferCountA;				// ��������ʮ
	char		ExchangeID[4];				// ����������
	char		InstrumentID[9];			// ֤ȯ����
	char		TimeStamp[13];				// ʱ���
	char		TradingPhase;				// ���׽׶�
	char		OpenRestriction;			// ��������
	char		MDStreamID[4];				// �������
	char		InstrumentStatus[7];		// ��Լ״̬
	double		PreIOPV;					// �����ֵ
	double		PERatio1;					// ��ӯ��һ
	double		PERatio2;					// ��ӯ�ʶ�
	double		UpperLimitPrice;			// ��ͣ��
	double		LowerLimitPrice;			// ��ͣ��
	double		WarrantPremiumRatio;		// Ȩ֤�����
	double		TotalWarrantExecQty;		// Ȩִ֤��������
	double		PriceDiff1;					// ����һ
	double		PriceDiff2;					// ������
	double		ETFBuyNumber;				// ETF�깺����
	double		ETFBuyAmount;				// ETF�깺����
	double		ETFBuyMoney;				// ETF�깺���
	double		ETFSellNumber;				// ETF��ر���
	double		ETFSellAmount;				// ETF�������
	double		ETFSellMoney;				// ETF��ؽ��
	double		WithdrawBuyNumber;			// ���볷������
	double		WithdrawBuyAmount;			// ���볷������
	double		WithdrawBuyMoney;			// ���볷�����
	double		TotalBidNumber;				// �����ܱ���
	double		BidTradeMaxDuration;		// ����ί�гɽ����ȴ�ʱ��
	double		NumBidOrders;				// ��ί�м�λ��
	double		WithdrawSellNumber;			// ������������
	double		WithdrawSellAmount;			// ������������
	double		WithdrawSellMoney;			// �����������
	double		TotalOfferNumber;			// �����ܱ���
	double		OfferTradeMaxDuration;		// ����ί�гɽ����ȴ�ʱ��
	double		NumOfferOrders;				// ����ί�м�λ��
};

struct CL2FAST_INDEX
{
	double		LastIndex;					// ����ָ��
	double		OpenIndex;					// ����ָ��
	double		CloseIndex;					// ����ָ��
	double		HighIndex;					// ���ָ��
	double		LowIndex;					// ���ָ��
	double		TurnOver;					// �ɽ����
	double		TotalVolume;				// �ɽ�����
	char		ExchangeID[4];				// ����������
	char		InstrumentID[9];			// ָ������
	char		TimeStamp[13];				// ʱ���
	char		MDStreamID[4];				// �������
};

struct CL2FAST_ORDER
{
	double		Price;						// ί�м۸�
	double		Volume;						// ί������
	int			OrderGroupID;				// ί����
	int			OrderIndex;					// ί�����
	char		OrderKind;					// ��������
	char		FunctionCode;				// ������
	char		ExchangeID[4];				// ����������
	char		InstrumentID[9];			// ��Լ����
	char		OrderTime[13];				// ʱ���
	char		MDStreamID[4];				// �������
};

struct CL2FAST_TRADE
{
	double		Price;						// �ɽ��۸�
	double		Volume;						// �ɽ�����
	int			TradeGroupID;				// �ɽ���
	int			TradeIndex;					// �ɽ����
	int			BuyIndex;					// ��ί�����
	int			SellIndex;					// ����ί�����
	char		OrderKind;					// ��������
	char		FunctionCode;				// ������
	char		ExchangeID[4];				// ����������
	char		InstrumentID[9];			// ��Լ����
	char		TradeTime[13];				// ʱ���
	char		OrderBSFlag;				// �����̱�־
	char		MDStreamID[4];				// �������
};
#pragma pack()

#endif
