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
	char  	Version;			/**< 版本号	1	二进制无符号整数。目前版本为1*/
	char	Chain;				/**< 报文链	1	ASCII码字符。*/
	unsigned short	SequenceSeries;		/**< 序列类别号	2	二进制无符号短整数。*/
	int	TransactionId;		/**<（TID）	FTD信息正文类型	4	二进制无符号整数。*/
	int	SequenceNumber;		/**<（SeqNo）	序列号	4	二进制无符号整数。*/
	unsigned short	FieldCount;			/**< 数据域数量	2	二进制无符号短整数。*/
	unsigned short	FTDCContentLength;	/**< FTDC信息正文长度	2	二进制无符号短整数。以字节为单位。*/
	int	RequestId;			/**< 请求编号(由发送请求者维护，应答中会带回)  4 二进制无符号整数。*/
};
struct TFieldHeader
{
	unsigned short FieldID;
	unsigned short Size;
};
///行情更新时间属性
class CMarketDataUpdateTimeField
{
public:
	///合约代码
	char InstrumentID[31];
	///最后修改时间
	char UpdateTime[9];
	///最后修改毫秒
	int	UpdateMillisec;
	///业务日期
	char ActionDay[9];
};
///行情最优价属性
class CMarketDataBestPriceField
{
public:
	///申买价一
	double	BidPrice1;
	///申买量一
	int	BidVolume1;
	///申卖价一
	double	AskPrice1;
	///申卖量一
	int	AskVolume1;
};
class CMarketDataStaticField
{
public:
	///今开盘
	double	OpenPrice;
	///最高价
	double	HighestPrice;
	///最低价
	double	LowestPrice;
	///今收盘
	double	ClosePrice;
	///涨停板价
	double	UpperLimitPrice;
	///跌停板价
	double	LowerLimitPrice;
	///今结算
	double	SettlementPrice;
	///今虚实度
	double	CurrDelta;
};
class CMarketDataLastMatchField
{
public:
	///最新价
	double	LastPrice;
	///数量
	int	Volume;
	///成交金额
	double	Turnover;
	///持仓量
	double	OpenInterest;
};
///行情申买二、三属性
class CMarketDataBid23Field
{
public:
	///申买价二
	double	BidPrice2;
	///申买量二
	int	BidVolume2;
	///申买价三
	double	BidPrice3;
	///申买量三
	int	BidVolume3;
};
///行情申卖二、三属性
class CMarketDataAsk23Field
{
public:
	///申卖价二
	double	AskPrice2;
	///申卖量二
	int	AskVolume2;
	///申卖价三
	double	AskPrice3;
	///申卖量三
	int	AskVolume3;
};
///行情申买四、五属性
class CMarketDataBid45Field
{
public:
	///申买价四
	double	BidPrice4;
	///申买量四
	int	BidVolume4;
	///申买价五
	double	BidPrice5;
	///申买量五
	int	BidVolume5;
};
///行情申卖四、五属性
class CMarketDataAsk45Field
{
public:
	///申卖价四
	double	AskPrice4;
	///申卖量四
	int	AskVolume4;
	///申卖价五
	double	AskPrice5;
	///申卖量五
	int	AskVolume5;
};
class CFTDMarketDataBaseField
{
public:
	//交易日
	char TradingDay[9];
	//上次结算价
	double PreSettlementPrice;
	//昨收盘
	double PreClosePrice;
	//昨持仓量
	double PreOpenInterest;
	//昨虚实度
	double PreDelta;
};
///深度行情
class CDepthMarketDataField
{
public:
	///交易日
	char	TradingDay[9];
	///合约代码
	char InstrumentID[31];
	///交易所代码
	char	ExchangeID[9];
	///合约在交易所的代码
	char ExchangeInstID[31];
	///最新价
	double	LastPrice;
	///上次结算价
	double	PreSettlementPrice;
	///昨收盘
	double	PreClosePrice;
	///昨持仓量
	double	PreOpenInterest;
	///今开盘
	double	OpenPrice;
	///最高价
	double	HighestPrice;
	///最低价
	double	LowestPrice;
	///数量
	int Volume;
	///成交金额
	double	Turnover;
	///持仓量
	double	OpenInterest;
	///今收盘
	double	ClosePrice;
	///本次结算价
	double	SettlementPrice;
	///涨停板价
	double	UpperLimitPrice;
	///跌停板价
	double	LowerLimitPrice;
	///昨虚实度
	double	PreDelta;
	///今虚实度
	double	CurrDelta;
	///最后修改时间
	char	UpdateTime[9];
	///最后修改毫秒
	int	UpdateMillisec;
	///申买价一
	double	BidPrice1;
	///申买量一
	int	BidVolume1;
	///申卖价一
	double	AskPrice1;
	///申卖量一
	int	AskVolume1;
	///申买价二
	double	BidPrice2;
	///申买量二
	int	BidVolume2;
	///申卖价二
	double	AskPrice2;
	///申卖量二
	int	AskVolume2;
	///申买价三
	double	BidPrice3;
	///申买量三
	int	BidVolume3;
	///申卖价三
	double	AskPrice3;
	///申卖量三
	int	AskVolume3;
	///申买价四
	double	BidPrice4;
	///申买量四
	int	BidVolume4;
	///申卖价四
	double	AskPrice4;
	///申卖量四
	int	AskVolume4;
	///申买价五
	double	BidPrice5;
	///申买量五
	int	BidVolume5;
	///申卖价五
	double	AskPrice5;
	///申卖量五
	int	AskVolume5;
	///当日均价
	double	AveragePrice;
	///业务日期
	char	ActionDay[9];
};
struct CFFEX_PDU
{
	double		LastPrice;			// 最新价+
	double		OpenPrice;			// 开盘价+
	double		ClosePrice;			// 收盘价+
	double		HighPrice;			// 最高价+
	double		LowPrice;			// 最低价+
	int         TradeVolume;		// 成交数量+
	double		TradeValue;			// 成交金额+
	double		OpenInterest;		// 持仓量+
	double		IOPV;				// 基金净值
	double		AuctionPrice;		// 动态参考价格
	double		BuyPrice1;			// 申买价一+
	double		BuyVolume1;			// 申买量一+
	double		SellPrice1;			// 申卖价一+
	double		SellVolume1;		// 申卖量一+
	double		BuyPrice2;			// 申买价二+
	double		BuyVolume2;			// 申买量二+
	double		SellPrice2;			// 申卖价二+
	double		SellVolume2;		// 申卖量二+
	double		BuyPrice3;			// 申买价三+
	double		BuyVolume3;			// 申买量三+
	double		SellPrice3;			// 申卖价三+
	double		SellVolume3;		// 申卖量三+
	double		BuyPrice4;			// 申买价四+
	double		BuyVolume4;			// 申买量四+
	double		SellPrice4;			// 申卖价四+
	double		SellVolume4;		// 申卖量四+
	double		BuyPrice5;			// 申买价五+
	double		BuyVolume5;			// 申买量五+
	double		SellPrice5;			// 申卖价五+
	double		SellVolume5;		// 申卖量五+
	char		InstrumentID[31+1];	// 证券代码+
	char		TimeStamp[13];		// 时间戳
	char		TradingPhase;		// 交易阶段
	char		OpenRestriction;	// 开仓限制
	char        UpdateTime[9+1];    // 最后修改时间+
	int     	UpdateMillisec;     // 最后修改毫秒+
	char        ActionDay[9+1];     // 业务日期+
	double	    UpperLimitPrice;    // 涨停板价+
	double	    LowerLimitPrice;    // 跌停板价+
	double	    SettlementPrice;    // 今结算+
	double	    CurrDelta;          // 今虚实度+
	char        TradingDay[9+1];    // 交易日+
	double      PreSettlementPrice; // 上次结算价+
	double      PreClosePrice;      // 昨收盘 +
	double      PreOpenInterest;    // 昨持仓量+
	double      PreDelta;           // 昨虚实度+
	char	    ExchangeID[9+1];    // 交易所代码+
	char        ExchangeInstID[31+1];//合约在交易所的代码+
	double	    AveragePrice;        //当日均价+
};
#pragma pack()



#endif//_CFFEX_PDU_V2_H_
