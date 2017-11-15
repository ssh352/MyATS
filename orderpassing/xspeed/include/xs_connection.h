#ifndef __XS_CONNECTION2_H__
#define __XS_CONNECTION2_H__
#include <vector>
#include "lockfree_classpool_workqueue.h"
#include "xs_order_aux.h"
#include "DFITCSECTraderApi.h"
#include "tbb/concurrent_hash_map.h"
#include <ctpbase_connection.h>

using namespace terra::marketaccess::orderpassing;
using namespace AtsType;



#ifdef _WIN32
#define OSD '\\'
#else
#define OSD '/'
#endif

//#define POLL_NUM 10

namespace xs
{
	typedef struct _localOrderMsg
	{
		int localId;
		int orderId;
		int accId;
		int nTradingType;
		std::string portfolio;

	}localOrderMsg;

	class xs_connection : public ctpbase_connection, public DFITCSECTraderSpi
	{
	public:

		typedef terra::common::lockfree_classpool_workqueue<DFITCSOPEntrustOrderRtnField> xs_order_rtn_queue;//交易所回执
		typedef terra::common::lockfree_classpool_workqueue<DFITCSOPRspEntrustOrderField> xs_order_rsp_queue;//券商回执
		typedef terra::common::lockfree_classpool_workqueue<DFITCSOPWithdrawOrderRtnField> xs_order_can_rtn_queue;
		typedef terra::common::lockfree_classpool_workqueue<DFITCSOPTradeRtnField> xs_trade_queue;

		terra::common::SingleLockFreeClassPool<DFITCSOPReqEntrustOrderField> xs_create_pool;
		terra::common::SingleLockFreeClassPool<DFITCSECReqWithdrawOrderField> xs_cancel_pool;

		//stock orders
		typedef terra::common::lockfree_classpool_workqueue<DFITCStockEntrustOrderRtnField> xs_stockorder_rtn_queue;
		typedef terra::common::lockfree_classpool_workqueue<DFITCStockWithdrawOrderRtnField> xs_stockorder_can_rtn_queue;
		typedef terra::common::lockfree_classpool_workqueue<DFITCStockTradeRtnField> xs_stocktrade_queue;

	public:
		xs_connection(const std::string &m_path,bool checkSecurities = true);
		//virtual ~xs_connection();

		// connection methods
		virtual void init_connection();
		virtual void release();
		virtual void connect();
		virtual void disconnect();


		void requset_op_instruments();
		void request_op_positions();
		void request_stock_instruments() {};
		void request_stock_positions();

		void request_srv_time();
		virtual void OnRspSOPQryTradeTime(DFITCSOPRspQryTradeTimeField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast);


		void request_investor_full_positions() override;
		//
		void req_RiskDegree() override;
		virtual void OnRspSOPQryCapitalAccountInfo(DFITCSOPRspQryCapitalAccountField *pData, DFITCSECRspInfoField *pRspInfo);
		//
		//DFITCSECOpenCloseFlagType compute_open_close(order* ord);

		//option orders
		//两个下单ack谁先收到就先处理谁。
		void OnSOPEntrustOrderRtnAsyn(DFITCSOPEntrustOrderRtnField * pData);
		void OnRspSOPEntrustOrderRtnAsyn(DFITCSOPRspEntrustOrderField * pData);
		void OnSOPWithdrawOrderRtnAsyn(DFITCSOPWithdrawOrderRtnField * pData);
		void OnSOPTradeRtnAsyn(DFITCSOPTradeRtnField * pData);

		//stock orders
		virtual void OnRspStockEntrustOrder(DFITCStockRspEntrustOrderField *pData, DFITCSECRspInfoField *pRspInfo);
		void OnStockEntrustOrderRtnAsyn(DFITCStockEntrustOrderRtnField* pData);
		void OnStockWithdrawOrderRtnAsyn(DFITCStockWithdrawOrderRtnField* pData);
		void OnStockTradeRtnAsyn(DFITCStockTradeRtnField* pData);


		int market_create_order_async(order* o, char* pszReason) override;
		int market_create_op_order(order* o, char* pszReason);
		int market_create_stock_order(order* o, char* pszReason);

		int market_cancel_order_async(order* o, char* pszReason) override;
		int market_cancel_op_order(order* o, char* pszReason);
		int market_cancel_stock_order(order* o, char* pszReason);

		//native callback
		virtual void OnFrontConnected();
		virtual void OnFrontDisconnected(int nReason);
		virtual void OnRspError(DFITCSECRspInfoField *pRspInfo);

		//option orders
		virtual void OnRspSOPUserLogin(DFITCSECRspUserLoginField *pData, DFITCSECRspInfoField *pRspInfo);
		virtual void OnRspSOPUserLogout(DFITCSECRspUserLogoutField *pData, DFITCSECRspInfoField *pRspInfo);
		virtual void OnRspSOPEntrustOrder(DFITCSOPRspEntrustOrderField *pData, DFITCSECRspInfoField *pRspInfo);
		virtual void OnRspSOPWithdrawOrder(DFITCSECRspWithdrawOrderField *pData, DFITCSECRspInfoField *pRspInfo); //todo
		virtual void OnRspSOPQryPosition(DFITCSOPRspQryPositionField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast);
		virtual void OnSOPEntrustOrderRtn(DFITCSOPEntrustOrderRtnField * pData);
		virtual void OnSOPTradeRtn(DFITCSOPTradeRtnField * pData);
		virtual void OnSOPWithdrawOrderRtn(DFITCSOPWithdrawOrderRtnField * pData);
		virtual void OnRspSOPQryContactInfo(DFITCSOPRspQryContactField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast);


		//stock orders
		virtual void OnRspStockUserLogin(DFITCSECRspUserLoginField *pData, DFITCSECRspInfoField *pRspInfo);
		virtual void OnRspStockUserLogout(DFITCSECRspUserLogoutField *pData, DFITCSECRspInfoField *pRspInfo);

		virtual void OnRspStockWithdrawOrder(DFITCSECRspWithdrawOrderField *pData, DFITCSECRspInfoField *pRspInfo); //todo

		virtual void OnStockEntrustOrderRtn(DFITCStockEntrustOrderRtnField * pData);
		virtual void OnStockTradeRtn(DFITCStockTradeRtnField * pData);
		virtual void OnStockWithdrawOrderRtn(DFITCStockWithdrawOrderRtnField * pData);
		virtual void OnRspStockQryPosition(DFITCStockRspQryPositionField *pData, DFITCSECRspInfoField *pRspInfo, bool bIsLast);

		//xs_outbound_queue*  get_outbound_queue() { return &m_outboundQueue; }
		xs_order_rtn_queue* get_order_rtn_queue() { return &m_orderRtnQueue; }
		xs_order_rsp_queue* get_order_rsp_queue() { return &m_orderRspQueue; }
		xs_order_can_rtn_queue* get_order_can_rtn_queue() { return &m_ordCanRtnQueue; }
		xs_trade_queue* get_trade_queue() { return &m_tradeQueue; }

		xs_stockorder_rtn_queue* get_stockorder_rtn_queue() { return &m_stockorderRtnQueue; }
		xs_stockorder_can_rtn_queue* get_stockorder_can_rtn_queue() { return &m_stockordCanRtnQueue; }
		xs_stocktrade_queue* get_stocktrade_queue() { return &m_stocktradeQueue; }

		//terra_safe_map<int, order*> *get_active(){ return &m_activeOrders; }
		std::string getMaturity(std::string& sMat);
		void insert_localId2order(int id, order* o);
		order *get_localId2order(int id);
		void insert_used_locId(int id);
		bool contain_used_locId(int id);

	protected:
		// connection methods
		bool init_config(const std::string &name, const std::string &ini) override;
		void process() override;
		virtual void cancel_num_warning(tradeitem* i) override;
		virtual void cancel_num_ban(tradeitem* i) override;



	private:
		int get_xs_instype(AtsType::InstrType::type _type);
		void ackBackup();
		void readLocal2Portfolio();
		void write_data2disk();

	protected:
		void request_op_login();
		void request_stock_login();
#ifdef Linux
		int efd;
		void  init_epoll_eventfd();
#endif


	private:

		std::string m_path;
		DFITCSECTraderApi* m_pUserApi;
		//xs_outbound_queue m_outboundQueue;

		xs_order_rtn_queue m_orderRtnQueue;
		xs_order_rsp_queue m_orderRspQueue;
		xs_order_can_rtn_queue m_ordCanRtnQueue;
		xs_trade_queue m_tradeQueue;

		xs_stockorder_rtn_queue m_stockorderRtnQueue;
		xs_stockorder_can_rtn_queue m_stockordCanRtnQueue;
		xs_stocktrade_queue m_stocktradeQueue;

		std::vector<std::string> m_sInstrVec;
		std::vector<std::string> m_etfName;

		lwtp m_startTime;

		int m_nRequestId;
		int m_nCurrentOrderRef;

		tbb::concurrent_queue<std::shared_ptr<localOrderMsg>> localMsgQueue;
		std::unordered_map<int, localOrderMsg> m_localId2Portfolio;

		tbb::concurrent_hash_map<int, order*> m_localId2order;
		
		tbb::concurrent_hash_map<int,int> m_used_locId;

		std::unordered_map<int, std::string> m_Append_localId2Portfolio;//acked but untrade
		int m_begin_Id;
		//char szReason[REASON_MAXLENGTH + 1];
		std::string m_sStockUsername;
		std::string m_sStockPassword;
		lwtp m_timepoint;

		friend class xs_order_aux;
	};
}

#endif // __XS_CONNECTION_H__

