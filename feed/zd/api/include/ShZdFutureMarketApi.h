/////////////////////////////////////////////////////////////////////////
///@system ��һ�����׽ӿ�ϵͳ
///@company �Ϻ��ڻ���Ϣ�������޹�˾
///@file ShZdFutureMarketApi.h
///@brief ����������ͻ��˽ӿ�
///@history 
///20161106	smithxiang	�������ļ�
/////////////////////////////////////////////////////////////////////////

#if !defined(SHZD_MARKETAPI_H)
#define SHZD_MARKETAPI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ShZdFutureUserApiStruct.h"

#ifdef WIN32 
#ifdef LIB_MARKET_API_EXPORT
#define MARKET_API_EXPORT __declspec(dllexport)
#else
#define MARKET_API_EXPORT __declspec(dllimport)
#endif
#else
#define MARKET_API_EXPORT
#endif

class  CSHZdMarketSpi
{
public:
	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected(){};	
	///��½�ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontLoginConnected(){};
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
	///����Ӧ��
	virtual void OnRspError(CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	///��������Ӧ��  
	virtual void OnRspSubMarketData(CTShZdSpecificInstrumentField *pSpecificInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(CTShZdSpecificInstrumentField *pSpecificInstrument, CTShZdRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CTShZdDepthMarketDataField *pDepthMarketData) {};
	///�ɽ�����֪ͨ,������������
	virtual void OnRtnFilledMarketData(CTShZdFilledDataField* pFilledMarketData){};
};

class MARKET_API_EXPORT CSHZdMarketApi
{
public:
	///����MdApi
	///@param pszFlowPath ����������Ϣ�ļ���Ŀ¼��Ĭ��Ϊ��ǰĿ¼
	///@return ��������UserApi
	///modify for udp marketdata
	static CSHZdMarketApi *CreateSHZdMarketApi(const char *pszFlowPath = "", const bool bIsUsingUdp=false);	
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
	///ע����������������ַ
	///@param pszFrontAddress����������������ַ��
	///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:17001���� 
	///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����17001������������˿ںš�
	virtual void RegisterFront(char *pszFrontAddress) = 0;	
	///ע����ǰ�û������ַ
	///@param pszNsAddress��ǰ�û���ַ��
	///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:12001���� 
	///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����12001������������˿ںš�
	///@remark RegisterNameServer������RegisterFront
	virtual void RegisterLoginFront(char *pszNsAddress) = 0;	
	///ע��ص��ӿ�
	///@param pSpi �����Իص��ӿ����ʵ��
	virtual void RegisterSpi(CSHZdMarketSpi *pSpi) = 0;	
	///�������顣
	///@param ppInstrumentID ��ԼID  
	///@param nCount Ҫ����/�˶�����ĺ�Լ����
	///@remark 
	virtual int SubscribeMarketData(char *ppInstrumentID[], int nCount) = 0;
	///�˶����顣
	///@param ppInstrumentID ��ԼID  
	///@param nCount Ҫ����/�˶�����ĺ�Լ����
	///@remark 
	virtual int UnSubscribeMarketData(char *ppInstrumentID[], int nCount) = 0;
	///�û���¼����
	virtual int ReqUserLogin(CTShZdReqUserLoginField *pReqUserLoginField, int nRequestID) = 0;
	///�ǳ�����
	virtual int ReqUserLogout(CTShZdUserLogoutField *pUserLogout, int nRequestID) = 0;

	///ע����Ϣ
	virtual int AuthonInfo(char* authonInfo)=0;
protected:
	~CSHZdMarketApi(){};
};

#endif
