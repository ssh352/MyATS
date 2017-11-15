#include "boost/shared_ptr.hpp"
#include <thrift/TProcessor.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/StdThreadFactory.h>
#include <thrift/concurrency/BoostThreadFactory.h>
#include <map>
#include <thrift/server/TNonblockingServer.h>

#include "simpleatsserver.h"
#include "simpleatshandler.h"
#include "atsserver.h"
#include "atsconfig.h"
#include "underlying.h"
#include "iaoption.h"
#include "iafuture.h"
#include "atsmanager.h"

using namespace ::apache::thrift::concurrency;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

using namespace  ::SimpleMsg;
using namespace  terra::atsserver;
using namespace  terra::ats;
using namespace  terra::instrument;

namespace simpleatsserver
{
	simple_ats_server* simple_ats_server::g_pSimpleAtsServer = nullptr;

	void feed_source_status_callback(string strSourceName, int status, string errorMsg)
	{
		printf_ex("feed_source_status_callback sourceName:%s,status:%d,errorMsg:%s\n", strSourceName.c_str(), status, errorMsg.c_str());
	}

	void feed_item_data_callback(string strSourceName, feed_item * feedItem)
	{
		if (feedItem == nullptr)
			return;

		//printf_ex("feed_item_data_callback sourceName:%s,code:%s\n", strSourceName.c_str(), feedItem->get_subject().c_str());
		//feedItem->dump();
	}

	simple_ats_server::simple_ats_server()
	{
		m_pServer = nullptr;		
		ats_server::get_instance()->init(std::bind(&simple_ats_server::pub_ats_event, this));
	}	
	simple_ats_server::~simple_ats_server()
	{
	}	
	void simple_ats_server::init_thrift()
	{	
        #ifndef Linux	
        TWinsockSingleton::create(); //only use it in windows;
        #endif
		boost::shared_ptr<simple_ats_handler> handler(new simple_ats_handler());

		boost::shared_ptr<TProcessor> processor(new SimpleAtsOperationProcessor(handler));

		int iPort = ats_config::get_instance()->get_specific_ats_port();
		boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(iPort));

		boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

		boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());		
		/**
		* ThreadManager class
		*
		* This class manages a pool of threads. It uses a ThreadFactory to create
		* threads.  It never actually creates or destroys worker threads, rather
		* it maintains statistics on number of idle threads, number of active threads,
		* task backlog, and average wait and service times.
		*
		* @version $Id:$
		*/
		boost::shared_ptr<ThreadManager>    threadManager = ThreadManager::newSimpleThreadManager();
		/**
		* A thread factory to create std::threads.
		*
		* @version $Id:$
		*/
#ifdef WIN32
		//typedef  StdThreadFactory ThreadFactory;
		boost::shared_ptr<StdThreadFactory> threadFactory = boost::shared_ptr<StdThreadFactory>(new StdThreadFactory());
#else
		//typedef  PosixThreadFactory ThreadFactory;
		boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
#endif
		 //only use it in windows		
		threadManager->threadFactory(threadFactory);
		/**
		* Starts the thread manager. Verifies all attributes have been properly
		* initialized, then allocates necessary resources to begin operation
		*/
		threadManager->start();
		/**
		* Manage clients using a thread pool.
		*/
		m_pServer = new TThreadPoolServer(processor, serverTransport,transportFactory, protocolFactory,threadManager);		

	}
	void simple_ats_server::start_thrift()
	{
		printf("start thrift server...\n");
		if (m_pServer != nullptr)
		{
			m_pServer->serve();
		}
	}
	bool simple_ats_server::add_simple_ats(SimpleAtsMsg * pAtsMsg, simple_ats * pAts)
	{
		create_ia_ats_msg(pAtsMsg,pAts);
		//此处有和simple_ats_factory重复的嫌疑
		ats_server::get_instance()->AbstractATSMap.add(pAts->get_name(), pAts);
		update_ia_ats_msg(pAtsMsg, pAts);
		return true;
	}
	bool simple_ats_server::create_ia_ats_msg(SimpleAtsMsg * pAtsMsg, simple_ats * pAts)
	{
		if (pAts == nullptr || pAtsMsg == nullptr)
			return false;

		pAtsMsg->Ats = ats_server::get_instance()->create_ats_msg(pAts);
		
		for(auto &it:pAts->AtsInstrumentList)
		{			
			ia_option* option = dynamic_cast<ia_option*>(it);
			if (option != nullptr)
			{
				TrdInstrumentMsg && msg = create_ia_future_msg(pAts, it);
				pAtsMsg->AllOptions.push_back(msg);
				//pAtsMsg->AllInstruments.push_back(msg);//有一定的冗余
			}			
			ia_future* future = dynamic_cast<ia_future*>(it);
			if (future != nullptr)
			{
				TrdInstrumentMsg && msg = create_ia_future_msg(pAts, it);
				pAtsMsg->AllFutures.push_back(msg);
				//pAtsMsg->AllInstruments.push_back(msg);//有一定的冗余
			}			
		}				
		
		for(auto &it:pAts->underlying_map)
		{
			pAtsMsg->Underlyings.push_back(ats_server::get_instance()->create_underlying_msg(it.second));		
		}

		return true;
	}	
	TrdInstrumentMsg simple_ats_server::create_ia_future_msg(simple_ats * ats, ats_instrument * instrument)
	{
		TrdInstrumentMsg msg;
		if (ats == nullptr || instrument == nullptr)
		{
			loggerv2::error("simple_ats_server::create_ia_future_msg instrument is null!\n");
			printf_ex("simple_ats_server::create_ia_future_msg instrument is null!\n");
			return msg;
		}
		msg.AtsInstrument = ats_server::get_instance()->create_ats_instrument_msg(instrument, true);
		return msg;
	}
	void simple_ats_server::update_ia_ats_msg(SimpleAtsMsg * pAtsMsg, simple_ats * pAts)
	{
		if (pAts == nullptr)
			return;

		ats_server::get_instance()->update_abstract_ats_msg(pAtsMsg->Ats,pAts);

		
		for (auto & v : pAtsMsg->AllFutures)
		{			
			ats_instrument * pInstrument = pAts->find(v.AtsInstrument.Code);
			if (pInstrument != nullptr)
			{
				update_ia_future_msg(v, pInstrument);
			}
		}
		for (auto & v: pAtsMsg->AllOptions)
		{			
			ats_instrument * pInstrument = pAts->find(v.AtsInstrument.Code);
			if (pInstrument != nullptr)
			{
				update_ia_future_msg(v, pInstrument);
			}
		}
		//for (auto & v: pAtsMsg->AllInstruments)
		//{			
		//	ats_instrument * pInstrument = pAts->find(v.AtsInstrument.Code);
		//	if (pInstrument != nullptr)
		//	{
		//		update_ia_future_msg(v, pInstrument);
		//	}
		//}
	}
	void simple_ats_server::update_ia_future_msg(TrdInstrumentMsg & futureMsg, ats_instrument* insturment)
	{		
		ats_server::get_instance()->update_ats_instrument_msg(futureMsg.AtsInstrument, insturment, true);
	}
	//void simple_ats_server::active_feed()
	//{			
	//	for(auto &it:*feed_source_container::get_instance())
	//	{								
	//		//pSource->set_decoder_callback(feed_item_data_callback);


	//		it.second->init_source();
	//	}
	//}			
	void simple_ats_server::pub_ats_event()
	{		
		for(auto &it:simple_ats_server::get_instance()->SimpleAtsMsgDictionary)
		{			
			abstract_ats* ats = ats_server::get_instance()->AbstractATSMap.get_by_key(it.second->Ats.Name);
			if (ats != nullptr)
			{
				update_ia_ats_msg(it.second, (simple_ats*)ats);
				ats_manager::get_instance()->net_mq_publisher->publish<SimpleAtsMsg>("IAATS", *it.second);
			}
			else
			{
				//printf_ex("publish_task::pub_ats_event ats is null!\n");
			}			
		}
	}
		
}