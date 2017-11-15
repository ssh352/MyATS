#ifndef __LTS_REQAPI_H__
#define __LTS_REQAPI_H__
#ifdef _WIN32
#include <WinSock2.h>
#endif

#include "SecurityFtdcUserApiStruct.h"
#include "SecurityFtdcUserApiDataType.h"
#include "SecurityFtdcQueryApi.h"
//#include "LTSSecWrapper.h"
#include <string>
namespace lts
{
	class lts_connection;

//<<<<<<< HEAD
	class lts_reqapi : public CSecurityFtdcQuerySpi//CSecurityFtdcQuerySpiBase
//=======
//	class lts_reqapi : public CSecurityFtdcQuerySpi//CSecurityFtdcQuerySpi//CSecurityFtdcQuerySpiBase
//>>>>>>> nas/x1
	{

	public:
		lts_reqapi(lts_connection* pConnection);
		virtual ~lts_reqapi();

		void init();
		void release();
		bool connect();
		bool disconnect();

		bool get_status() { return m_isAlive; }
		void set_status(bool stat) { m_isAlive = stat; }


		// lts callbacks
		 void OnFrontConnected();
		 void OnFrontDisconnected(int nReason);
		 void OnHeartBeatWarning(int nTimeLapse){};
		 void OnRspError(CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
		 void OnRspUserLogin(CSecurityFtdcRspUserLoginField *pRspUserLogin, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
		 void OnRspUserLogout(CSecurityFtdcUserLogoutField *pUserLogout, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		 void OnRspFetchAuthRandCode(CSecurityFtdcAuthRandCodeField *pAuthRandCode, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
		 void OnRspQryExchange(CSecurityFtdcExchangeField *pExchange, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
		 void OnRspQryInstrument(CSecurityFtdcInstrumentField *pInstrument, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
		 void OnRspQryInvestor(CSecurityFtdcInvestorField *pInvestor, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

		 void OnRspQryTradingAccount(CSecurityFtdcTradingAccountField *pTradingAccount, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;
		 

		 void OnRspQryInvestorPosition(CSecurityFtdcInvestorPositionField *pInvestorPosition, CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;


		 bool ReqQryTradingAccount(CSecurityFtdcQryTradingAccountField *pQryTradingAccount);
		 bool ReqQryInvestorPosition(CSecurityFtdcQryInvestorPositionField *pQryInvestorPosition);

		 void getOptionsFeatures(const char* Code, std::string& underlying, std::string& CallPut, std::string& instrumentCalss);
		 std::string getMaturity(std::string sMat);
	
	protected:
		void Process();
		void request_login(CSecurityFtdcAuthRandCodeField *pAuthRandCode);
		void request_auth();
		int request_instruments();



	protected:
		lts_connection* m_pConnection;
		CSecurityFtdcQueryApi* m_pQueryApi;
		//bool m_connectionStatus;
		int m_nRequestId;
		bool m_isAlive=false;
		bool m_bUserReqDiscon = false;
	friend class lts_connection;
	};
}


#endif