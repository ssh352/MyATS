#include "terra_logger.h"
#include "defaultdatetimepublisher.h"
#include "boost/log/sinks/bounded_fifo_queue.hpp"
#include "boost/log/sinks/block_on_overflow.hpp"
namespace terra
{
	namespace common
	{
		log_record_queue * loggerv2::g_log_queue = nullptr;


		log_record_vector::~log_record_vector()
		{
			remove_all();
		}
		void log_record_vector::remove_all()
		{			
			for (auto & it : *this)
			{				
				delete it;				
			}
			this->clear();
		}
		void log_record_queue::get_queue(log_record_vector & v)
		{
			while (m_queue.empty()==false)
			{
				log_record * record = this->Pop();
				v.push_back(record);
			}
		}

		void log_record_queue::insert(std::string& msg)
		{
			log_record * record = new log_record();

			char          buf[1024];
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%s", date_time_publisher_gh::get_instance()->get_now_str().c_str());

			record->time = buf;
			record->msg =  msg;
			this->Push(record);
		}

		log_record_queue * loggerv2::get_instance()
		{
			if (g_log_queue == nullptr)
			{
				g_log_queue = new log_record_queue();
			}
			return g_log_queue;
		}
		void loggerv2::init(std::string& fileName, logging::trivial::severity_level slv)
    	{
			boost::filesystem::path my_path(fileName);			
			std::string sPath = my_path.parent_path().generic_string();

			typedef boost::log::sinks::asynchronous_sink< boost::log::sinks::text_file_backend, boost::log::sinks::bounded_fifo_queue< 5,sinks::block_on_overflow >  > file_sink;

			boost::shared_ptr< file_sink > sink = boost::make_shared<file_sink>(
				keywords::file_name = fileName+"_%5N.log", // fileName + "%Y%m%d_%H%M%S_%5N.log",      // file name pattern
				keywords::rotation_size = 64 * 1024 * 1024,                    // rotation size, in characters
				keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0)
				);
			
			// Set up where the rotated files will be stored
			sink->locked_backend()->set_file_collector(boost::log::sinks::file::make_collector(
				keywords::target = sPath.c_str(),                          // where to store rotated files
				//keywords::max_size = 16 * 1024 * 1024,              // maximum total size of the stored files, in bytes
				keywords::min_free_space = 100 * 1024 * 1024        // minimum free space on the drive, in bytes
				));

			// Upon restart, scan the target directory for files matching the file_name pattern
			sink->locked_backend()->scan_for_files();
			sink->locked_backend()->auto_flush(true);

			sink->set_formatter
				(
				expr::format("%1% <%2%> - %3%")
				% expr::format_date_time(_timestamp, "%H:%M:%S.%f")
				% _severity
				% expr::smessage
				);

			logging::core::get()->set_filter
				(
				logging::trivial::severity >= slv
				);

			// Add it to the core
			logging::core::get()->add_sink(sink);

			// Add some attributes too
			logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
			//logging::add_console_log(std::clog, keywords::format = "%TimeStamp%: %Message%");
		}


		void loggerv2::info(const std::string fmt, ...)
		{

			std::string s;
			int n, size = 100;
			bool b = false;
			va_list marker;

			while (!b)
			{
				s.resize(size);
				va_start(marker, fmt);
				n = vsnprintf((char*)s.c_str(), size, fmt.c_str(), marker);
				va_end(marker);
				if ((n > 0) && ((b = (n < size)) == true)) s.resize(n); else size *= 2;
			}
			BOOST_LOG_TRIVIAL(info) << s;
		}


		void loggerv2::error(const std::string fmt, ...)
		{

			std::string s;
			int n, size = 100;
			bool b = false;
			va_list marker;

			while (!b)
			{
				s.resize(size);
				va_start(marker, fmt);
				n = vsnprintf((char*)s.c_str(), size, fmt.c_str(), marker);
				va_end(marker);
				if ((n > 0) && ((b = (n < size)) == true)) s.resize(n); else size *= 2;
			}
			BOOST_LOG_TRIVIAL(error) << s;
			loggerv2::get_instance()->insert(s);
		}

		void loggerv2::warn(const std::string fmt, ...)
		{

			std::string s;
			int n, size = 100;
			bool b = false;
			va_list marker;

			while (!b)
			{
				s.resize(size);
				va_start(marker, fmt);
				n = vsnprintf((char*)s.c_str(), size, fmt.c_str(), marker);
				va_end(marker);
				if ((n > 0) && ((b = (n < size)) == true)) s.resize(n); else size *= 2;
			}
			BOOST_LOG_TRIVIAL(warning) << s;
			loggerv2::get_instance()->insert(s);
		}

		void loggerv2::debug(const std::string fmt, ...)
		{

			std::string s;
			int n, size = 100;
			bool b = false;
			va_list marker;

			while (!b)
			{
				s.resize(size);
				va_start(marker, fmt);
				n = vsnprintf((char*)s.c_str(), size, fmt.c_str(), marker);
				va_end(marker);
				if ((n > 0) && ((b = (n < size)) == true)) s.resize(n); else size *= 2;
			}
			BOOST_LOG_TRIVIAL(debug) << s;
		}

	}
}
