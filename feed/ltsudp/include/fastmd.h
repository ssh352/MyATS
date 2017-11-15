#ifndef _FASTMD_H_
#define _FASTMD_H_
//#define WIN32

#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#pragma pack(1)
struct CFAST_MD
{
	double		LastPrice;			// 最新价
	double		OpenPrice;			// 今日开盘价
	double		HighPrice;			// 最高价
	double		LowPrice;			// 最低价
	double		TradeVolume;		// 成交数量
	double		TradeValue;			// 成交金额

	double		BuyPrice1;			// 申买价一
	int			BuyVolume1;			// 申买量一
	double		SellPrice1;			// 申卖价一
	int			SellVolume1;		// 申卖量一
	double		BuyPrice2;			// 申买价二
	int			BuyVolume2;			// 申买量二
	double		SellPrice2;			// 申卖价二
	int			SellVolume2;		// 申卖量二
	double		BuyPrice3;			// 申买价三
	int			BuyVolume3;			// 申买量三
	double		SellPrice3;			// 申卖价三
	int			SellVolume3;		// 申卖量三
	double		BuyPrice4;			// 申买价四
	int			BuyVolume4;			// 申买量四
	double		SellPrice4;			// 申卖价四
	int			SellVolume4;		// 申卖量四
	double		BuyPrice5;			// 申买价五
	int			BuyVolume5;			// 申买量五
	double		SellPrice5;			// 申卖价五
	int			SellVolume5;		// 申卖量五
	
	char		InstrumentID[9];	// 证券代码
	char		TimeStamp[13];		// 时间戳
};
#pragma pack()



#endif
