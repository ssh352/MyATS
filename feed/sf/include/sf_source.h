#ifndef __SF_SOURCE_V2_H__
#define __SF_SOURCE_V2_H__
#include "feedsource.h"
#include "zmqserver.h"
#include "Simulation_types.h"
using namespace zmq;
using namespace terra::common;
using namespace terra::feedcommon;
using namespace Simulation;
namespace feed
{
	namespace sf
	{	
		typedef terra::common::LockFreeWorkQueue<FeedMsg> outbound_queue;

		class sf_source : public feed_source
		{
		public:
			sf_source(const string & sourceName, const string & hostname, const string & service, const string & db,const string & ethName);
			virtual ~sf_source();
		public:
			virtual void init_source();
			void release_source() override;
			void process_msg(FeedMsg* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			bool setUDPSockect(const char * pBroadcastIP, int nBroadcastPort);			
			void start_receiver();
			void process_data(uint8_t* buffer, size_t len);
			virtual bool subscribe(feed_item * feed_item);
		protected:
			void process() override;			
		protected:			
			outbound_queue m_queue;
			void process_msg(FeedMsg* pMsg, feed_item * feed_item);
			zmq_client     *m_zmq_receiver;
			std::thread    m_receiver_thread;
		};
	}
}
#endif //__SF_SOURCE_V2_H__

