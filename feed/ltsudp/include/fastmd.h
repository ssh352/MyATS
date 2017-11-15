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
	double		LastPrice;			// ���¼�
	double		OpenPrice;			// ���տ��̼�
	double		HighPrice;			// ��߼�
	double		LowPrice;			// ��ͼ�
	double		TradeVolume;		// �ɽ�����
	double		TradeValue;			// �ɽ����

	double		BuyPrice1;			// �����һ
	int			BuyVolume1;			// ������һ
	double		SellPrice1;			// ������һ
	int			SellVolume1;		// ������һ
	double		BuyPrice2;			// ����۶�
	int			BuyVolume2;			// ��������
	double		SellPrice2;			// �����۶�
	int			SellVolume2;		// ��������
	double		BuyPrice3;			// �������
	int			BuyVolume3;			// ��������
	double		SellPrice3;			// ��������
	int			SellVolume3;		// ��������
	double		BuyPrice4;			// �������
	int			BuyVolume4;			// ��������
	double		SellPrice4;			// ��������
	int			SellVolume4;		// ��������
	double		BuyPrice5;			// �������
	int			BuyVolume5;			// ��������
	double		SellPrice5;			// ��������
	int			SellVolume5;		// ��������
	
	char		InstrumentID[9];	// ֤ȯ����
	char		TimeStamp[13];		// ʱ���
};
#pragma pack()



#endif
