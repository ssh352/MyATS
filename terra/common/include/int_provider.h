#ifndef __INT_PROVIDER_H__
#define __INT_PROVIDER_H__

#include <atomic>
#include <thread>
#include <boost/filesystem/fstream.hpp>


namespace terra
{
	namespace common
	{
		class int_provider
		{
		public:
			int_provider(int timer = 100);
			virtual ~int_provider();
			void set_current_int(int n);
			int get_current_int();
			int get_next_int();
			bool is_alive();
			void is_alive(bool b);

			bool start(int n = 0);
			bool stop();

			void set_filename(std::string name){ m_filename = name; }

		protected:
			void Process();

			std::thread m_thread;
			std::atomic_int m_currentInt;
			std::string m_filename;
			boost::filesystem::ofstream m_ofstream;
			
			int m_timer;
			std::atomic<bool> m_isAlive;
		};
	}
}
#endif