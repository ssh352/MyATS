#ifndef _ZMQ_PUBLISHER_V2_H_
#define _ZMQ_PUBLISHER_V2_H_
#pragma once
#include "common.h"
#include "zmq.hpp"
using namespace zmq;
namespace terra
{
	namespace common
	{
		class zmq_publisher
		{
		public:
			zmq_publisher(zmq::context_t& context);
			~zmq_publisher();
		public:
			void init(const char* strPub, const char* strRep, const char* strSS);
			void process_msg(){}
			void process_idle();
		private:
			zmq::socket_t* m_socket_pub;
			zmq::socket_t* m_socket_rep;
			zmq::socket_t* m_socket_snapshot;
			zmq::context_t& m_context;
		};
	}
}
#endif //_ZMQ_PUBLISHER_V2_H_


