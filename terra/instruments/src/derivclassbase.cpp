#include "derivclassbase.h"
#include "abstractderivative.h"
#include "maturity.h"
namespace terra
{
	namespace instrument
	{

		derivclassbase::derivclassbase(underlying * pUnderlying, std::string & className, int pointValue, currency * pCurrency) :instrumentclass(className, pointValue, pCurrency)
		{
			m_pUnderlying = pUnderlying;
		}



		derivclassbase::~derivclassbase()
		{
		}

		void derivclassbase::save(std::string &filename)
		{
			boost::property_tree::ptree pt_root;						
			pt_root.put("MaturityTime",to_simple_string( this->m_MaturityTime));
			instrumentclass::save(pt_root);
			/*
			pt_root.put("ClassName", get_name());
			pt_root.put("PointValue", this->get_point_value());

			boost::property_tree::ptree child_currency;
			child_currency.put("Name", this->get_currency_name());
			child_currency.put("ToReference", this->get_currency()->get_to_reference());
			pt_root.add_child("Currency", child_currency);

			boost::property_tree::ptree child_fees;						
			child_fees.put("FeesFloatExchange", math2::round_ex(this->get_fees_float_exchange()));
			child_fees.put("FeesFloatBroker", math2::round_ex(this->get_fees_float_broker()));
			child_fees.put("FeesFixExchange", math2::round_ex(this->get_fees_fix_exchange()));
			child_fees.put("FeesFixBroker", math2::round_ex(this->get_fees_fix_broker()));
			child_fees.put("FeesSellAmount", math2::round_ex(this->get_fees_sell_amount()));
			pt_root.add_child("FeesStructure", child_fees);

			pt_root.put("ImplicitPreopen", this->m_bImplicitPreopen);
			pt_root.put("ImplicitPreopenTh", this->m_bImplicitPreopenTh);					
			*/
			boost::property_tree::write_json(filename, pt_root);
		}
		void derivclassbase::load(std::string &filename)
		{
			instrumentclass::load(filename);
			boost::property_tree::ptree root;
			boost::property_tree::read_json(filename, root);
			if (root.size() <= 0)
			{
				loggerv2::error("derivclassbase::load Can't read the file: %s", filename.c_str());
				return;
			}
			string strMaturityTime = root.get<string>("MaturityTime");
			time_duration dt(duration_from_string(strMaturityTime));
			//dt.set_time(strMaturityTime);
			this->set_maturity_time(dt);
		}
		void derivclassbase::set_maturity_time(time_duration dt)
		{
			this->m_MaturityTime = dt;
			
			for(auto &it : m_abstract_derivative_map)
			{				
				it.second->on_maturity_time_changed();				
			}
		}
		void derivclassbase::show()
		{
			loggerv2::info("derivclassbase::show enter");
			instrumentclass::show();
			loggerv2::info("derivclassbase::show MaturityTime:%s",to_simple_string(this->get_maturity_time()).data());
			loggerv2::info("derivclassbase::show MaturityList,size:%d", m_maturity_map.size());
						
			for(auto &it : m_maturity_map)			
			{				
				it.second->show();
			}

			loggerv2::info("derivclassbase::show abstractderivativeList,size:%d", m_abstract_derivative_map.size());
			
			for(auto &it : m_abstract_derivative_map)
			{			
				loggerv2::info("derivclassbase::show abstractderivativeName:%s", it.second->get_code().c_str());
			}

			loggerv2::info("derivclassbase::show end");
		}
	}
}
