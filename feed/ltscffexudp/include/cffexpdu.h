#ifndef _CFFEX_PDU_V2_H_
#define _CFFEX_PDU_V2_H_
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
struct TFTDCHeader
{   
	char  	Version;			/**< �汾��	1	�������޷���������Ŀǰ�汾Ϊ1*/
	char	Chain;				/**< ������	1	ASCII���ַ���*/
	unsigned short	SequenceSeries;		/**< ��������	2	�������޷��Ŷ�������*/
	int	TransactionId;		/**<��TID��	FTD��Ϣ��������	4	�������޷���������*/
	int	SequenceNumber;		/**<��SeqNo��	���к�	4	�������޷���������*/
	unsigned short	FieldCount;			/**< ����������	2	�������޷��Ŷ�������*/
	unsigned short	FTDCContentLength;	/**< FTDC��Ϣ���ĳ���	2	�������޷��Ŷ����������ֽ�Ϊ��λ��*/
	int	RequestId;			/**< ������(�ɷ���������ά����Ӧ���л����)  4 �������޷���������*/
};
struct TFieldHeader
{
	unsigned short FieldID;
	unsigned short Size;
};
///�������ʱ������
class CMarketDataUpdateTimeField
{
public:
	///��Լ����
	char InstrumentID[31];
	///����޸�ʱ��
	char UpdateTime[9];
	///����޸ĺ���
	int	UpdateMillisec;
	///ҵ������
	char ActionDay[9];
};
///�������ż�����
class CMarketDataBestPriceField
{
public:
	///�����һ
	double	BidPrice1;
	///������һ
	int	BidVolume1;
	///������һ
	double	AskPrice1;
	///������һ
	int	AskVolume1;
};
class CMarketDataStaticField
{
public:
	///����
	double	OpenPrice;
	///��߼�
	double	HighestPrice;
	///��ͼ�
	double	LowestPrice;
	///������
	double	ClosePrice;
	///��ͣ���
	double	UpperLimitPrice;
	///��ͣ���
	double	LowerLimitPrice;
	///�����
	double	SettlementPrice;
	///����ʵ��
	double	CurrDelta;
};
class CMarketDataLastMatchField
{
public:
	///���¼�
	double	LastPrice;
	///����
	int	Volume;
	///�ɽ����
	double	Turnover;
	///�ֲ���
	double	OpenInterest;
};
///�����������������
class CMarketDataBid23Field
{
public:
	///����۶�
	double	BidPrice2;
	///��������
	int	BidVolume2;
	///�������
	double	BidPrice3;
	///��������
	int	BidVolume3;
};
///������������������
class CMarketDataAsk23Field
{
public:
	///�����۶�
	double	AskPrice2;
	///��������
	int	AskVolume2;
	///��������
	double	AskPrice3;
	///��������
	int	AskVolume3;
};
///���������ġ�������
class CMarketDataBid45Field
{
public:
	///�������
	double	BidPrice4;
	///��������
	int	BidVolume4;
	///�������
	double	BidPrice5;
	///��������
	int	BidVolume5;
};
///���������ġ�������
class CMarketDataAsk45Field
{
public:
	///��������
	double	AskPrice4;
	///��������
	int	AskVolume4;
	///��������
	double	AskPrice5;
	///��������
	int	AskVolume5;
};
class CFTDMarketDataBaseField
{
public:
	//������
	char TradingDay[9];
	//�ϴν����
	double PreSettlementPrice;
	//������
	double PreClosePrice;
	//��ֲ���
	double PreOpenInterest;
	//����ʵ��
	double PreDelta;
};
///�������
class CDepthMarketDataField
{
public:
	///������
	char	TradingDay[9];
	///��Լ����
	char InstrumentID[31];
	///����������
	char	ExchangeID[9];
	///��Լ�ڽ������Ĵ���
	char ExchangeInstID[31];
	///���¼�
	double	LastPrice;
	///�ϴν����
	double	PreSettlementPrice;
	///������
	double	PreClosePrice;
	///��ֲ���
	double	PreOpenInterest;
	///����
	double	OpenPrice;
	///��߼�
	double	HighestPrice;
	///��ͼ�
	double	LowestPrice;
	///����
	int Volume;
	///�ɽ����
	double	Turnover;
	///�ֲ���
	double	OpenInterest;
	///������
	double	ClosePrice;
	///���ν����
	double	SettlementPrice;
	///��ͣ���
	double	UpperLimitPrice;
	///��ͣ���
	double	LowerLimitPrice;
	///����ʵ��
	double	PreDelta;
	///����ʵ��
	double	CurrDelta;
	///����޸�ʱ��
	char	UpdateTime[9];
	///����޸ĺ���
	int	UpdateMillisec;
	///�����һ
	double	BidPrice1;
	///������һ
	int	BidVolume1;
	///������һ
	double	AskPrice1;
	///������һ
	int	AskVolume1;
	///����۶�
	double	BidPrice2;
	///��������
	int	BidVolume2;
	///�����۶�
	double	AskPrice2;
	///��������
	int	AskVolume2;
	///�������
	double	BidPrice3;
	///��������
	int	BidVolume3;
	///��������
	double	AskPrice3;
	///��������
	int	AskVolume3;
	///�������
	double	BidPrice4;
	///��������
	int	BidVolume4;
	///��������
	double	AskPrice4;
	///��������
	int	AskVolume4;
	///�������
	double	BidPrice5;
	///��������
	int	BidVolume5;
	///��������
	double	AskPrice5;
	///��������
	int	AskVolume5;
	///���վ���
	double	AveragePrice;
	///ҵ������
	char	ActionDay[9];
};
struct CFFEX_PDU
{
	double		LastPrice;			// ���¼�+
	double		OpenPrice;			// ���̼�+
	double		ClosePrice;			// ���̼�+
	double		HighPrice;			// ��߼�+
	double		LowPrice;			// ��ͼ�+
	int         TradeVolume;		// �ɽ�����+
	double		TradeValue;			// �ɽ����+
	double		OpenInterest;		// �ֲ���+
	double		IOPV;				// ����ֵ
	double		AuctionPrice;		// ��̬�ο��۸�
	double		BuyPrice1;			// �����һ+
	double		BuyVolume1;			// ������һ+
	double		SellPrice1;			// ������һ+
	double		SellVolume1;		// ������һ+
	double		BuyPrice2;			// ����۶�+
	double		BuyVolume2;			// ��������+
	double		SellPrice2;			// �����۶�+
	double		SellVolume2;		// ��������+
	double		BuyPrice3;			// �������+
	double		BuyVolume3;			// ��������+
	double		SellPrice3;			// ��������+
	double		SellVolume3;		// ��������+
	double		BuyPrice4;			// �������+
	double		BuyVolume4;			// ��������+
	double		SellPrice4;			// ��������+
	double		SellVolume4;		// ��������+
	double		BuyPrice5;			// �������+
	double		BuyVolume5;			// ��������+
	double		SellPrice5;			// ��������+
	double		SellVolume5;		// ��������+
	char		InstrumentID[31+1];	// ֤ȯ����+
	char		TimeStamp[13];		// ʱ���
	char		TradingPhase;		// ���׽׶�
	char		OpenRestriction;	// ��������
	char        UpdateTime[9+1];    // ����޸�ʱ��+
	int     	UpdateMillisec;     // ����޸ĺ���+
	char        ActionDay[9+1];     // ҵ������+
	double	    UpperLimitPrice;    // ��ͣ���+
	double	    LowerLimitPrice;    // ��ͣ���+
	double	    SettlementPrice;    // �����+
	double	    CurrDelta;          // ����ʵ��+
	char        TradingDay[9+1];    // ������+
	double      PreSettlementPrice; // �ϴν����+
	double      PreClosePrice;      // ������ +
	double      PreOpenInterest;    // ��ֲ���+
	double      PreDelta;           // ����ʵ��+
	char	    ExchangeID[9+1];    // ����������+
	char        ExchangeInstID[31+1];//��Լ�ڽ������Ĵ���+
	double	    AveragePrice;        //���վ���+
};
#pragma pack()



#endif//_CFFEX_PDU_V2_H_
