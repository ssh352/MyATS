#include "defaultdatetimepublisher.h"
//#include "terra_time.h"
namespace terra
{
	namespace common
	{
		default_date_time_publisher * date_time_publisher_gh::g_default_date_time_publisher = nullptr;

		default_date_time_publisher::default_date_time_publisher()
		{
			//m_lBias = ticks(0);
		}
		default_date_time_publisher::~default_date_time_publisher()
		{
		}

		void default_date_time_publisher::set_local_date_time(const long& value)
		{
			set_bias(value - microsec_clock::local_time().time_of_day().total_milliseconds());
		}

		ptime default_date_time_publisher::now()
		{	
		/*	date_time dt;
			auto t = get_ticks();
			t += m_lBias;			
			time_t time = std::chrono::duration_cast<seconds>(t).count();			
			dt.set_date(time);			
			return dt;*/
			return microsec_clock::local_time() + m_lBias;
		}
		date default_date_time_publisher::today()
		{
			/*auto t = std::chrono::system_clock::now();
			date_time date;
			date.set(t);
			return date;*/
			return now().date();
		}

		time_duration default_date_time_publisher::get_time_of_day()
		{/*
			auto time_v = now() - today();
			date_time dt(time_v);

			int hh = 0, mi = 0, ss = 0;
			dt.get_time(&hh, &mi, &ss);

			time_duration span(hh,mi,ss);*/
			
			return now().time_of_day();
		}

		string default_date_time_publisher::get_time_of_day_str()
		{
			return  to_simple_string(get_time_of_day());
		}

		string default_date_time_publisher::get_now_str()
		{
			return to_iso_extended_string(now());
		}

		/*	void default_date_time_publisher::compute_bias(const ticks & value)
		{			
			m_lBias = value - get_ticks();
		}
		long default_date_time_publisher::get_ticks()
		{			
			return  std::chrono::duration_cast<ticks>(system_clock::now().time_since_epoch());
		}
		long default_date_time_publisher::distance_milliseconds(const ticks & now, const ticks & last)
		{
			return now - last;
		}*/
	}
}
