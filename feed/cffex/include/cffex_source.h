#ifndef __CFFEX_SOURCE_V2_H__
#define __CFFEX_SOURCE_V2_H__

#include "cffex_decoder.h"
#include "ThostFtdcUserApiStruct.h"
#include "feedsource.h"


using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace cffex
	{
		typedef lockfree_classpool_workqueue<CThostFtdcDepthMarketDataField> outbound_queue;

		class cffex_source : public feed_source
		{
		public:
			cffex_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, const string& is_FQR = "", string pub = "", string url = "", string req_url = "");
			//virtual ~cffex_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_msg(CThostFtdcDepthMarketDataField* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			bool FQR;
			void receive_FQR(CThostFtdcForQuoteRspField * pForQuoteRsp);
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
		protected:
			//cffex_decoder     m_decoder;
			outbound_queue    m_queue;

			//std::thread m_thread;
			//boost::asio::io_service io;
			//void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			//void set_kernel_timer_thread();

			void update_item(CThostFtdcDepthMarketDataField* pMsg, feed_item * feed_item);
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
			
		};
	}
}
#endif //__CFFEX_SOURCE_V2_H__

