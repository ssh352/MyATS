#ifndef _DEFAULT_DATE_TIME_PUBLISHER_V2_H_
#define _DEFAULT_DATE_TIME_PUBLISHER_V2_H_
#pragma once
#include "common.h"
namespace terra
{
	namespace common
	{


		class default_date_time_publisher
		{
		public:
			default_date_time_publisher();
			~default_date_time_publisher();
		private:
			time_duration m_lBias;
		public:
			void set_local_date_time(const long & value);

			void set_bias(const long & value){ m_lBias = milliseconds(value); }
			time_duration get_bias(){ return m_lBias; }
			ptime now();
			date today();
			time_duration get_time_of_day();
			string get_time_of_day_str();
			string get_now_str();

		};
		class date_time_publisher_gh
		{
		private:
			static default_date_time_publisher * g_default_date_time_publisher;
		public:
			static default_date_time_publisher * get_instance()
			{
				if (g_default_date_time_publisher == nullptr)
				{
					g_default_date_time_publisher = new default_date_time_publisher();
				}
				return g_default_date_time_publisher;
			}
		};
	}
}
#endif //_DEFAULT_DATE_TIME_PUBLISHER_V2_H_


