#include "instrumentclass.h"
#include "instrumentcommon.h"
#include "tickrule.h"
namespace terra
{
	namespace instrument
	{

		instrumentclass::instrumentclass(std::string & className, int pointValue, currency * pCurrency)
			:m_strClassName(className), m_PointValue(pointValue), m_pRefCurrency(pCurrency)
		{
			m_pFeesStructure = new feesstructure();
			this->m_bImplicitPreopen = false;
			this->m_bImplicitPreopenTh = false;
		}


		instrumentclass::~instrumentclass()
		{
			if (m_pFeesStructure != nullptr)
			{
				delete m_pFeesStructure;
			}
			if (m_ptickrule != nullptr)
			{
				delete m_ptickrule;
			}
		}
		void instrumentclass::save(std::string &filename)
		{
			boost::property_tree::ptree pt_root;
			instrumentclass::save(pt_root);			
			boost::property_tree::write_json(filename, pt_root);
		}
		void instrumentclass::load(std::string &filename)
		{
			boost::property_tree::ptree root;
			boost::property_tree::read_json(filename, root);
			if (root.size() <= 0)
			{
				loggerv2::error("instrumentclass::load Can't read the file: %s", filename.c_str());
				return;
			}
			//string strClassName = root.get<string>("ClassName");
			//string strPointValue = root.get<string>("PointValue");
			/*string strImplicitPreopen = root.get<string>("ImplicitPreopen");
			string strImplicitPreopenTh = root.get<string>("ImplicitPreopenTh");
			if (strImplicitPreopen == "false")
				this->m_bImplicitPreopen = false;
			else
			{
				this->m_bImplicitPreopen = true;
			}*/
			m_bImplicitPreopen = root.get<bool>("ImplicitPreopen");
			/*if (strImplicitPreopenTh == "false")
			{
				this->m_bImplicitPreopenTh = false;
			}
			else
			{
				this->m_bImplicitPreopenTh = false;
			}*/
			m_bImplicitPreopenTh = root.get<bool>("ImplicitPreopenTh");
			ptree p1 = root.get_child("FeesStructure");
			/*	string  strFeesFloatExchange = p1.get<string>("FeesFloatExchange");
				string  strFeesFloatBroker = p1.get<string>("FeesFloatBroker");
				string  strFeesFixExchange = p1.get<string>("FeesFixExchange");
				string  strFeesFixBroker = p1.get<string>("FeesFixBroker");
				string  strFeesSellAmount = p1.get<string>("FeesSellAmount");*/
			if (this->m_pFeesStructure != nullptr)
			{
				/*
				m_pFeesStructure->set_fees_float_exchange(p1.get<double>("FeesFloatExchange",0));
				m_pFeesStructure->set_fees_float_broker(p1.get<double>("FeesFloatBroker", 0));
				m_pFeesStructure->set_fees_fix_exchange(p1.get<double>("FeesFixExchange", 0));
				m_pFeesStructure->set_fees_fix_broker(p1.get<double>("FeesFixBroker", 0));
				m_pFeesStructure->set_fees_sell_amount(p1.get<double>("FeesSellAmount", 0));
				*/
				m_pFeesStructure->load(p1);
			}
		}
		void instrumentclass::show()
		{
			loggerv2::info("instrumentclass::show enter");
			loggerv2::info("instrumentclass::show name:%s", m_strClassName.c_str());
			loggerv2::info("instrumentclass::show pointValue:%d", this->m_PointValue);
			if (this->m_pRefCurrency != nullptr)
			{
				loggerv2::info("instrumentclass::show currency.name:%s", m_pRefCurrency->to_string().c_str());
				loggerv2::info("instrumentclass::show currency.toRef:%f", m_pRefCurrency->get_to_reference());
			}
			if (this->m_ptickrule != nullptr)
			{
				loggerv2::info("instrumentclass::show tickrule:%s", m_ptickrule->get_name().c_str());
			}
			loggerv2::info("instrumentclass::show end");
		}
		string instrumentclass::get_currency_name()
		{
			if (m_pRefCurrency == nullptr)
				return "";
			return m_pRefCurrency->get_name();
		}

		currency* instrumentclass::get_currency()
		{
			return m_pRefCurrency;
		}
		void instrumentclass::save(boost::property_tree::ptree & pt)
		{
			pt.put("ClassName", get_name());
			pt.put("PointValue", this->get_point_value());

			boost::property_tree::ptree child_currency;
			child_currency.put("Name", this->get_currency_name());
			child_currency.put("ToReference", this->get_currency()->get_to_reference());
			pt.add_child("Currency", child_currency);

			boost::property_tree::ptree child_fees;
			/*
			child_fees.put("FeesFloatExchange", math2::round_ex(this->get_fees_float_exchange()));
			child_fees.put("FeesFloatBroker", math2::round_ex(this->get_fees_float_broker()));
			child_fees.put("FeesFixExchange", math2::round_ex(this->get_fees_fix_exchange()));
			child_fees.put("FeesFixBroker", math2::round_ex(this->get_fees_fix_broker()));
			child_fees.put("FeesSellAmount", math2::round_ex(this->get_fees_sell_amount()));
			*/
			if (this->m_pFeesStructure != nullptr)
			{
				m_pFeesStructure->save(child_fees);
			}
			pt.add_child("FeesStructure", child_fees);

			pt.put("ImplicitPreopen", this->m_bImplicitPreopen);
			pt.put("ImplicitPreopenTh", this->m_bImplicitPreopenTh);
		}
	}
}