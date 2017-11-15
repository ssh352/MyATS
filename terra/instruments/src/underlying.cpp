#include <string>
#include "underlying.h"
#include "optionclass.h"
#include "futureclass.h"
#include "index.h"
#include "etf.h"
#include "maturity.h"
#include "referential.h"
namespace terra
{

	namespace instrument
	{
		underlying::underlying(std::string & name) :m_strName(name)
		{
			m_dRate = 0.0;
			this->m_pRefETF = nullptr;
			this->m_pRefIndex = nullptr;
			underlying_instrument = referential::get_instance()->get_instrument_map().get_by_key(name);
			underlying_future = referential::get_instance()->get_future_map().get_by_key(name);
		}
		underlying::~underlying()
		{
		}
		bool underlying::append(optionclass * poptionclass)
		{
			if (poptionclass == nullptr)
				return false;
			if (this->m_ref_option_class_map.contain_key(poptionclass->get_name()) == false)
			{
				//loggerv2::info("underlying::append underlying:%s,optionclassName:%s", to_string().c_str(), poptionclass->to_string().c_str());
				m_ref_option_class_map[poptionclass->get_name()] = poptionclass;
			}
			else
			{
				//loggerv2::info("underlying::append underlying:%s,already include the optionclassName:%s", to_string().c_str(), poptionclass->to_string().c_str());
			}
			return true;
		}
		bool underlying::append(futureclass * pfutureclass)
		{
			if (pfutureclass == nullptr)
				return false;
			if (this->m_ref_future_class_map.contain_key(pfutureclass->get_name()) == false)
			{
				//loggerv2::info("underlying::append underlying:%s,futureclassName:%s", to_string().c_str(), pfutureclass->to_string().c_str());
				m_ref_future_class_map[pfutureclass->get_name()] = pfutureclass;
			}
			else
			{
				//loggerv2::info("underlying::append underlying:%s,already include the futureclassName:%s", to_string().c_str(), pfutureclass->to_string().c_str());
			}
			return true;
		}
		void underlying::set(etf * pETF)
		{
			this->m_pRefETF = pETF;
		}
		void underlying::set(index * pIndex)
		{
			m_pRefIndex = pIndex;
		}
		int underlying::compute_days_off(date dt)
		{
			int nbDaysOff = 0;
			/*time_t toDay;
			time(&toDay);
			date_time_ex   dtTodayDateTime;
			dtTodayDateTime.set_date(toDay);*/
			int nDays = (dt -day_clock::local_day()).days();
			
			
			for (int i = 0; i < nDays; i++)
			{
				date dt2 = day_clock::local_day() + date_duration(i);
				
				if (dt2.day_of_week() == Sunday || dt2.day_of_week() == Saturday || m_days_off_list.contain_key(dt2) == true)
				{
					nbDaysOff++;
				}
			}
			return nbDaysOff;
		}
		void underlying::compute_days_off()
		{
			for (auto & v : m_ref_option_class_map)
			{
				optionclass* option = v.second;
				for (auto & k : option->get_maturity_map())
				{
					k.second->compute_days_off();
				}
			}
		}
		void underlying::load(std::string & filename)
		{
			boost::property_tree::ptree root;
			boost::property_tree::read_json(filename, root);
			if (root.size() <= 0)
				return;
			//std::string strRate = root.get<std::string>("Rate");
			this->m_dRate = root.get<double>("Rate",0);

			//2.ถมสýื้
			ptree p1 = root.get_child("DaysOffList");
			for (auto& it : p1)
			{
				string strDateTime = it.second.get<std::string>("");
				date dateTime(from_string(strDateTime));
				//dateTime.set_date(strDateTime);
				this->add_day_off(dateTime);
			}
		}
		void underlying::save(std::string & filename)
		{
			boost::property_tree::ptree pt_root;
			boost::property_tree::ptree children;
			boost::property_tree::ptree child;
			for (auto & v : this->m_days_off_list)
			{
				//child.put("", v.second .get_string(terra::common::date_time_ex::date_format::FN3));
				child.put("",to_iso_extended_string(v.second));
				children.push_back(std::make_pair("", child));				
			}
			pt_root.add_child("DaysOffList", children);
			pt_root.put("Name",get_name());			
		
			pt_root.put("Rate",get_rate());

			boost::property_tree::write_json(filename, pt_root);
		}
		void underlying::add_day_off(date & dt)
		{
			this->m_days_off_list.add(dt, dt);
			compute_days_off();
		}
		void underlying::show()
		{
			loggerv2::info("underlying::show enter----------");
			loggerv2::info("underlying::show name:%s", m_strName.c_str());
			if (this->m_pRefETF != nullptr)
			{
				loggerv2::info("underlying::show etf:%s", m_pRefETF->get_code().c_str());
			}
			else
			{
				loggerv2::info("underlying::show etf:null");
			}
			if (this->m_pRefIndex != nullptr)
			{
				loggerv2::info("underlying::show index:%s", m_pRefIndex->get_code().c_str());
			}
			else
			{
				loggerv2::info("underlying::show index:null");
			}

			loggerv2::info("underlying::show m_ref_option_class_map.size:%d", m_ref_option_class_map.size());
			for (auto& iter : m_ref_option_class_map)
			{
				optionclass * poptionclass = iter.second;
				loggerv2::info("underlying::show optionclass:%s", poptionclass->get_name().c_str());
			}

			loggerv2::info("underlying::show m_ref_future_class_map.size:%d", m_ref_future_class_map.size());
			for (auto& iter : m_ref_future_class_map)
			{
				futureclass * pfutureclass = iter.second;
				loggerv2::info("underlying::show futureclass:%s", pfutureclass->get_name().c_str());
			}

			loggerv2::info("underlying::show end----------");
		}
	}
}