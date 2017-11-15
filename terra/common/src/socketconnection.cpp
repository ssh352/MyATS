#include "socketconnection.h"
#include "terra_logger.h"
namespace terra
{
	namespace common
	{
		socket_connection::socket_connection(zmq::context_t& context) : m_context(context)
		{
			m_socket_pub = new zmq::socket_t(context, ZMQ_PUB);
		}
		socket_connection::~socket_connection()
		{
			m_socket_pub->close();
			delete m_socket_pub;
			m_socket_pub = nullptr;			
		}
		void socket_connection::init(const char* strPub)
		{
			if (strPub == nullptr || strlen(strPub) < 1)
			{
				printf_ex("socket_connection::init fail because the strPub is null!\n");
				//terra_logger::error("socket_connection::init fail because the strPub is null!\n");
				return;
			}			
			m_socket_pub->bind(strPub);
		}
//#ifdef _WWW
//		template<typename T>  void socket_connection::serializeAndSend(const T& msg, std::string destination)
//		{
//			//serialize
//			uint8_t* buf_ptr;
//			uint32_t sz;
//			shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer);
//			shared_ptr<TBinaryProtocol> bin_proto(new TBinaryProtocol(mem_buf));
//
//			msg.write(bin_proto.get());
//			mem_buf->getBuffer(&buf_ptr, &sz);
//
//			//send
//			zmq::message_t msgdestination(destination.size());
//			memcpy(msgdestination.data(), destination.data(), destination.size() + 1);
//
//			zmq::message_t msgbody(sz);
//			std::memcpy(msgbody.data(), buf_ptr, sz);
//
//			//send userid
//			m_socket_pub->send(msgdestination, ZMQ_SNDMORE);
//			m_socket_pub->send(msgbody);
//		}	
//#endif
	}
}
