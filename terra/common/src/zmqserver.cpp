#include "zmqserver.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
namespace terra
{
	namespace common
	{
		zmq_server::zmq_server(zmq::context_t& context) :m_context(context)
		{
			/*
			ZMQ_PULL:
			���νڵ������socket�Ͻ���zmq_recv()������ȡ���η�������Ϣ��zmq_send()�ڴ�socket����û������ġ�
			*/
			m_socket_receiver = new zmq::socket_t(context, ZMQ_PULL);
			/*
			ZMQ_PUB:
			��Ҫ��������Ϣ����������ɢ����Ϣ�ġ����������ϵ�peer(ZMQ_SUB)�����յ�����ɢ������Ϣ��
			zmq_recv(3) ���API�ǲ����������socket�ϵģ�ԭ���Զ��׼���
			��zmq_send�����ڸ�socket��ʱ����Զ���������ģ�����������쳣����������Ϣ��ᱻ����
			*/
			m_socket_sender = new zmq::socket_t(context, ZMQ_PUB);
		}
		zmq_server::~zmq_server()
		{
			m_socket_receiver->close();
			m_socket_sender->close();

			delete m_socket_receiver;
			delete m_socket_sender;

			m_socket_receiver = nullptr;
			m_socket_sender = nullptr;
		}
		void zmq_server::init(const char* szReceiverListenIP, const char* szSenderListenIP)
		{
			loggerv2::info("Binding ZMQ Server Socket to(in/out) (%s / %s)", szReceiverListenIP, szSenderListenIP);
			int nRcvTO = 1;
			m_socket_receiver->setsockopt(ZMQ_RCVTIMEO, &nRcvTO, sizeof(nRcvTO));
			/*
			1.����bind()����֮��Ϊsocket()�����������׽��ֹ���һ����Ӧ��ַ�����͵������ַ�����ݿ���ͨ�����׽��ֶ�ȡ��ʹ��
			2.bind()����������������Ҫ���õģ�ֻ���û���������һ������ĵ�ַ��˿��������ʱ�����Ҫ�������������
			3.����û�����û�������Ҫ����ô������������ں˵��Զ���ѡַ����������Զ���ַѡ�񣬶�����Ҫ����bind()������ͬʱҲ���ⲻ��Ҫ�ĸ��Ӷȡ�
			4.��һ������£����ڷ���������������Ҫ����bind()���������ڿͻ���������Ҫ����bind()������
			*/
			m_socket_receiver->bind(szReceiverListenIP);
			m_socket_sender->bind(szSenderListenIP);
			m_last_time = microsec_clock::local_time();
		}
		void zmq_server::process_msg()
		{
			zmq::pollitem_t items[] = { { *m_socket_receiver, 0, ZMQ_POLLIN, 0 } };
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
					received = m_socket_receiver->recv(&msg, ZMQ_DONTWAIT);
				}
				catch (...)
				{
					loggerv2::info("receive error");
					return;
				}

				if (!received)
					return;

				//loggerv2::info("to remove ! start de-serializing. msg size is %d", msg.size());
//#ifdef _WWW_
//				arm_runner2broker_msg runnerMsg = arm_runner2broker_msg();
//				boost::shared_ptr<TMemoryBuffer> inputTransport(new TMemoryBuffer((uint8_t*)msg.data(), msg.size()));
//				boost::shared_ptr<TBinaryProtocol> binaryProtcol(new TBinaryProtocol(inputTransport));
//				runnerMsg.read(binaryProtcol.get());			
//#endif
			}
		}
		void zmq_server::process_idle()
		{

		}
		template<typename T>  void zmq_server::serializeAndSend(const T& msg, std::string destination)
		{

			//loggerv2::info("serializeAndSend msg now");
			//serialize
			uint8_t* buf_ptr;
			uint32_t sz;
			boost::shared_ptr<TMemoryBuffer> mem_buf(new TMemoryBuffer);
			boost::shared_ptr<TBinaryProtocol> bin_proto(new TBinaryProtocol(mem_buf));

			msg.write(bin_proto.get());
			mem_buf->getBuffer(&buf_ptr, &sz);

			//send
			zmq::message_t msgdestination(destination.size());
			memcpy(msgdestination.data(), destination.data(), destination.size() + 1);

			zmq::message_t msgbody(sz);
			std::memcpy(msgbody.data(), buf_ptr, sz);

			//send userid
			m_socket_sender->send(msgdestination, ZMQ_SNDMORE);
			m_socket_sender->send(msgbody);

		}
		//
		zmq_client::zmq_client(zmq::context_t& context, msg_event_handler handler, string destination) :m_context(context), m_destination(destination)
		{
			m_msg_event_handler = handler;
			/*
			ZMQ_PULL:
			���νڵ������socket�Ͻ���zmq_recv()������ȡ���η�������Ϣ��zmq_send()�ڴ�socket����û������ġ�
			*/
			m_socket_receiver = new zmq::socket_t(context, ZMQ_SUB);
		}
		zmq_client::~zmq_client()
		{
			this->release();
		}
		void zmq_client::release()
		{
			if (m_socket_receiver != nullptr)
			{
				m_socket_receiver->close();
				delete m_socket_receiver;
				m_socket_receiver = nullptr;
			}
		}
		bool zmq_client::init(const char* szSenderListenIP)
		{			
			loggerv2::info("zmq_client::init:%s",szSenderListenIP);			
#if 1
			int nRcvTO = 1;
			m_socket_receiver->setsockopt(ZMQ_RCVTIMEO, &nRcvTO, sizeof(nRcvTO));
			m_socket_receiver->setsockopt(ZMQ_SUBSCRIBE,"",0);//ok			
#else
			int nRcvTO = 1;
			m_socket_receiver->setsockopt(ZMQ_RCVTIMEO, &nRcvTO, sizeof(nRcvTO));
#endif
			m_socket_receiver->connect(szSenderListenIP);
			return true;
		}
		void zmq_client::process_msg()
		{
			zmq::message_t msg;
#if 0
			zmq::pollitem_t items[] = { { *m_socket_receiver, 0, ZMQ_POLLIN, 0 } };			
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
#endif
			bool received = false;
			while (true/*items[0].revents & ZMQ_POLLIN*/)
			{
				try{
#if 1
					received = m_socket_receiver->recv(&msg, ZMQ_DONTWAIT);
#else
					received = m_socket_receiver->recv(&msg);
#endif
				}
				catch (...)
				{
					loggerv2::info("receive error");
					return;
				}

				if (!received)
				{
					sleep_by_milliseconds(1000);
					continue;
				}
								
				loggerv2::info("zmq_client::process_msg:start de-serializing. msg size is %d,%s,m_destination:%s", msg.size(), string((char*)msg.data(), msg.size()).c_str(), m_destination.c_str());
				string destination = string((char*)msg.data(), msg.size());

				if (m_msg_event_handler)				
				{
					int more = 0;
					size_t more_size = sizeof(more);
					m_socket_receiver->getsockopt(ZMQ_RCVMORE, &more, &more_size);
					if (more > 0)
					{
						m_socket_receiver->recv(&msg, ZMQ_DONTWAIT);
						if (m_destination == destination)
						{
							m_msg_event_handler((uint8_t*)msg.data(), msg.size());
						}						
					}
				}
			}
		}
		//
	}
}