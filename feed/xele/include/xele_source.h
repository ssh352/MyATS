#ifndef __XELE_SOURCE_V2_H__
#define __XELE_SOURCE_V2_H__
#include "feedsource.h"
#include "XeleFtdcMduserApi.h"

using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace xele
	{
		typedef terra::common::lockfree_classpool_workqueue<CXeleShfeMarketDataUnion> outbound_queue;

		class xele_source;
		class FeedXeleSpi : public CXeleMdSpi
		{
			
		public:
			FeedXeleSpi(){ ptr = nullptr; };
			~FeedXeleSpi(){};

			xele_source *ptr;

			virtual void OnFrontDisconnected(int nReason) override;

			virtual void OnFrontUserLoginSuccess() override
			{
				std::cout << "Xele OnFrontUserLoginSuccess" << endl;
			}

		};
		

		class xele_source : public feed_source
		{
		public:
			xele_source(const string & sourceName, const string & frontAddr, const string & BCAddr, string &ip, string &port, const string & UserID, const string & Passwd, const string & ethName, string &db, int mcore = -1, string pub = "", string url = "", string req_url = "");
			virtual ~xele_source();
		public:
			virtual void init_source();
			void release_source() override;
			void process_msg(CXeleShfeMarketDataUnion* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }

			bool Connect_and_Login();
			void start_receiver();
			int get_code(){ return m_core; }
			FeedXeleSpi m_Spi;
			CXeleMdApi* m_Api = nullptr;
			

		protected:
			void process() override;
		protected:
			string m_strFrontAddr;
			string m_strBCAddr;
			string m_strEthName;
			string m_strUserID;
			string m_strPasswd;
			int m_core;
			outbound_queue m_queue;
			void process_msg(CXeleShfeMarketDataUnion* pMsg, feed_item * feed_item);
#ifdef Linux
                        int efd;
                        void  init_epoll_eventfd();
						pthread_t xele_id;
#endif
		};
	}
}
#endif //__SL2_SOURCE_V2_H__

