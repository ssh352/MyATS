#ifndef _SIMPLE_ATS_SVERVER_V2_H_
#define _SIMPLE_ATS_SVERVER_V2_H_
#pragma once
//#ifdef _WIN32
//#include <WinSock2.h>
//#endif
#include "common.h"
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include "simpleats.h"
#include "abstractworkflowtask.h"
#include "feedsourcefactory.h"
#include "SimpleAtsOperation.h"
using namespace SimpleMsg;
using namespace terra::common;
using namespace ::apache::thrift::server;
using namespace terra::ats;
using namespace simpleats;
namespace simpleatsserver
{		
	class simple_ats_server
	{
	private:
		simple_ats_server();
		~simple_ats_server();
	public:				
		static simple_ats_server * g_pSimpleAtsServer;
		static simple_ats_server * get_instance()
		{
			if (g_pSimpleAtsServer == nullptr)
			{
				g_pSimpleAtsServer = new simple_ats_server();
			}
			return g_pSimpleAtsServer;
		}
	public:
		map_ex<string, SimpleAtsMsg*> SimpleAtsMsgDictionary;
	private:
		//thread_1:thrift线程池,实现与client的双向通信,如返回client关注的合约静态信息,refer to the indexArbTask
		TThreadPoolServer * m_pServer;	
	public:
		void init_thrift();
		//void active_feed();
		void start_thrift();				
		bool add_simple_ats(SimpleAtsMsg * pAtsMsg, simple_ats * pAts);
		bool create_ia_ats_msg(SimpleAtsMsg * pAtsMsg, simple_ats * pAts);		
		TrdInstrumentMsg create_ia_future_msg(simple_ats * ats, ats_instrument * instrument);
		void update_ia_ats_msg(SimpleAtsMsg * pAtsMsg, simple_ats * pAts);
		void update_ia_future_msg(TrdInstrumentMsg & futureMsg, ats_instrument* insturment);
		void pub_ats_event();
	};
}
#endif //_SIMPLE_ATS_SVERVER_V2_H_


