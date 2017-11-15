#ifndef _ATS_SERVER_V2_H_
#define _ATS_SERVER_V2_H_
#pragma once
#include "common.h"
#include "abstractats.h"
#include "position.h"
#include "underlying.h"
#include "instrumentclass.h"
#include "maturity.h"
#include "AtsGeneral_types.h"
#include "feeditem.h"
#include "abstractworkflowtask.h"
#include "orderdatacollection.h"
#include <boost/lockfree/queue.hpp>
#include "tbb/concurrent_queue.h"

using namespace terra::marketaccess::orderpassing;
using namespace terra::common;
using namespace terra::ats;
using namespace terra::instrument;
using namespace AtsGeneral;
using namespace terra::feedcommon;
namespace terra
{
	namespace atsserver
	{		
		typedef std::function<void()> pub_ats_event_handler;

		class publish_task : public itask
		{
		public:
			publish_task(lwdur delay = std::chrono::milliseconds(0));
			~publish_task(){};
			virtual void execute();
			void pub_feed_status();
			void pub_connection_status();
			void pub_logs();
			void pub_orders();
			void pub_quotes();

			void update_order_msg(OrderMsg &msg, order * temp);

			void pub_execs();
			void pub_intra_data();
			void intradata_update_event(terra::ats::atsintradata* source, ptime lastTs, map<string, double>& lastData);
			pub_ats_event_handler handler;
			order_data_collection m_order_data;

			//boost::lockfree::spsc_queue<std::shared_ptr<IntraDataMsg>, boost::lockfree::capacity<1024> > instrDataQueue;
			/*inline static boost::lockfree::queue<IntraDataMsg *>& get_instance() 
			{
				static boost::lockfree::queue<IntraDataMsg*> instrDataQueue(1024);
				return instrDataQueue;
			}*/
			tbb::concurrent_queue<std::shared_ptr<IntraDataMsg>> instrDataQueue;
		private:
			int open_buy;
			int close_buy;
			int open_sell;
			int close_sell;

			map<string, int> acks;
			map<string, int> cancels;
		};
		class ats_server
		{
		private:
			ats_server();
			static ats_server * g_AtsServer;
		public:
			~ats_server();
		public:
			static ats_server * get_instance()
			{
				if (g_AtsServer == nullptr)
				{
					g_AtsServer = new ats_server();
				}
				return g_AtsServer;
			}
		public:						
			void init(pub_ats_event_handler pub);
			void start_publish_work_flow();
			void stop_publish_work_flow();
			AtsMsg create_ats_msg(abstract_ats * ats);
			AtsInstrumentMsg create_ats_instrument_msg(ats_instrument * instrument, bool has_depth);
			PositionMsg create_position_msg(position * position);
			FeedMsg create_feed_msg(ats_instrument * instrument, bool has_depth);
			UnderlyingMsg create_underlying_msg(underlying * underlying);
			InstrumentClassMsg create_instrument_class_msg(instrumentclass * pInstrumentclass);
			FeesStructMsg create_fees_struct_msg(instrumentclass * pInstrumentclass);
			MaturityMsg create_maturity_msg(maturity *pMaturity);
			void update_abstract_ats_msg(AtsMsg & msg, abstract_ats * ats);
			void update_ats_instrument_msg(AtsInstrumentMsg & msg, ats_instrument* instrument, bool has_depth);
			void update_position_msg(PositionMsg & msg, position * position);
			void update_feed_msg(FeedMsg & msg, feed_item * item, bool has_depth);			
			void update_maturity_msg(MaturityMsg & msg, maturity * item);
			bool CheckAesMsg(const AESDataMsg& msg);
			void dokill();
		public:
			map_ex<string, abstract_ats*> AbstractATSMap;
			//ptime           LastHeartBeatTime;			
			work_flow_thread    publish_work_flow;
			publish_task * task;
			bool is_verify;
			ptime kill_time;
			bool m_enable_stop;

		};
	}
}
#endif


