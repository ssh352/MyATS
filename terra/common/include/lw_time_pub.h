#ifndef _LW_TIME_PUB_H_
#define _LW_TIME_PUB_H_
#pragma once
#include "common.h"
#include "singleton.hpp"
namespace terra
{
	namespace common
	{
		class lw_time_pub :public SingletonBase<lw_time_pub>
		{
		private:
			lwdur m_lBias;
		public:
			void set_local_date_time(lwtp & value);
			void set_local_date_time(const long& mill);

			void set_bias(const long & value){ m_lBias = std::chrono::microseconds(value); }
			lwdur get_bias(){ return m_lBias; }
			lwtp now();
			//date today();
			lwdur get_time_of_day();
			string get_time_of_day_str();
			string get_now_str();
		};

	}
}
#endif //_LW_TIME_PUB_H_