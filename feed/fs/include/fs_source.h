#ifndef __FS_SOURCE_V2_H__
#define __FS_SOURCE_V2_H__
#include <string>
//#include "fs_decoder.h"
#include "SgitFtdcUserApiStruct.h"
#include "feedsource.h"
#include <thread>
#include <functional> 
#include "lockfree_classpool_workqueue.h"

#include "boost/asio.hpp"
#include <boost/asio/high_resolution_timer.hpp>
#include "FastMemcpy.h"
#ifdef Linux
#include <sys/eventfd.h>
#endif
//#include "LockFreeClassPool.h"
using namespace terra::common;
using namespace terra::feedcommon;
//using namespace fstech;
namespace feed
{
	namespace fs
	{
		typedef terra::common::lockfree_classpool_workqueue<fstech::CThostFtdcDepthMarketDataField> outbound_queue;

		class fs_source : public feed_source
		{
		public:
			fs_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db);
			//virtual ~fs_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_msg(fstech::CThostFtdcDepthMarketDataField* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
		protected:
			//fs_decoder     m_decoder;
			outbound_queue    m_queue;
			void process_msg(fstech::CThostFtdcDepthMarketDataField* pMsg, feed_item * feed_item);
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
			/*std::thread m_thread;
			boost::asio::io_service io;
			void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);*/
			//void set_kernel_timer_thread();
		};
	}
}
#endif //__FS_SOURCE_V2_H__

