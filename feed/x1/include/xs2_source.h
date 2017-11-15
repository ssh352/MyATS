#ifndef __XS2_SOURCE_H__
#define __XS2_SOURCE_H__

#include "xs2_decoder.h"
#include "DFITCApiStruct.h"
#include "feedsource.h"



using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace xs2
	{
		typedef terra::common::lockfree_classpool_workqueue<DFITCDepthMarketDataField> outbound_queue;

		class xs2_source : public feed_source
		{
		public:
			xs2_source(const string &sourceName, const string &hostname, const string &service, const string &user, const string &password, const string &db, string pub = "", string url = "");
			//virtual ~xs2_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_msg(DFITCDepthMarketDataField*  pMsg);
			outbound_queue* get_queue() { return &m_queue; }
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
			void process_msg(DFITCDepthMarketDataField* pMsg, feed_item * feed_item);
		protected:
			outbound_queue m_queue;
			//xs2_decoder m_decoder;

			//std::thread m_thread;
			//boost::asio::io_service io;
			//void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			//void set_kernel_timer_thread();
			
		};
	}
}
#endif //__XS2_SOURCE_H__

