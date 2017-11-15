#include "int_provider.h"
#include "terra_logger.h"
namespace terra
{
	namespace common
	{
		int_provider::int_provider(int timer)
		{
			m_timer = timer;
			m_currentInt = 0;
			m_isAlive = true;
		}

		int_provider::~int_provider()
		{
		
		}

		void int_provider::set_current_int(int currentInt)
		{
			m_currentInt = currentInt;
		}

		int int_provider::get_current_int()
		{
			// No need to lock as the write is always done from the same thread (only ad is done by the persistence thread).
			return m_currentInt;
		}

		int int_provider::get_next_int()
		{
			return ++m_currentInt;
		}

		bool int_provider::is_alive()
		{
			return m_isAlive;
		}

		void int_provider::is_alive(bool b)
		{
			m_isAlive = b;
		}

		bool int_provider::start(int n)
		{
			boost::filesystem::path p(m_filename.c_str());

			// 1 - If file exists and has been created today, reload int
			if (boost::filesystem::exists(p))
			{
					boost::filesystem::ifstream stream(p);
					if (stream.good())
					{
						std::string lineRead;
						std::getline(stream, lineRead);
						stream.close();


						if (lineRead.length() > 0)
							m_currentInt = n>atoi(lineRead.c_str())?n:atoi(lineRead.c_str());
						else
							m_currentInt = n;
					}
			}
			else //if file doesn't exist, we will create it in the next step.
				m_currentInt = n;


			// 2 - Create OutputFile
			boost::filesystem::create_directories(p.parent_path());
			m_ofstream.open(p, std::ios::out | std::ios::trunc);
			if (m_ofstream.bad() || m_ofstream.fail())
			{
				loggerv2::error("int_provider: creation of output file [%s] failed...", m_filename.c_str());
				return false;
			}


			// 3 - Start Persister Thread
			//SetPriority = ThreadPriority.BelowNormal;
			//RTThread::Create();

			std::thread th(std::bind(&int_provider::Process, this));
			m_thread.swap(th);
			return true;
		}

		bool int_provider::stop()
		{
			// Terminate Thread
			is_alive(false);
			//RTThread::Join();
			m_thread.join();
			// Close File
			m_ofstream.close();

			return true;
		}

		void int_provider::Process()
		{
			while (is_alive())
			{
				int c;
				
				c = m_currentInt;
				


				m_ofstream.seekp(std::ios_base::beg);
				m_ofstream << c;
				m_ofstream.flush();

				// Sleep 100 ms by default
				sleep_by_milliseconds(m_timer);
				//Sleep(m_timer);
			}
		}
	}
}