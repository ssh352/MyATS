#ifndef __ZD_SOURCE_V2_H__
#define __ZD_SOURCE_V2_H__


#include "ShZdFutureMarketApi.h"
#include "feedsource.h"


using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace zd
	{
		typedef lockfree_classpool_workqueue<CTShZdDepthMarketDataField> outbound_queue;

		class zd_source : public feed_source
		{
		public:
			zd_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, const string& authServerIp, const string& authServerPort, const string& authCode, string pub = "", string url = "", string req_url = "");
			//virtual ~zd_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_msg(CTShZdDepthMarketDataField* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
#if 0
			bool FQR;
			void receive_FQR(CTShZdDepthMarketDataField * pForQuoteRsp);
#endif
			string get_auth_server_ip(){ return m_auth_server_ip; }
			string get_auth_port(){ return m_auth_port; }
			string get_auth_code(){ return m_auth_code; }
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
		protected:
			//cffex_decoder     m_decoder;
			outbound_queue    m_queue;

			string m_auth_server_ip;
			string m_auth_port;
			string m_auth_code;

			//std::thread m_thread;
			//boost::asio::io_service io;
			//void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			//void set_kernel_timer_thread();

			void update_item(CTShZdDepthMarketDataField* pMsg, feed_item * feed_item);
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
			
		};
	}
}
#endif //__ZD_SOURCE_V2_H__

