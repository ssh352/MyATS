#ifndef _ZMQ_SERVER_V2_H_
#define _ZMQ_SERVER_V2_H_
#pragma once
#include "common.h"
#include "zmq.hpp"
using namespace zmq;
namespace terra
{
	namespace common
	{
		typedef std::function<void(uint8_t* buffer, size_t len)> msg_event_handler;

		class zmq_server
		{
		public:
			zmq_server(zmq::context_t& context);
			~zmq_server();
		public:
			void init(const char* szReceiverListenIP, const char* szSenderListenIP);
			void process_msg();
			void process_idle();
			zmq::context_t& get_context() { return m_context; }
			template<typename T>  void serializeAndSend(const T& msg, std::string destination = "ALL");
		private:
			zmq::socket_t* m_socket_receiver;
			zmq::socket_t* m_socket_sender;
			zmq::context_t& m_context;
			ptime m_last_time;
		};
		//only receive
		class zmq_client
		{
		public:
			zmq_client(zmq::context_t& context, msg_event_handler handler,string destination);
			~zmq_client();
		public:
			//peer ip:port
			//zmq_publisher_rep=tcp://192.168.1.26:6665
			bool init(const char* szSenderListenIP);
			void process_msg();
			void release();
		private:
			zmq::socket_t* m_socket_receiver;
			zmq::context_t& m_context;
			msg_event_handler m_msg_event_handler;
			string m_destination;
		};
		//
	}
}
#endif //_ZMQ_SERVER_V2_H_


