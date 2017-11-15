#include "zmqpublisher.h"
namespace terra
{
	namespace common
	{
		zmq_publisher::zmq_publisher(zmq::context_t& context) :m_context(context)
		{
			/*
			ZMQ_PUB主要用来让消息发布者用来散发消息的。所有连接上的peer(ZMQ_SUB)都能收到由它散发的消息。 
			zmq_recv(3) 这个API是不能用在这个socket上的，原因显而易见。
			而zmq_send作用在该socket上时是永远不会阻塞的，如果订阅者异常，发出的消息则会被丢弃
			*/
			m_socket_pub = new zmq::socket_t(context, ZMQ_PUB);
			/*
			ZMQ_PULL:
			下游节点在这个socket上进行zmq_recv()，来收取上游发来的消息。zmq_send()在此socket上是没有意义的。
			*/
			m_socket_rep = new zmq::socket_t(context, ZMQ_PULL);
			m_socket_snapshot = new zmq::socket_t(context, ZMQ_PUB);
		}

		zmq_publisher::~zmq_publisher()
		{
			m_socket_pub->close();
			m_socket_rep->close();
			m_socket_snapshot->close();
			delete m_socket_pub;
			delete m_socket_rep;
			delete m_socket_snapshot;
			m_socket_pub = nullptr;
			m_socket_rep = nullptr;
			m_socket_snapshot = nullptr;
		}

		void zmq_publisher::init(const char* strPub, const char* strRep, const char* strSS){
			loggerv2::info("Binding ZMQ Publisher Pub Socket to %s, SnapShot Socket to %s", strPub, strSS);
			m_socket_pub->bind(strPub); //could be "tcp://*:5556" or "ipc://feeder.ipc"
			m_socket_snapshot->bind(strSS);
			//must set timeout to a finite value.
			int to = 1;
			m_socket_rep->setsockopt(ZMQ_RCVTIMEO, &to, sizeof(to));
			loggerv2::info("Binding ZMQ Publisher Rep Socket to %s", strRep);
			m_socket_rep->bind(strRep);
			return;
		}
		void zmq_publisher::process_idle()
		{
			//loggerv2::info("ZmqPublisher::process_idle..");
			zmq::pollitem_t items[] = { { *m_socket_rep, 0, ZMQ_POLLIN, 0 } };
			zmq::message_t msg;
			int rc = 0;
			try{
				rc = zmq::poll(&items[0], 1, 1);
			}
			catch (...)
			{
				loggerv2::info("poll error");
				return;
			}
			if (rc < 0)
				return;
			bool received = false;
			while (items[0].revents & ZMQ_POLLIN)
			{
				try{
					received = m_socket_rep->recv(&msg, ZMQ_DONTWAIT);
				}
				catch (...)
				{
					loggerv2::info("receive error");
					return;
				}
				if (!received)
					return;
				//loggerv2::info("to remove ! start de-serializing. msg size is %d",msg.size());
//#ifdef _WWW_
//				boost::shared_ptr<TMemoryBuffer> inputTransport(new TMemoryBuffer((uint8_t*)msg.data(), msg.size()));
//				boost::shared_ptr<TBinaryProtocol> binaryProtcol(new TBinaryProtocol(inputTransport));
//				arm_feed_msg feedMsg = arm_feed_msg();			
//				feedMsg.read(binaryProtcol.get());		
//#endif
			}
			return;
		}

	}
}