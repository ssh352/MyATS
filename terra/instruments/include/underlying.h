#ifndef _UNDERLYING_H_
#define _UNDERLYING_H_
#include "instrumentcommon.h"

using namespace boost::gregorian;
namespace terra
{
	
	namespace instrument
	{
		class financialinstrument;
		class future;
		class optionclass;
		class futureclass;
		class etf;
		class index;
		class underlying
		{

		public:
			underlying(std::string & name);
			~underlying();
		public:
			//std::string  to_string(){ return m_strName; }
			std::string&  get_name() { return m_strName; }
			bool append(optionclass * poptionclass);
			bool append(futureclass * pfutureclass);
			void set(etf * pETF);
			int  compute_days_off(date dt);
			void compute_days_off();
			void load(std::string & filename);
			void save(std::string & filename);
			void add_day_off(date & dt);
			void set(index * pIndex);
			etf* get_etf(){ return m_pRefETF; }
			index * get_index(){ return m_pRefIndex; }
			map_ex<string, optionclass*> & get_option_class_list()
			{
				return m_ref_option_class_map;
			}
			map_ex<string, futureclass*> & get_future_class_list()
			{
				return m_ref_future_class_map;
			}
			map_ex<date, date> & get_days_off_list()
			{
				return m_days_off_list;
			}
			double get_rate(){ return m_dRate; }
			void   set_rate(double value){ m_dRate = value; }
			void show();
			financialinstrument* get_underlying_financialinstrument(){ return underlying_instrument; }
			future* get_underlying_future(){ return underlying_future; }
		protected:
			map_ex<string, optionclass*>  m_ref_option_class_map;
			map_ex<string, futureclass*>  m_ref_future_class_map;
			map_ex<date, date>   m_days_off_list;
			std::string                 m_strName;			
			etf  * m_pRefETF;
			double m_dRate;
			index *                  m_pRefIndex;
			financialinstrument* underlying_instrument;
			future* underlying_future;
		};
	}
}
#endif //_UNDERLYING_H_


