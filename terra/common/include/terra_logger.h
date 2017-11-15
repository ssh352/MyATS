#ifndef _TERRA_LOGGER_H_
#define _TERRA_LOGGER_H_
#include "common.h"
#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include "LockFreeWorkQueue.h"
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
//namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
//namespace keywords = boost::log::keywords;
using boost::shared_ptr;
using namespace logging;

BOOST_LOG_ATTRIBUTE_KEYWORD(_timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(_severity, "Severity", boost::log::BOOST_LOG_VERSION_NAMESPACE::trivial::severity_level)
#pragma once
namespace terra
{
	namespace common
	{
		class log_record
		{
		public:
			std::string time;
			std::string msg;
		};
		
		class log_record_vector : public std::vector<log_record*>
		{
		public:
			log_record_vector(){}
			~log_record_vector();
		public:
			void remove_all();
		};

		class log_record_queue : public LockFreeWorkQueue<log_record>
		{
		public:
			void get_queue(log_record_vector & v);
			void insert(std::string& msg);
		};

		class loggerv2
		{
		private:
			static log_record_queue * g_log_queue;
		public:			
			static log_record_queue * get_instance();
			static void init(std::string& fileName, logging::trivial::severity_level slv = logging::trivial::severity_level::trace);

			static void info(const char* str) { BOOST_LOG_TRIVIAL(info) << str; }
			static void error(const char* str) { BOOST_LOG_TRIVIAL(error) << str; }// loggerv2::get_instance()->insert(std::move(std::string(str)));};
			static void debug(const char* str) { BOOST_LOG_TRIVIAL(debug) << str; }
			static void warn(const char* str) { BOOST_LOG_TRIVIAL(warning) << str; } //loggerv2::get_instance()->insert(std::move(std::string(str)));};

			static void info(const std::string fmt, ...);
			static void error(const std::string fmt, ...);
			static void debug(const std::string fmt, ...);
			static void warn(const std::string fmt, ...);

			//static void format(std::string &s, const std::string fmt, ...);								
		};
	}
}
#endif //_TERRA_LOGGER_H_


