#include "zmqpublisher.h"
namespace terra
{
	namespace common
	{
		zmq_publisher::zmq_publisher(zmq::context_t& context) :m_context(context)
		{
			/*
			ZMQ_PUB��Ҫ��������Ϣ����������ɢ����Ϣ�ġ����������ϵ�peer(ZMQ_SUB)�����յ�����ɢ������Ϣ�� 
			zmq_recv(3) ���API�ǲ����������socket�ϵģ�ԭ���Զ��׼���
			��zmq_send�����ڸ�socket��ʱ����Զ���������ģ�����������쳣����������Ϣ��ᱻ����
			*/
			m_socket_pub = new zmq::socket_t(context, ZMQ_PUB);
			/*
			ZMQ_PULL:
			���νڵ������socket�Ͻ���zmq_recv()������ȡ���η�������Ϣ��zmq_send()�ڴ�socket����û������ġ�
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