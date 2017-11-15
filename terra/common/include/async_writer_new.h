#ifndef ASYNC_STREAM_WRITER_H_
#define ASYNC_STREAM_WRITER_H_
#pragma once
#include "common.h"
#include <thread>
#include "LockFreeWorkQueue.h"
namespace terra
{
	namespace common
	{
		class async_writer_new
		{
		public:
			async_writer_new();
			~async_writer_new();
		public:			
			bool open();
			bool is_open(){ return m_stream.is_open(); }
			void write(string & msg)
			{ 				
				m_queue.Push(new string(msg));
			}
			void process_pending_msg();
			void close();
			string get_file_name(){ return m_file_name; }
			void set_file_name(string filename);
			string get_header(){ return m_header; }
			void set_header(string header){ m_header = header; }
		protected:
			string m_file_name;
			string m_header;
			LockFreeWorkQueue<string> m_queue;
			boost::filesystem::ofstream m_stream;
			std::thread m_thread;
		};
	}
}
#endif //ASYNC_STREAM_WRITER_H_


