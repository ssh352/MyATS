#ifndef __LTSUDP_SOURCE_V2_H__
#define __LTSUDP_SOURCE_V2_H__


#include "ltsudp_decoder.h"
#include "feedsource.h"


struct CFAST_MD;
struct CL2FAST_MD;
using namespace terra::common;
using namespace terra::feedcommon;
namespace feed
{
	namespace ltsudp
	{
		typedef terra::common::LockFreeWorkQueue<CFAST_MD> outbound_queue;

		typedef terra::common::LockFreeWorkQueue<CL2FAST_MD> l2_outbound_queue;

		class ltsudp_source : public feed_source
		{
		public:
			ltsudp_source(const string & sourceName, const string & hostname, const string & service, const string & db, string pub = "", string url = "", string req_url = "");
			virtual ~ltsudp_source();
		public:
			virtual void init_source();
			void release_source() override;
			void process_msg(CFAST_MD* pMsg);
			void process_l2_msg(CL2FAST_MD* pMsg);
			// get/set
			inline outbound_queue* get_queue() { return &m_queue; }
			inline l2_outbound_queue* get_l2_queue() { return &m_l2_queue; }
			bool setUDPSockect(const char * pBroadcastIP, int nBroadcastPort);
			void Listhen2Udp();
			virtual bool subscribe(feed_item * feed_item);
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
		protected:
			std::thread    m_tListhen2Udp;
			outbound_queue m_queue;
			l2_outbound_queue m_l2_queue;
			int			   m_UDPSockID;
			//ltsudp_decoder m_decoder;

			//std::thread m_thread;
			//boost::asio::io_service io;
			/*virtual void process(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			void set_kernel_timer_thread();*/
			void process_msg(CFAST_MD* pMsg, feed_item * feed_item);

			void process_l2_msg(CL2FAST_MD* pMsg, feed_item * feed_item);
		};
	}
}
#endif //__ltsudp_SOURCE_V2_H__

