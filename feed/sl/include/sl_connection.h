#ifndef __SL_FEED_V2_H__
#define __SL_FEED_V2_H__
#include "sl_source.h"
#include "EESQuoteApi.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace sl
	{
		class sl_connection : /*public RTThread, */public EESQuoteEvent, public feed_connection
		{
		public:
			sl_connection(sl_source* pSource);
			virtual ~sl_connection();

			virtual void init();
			virtual void cleanup();
			virtual void create(){}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			// lts callbacks		 
#if 0
			virtual void OnRspError(_LTS_::CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnFrontConnected();
			virtual void OnFrontDisconnected(int nReason);
			virtual void OnHeartBeatWarning(int nTimeLapse);
			virtual void OnRspUserLogin(_LTS_::CSecurityFtdcUserLoginField* pRspUserLogin, _LTS_::CSecurityFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspSubL2MarketData(_LTS_::CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, _LTS_::CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRspUnSubL2MarketData(_LTS_::CSecurityFtdcSpecificInstrumentField *pSpecificInstrument, _LTS_::CSecurityFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
			virtual void OnRtnL2MarketData(_LTS_::CSecurityFtdcL2MarketDataField *pL2MarketData);
			virtual void Process();
#else
			/// \brief �����������ӳɹ�����¼ǰ����, ������鲥ģʽ���ᷢ��, ֻ���ж�InitMulticast����ֵ����
			virtual void OnEqsConnected();

			/// \brief ���������������ӳɹ������Ͽ�ʱ���ã��鲥ģʽ���ᷢ�����¼�
			virtual void OnEqsDisconnected();

			/// \brief ����¼�ɹ�����ʧ��ʱ���ã��鲥ģʽ���ᷢ��
			/// \param bSuccess ��½�Ƿ�ɹ���־  
			/// \param pReason  ��½ʧ��ԭ��  
			virtual void OnLoginResponse(bool bSuccess, const char* pReason);

			/// \brief �յ�����ʱ����,�����ʽ����instrument_type��ͬ����ͬ
			/// \param chInstrumentType  EES��������
			/// \param pDepthQuoteData   EESͳһ����ָ��  
			virtual void OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* pDepthQuoteData);

			/// \brief ��־�ӿڣ���ʹ���߰���д��־��
			/// \param nlevel    ��־����
			/// \param pLogText  ��־����
			/// \param nLogLen   ��־����
			virtual void OnWriteTextLog(EesEqsLogLevel nlevel, const char* pLogText, int nLogLen);

			/// \brief ע��symbol��Ӧ��Ϣ��ʱ���ã��鲥ģʽ��֧������ע��
			/// \param chInstrumentType  EES��������
			/// \param pSymbol           ��Լ����
			/// \param bSuccess          ע���Ƿ�ɹ���־
			virtual void OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);

			/// \brief  ע��symbol��Ӧ��Ϣ��ʱ���ã��鲥ģʽ��֧������ע��
			/// \param chInstrumentType  EES��������
			/// \param pSymbol           ��Լ����
			/// \param bSuccess          ע���Ƿ�ɹ���־
			virtual void OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);

			/// \brief ��ѯsymbol�б���Ӧ��Ϣ��ʱ���ã��鲥ģʽ��֧�ֺ�Լ�б��ѯ
			/// \param chInstrumentType  EES��������
			/// \param pSymbol           ��Լ����
			/// \param bLast             ���һ����ѯ��Լ�б���Ϣ�ı�ʶ
			/// \remark ��ѯ��Լ�б���Ӧ, last = trueʱ��������������Ч���ݡ�
			virtual void OnSymbolListResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bLast);

		private:
			bool request_login();
#endif
		private:
			sl_source*   m_pSource;
			EESQuoteApi* m_pUserApi;
			int m_nRequestId;
		};
	}
}
#endif // __sl_feed_V2_H__

