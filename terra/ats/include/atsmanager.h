#ifndef _ATS_MANAGER_V2_H_
#define _ATS_MANAGER_V2_H_
#pragma once
#include "referential.h"
#include "socketconnection.h"
#include "abstractworkflowtask.h"
#include "work_flow.h"
using namespace terra::instrument;
using namespace terra::common;
namespace terra
{
	namespace ats
	{
		//class feed_task : public itask
		//{
		//public:
		//	virtual void execute();
		//};
		class ats_manager
		{
		private:
			ats_manager();
			~ats_manager();
			static ats_manager * g_ATSManager;
		public:
			static ats_manager * get_instance()
			{
				if (g_ATSManager == nullptr)
				{
					g_ATSManager = new ats_manager();
				}
				return g_ATSManager;
			}
		public:
			//referential Referential;
			//tradingperiodmanager TradingPeriodManager;
			socket_connection *  net_mq_publisher;
			//work_flow m_feed_decoder_work_flow;
			//terra_memory_log_device<toolkit::common::RTMutex> memory_log_device;			
		public:
			void initialize_logger();
			void initialize_referential();
			void initialize_feed_sources();
			void initialize_order_passing();
			void initialize_position();
			void initialize_security();
			void close_all();
			void active_feed();
			void active_io_service();
			void active_twap_thread_pool();
		};
	}
}
#endif //_ATS_MANAGER_V2_H_


