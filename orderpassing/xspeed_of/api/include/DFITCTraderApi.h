/**
* ��Ȩ����(C)2012-2016, �����ɴ���Ϣ�������޹�˾
* �ļ����ƣ�DFITCTraderApi.h
* �ļ�˵��������XSpeed���׽ӿ�
* ��ǰ�汾��1.0.15.5
* ���ߣ�XSpeed��Ŀ��
* �������ڣ�2016��3��18��
*/

#ifndef DFITCTRADERAPI_H_
#define DFITCTRADERAPI_H_

#include "DFITCApiStruct.h"


#ifdef WIN32
    #ifdef DFITCAPI_EXPORTS
        #define DFITCTRADERAPI_API __declspec(dllexport)
    #else
        #define DFITCTRADERAPI_API __declspec(dllimport)
    #endif//DFITCAPI_EXPORTS
#else
    #define DFITCTRADERAPI_API
#endif//WIN32

namespace DFITCXSPEEDAPI
{
    class DFITCTraderSpi
    {
    public:

        /* ��������������Ӧ:���ͻ����뽻�׺�̨�轨����ͨ������ʱ����δ��¼ǰ�����ͻ���API���Զ������ǰ�û�֮������ӣ�
         * ��������ã����Զ��������ӣ������ø÷���֪ͨ�ͻ��ˣ� �ͻ��˿�����ʵ�ָ÷���ʱ������ʹ���ʽ��˺Ž��е�¼��
         *���÷�������Api��ǰ�û��������Ӻ󱻵��ã��õ��ý�����˵��tcp�����Ѿ������ɹ����û���Ҫ���е�¼���ܽ��к�����ҵ�������
         *  ��¼ʧ����˷������ᱻ���á���
         */
        virtual void OnFrontConnected(){};

        /**
         * �������Ӳ�������Ӧ�����ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
         * @param  nReason:����ԭ��
         *        0x1001 �����ʧ��
         *        0x1002 ����дʧ��
         *        0x2001 ����������ʱ
         *        0x2002 ��������ʧ��
         *        0x2003 �յ�������
         */
        virtual void OnFrontDisconnected(int nReason){};
        /**
         * ��½������Ӧ:���û�������¼�����ǰ�û�������Ӧʱ�˷����ᱻ���ã�֪ͨ�û���¼�Ƿ�ɹ���
         * @param pUserLoginInfoRtn:�û���¼��Ϣ�ṹ��ַ��
         * @param pErrorInfo:����¼ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspUserLogin(struct DFITCUserLoginInfoRtnField * pRspUserLogin, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * �ǳ�������Ӧ:���û������˳������ǰ�û�������Ӧ�˷����ᱻ���ã�֪ͨ�û��˳�״̬��
         * @param pUserLogoutInfoRtn:�����û��˳���Ϣ�ṹ��ַ��
         * @param pErrorInfo:���ǳ�ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspUserLogout(struct DFITCUserLogoutInfoRtnField * pRspUserLogout, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * �ڻ�ί�б�����Ӧ:���û�¼�뱨����ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pOrderRtn:�����û��µ���Ϣ�ṹ��ַ��
         * @param pErrorInfo:������ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspInsertOrder(struct DFITCOrderRspDataRtnField * pRspOrder, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * �ڻ�ί�г�����Ӧ:���û�������ǰ�÷�����Ӧ�Ǹ÷����ᱻ���á�
         * @param pOrderCanceledRtn:���س�����Ӧ��Ϣ�ṹ��ַ��
         * @param pErrorInfo:������ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspCancelOrder(struct DFITCOrderRspDataRtnField * pRspOrderCancel, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ����ر�
         * @param pErrorInfo:������Ϣ�Ľṹ��ַ��
         */
        virtual void OnRtnErrorMsg(struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * �ɽ��ر�:��ί�гɹ����׺�˷����ᱻ���á�
         * @param pRtnMatchData:ָ��ɽ��ر��Ľṹ��ָ�롣
         */
        virtual void OnRtnMatchedInfo(struct DFITCMatchRtnField * pRtnMatchData){};

        /**
         * ί�лر�:�µ�ί�гɹ��󣬴˷����ᱻ���á�
         * @param pRtnOrderData:ָ��ί�лر���ַ��ָ�롣
         */
        virtual void OnRtnOrder(struct DFITCOrderRtnField * pRtnOrderData){};

        /**
         * �����ر�:�������ɹ���÷����ᱻ���á�
         * @param pCancelOrderData:ָ�򳷵��ر��ṹ�ĵ�ַ���ýṹ�������������Լ�������Ϣ��
         */
        virtual void OnRtnCancelOrder(struct DFITCOrderCanceledRtnField * pRtnCancelOrderData){};

        /**
         * ��ѯ����ί����Ӧ:���û�����ί�в�ѯ�󣬸÷����ᱻ���á�
         * @param pRtnOrderData:ָ��ί�лر��ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryOrderInfo(struct DFITCOrderCommRtnField * pOrderData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ��ѯ���ճɽ���Ӧ:���û������ɽ���ѯ��÷����ᱻ���á�
         * @param pRtnMatchData:ָ��ɽ��ر��ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryMatchInfo(struct DFITCMatchedRtnField * pMatchData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * �ֲֲ�ѯ��Ӧ:���û������ֲֲ�ѯָ���ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pPositionInfoRtn:���سֲ���Ϣ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryPosition(struct DFITCPositionInfoRtnField * pPositionInfo, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * �ͻ��ʽ��ѯ��Ӧ:���û������ʽ��ѯָ���ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pCapitalInfoRtn:�����ʽ���Ϣ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspCustomerCapital(struct DFITCCapitalInfoRtnField * pCapitalInfo, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ��������Լ��ѯ��Ӧ:���û�������Լ��ѯָ���ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pInstrumentData:���غ�Լ��Ϣ�ṹ�ĵ�ַ��
         * @param pErrorInfo:������Ϣ�ṹ�������ѯ���������򷵻ش�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryExchangeInstrument(struct DFITCExchangeInstrumentRtnField * pInstrumentData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ������Լ��ѯ��Ӧ:���û�����������Լ��ѯָ���ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pAbiInstrumentData:����������Լ��Ϣ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspArbitrageInstrument(struct DFITCAbiInstrumentRtnField * pAbiInstrumentData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ��ѯָ����Լ��Ӧ:���û�����ָ����Լ��ѯָ���ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pInstrument:����ָ����Լ��Ϣ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQrySpecifyInstrument(struct DFITCInstrumentRtnField * pInstrument, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ��ѯ�ֲ���ϸ��Ӧ:���û�������ѯ�ֲ���ϸ��ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pPositionDetailRtn:���سֲ���ϸ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryPositionDetail(struct DFITCPositionDetailRtnField * pPositionDetail, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ����֪ͨ��Ӧ:���ڽ���XSPEED��̨�ֶ�����֪ͨ����֧��ָ���ͻ���Ҳ֧��ϵͳ�㲥��
         * @param pTradingNoticeInfo: �����û��¼�֪ͨ�ṹ�ĵ�ַ��
         */
        virtual void OnRtnTradingNotice(struct DFITCTradingNoticeInfoField * pTradingNoticeInfo){};

        /**
         * �����޸���Ӧ:�����޸��ʽ��˻���¼���롣
         * @param pResetPassword: ���������޸Ľṹ�ĵ�ַ��
         * @param pErrorInfo:���޸�����ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspResetPassword(struct DFITCResetPwdRspField * pResetPassword, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ���ױ����ѯ��Ӧ:���ؽ��ױ�����Ϣ
         * @param pTradeCode: ���ؽ��ױ����ѯ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryTradeCode(struct DFITCQryTradeCodeRtnField * pTradeCode, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * �˵�ȷ����Ӧ:���ڽ��տͻ��˵�ȷ��״̬��
         * @param pBillConfirm: �����˵�ȷ�Ͻṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspBillConfirm(struct DFITCBillConfirmRspField * pBillConfirm, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ��ѯ�ͻ�Ȩ����㷽ʽ��Ӧ:���ؿͻ�Ȩ�����ķ�ʽ
         * @param pEquityComputMode: ���ؿͻ�Ȩ����㷽ʽ�ṹ�ĵ�ַ��
         */
        virtual void OnRspEquityComputMode(struct DFITCEquityComputModeRtnField * pEquityComputMode){};

        /**
         * �ͻ������˵���ѯ��Ӧ:�����˵���Ϣ
         * @param pQryBill: ���ؿͻ������˵���ѯ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryBill(struct DFITCQryBillRtnField *pQryBill, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ����IDȷ����Ӧ:���ڽ��ճ�����Ϣ��
         * @param pProductRtnData: ���س���IDȷ����Ӧ�ṹ�ĵ�ַ��
         */
        virtual void OnRspConfirmProductInfo(struct DFITCProductRtnField * pRspProductData){};


        /**
         * ������ȷ����Ӧ:���ڽ��ս�������Ϣ��
         * @param DFITCTradingDayRtnField: ���ؽ���������ȷ����Ӧ�ṹ�ĵ�ַ��
         */
        virtual void OnRspTradingDay(struct DFITCTradingDayRtnField * pTradingDayRtnData){};

        /**
         * �����̱�����Ӧ
         * @param pRspQuoteData:ָ�������̱�����Ӧ��ַ��ָ�롣
         * @param pErrorInfo:������ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspQuoteInsert(struct DFITCQuoteRspField * pRspQuote, struct DFITCErrorRtnField * pErrorInfo) {};

        /**
         * �����̱����ر�
         * @param pRtnQuoteData:ָ�������̱����ر���ַ��ָ�롣
         */
        virtual void OnRtnQuoteInsert(struct DFITCQuoteRtnField * pRtnQuote){};

        /**
         * �����̳�����Ӧ
         * @param pRspQuoteCanceledData:ָ�������̳�����Ӧ��ַ��ָ�롣
         * @param pErrorInfo:������ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspQuoteCancel(struct DFITCQuoteRspField * pRspQuoteCancel, struct DFITCErrorRtnField * pErrorInfo)  {};

        /**
         * �����̳����ر�
         * @param pRtnQuoteCanceledData:ָ�������̳����ر���ַ��ָ�롣
         */
        virtual void OnRtnQuoteCancel(struct DFITCQuoteCanceledRtnField * pRtnQuoteCanceled) {};

        /**
         * �����̳ɽ��ر�
         * @param pRtnQuoteMatchedData:ָ�������̳ɽ��ر���ַ��ָ�롣
         */
        virtual void OnRtnQuoteMatchedInfo(struct DFITCQuoteMatchRtnField * pRtnQuoteMatched) {};

        /**
         * ����������������Ӧ
         * @param pRspStripCancelOrderData:ָ������������Ӧ��ַ��ָ�롣
         * @param pErrorInfo:����������ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspCancelAllOrder(struct DFITCCancelAllOrderRspField *pRspCancelAllOrderData, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ѯ��֪ͨ��ѯ��Ӧ
         * @param pRtnQryQuoteNoticeData:��ѯѯ��֪ͨ�ر��ṹ��ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryQuoteNotice(struct DFITCQryQuoteNoticeRtnField * pQryQuoteNoticeData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ѯ����Ӧ
         * @param pRspForQuoteData:ѯ��������Ӧ�ṹ��ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspForQuote(struct DFITCForQuoteRspField * pRspForQuoteData, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ѯ�ۻر�
         * @param pRtnForQuoteData:ѯ�ۻر��ṹ��ַ��
         */
        virtual void OnRtnForQuote(struct DFITCForQuoteRtnField * pRtnForQuote){};

        /**
         * ��ѯ���ձ���ί����Ӧ
         * @param pRtnQuoteOrderData:ָ�򱨼۲�ѯ�ر��ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryQuoteOrderInfo(struct DFITCQuoteOrderRtnField * pQuoteOrderData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ѯ��ί�в�ѯ��Ӧ
         * @param pRtnQryForQuoteData:ָ��ѯ��ί�в�ѯ��Ӧ��ַ��ָ�롣
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryForQuote(struct DFITCQryForQuoteRtnField * pQryForQuoteData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ��ѯת��������Ӧ���ݲ�֧��
         * @param pTransferBank:ָ���ѯת�����лر���ַ��ָ�롣
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryTransferBank(struct DFITCTransferBankRspField * pTransferBank, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast) {};

        /**
         * ��ѯת����ˮ��Ӧ���ݲ�֧��
         * @param pTransferSerial:ָ���ѯת����ˮ�ر���ַ��ָ�롣
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryTransferSerial(struct DFITCTransferSerialRspField * pTransferSerial, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast) {};

        /**
         * �ڻ����������ʽ�ת�ڻ�Ӧ���ݲ�֧��
         * @param pRspTransfer:ָ���ڻ����������ʽ�ת�ڻ�Ӧ���ַ��ָ�롣
         * @param pErrorInfo:��ת��ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspFromBankToFutureByFuture(struct DFITCTransferRspField * pRspTransfer, struct DFITCErrorRtnField * pErrorInfo) {};

        /**
         * �ڻ������ڻ��ʽ�ת����Ӧ���ݲ�֧��
         * @param pRspTransfer:ָ���ڻ������ڻ��ʽ�ת����Ӧ���ַ��ָ�롣
         * @param pErrorInfo:��ת��ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspFromFutureToBankByFuture(struct DFITCTransferRspField * pRspTransfer, struct DFITCErrorRtnField * pErrorInfo) {};

        /**
         * �ڻ����������ʽ�ת�ڻ�֪ͨ���ݲ�֧��
         * @param pRtnTransfer:ָ���ڻ����������ʽ�ת�ڻ�֪ͨ��ַ��ָ�롣
         * @param pErrorInfo:��ת��ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRtnFromBankToFutureByFuture(DFITCTransferRtnField * pRtnTransfer, struct DFITCErrorRtnField * pErrorInfo) {};

        /**
         * �ڻ������ڻ��ʽ�ת����֪ͨ���ݲ�֧��
         * @param pRtnTransfer:ָ���ڻ������ڻ��ʽ�ת����֪ͨ��ַ��ָ�롣
         * @param pErrorInfo:��ת��ʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRtnFromFutureToBankByFuture(DFITCTransferRtnField * pRtnTransfer, struct DFITCErrorRtnField * pErrorInfo) {};

        /**
         * ���з�������ڻ�ת����֪ͨ���ݲ�֧��
         * @param pRspRepeal:ָ���ڻ������ڻ��ʽ�ת���г���֪ͨ��ַ��ָ�롣
         */
        virtual void OnRtnRepealFromFutureToBankByBank(DFITCRepealRtnField * pRspRepeal) {};

        /**
         * ������״̬��ѯ��Ӧ
         * @param pRspExchangeStatusData:ָ������״̬��ѯ��Ӧ��ַ��ָ�롣
         */
        virtual void OnRspQryExchangeStatus(struct DFITCExchangeStatusRspField * pRspExchangeStatusData){};

        /**
         * ������״̬֪ͨ
         * @param pRtnExchangeStatusData:ָ������״̬֪ͨ��ַ��ָ�롣
         */
        virtual void OnRtnExchangeStatus(struct DFITCExchangeStatusRtnField * pRtnExchangeStatusData){};

        /**
         * �����ѯ��Ӧ
         * @param pDepthMarketData:ָ�������ѯ��Ӧ��ַ��ָ�롣
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryDepthMarketData(struct DFITCDepthMarketDataField * pDepthMarketData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ���ʲ�ѯ��Ӧ���ݲ�֧��
         * @param pExchangeRate:ָ����ʲ�ѯ��Ӧ��ַ��ָ�롣
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryExchangeRate(struct DFITCExchangeRateField *pExchangeRate, DFITCErrorRtnField *pRspInfo, bool bIsLast) {};

        /**
         * ���鴥����ѯ��Ӧ:���û��������鴥����ѯָ���ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pQryPricesTriggerRspData:���������ѯ��Ϣ�ṹ�ĵ�ַ��
         * @param pErrorInfo:������Ϣ�ṹ��������鴥�����������򷵻ش�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryPricesTrigger(struct DFITCQryPricesTriggerField  *pQryPricesTriggerRspData, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ������ί�б�����Ӧ:���û�¼�뱨����ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pOrderRsp:�����û��µ���Ϣ�ṹ��ַ��
         * @param pErrorInfo:������ʧ�ܣ����ش�����Ϣ��ַ��
         */
        virtual void OnRspExtInsertOrder(struct DFITCExtOrderRspDataField * pOrderRsp, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ������ί�г�����Ӧ:���û�������ǰ�÷�����Ӧ�Ǹ÷����ᱻ���á�
         * @param pOrderCancelRsp:���س�����Ӧ��Ϣ�ṹ��ַ��
         * @param pErrorInfo:������ʧ�ܣ����ش�����Ϣ��ַ��
         */
        virtual void OnRspExtCancelOrder(struct DFITCExtOrderRspDataField * pOrderCancelRsp, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ������ί�лر�:���û�¼�뱨������������÷����ᱻ���á�
         * @param pOrderRtn:�����û��µ���Ϣ�ṹ��ַ��
         */
        virtual void OnRtnPricesTrigger(struct DFITCPricesTriggerRtnField * pOrderRtn){};

        /**
         * ������������������ر�
         * @param pOrderCancel:���ض�����Ϣ�ṹ��ַ��
         */
        virtual void OnErrRtnCancelOrder(struct DFITCOrderCancelErrField * pOrderCancel) {};

        /**
         * �����̳�����������������ر�
         * @param pQuoteCancel:���ر�����Ϣ�ṹ��ַ��
         */
        virtual void OnErrRtnQuoteCancel(struct DFITCQuoteCancelErrField * pQuoteCancel) {};

        /**
         * ��ѯ�˵�ȷ����Ӧ
         * @param pBillConfirmRsp: ���ؽ����˵�ȷ��״̬�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         */
        virtual void OnRspQryBillConfirm(struct DFITCQryBillConfirmRspField * pBillConfirmRsp, struct DFITCErrorRtnField * pErrorInfo){};

        /**
         * ����֪ͨ��ѯ��Ӧ:���û�������ѯ����֪ͨ�ӿں�ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pTradingNotice: �����û��¼�֪ͨ�ṹ�ĵ�ַ��
         */
        virtual void OnRspQryTradingNotice(struct DFITCTradingNoticeField * pTradingNotice, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};

        /**
         * ��ѯ��ϳֲ���ϸ��Ӧ:���û�������ѯ�ֲ���ϸ��ǰ�÷�����Ӧʱ�÷����ᱻ���á�
         * @param pPositionDetailRtn:���سֲ���ϸ�ṹ�ĵ�ַ��
         * @param pErrorInfo:����ѯʧ�ܣ����ش�����Ϣ��ַ���ýṹ���д�����Ϣ��
         * @param bIsLast:�����Ƿ������һ����Ӧ��Ϣ��0 -��   1 -�ǣ���
         */
        virtual void OnRspQryArbitrageCombineDetail(struct DFITCArbitrageCombineDetailRtnField * pPositionDetail, struct DFITCErrorRtnField * pErrorInfo, bool bIsLast){};
    };//end of DFITCTraderSpi

    class DFITCTRADERAPI_API DFITCTraderApi
    {
    public:
        DFITCTraderApi();
        virtual ~DFITCTraderApi();
    public:
        /**
         * ��̬����������һ��apiʵ��
         * @return ��������UserApi
         */
        static DFITCTraderApi * CreateDFITCTraderApi(const char *pszFlowPath = "");

        /**
         * ��ȡAPI�汾��
         * @param nMajorVersion ���汾��
         * @param nMinorVersion �Ӱ汾��
         * @return API��ʶ�ַ���
         */
        static const char *GetVersion(int &nMajorVersion, int &nMinorVersion);

        /**
         * ɾ���ӿڶ���������ʹ�ñ��ӿڶ���ʱ,���øú���ɾ���ӿڶ���
         */
        virtual void Release(void) = 0;

        /**
         * �ͷ���������socket���ӣ�������һ�������̣߳� ͬʱ�÷���ע��һ���ص�������
         * @param pszFrontAddr:����ǰ�������ַ��
         *                     �����ַ�ĸ�ʽΪ:"protocol://ipaddress:port",��"tcp://172.21.200.103:10910"
         *                     ����protocol��ֵΪtcp��ʽ��
         *                     ipaddress��ʾ����ǰ�õ�IP,port��ʾ����ǰ�õĶ˿�
         * @param *pSpi:��DFITCTraderSpi����ʵ��
         * @return 0 - ��ʼ���ɹ�; -1 - ��ʼ��ʧ�ܡ�
         */
        virtual int Init(char * pszFrontAddr, DFITCTraderSpi * pSpi) = 0;

        /**
         * �ȴ��ӿ��߳̽������С�
         * @return �߳��˳����롣
         */
        virtual int Join(void) = 0;

        /**
        * ����˽����
        * @param nResumeType: ˽�����ش���ʽ
        *        DFITC_TERT_RESTART:�ӱ������տ�ʼ�ش�
        *        DFITC_TERT_RESUME:���ϴ��յ�������
        *        DFITC_TERT_QUICK:ֻ���͵�¼��˽����������
        * @remark �÷���Ҫ��ReqUserLogin����ǰ���á����������򲻻��յ�˽���������ݡ�
        */
        virtual void SubscribePrivateTopic(DFITC_TE_RESUME_TYPE nResumeType) = 0;

        /**
        * ��API��־�ļ�
        * @param pszLogFileName: ��־�ļ���
        * @return 0  �����ɹ�
        * @return -1 ����־�ļ�ʧ��
        */
        virtual int OpenApiLog(const char * pszLogFileName) = 0;

        /**
         * �û�������¼����
         * @param pUserLoginData:ָ���û���¼����ṹ�ĵ�ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqUserLogin(struct DFITCUserLoginField * pUserLoginData) = 0;

        /**
         * �û������ǳ�����
         * @param pUserLogoutData:ָ���û���¼����ṹ�ĵ�ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqUserLogout(struct DFITCUserLogoutField * pUserLogoutData) = 0;

        /**
         * �ڻ���Ȩί�б�������
         * @param pInsertOrderData:�û����󱨵���Ϣ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqInsertOrder(struct DFITCInsertOrderField * pInsertOrderData) = 0;

        /**
         * �ڻ���Ȩ��������
         * @param pCancelOrderData:�û����󳷵���Ϣ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         * (����ṩ��̨ί�к�(��̨ί�кŴ���-1)����ֻʹ�ù�̨ί�кŴ���ֻ�е���̨ί�к�С��0ʱ����ʹ�ñ���ί�кŽ��г���)
         */
        virtual int ReqCancelOrder(struct DFITCCancelOrderField * pCancelOrderData) = 0;

        /**
         * �ֲֲ�ѯ����
         * @param pPositionData:�û�����ֲ���Ϣ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         *�����û���ṩ��Լ���룬���ѯȫ���ֲ���Ϣ����
         */
        virtual int ReqQryPosition(struct DFITCPositionField * pPositionData) = 0;

        /**
         * �ͻ��ʽ��ѯ����
         * @param pCapitalData:�����ʽ��ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryCustomerCapital(struct DFITCCapitalField * pCapitalData) = 0;

        /**
         * ��ѯ��������Լ�б�����������
         * @param pExchangeInstrumentData:��������Լ��ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryExchangeInstrument(struct DFITCExchangeInstrumentField * pExchangeInstrumentData) = 0;

        /**
         * ��ѯ������������Լ�б�
         * @param pAbtriInstrumentData:������������Լ��ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryArbitrageInstrument(struct DFITCAbiInstrumentField * pAbtriInstrumentData) = 0;

        /**
         * ����ί�в�ѯ����
         * @param pOrderData:����ί�в�ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryOrderInfo(struct DFITCOrderField * pOrderData) = 0;

        /**
         * ���ճɽ���ѯ����
         * @param pMatchData:���ճɽ���ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryMatchInfo(struct DFITCMatchField * pMatchData) = 0;

        /**
         * ָ����Լ��Ϣ����Լ��֤���ʣ��������ʣ���ѯ����
         * @param pInstrument:ָ����Լ��ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQrySpecifyInstrument(struct DFITCSpecificInstrumentField * pInstrument) = 0;

        /**
         * �ֲ���ϸ��ѯ����
         * @param pPositionDetailData:�ֲ���ϸ��ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryPositionDetail(struct DFITCPositionDetailField * pPositionDetailData) = 0;

        /**
         * ����IDȷ������
         * @param pConfirmProductData:����IDȷ�Ͻṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ�� -���� -����쳣��
         */
        virtual int ReqConfirmProductInfo(struct DFITCProductField * pConfirmProductData) = 0;

        /**
         * �����޸�����
         * @param pResetPasswordData:�����޸Ľṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqResetPassword(struct DFITCResetPwdField * pResetPasswordData) = 0;

        /**
         * �˵�ȷ������
         * @param pBillConfirmData:�˵�ȷ�Ͻṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqBillConfirm(struct DFITCBillConfirmField * pBillConfirmData) = 0;

        /**
         * ���ױ����ѯ����
         * @param pTradeCodeData:���ױ����ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryTradeCode(struct DFITCQryTradeCodeField * pTradeCodeData) = 0;

        /**
         * ��ѯ�ͻ�Ȩ����㷽ʽ����
         * @return 0 - �����ͳɹ�; -1 - ������ʧ�ܡ�
         */
        virtual int ReqEquityComputMode() = 0;

        /**
         * �ͻ��˵������ѯ����
         * @param pQryBillData:�ͻ��˵���������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryBill(struct DFITCQryBillField * pQryBillData) = 0;

        /**
         * �����ղ�ѯ����
         * @param pTradingDay:�����ղ�ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ�� -���� -����쳣��
         */
        virtual int ReqTradingDay(struct DFITCTradingDayField * pTradingDay) = 0;

        /**
         * ѯ��֪ͨ��ѯ����
         * @param pQryQuoteNoticeData:��ѯѯ��֪ͨ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryQuoteNotice(struct DFITCQryQuoteNoticeField * pQryQuoteNoticeData) = 0;

        /**
         * �����̱�������
         * @param pQuoteInsertOrderData:�����̱�������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQuoteInsert(struct DFITCQuoteInsertField * pQuoteInsertOrderData) = 0;

        /**
         * �����̳�������
         * @param pQuoteCancelOrderData:�����̳�������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQuoteCancel(struct DFITCCancelOrderField * pQuoteCancelOrderData) = 0;

        /**
         * ��������������
         * @param pCancelAllOrderData:������������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqCancelAllOrder(struct DFITCCancelAllOrderField * pCancelAllOrderData) = 0;

        /**
         * ѯ������
         * @param pForQuoteData:ѯ������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqForQuote(struct DFITCForQuoteField * pForQuoteData) = 0;

        /**
         * ѯ��ί�в�ѯ����
         * @param pQryForQuoteData:ѯ��ί�в�ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryForQuote(struct DFITCQryForQuoteField * pQryForQuoteData) = 0;

        /**
         * �����̱���ί�в�ѯ
         * @param pQuoteOrderData:�����̱���ί�в�ѯ�ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryQuoteOrderInfo(struct DFITCQuoteOrderField * pQuoteOrderData) = 0;

        /**
         * ��ѯת�����У��ݲ�֧��
         * @param pQryTransferBank:��ѯת����������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryTransferBank(struct DFITCQryTransferBankField * pQryTransferBank) = 0;

        /**
         * ��ѯת����ˮ���ݲ�֧��
         * @param pQryTransferSerial:��ѯת����ˮ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryTransferSerial(struct DFITCQryTransferSerialField * pQryTransferSerial) = 0;

        /**
         * �ڻ����������ʽ�ת�ڻ����ݲ�֧��
         * @param pReqTransfer:�����ʽ�ת�ڻ�����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqFromBankToFutureByFuture(struct DFITCReqTransferField * pReqTransfer) = 0;

        /**
         * �ڻ������ڻ��ʽ�ת���У��ݲ�֧��
         * @param pReqTransfer:�ڻ��ʽ�ת��������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqFromFutureToBankByFuture(struct DFITCReqTransferField * pReqTransfer) = 0;

        /**
         * ������״̬��ѯ
         * @param pQryExchangeStatusData:������״̬��ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryExchangeStatus(struct DFITCQryExchangeStatusField *pQryExchangeStatusData) = 0;

        /**
         * �����ѯ����
         * @param pQryDepthMarketData:�����ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryDepthMarketData(struct DFITCQryDepthMarketDataField *pQryDepthMarketData) = 0;

        /**
         * ���ʲ�ѯ�����ݲ�֧��
         * @param pQryExchangeRate:���ʲ�ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryExchangeRate(struct DFITCQryExchangeRateField *pQryExchangeRate) = 0;

        /**
         * ���鴥������
         * @param pPricesTriggerData:���鴥������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqPricesTrigger(struct DFITCPricesTriggerField * pPricesTriggerData) = 0;

        /**
         * ��������ѯ����
         * @param pQryEXOrderData:��������ѯ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryExtOrder(struct DFITCQryExtOrderField  *pQryEXOrderData) = 0;

        /**
         * ��������������
         * @param pCancelEXOrderData:��������������ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqCancelExtOrder(struct DFITCCancelExtOrderField  *pCancelEXOrderData) = 0;

        /**
         * �����ѯ�����˵���Ϣȷ��
         * @param pQryBillConfirm: ��ѯ�˵��Ƿ�ȷ�ϵ�����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryBillConfirm(struct DFITCQryBillConfirmField * pQryBillConfirm) = 0;

        /**
         * ��ѯ����֪ͨ����
         * @param pQryTradingNotice:��ѯ����֪ͨ����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryTradingNotice(struct DFITCQryTradingNoticeField * pQryTradingNotice) = 0;

        /**
         * ��ϳֲ���ϸ��ѯ����
         * @param pQryArbitrageCombineDetail:��ѯ��ϳֲ�����ṹ��ַ��
         * @return 0 - �����ͳɹ� -1 - ������ʧ��  -���� -����쳣��
         */
        virtual int ReqQryArbitrageCombineDetail(struct DFITCArbitrageCombineDetailField *pQryArbitrageCombineDetail ) = 0;
    };//end of DFITCTraderApi
}
//end of namespace


#endif//DFITCTRADERAPI_H_

