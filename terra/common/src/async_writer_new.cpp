#include "async_writer_new.h"

namespace terra
{
	namespace common
	{
		async_writer_new::async_writer_new()
		{
		}
		async_writer_new::~async_writer_new()
		{
			this->close();
		}
		bool async_writer_new::open()
		{				
			if (m_file_name.empty())
				return false;
			boost::filesystem::path p(m_file_name);
			if (!boost::filesystem::exists(p))
			{
				m_stream.open(m_file_name);
				if (!this->m_header.empty())
				{
					m_stream << m_header << std::endl;
					
				}
			}
			else
			{
				m_stream.open(m_file_name,ios::out | ios::app);
			}			
			std::thread t(std::bind(&async_writer_new::process_pending_msg,this));
			t.detach();
			m_thread.swap(t);
			return true;
		}
		void async_writer_new::process_pending_msg()
		{			
			while (true)
			{
				if (m_queue.read_available() > 0)
				{
					string * p = m_queue.Pop();
					if (p != nullptr)
					{
						m_stream << *p << std::endl;
						
						delete p;
						p = nullptr;
					}
				}
				else
				{
					sleep_by_milliseconds(500);
				}
			}
		}
		void async_writer_new::close()
		{
			if (m_thread.joinable()==true)
				m_thread.join();
			if (m_stream.is_open()==true )
				m_stream.close();
		}
		void async_writer_new::set_file_name(string filename)
		{
			m_file_name = filename;
			if (this->is_open())
			{
				this->close();
			}
			this->open();
		}
	}
}
