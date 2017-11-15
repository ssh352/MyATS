#ifndef __LTSSECWRAPPER_H_
#define __LTSSECWRAPPER_H_

#ifdef LTSSECWRAPPER_EXPORTS
#define LTSSECWRAPPER_API __declspec(dllexport)
#else
//#define LTSSECWRAPPER_API __declspec(dllimport)
#define LTSSECWRAPPER_API 
#endif

#include "SecurityFtdcTraderApi.h"
#include "SecurityFtdcQueryApi.h"
#include "stdio.h"

///查询SPI;
class LTSSECWRAPPER_API CSecurityFtdcQuerySpiBase : public CSecurityFtdcQuerySpi{
protected:
	char	m_cUserID[128];
	char	m_cPassword[128];
	int		m_nLastErrorID;
	char	m_cLastErrorMsg[512];
	CSecurityFtdcQueryApi* m_pQueryAPI;

protected:
	int	 ReqRandomCode();
	int ReqUserLogin(const char* nRandomCode);
	

public:
	CSecurityFtdcQuerySpiBase();

	virtual ~CSecurityFtdcQuerySpiBase();

	void SetQueryAPI(CSecurityFtdcQueryApi* pAPI);

	CSecurityFtdcQueryApi* GetQueryAPI();

	void SetUser(const char* pszUser);

	const char* GetUser();

	void SetPassword(const char* pszPass);

	const char* GetPassword();


	///子类可以不需要再实现这个方法，如果要实现，要首先调用：CSecurityFtdcQuerySpiBase::OnFrontConnected();
	virtual void OnFrontConnected();

	///子类可以不需要再实现这个方法，如果要实现，要首先调用：CSecurityFtdcQuerySpiBase::OnRspFetchAuthRandCode;
	virtual void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


///交易SPI
class LTSSECWRAPPER_API CSecurityFtdcTraderSpiBase : public CSecurityFtdcTraderSpi{
protected:
	char	m_cUserID[128];
	char	m_cPassword[128];
	int		m_nLastErrorID;
	char	m_cLastErrorMsg[512];
	CSecurityFtdcTraderApi* m_pTraderAPI;

protected:
	int	 ReqRandomCode();
	int ReqUserLogin(const char* nRandomCode);
	

public:
	CSecurityFtdcTraderSpiBase();

	virtual ~CSecurityFtdcTraderSpiBase();

	void SetTraderAPI(CSecurityFtdcTraderApi* pAPI);

	CSecurityFtdcTraderApi* GetTraderAPI();

	void SetUser(const char* pszUser);

	const char* GetUser();

	void SetPassword(const char* pszPass);

	const char* GetPassword();

	///子类可以不需要再实现这个方法，如果要实现，要首先调用：CSecurityFtdcQuerySpiBase::OnFrontConnected();
	virtual void OnFrontConnected();

	///子类可以不需要再实现这个方法，如果要实现，要首先调用：CSecurityFtdcQuerySpiBase::OnRspFetchAuthRandCode;
	virtual void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


class LTSSECWRAPPER_API CLTSAgent{
public:
	static CSecurityFtdcQueryApi* CreateQueryApi(const char *pszUserID,const char *pszPwd,const char * pszServersGroup,CSecurityFtdcQuerySpiBase* pSpiBase,const char *pszFlowPath = "");		

	static CSecurityFtdcTraderApi* CreateTraderApi(const char *pszUserID,const char *pszPwd,const char * pszServersGroup,CSecurityFtdcTraderSpiBase* pSpiBase,SECURITY_TE_RESUME_TYPE nPrivate,SECURITY_TE_RESUME_TYPE nPublic,const char *pszFlowPath = "");	
};


#endif