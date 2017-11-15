#ifndef _SOCKET_CONNECTION_V2_H_
#define _SOCKET_CONNECTION_V2_H_
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
		class socket_connection 
		{
		public:
			socket_connection(zmq::context_t& context);
			~socket_connection();
		public:
			void init(const char* strPub);						
			template<typename T>  void publish(string destination, const T& msg)
			{
				serializeAndSend<T>(msg, destination);
			}
			/* Compress gzip data */
			/* data 原数据 ndata 原数据长度 zdata 压缩后数据 nzdata 压缩后长度 */
			static int gzcompress(Bytef *data, uLong ndata,Bytef *zdata, uLong *nzdata)
			{
				z_stream c_stream;
				int err = 0;

				if (data && ndata > 0) {
					c_stream.zalloc = NULL;
					c_stream.zfree = NULL;
					c_stream.opaque = NULL;
					//只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer
					if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
						MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) return -1;
					c_stream.next_in = data;
					c_stream.avail_in = ndata;
					c_stream.next_out = zdata;
					c_stream.avail_out = *nzdata;
					while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata) {
						if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
					}
					if (c_stream.avail_in != 0) return c_stream.avail_in;
					for (;;) {
						if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
						if (err != Z_OK) return -1;
					}
					if (deflateEnd(&c_stream) != Z_OK) return -1;
					*nzdata = c_stream.total_out;
					return 0;
				}
				return -1;
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

					//if (destination != "Log")
					//{
						boost::shared_ptr<TMemoryBuffer> jbuf(new TMemoryBuffer);
						boost::shared_ptr<TDebugProtocol> jpro(new TDebugProtocol(jbuf));
		
						msg.write(jpro.get());
						/*boost::shared_ptr<string> str_buf(new string);
						*str_buf = mem_buf->getBufferAsString();*/
						//jpro->writeString()
						std::ofstream in1;
						std::string strname = destination + ".json";
						in1.open(strname.c_str(), std::ios::out);
						in1 << "ser_num::" << ++ser_num << std::endl;
						in1 << jbuf->getBufferAsString() << endl;
						in1.close();
					//}
					Bytef* buffer = new Bytef[compressBound(sz) + 100];//100k bytes
					uLong len = sizeof(buffer);
					len = compressBound(sz) + 100;
					memset(buffer, 0, len);

					/*int ret = */
					gzcompress(buf_ptr, sz, buffer, &len);
					buf_ptr = buffer;
					sz = len;

					//3.send
					zmq::message_t msgdestination(destination.size());
					memcpy(msgdestination.data(), destination.data(), destination.size() + 1);

					zmq::message_t msgbody(sz);
					std::memcpy(msgbody.data(), buf_ptr, sz);

					//4.send userid
					m_socket_pub->send(msgdestination, ZMQ_SNDMORE);
					m_socket_pub->send(msgbody);
					delete[] buffer;
				}
				catch (exception& e)
				{

					cout << "serializeAndSend exception: " << e.what() << endl;
					

					loggerv2::error("serializeAndSend exception : :%s", e.what());
					//if (destination != "Log")
					{
						cout << destination << "  " << ser_num << endl;
						loggerv2::error("serializeAndSend exception : :%s", destination.c_str(), ser_num);
						boost::shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer);
						boost::shared_ptr<TDebugProtocol> bin_proto(new TDebugProtocol(mem_buf));
						msg.write(bin_proto.get());
						/*boost::shared_ptr<string> str_buf(new string);
						*str_buf = mem_buf->getBufferAsString();*/
						const string &str = mem_buf->getBufferAsString();

						std::ofstream in;
						in << "ser_num::" << ++ser_num << std::endl;
						std::string strname = destination + "_error.json";
						in.open(strname.c_str(), std::ios::app | std::ios::out);
						in << str << endl;
						in.close();
					}

					

				}
			}
		protected:
			zmq::socket_t* m_socket_pub;
			zmq::context_t& m_context; 
			int ser_num = 0;
		};
	}
}
#endif //_SOCKET_CONNECTION_V2_H_


