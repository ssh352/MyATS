/////////////////////////////////////////////////////////////////////////
///@system ��һ��������ϵͳ
///@company �Ϻ��ڻ���Ϣ�������޹�˾
///@file ShZdFutureTraderApi.h
///@brief �����˽��׿ͻ��˽ӿ�
///@history 
///20161106	smithxiang	�������ļ�
/////////////////////////////////////////////////////////////////////////

#if !defined(TSHZd_TRADEAPI_H)
#define TSHZd_TRADEAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ShZdFutureUserApiStruct.h"

#ifdef WIN32 
#ifdef LIB_TRADER_API_EXPORT
#define TRADER_API_EXPORT __declspec(dllexport)
#else
#define TRADER_API_EXPORT __declspec(dllimport)
#endif
#else
#define TRADER_API_EXPORT
#endif
class CSHZdTraderSpi
{
public:
	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected(){};
	
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason){};
		
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse){};
	
	///��¼������Ӧ
	virtual void OnRspUserLogin(CTShZdRspUserLoginField *pRspUserLogin, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CTShZdUserLogoutField *pUserLogout, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�û��������������Ӧ
	virtual void OnRspUserPasswordUpdate(CTShZdUserPasswordUpdateField *pUserPasswordUpdate, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///����¼��������Ӧ
	virtual void OnRspOrderInsert(CTShZdInputOrderField *pInputOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///Ԥ��¼��������Ӧ��ֱ����ʱ��֧�֣�
	virtual void OnRspParkedOrderInsert(CTShZdParkedOrderField *pParkedOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///Ԥ�񳷵�¼��������Ӧ��ֱ����ʱ��֧�֣�
	virtual void OnRspParkedOrderAction(CTShZdParkedOrderActionField *pParkedOrderAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///��������������Ӧ			  
	virtual void OnRspOrderAction(CTShZdInputOrderActionField *pInputOrderAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///ɾ��Ԥ����Ӧ��ֱ����ʱ��֧�֣�
	virtual void OnRspRemoveParkedOrder(CTShZdRemoveParkedOrderField *pRemoveParkedOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///ɾ��Ԥ�񳷵���Ӧ��ֱ����ʱ��֧�֣�
	virtual void OnRspRemoveParkedOrderAction(CTShZdRemoveParkedOrderActionField *pRemoveParkedOrderAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯ������Ӧ
	virtual void OnRspQryOrder(CTShZdOrderField *pOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯ�ɽ���Ӧ
	virtual void OnRspQryTrade(CTShZdTradeField *pTrade, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯͶ���ֲ߳���Ӧ
	virtual void OnRspQryInvestorPosition(CTShZdInvestorPositionField *pInvestorPosition, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯ�ʽ��˻���Ӧ
	virtual void OnRspQryTradingAccount(CTShZdTradingAccountField *pTradingAccount, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	
	///�����ѯ��������Ӧ
	virtual void OnRspQryExchange(CTShZdExchangeField *pExchange, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(CTShZdInstrumentField *pInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯͶ���ֲ߳���ϸ��Ӧ
	virtual void OnRspQryInvestorPositionDetail(CTShZdInvestorPositionDetailField *pInvestorPositionDetail, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
		
	///�����ѯת����ˮ��Ӧ(�����)
	virtual void OnRspQryTransferSerial(CTShZdTransferSerialField *pTransferSerial, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
		
	///����Ӧ��
	virtual void OnRspError(CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///����֪ͨ
	virtual void OnRtnOrder(CTShZdOrderField *pOrder) {};

	///�ɽ�֪ͨ
	virtual void OnRtnTrade(CTShZdTradeField *pTrade) {};

	///�����ʽ�֪ͨ���������ɽ����������ĵ���
	virtual void OnRtnTradeMoney(CTShZdTradeMoneyField *pTradeMoney){};

	///����¼�����ر�
	virtual void OnErrRtnOrderInsert(CTShZdInputOrderField *pInputOrder, CTShZdRspInfoField *pRspInfo) {};
	
	///��ʾ������У�����ֱ����ʱ��֧�֣�
	virtual void OnRtnErrorConditionalOrder(CTShZdErrorConditionalOrderField *pErrorConditionalOrder) {};

	///�����ѯԤ����Ӧ��ֱ����ʱ��֧�֣�
	virtual void OnRspQryParkedOrder(CTShZdParkedOrderField *pParkedOrder, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///�����ѯԤ�񳷵���Ӧ��ֱ����ʱ��֧�֣�
	virtual void OnRspQryParkedOrderAction(CTShZdParkedOrderActionField *pParkedOrderAction, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

	///ֱ�￪����ʱ��
	virtual void OnRtnOpenCloseTime(CTShZdOpenCloseTimeField *openCloseTime,int nRequestID,bool bIsLast){};
	
	///ֱ��֧���г��Ŀ�����ʱ��
	virtual void OnRtnMarketOpenCloseTime(CTShZdMarketOpenCloseTimeField *openCloseTime,int nRequestID,bool bIsLast){};
	
	///ֱ��֧�ֲ�Ʒ�Ŀ�����ʱ��
	virtual void OnRtnCommonOpenCloseTime(CTShZdCommonOpenCloseTimeField *commonTime,int nRequestID,bool bIsLast){};
	
	///ֱ����ʲ�ѯ����
	virtual void OnRspMoneyRatio(CTShZdMoneyRatioField * rspRation,int nRequestID,bool bIsLast){};
};

class TRADER_API_EXPORT CSHZdTraderApi
{
public:
	///����TraderApi
	///@param pszFlowPath ����������Ϣ�ļ���Ŀ¼��Ĭ��Ϊ��ǰĿ¼
	///@return ��������UserApi
	//modify for udp marketdata
	static CSHZdTraderApi *CreateSHZdTraderApi(const char *pszFlowPath = "", const bool bIsUsingUdp=false);
	
	///ɾ���ӿڶ�����
	///@remark ����ʹ�ñ��ӿڶ���ʱ,���øú���ɾ���ӿڶ���
	virtual void Release() = 0;	
	///��ʼ��
	///@remark ��ʼ�����л���,ֻ�е��ú�,�ӿڲſ�ʼ����
	virtual void Init() = 0;
		
	///��ȡ��ǰ������
	///@retrun ��ȡ���Ľ�����
	///@remark ֻ�е�¼�ɹ���,���ܵõ���ȷ�Ľ�����
	virtual const char *GetTradingDay() = 0;
	
	///ע��ǰ�û������ַ
	///@param pszFrontAddress��ǰ�û������ַ��
	///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:17001���� 
	///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����17001������������˿ںš�
	virtual void RegisterFront(char *pszFrontAddress) = 0;	
	///ע��ص��ӿ�
	///@param pSpi �����Իص��ӿ����ʵ��
	virtual void RegisterSpi(CSHZdTraderSpi *pSpi) = 0;			
	///�û���¼����
	virtual int ReqUserLogin(CTShZdReqUserLoginField *pReqUserLoginField, int nRequestID) = 0;
	
	///�ǳ�����
	virtual int ReqUserLogout(CTShZdUserLogoutField *pUserLogout, int nRequestID) = 0;

	///�û������������
	virtual int ReqUserPasswordUpdate(CTShZdUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID) = 0;

	///����¼������
	virtual int ReqOrderInsert(CTShZdInputOrderField *pInputOrder, int nRequestID) = 0;

	///Ԥ��¼������ֱ����ʱ��֧�֣�
	virtual int ReqParkedOrderInsert(CTShZdParkedOrderField *pParkedOrder, int nRequestID) = 0;

	///Ԥ�񳷵�¼������ֱ����ʱ��֧�֣�
	virtual int ReqParkedOrderAction(CTShZdParkedOrderActionField *pParkedOrderAction, int nRequestID) = 0;

	///������������
	virtual int ReqOrderAction(CTShZdOrderActionField *pInputOrderAction, int nRequestID) = 0;

	///����ɾ��Ԥ�񵥣�ֱ����ʱ��֧�֣�
	virtual int ReqRemoveParkedOrder(CTShZdRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID) = 0;

	///����ɾ��Ԥ�񳷵���ֱ����ʱ��֧�֣�
	virtual int ReqRemoveParkedOrderAction(CTShZdRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID) = 0;

	///�����ѯ����
	virtual int ReqQryOrder(CTShZdQryOrderField *pQryOrder, int nRequestID) = 0;

	///�����ѯ�ɽ�
	virtual int ReqQryTrade(CTShZdQryTradeField *pQryTrade, int nRequestID) = 0;

	///�����ѯͶ���ֲֻ߳���
	virtual int ReqQryInvestorPosition(CTShZdQryInvestorPositionField *pQryInvestorPosition, int nRequestID) = 0;

	///�����ѯ�ʽ��˻�
	virtual int ReqQryTradingAccount(CTShZdQryTradingAccountField *pQryTradingAccount, int nRequestID) = 0;

	///�����ѯ������
	virtual int ReqQryExchange(CTShZdQryExchangeField *pQryExchange, int nRequestID) = 0;

	///�����ѯ��Լ
	virtual int ReqQryInstrument(CTShZdQryInstrumentField *pQryInstrument, int nRequestID) = 0;

	///�����ѯͶ���ֲ߳���ϸ
	virtual int ReqQryInvestorPositionDetail(CTShZdQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID) = 0;

	///�����ѯ�����
	virtual int ReqQryTransferSerial(CTShZdQryTransferSerialField *pQryTransferSerial, int nRequestID) = 0;
		
	///�����ѯԤ�񵥣�ֱ����ʱ��֧�֣�
	virtual int ReqQryParkedOrder(CTShZdQryParkedOrderField *pQryParkedOrder, int nRequestID) = 0;

	///�����ѯԤ�񳷵���ֱ����ʱ��֧�֣�
	virtual int ReqQryParkedOrderAction(CTShZdQryParkedOrderActionField *pQryParkedOrderAction, int nRequestID) = 0;
	
	///��ѯֱ�￪����ʱ��
	virtual int ReqQueryOpenCloseTime(CTShZdReqOpenCloseTimeField *OpenCloseTime,int nRequestID)=0;
	
	///��ѯֱ��֧�ֵ��г���ʼ��ʱ��
	virtual int ReqQueryMarketOpenCloseTime(CTShZdReqMarketOpenCloseTimeField *OpenCloseTime,int nRequestID)=0;
	
	///��ѯֱ��֧�ֵĲ�Ʒ�Ŀ�����ʱ��
	virtual int ReqQueryCommonOpenCloseTime(CTShZdReqCommonOpenCloseTimeField *comTime,int nRequestID)=0;

	///��ѯֱ�����
	virtual int ReqQueryMoneyRatio(CTShZdReqMoneyRatioField* reqRation,int nRequestID)=0;

	///ע����Ϣ
	virtual int AuthonInfo(char* authonInfo)=0;
	
protected:
	~CSHZdTraderApi(){};
};


#endif
