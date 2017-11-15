#include "uncompress_socket_connection.h"

namespace terra
{
	namespace common
	{
		uncompress_socket_connection::uncompress_socket_connection(zmq::context_t& context) : m_context(context)
		{
			m_socket_pub = new zmq::socket_t(context, ZMQ_PUB);
		}
		uncompress_socket_connection::~uncompress_socket_connection()
		{
			m_socket_pub->close();
			delete m_socket_pub;
			m_socket_pub = nullptr;
		}
		void uncompress_socket_connection::init(const char* strPub)
		{
			if (strPub == nullptr || strlen(strPub) < 1)
			{
				printf_ex("socket_connection::init fail because the strPub is null!\n");
				return;
			}
			m_socket_pub->bind(strPub);
		}
	}
}
