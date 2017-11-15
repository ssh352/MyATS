#include "sf_source.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
namespace feed
{
	namespace sf
	{		
		zmq::context_t   g_zmq_context(4);		

		sf_source::sf_source(const string & sourceName, const string & hostname, const string & service, const string & db, const string & ethName)
			: feed_source("sf", sourceName, hostname, service, "", "", "", db)
		{
			//
			m_zmq_receiver = new zmq_client(g_zmq_context, boost::bind(&sf_source::process_data, this, _1, _2),"FeedMsg");
			//
		}
		sf_source::~sf_source()
		{
			release_source();
		}
		void sf_source::init_source()
		{	
			get_queue()->setHandler(boost::bind(&sf_source::process_msg, this,_1));			

			init_process(io_service_type::feed);
			
			load_database();
			
			if (setUDPSockect(get_service_name().c_str(), atoi(get_port().c_str())))
			{
				update_state(AtsType::FeedSourceStatus::Up, "");

				m_receiver_thread = std::thread(&sf_source::start_receiver, this);
			}
		}		
		bool sf_source::setUDPSockect(const char * pBroadcastIP, int nBroadcastPort)
		{
			if (m_zmq_receiver == nullptr)
				return false;
			//to do ...ip+port
			////zmq_publisher_rep=tcp://192.168.1.26:6665
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%s:%d", pBroadcastIP, nBroadcastPort);
			return m_zmq_receiver->init(buffer);
		}		
		void sf_source::start_receiver()
		{
			m_zmq_receiver->process_msg();
		}
		void sf_source::release_source()
		{
			feed_source::release_source();
			if (m_zmq_receiver != nullptr)
			{
				m_zmq_receiver->release();
				delete m_zmq_receiver;
				m_zmq_receiver = nullptr;
			}
		}
		void sf_source::process_data(uint8_t* buffer, size_t len)
		{
			boost::shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer(buffer, len));
			boost::shared_ptr<TCompactProtocol> bin_proto(new TCompactProtocol(mem_buf));

			FeedMsg *pMsg=new FeedMsg();
			pMsg->read(bin_proto.get());
			m_queue.Push(pMsg);

			//to do ...			
			//loggerv2::info("sf_source::process_data %s,%s,%d,%f,%d,%f\n", pMsg->Code.c_str(), pMsg->MarketTime.c_str(), pMsg->BidQtys[0], pMsg->BidDepths[0], pMsg->AskQtys[0], pMsg->AskDepths[0]);
			//printf_ex("sf_source::process_data %s,%s,%d,%f,%d,%f\n", pMsg->Code.c_str(), pMsg->MarketTime.c_str(), pMsg->BidQtys[0], pMsg->BidDepths[0], pMsg->AskQtys[0], pMsg->AskDepths[0]);

		}
		void sf_source::process_msg(FeedMsg* pMsg)
		{			
			//loggerv2::info("sf_source::process_msg %s,%s,%d,%f,%d,%f\n", pMsg->Code.c_str(), pMsg->MarketTime.c_str(), pMsg->BidQtys[0], pMsg->BidDepths[0], pMsg->AskQtys[0], pMsg->AskDepths[0]);
			//printf_ex("sf_source::process_msg %s,%s,%d,%f,%d,%f\n", pMsg->Code.c_str(), pMsg->MarketTime.c_str(), pMsg->BidQtys[0], pMsg->BidDepths[0], pMsg->AskQtys[0], pMsg->AskDepths[0]);

			std::string sFeedCode = pMsg->Code;
			feed_item* afeed_item = get_feed_item(sFeedCode);
			if (afeed_item == nullptr)
			{
				//loggerv2::info("sf_source::process_msg %s not found", pMsg->Code.c_str());
				return;
			}
			//
			process_msg(pMsg, afeed_item);
			return post(afeed_item);
		}
		void sf_source::process()
		{
			get_queue()->Pops_Handle();
		}		

		void sf_source::process_msg(FeedMsg* pMsg, feed_item * feed_item)
		{
			if (feed_item == nullptr || pMsg == nullptr)
				return;
			process_depth(0, pMsg->BidQtys[0], pMsg->BidDepths[0] != NO_PRICE ? pMsg->BidDepths[0] : 0., pMsg->AskDepths[0] != NO_PRICE ? pMsg->AskDepths[0] : 0., pMsg->AskQtys[0], feed_item);			
			feed_item->set_last_price(pMsg->Last);
			feed_item->set_daily_volume(pMsg->DailyVolume);
		}

		bool sf_source::subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || get_status() == AtsType::FeedSourceStatus::Down)
			{
				loggerv2::error("subscribe feed fail,feed_itme invailable,or feedsourceStatus:%s is %d", Name.c_str(), get_status());
				return false;
			}
			if (feed_item->is_subsribed() == true)
				return true;
			add_feed_item(feed_item);
			feed_item->subscribe();
			return true;
		}		
	}
}
