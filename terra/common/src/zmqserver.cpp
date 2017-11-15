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
			下游节点在这个socket上进行zmq_recv()，来收取上游发来的消息。zmq_send()在此socket上是没有意义的。
			*/
			m_socket_receiver = new zmq::socket_t(context, ZMQ_PULL);
			/*
			ZMQ_PUB:
			主要用来让消息发布者用来散发消息的。所有连接上的peer(ZMQ_SUB)都能收到由它散发的消息。
			zmq_recv(3) 这个API是不能用在这个socket上的，原因显而易见。
			而zmq_send作用在该socket上时是永远不会阻塞的，如果订阅者异常，发出的消息则会被丢弃
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
			1.调用bind()函数之后，为socket()函数创建的套接字关联一个相应地址，发送到这个地址的数据可以通过该套接字读取与使用
			2.bind()函数并不是总是需要调用的，只有用户进程想与一个具体的地址或端口相关联的时候才需要调用这个函数。
			3.如果用户进程没有这个需要，那么程序可以依赖内核的自动的选址机制来完成自动地址选择，而不需要调用bind()函数，同时也避免不必要的复杂度。
			4.在一般情况下，对于服务器进程问题需要调用bind()函数，对于客户进程则不需要调用bind()函数。
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
			下游节点在这个socket上进行zmq_recv()，来收取上游发来的消息。zmq_send()在此socket上是没有意义的。
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