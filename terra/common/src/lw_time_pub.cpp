#include "lw_time_pub.h"

namespace terra
{
	namespace common
	{

		void lw_time_pub::set_local_date_time(lwtp& value)
		{
			m_lBias = std::chrono::duration_cast<std::chrono::microseconds>(value - get_lwtp_now());
		}

		void lw_time_pub::set_local_date_time(const long& mill)
		{
			lwtp tp = get_lwtp_now();
			std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::milliseconds> tpp = std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
			m_lBias = std::chrono::microseconds(mill - tpp.time_since_epoch().count());
			//set_bias(value - microsec_clock::local_time().time_of_day().total_milliseconds());
		}

		lwtp lw_time_pub::now()
		{
			return get_lwtp_now() + m_lBias;
		}

		lwdur lw_time_pub::get_time_of_day()
		{
			lwtp tp= now();
			auto tt = std::chrono::system_clock::to_time_t(tp);
			struct tm* ptm = localtime(&tt);

			auto micdur = tp - std::chrono::seconds(tt);
			long mic = std::chrono::time_point_cast<std::chrono::microseconds>(micdur).time_since_epoch().count();
			lwdur ldur = std::chrono::hours(ptm->tm_hour) + std::chrono::minutes(ptm->tm_min) + std::chrono::minutes(ptm->tm_sec) + std::chrono::microseconds(mic);
		
			return ldur;
		}

		string lw_time_pub::get_time_of_day_str()
		{
			lwtp tp = now();
			auto tt = std::chrono::system_clock::to_time_t(tp);
			struct tm* ptm = localtime(&tt);

			char date[20] = { 0 };
			sprintf(date, "02d:%02d:%02d",
				(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

			return std::string(date);
		}

		string lw_time_pub::get_now_str()
		{
			lwtp tp = now();
			auto tt = std::chrono::system_clock::to_time_t(tp);
			struct tm* ptm = localtime(&tt);

			char date[20] = { 0 };
			sprintf(date, "%04d-%02d-%02d% 02d:%02d:%02d",
				(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
				(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

			return std::string(date);
		}

	}
}
