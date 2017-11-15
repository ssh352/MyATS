#ifndef _UNCOMPRESS_SOCKET_CONNECTION_V2_H_
#define _UNCOMPRESS_SOCKET_CONNECTION_V2_H_
#pragma once
#include "common.h"
#include "zmq.hpp"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/protocol/TDebugProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TZlibTransport.h>
#include "urlDEcode.h"
//using apache::thrift::transport::TMemoryBuffer;
//using apache::thrift::protocol::TProtocol;

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace zmq;
namespace terra
{
	namespace common
	{
		class uncompress_socket_connection
		{
		public:
			uncompress_socket_connection(zmq::context_t& context);
			~uncompress_socket_connection();
		public:
			void init(const char* strPub);
			template<typename T>  void publish(string destination, const T& msg)
			{
				serializeAndSend<T>(msg, destination);
			}

		protected:
			template<typename T>  void serializeAndSend(const T& msg, std::string destination = "ALL")
			{
				try
				{
					//1.serialize
					uint8_t* buf_ptr = nullptr;
					uint32_t sz = 0;

					boost::shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer);
					boost::shared_ptr<TCompactProtocol> bin_proto(new TCompactProtocol(mem_buf));
					msg.write(bin_proto.get());
					mem_buf->getBuffer(&buf_ptr, &sz);

					//Bytef* buffer = new Bytef[compressBound(sz) + 100];//100k bytes
					//uLong len = sizeof(buffer);
					//len = compressBound(sz) + 100;
					//memset(buffer, 0, len);

					/*int ret = */
					/*gzcompress(buf_ptr, sz, buffer, &len);
					buf_ptr = buffer;
					sz = len;*/

					//3.send
					zmq::message_t msgdestination(destination.size());
					memcpy(msgdestination.data(), destination.data(), destination.size() + 1);

					zmq::message_t msgbody(sz);
					std::memcpy(msgbody.data(), buf_ptr, sz);

					//4.send userid
					m_socket_pub->send(msgdestination, ZMQ_SNDMORE);
					m_socket_pub->send(msgbody);
				}
				catch (exception& e)
				{
					cout << "serializeAndSend exception: " << e.what() << endl;
					loggerv2::error("serializeAndSend exception : :%s", e.what());

				}
			}
		protected:
			zmq::socket_t* m_socket_pub;
			zmq::context_t& m_context;
			int ser_num = 0;
		};
	}
}
#endif //_UNCOMPRESS_SOCKET_CONNECTION_V2_H_


