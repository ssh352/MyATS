#ifndef _TERRA_COMMON_V2_H_
#define _TERRA_COMMON_V2_H_
#pragma once
//#ifdef _WIN32
//#include <WinSock2.h>
//#endif


#ifdef _WIN32
#ifndef _WIN32_LEAN_AND_MEAN_
#define _WIN32_LEAN_AND_MEAN_
#endif
#include <winsock2.h>
//#include<windows.h>
#endif
#include <string>
#include <list>
#include <map>
#include <vector>
#include <unordered_map>
#include "terra_logger.h"
#include "boost/thread/shared_mutex.hpp"
#include "boost/thread/locks.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <stdio.h>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#define BOOST_DATE_TIME_HAS_HIGH_PRECISION_CLOCK
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp" 
#include "boost/version.hpp"
#include <chrono>
using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;


namespace terra
{
	namespace common
	{
#if BOOST_VERSION <=105900
		inline ptime from_iso_extended_string(const std::string& s) {
			return boost::date_time::parse_delimited_time<ptime>(s, 'T');
		}
#endif
		typedef boost::shared_lock<boost::shared_mutex> boost_read_lock;
		typedef boost::unique_lock<boost::shared_mutex> boost_write_lock;
		typedef std::chrono::duration<long long, std::micro> lwdur;
		typedef std::chrono::time_point<std::chrono::high_resolution_clock> lwtp;//lwtp - lwtp = std::chrono::duration<__int64,struct std::ratio<1,10000000>>(windows) or std::chrono::duration<__int64,struct std::ratio<1,1000000000>> in linux

	    static std::chrono::time_point<std::chrono::high_resolution_clock> lw_min_time(std::chrono::duration<int>(0));
		static lwtp get_lwtp_now()
		{
			return std::chrono::high_resolution_clock::now();
		}

		static std::string lwtp_to_simple_datetime_string(const lwtp &tp)
		{
			auto tt = std::chrono::system_clock::to_time_t(tp);
			struct tm* ptm = localtime(&tt);

			char date[20] = { 0 };
			sprintf(date, "%04d-%02d-%02d% 02d:%02d:%02d",
				(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
				(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

			return std::string(date);
		}

		static std::string lwtp_to_simple_time_string(const lwtp &tp)
		{
			auto tt = std::chrono::system_clock::to_time_t(tp);
			struct tm* ptm = localtime(&tt);

			char date[9] = { 0 };
			if (ptm != nullptr)
			{
				sprintf(date, "%02d:%02d:%02d", (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
			}
			return std::string(date);
		}


		inline int get_hour_from_lwtp(const lwtp &tp)
		{
			auto tt = std::chrono::system_clock::to_time_t(tp);
			struct tm* ptm = localtime(&tt);
			return ptm->tm_hour;
		}

		static lwtp ptime_to_lwtp(ptime &pt)
		{
			tm tm1 = to_tm(pt);
			time_t tt = mktime(&tm1);
			std::chrono::seconds sec(tt);
			lwtp tp(sec);
			return tp;
		}


		inline lwtp string_to_lwtp(const boost::gregorian::date &date, char *timestr)
		{
			ptime tradeTime(date, duration_from_string(timestr));
			lwtp tp = ptime_to_lwtp(tradeTime);
			return tp;
		}

		inline lwtp string_to_lwtp(const char *timestr)
		{
			ptime tradeTime = time_from_string(timestr);;
			lwtp tp = ptime_to_lwtp(tradeTime);
			return tp;
		}


		/*
		对于类,要求重载操作符==,=
		如果T1 is class,且要求支持sort，则需重载操作符<
		*/
		template<class T1, class T2> class map_ex : public std::map<T1, T2>
		{
		public:
			bool contain_key(T1 t1)
			{
				return (this->find(t1) != this->end());
			}
			T2 get_by_key(T1 t1)
			{
				typename map_ex<T1, T2>::iterator it = this->find(t1);
				return (it != this->end() ? (*it).second : nullptr);
			}
			void add(T1 key, T2 value)
			{
				this->insert(typename std::map<T1, T2>::value_type(key, value));
			}
			void remove(T1 key)
			{
				typename map_ex<T1, T2>::iterator it = this->find(key);
				if (it != this->end())
				{
					this->erase(it);
				}
			}
		};
			template<class T1, class T2> class unordered_map_ex : public std::unordered_map<T1, T2>
			{
			public:
				bool contain_key(T1 t1)
				{
					return (this->find(t1) != this->end());
				}
				T2 get_by_key(T1 t1)
				{
					typename unordered_map_ex<T1, T2>::iterator it = this->find(t1);
					return (it != this->end() ? (*it).second : nullptr);
				}
				void add(T1 key, T2 value)
				{
					this->insert(typename std::unordered_map<T1, T2>::value_type(key, value));
				}
				void remove(T1 key)
				{
					typename unordered_map_ex<T1, T2>::iterator it = this->find(key);
					if (it != this->end())
					{
						this->erase(it);
					}
				}
			};						
			/*
			0:a=b
			1:a>b
			-1:a<b
			*/
#define Epsilon 0.000001	
			class math2
			{
			public:
				static bool eq(double a, double b)
				{
					return fabs(a - b) < Epsilon;
				}

				static bool inf(double a, double b)
				{
					return a < b - Epsilon;
				}

				static bool inf_eq(double a, double b)
				{
					return a < b + Epsilon;
				}

				static bool sup(double a, double b)
				{
					return a > b + Epsilon;
				}

				static bool sup_eq(double a, double b)
				{
					return a > b - Epsilon;
				}

				static bool is_zero(double a)
				{
					return fabs(a) < Epsilon;
				}

				static bool not_zero(double a)
				{
					return !is_zero(a);
				}


				static double floor_ex(double a)
				{
					return floor(a + Epsilon);
				}
				static double round(double value, int iPrecision)
				{
					static char buffer[64];
					double dValue = 0;
					memset(buffer, 0, sizeof(buffer));
					switch (iPrecision)
					{
					case 1:
						sprintf(buffer, "%.1f", value);
						dValue = atof(buffer);
						break;
					case 2:
						sprintf(buffer, "%.2f", value);
						dValue = atof(buffer);
						break;
					case 3:
						sprintf(buffer, "%.3f", value);
						dValue = atof(buffer);
						break;
					default:
						dValue = value;
						break;
					}
					return dValue;
				}
				static string round_ex(double value, int iPrecision=2)
				{
					char buffer[64];
					memset(buffer, 0, sizeof(buffer));
					switch (iPrecision)
					{
					case 1:
						sprintf(buffer, "%.1f", value);						
						break;
					case 2:
						sprintf(buffer, "%.2f", value);
						break;
					case 3:
						sprintf(buffer, "%.3f", value);						
						break;
					default:
						break;
					}
					return buffer;
				}
			};
			void printf_ex(const char *format, ...);

			void sleep_by_milliseconds(int m_millis);
	}
}
#endif //_TERRA_COMMON_V2_H_


