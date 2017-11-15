#include "atsdailydata.h"
#include "atsconfig.h"
#include "abstractats.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>  
#include "atsinstrument.h"
using namespace terra::common;
using namespace boost::property_tree;
using namespace boost::filesystem;
namespace terra
{
	namespace ats
	{
		ats_daily_data::ats_daily_data(abstract_ats * ats) :m_ats(ats)
		{
		}
		ats_daily_data::~ats_daily_data()
		{
		}
		bool ats_daily_data::load_daily()
		{
			if (m_ats == nullptr)
				return false;
			boost::filesystem::path p;
			p.clear();
			p.append(ats_config::get_instance()->get_daily_directory());
			p.append(m_ats->get_name() + ".json");
			string strFile = p.string();
			if (!boost::filesystem::exists(p))
			{
				loggerv2::error("ats_daily_data::load_daily file: %s not exist!", strFile.c_str());
				return false;
			}
			boost::property_tree::ptree root;
			boost::property_tree::read_json(strFile, root);
			ptree p1 = root.get_child("dailylist");
			for (auto & it : p1)
			{
				string code = it.second.get<string>("Code");
				int ManualPosition = it.second.get<int>("ManualPosition");
				string UseManualPosition = it.second.get<string>("UseManualPosition");
				int YstPositionType = it.second.get<int>("YstPositionType");				
				ats_instrument * pInstrument = m_ats->find(code);
				if (pInstrument != nullptr && pInstrument->get_position() != nullptr)
				{
					pInstrument->get_position()->set_yesterday_position_manual(ManualPosition);
					switch (YstPositionType)
					{
					case 0:
						pInstrument->get_position()->set_yesterday_position_type(YesterdayPositionType::Local);
						break;
					case 1:
						pInstrument->get_position()->set_yesterday_position_type(YesterdayPositionType::External);
						break;
					default:
						break;
					}
					if (UseManualPosition == "false")
					{
						pInstrument->get_position()->set_use_manual_position(false);
					}
					else
					{
						pInstrument->get_position()->set_use_manual_position(true);
					}
				}
			}
			return true;
		}
		bool ats_daily_data::save_daily()
		{
			if (m_ats == nullptr)
				return false;
			boost::filesystem::path p;
			p.clear();
			p.append(ats_config::get_instance()->get_daily_directory());
			p.append(m_ats->get_name() + ".json");
			string strFile = p.string();			
			boost::property_tree::ptree pt_root;
			boost::property_tree::ptree children;
			boost::property_tree::ptree child;
			for (auto & it : m_ats->AtsInstrumentList)
			{
				child.put("Code", it->get_instrument()->get_code());
				if (it->get_position() != nullptr)
				{
					child.put("ManualPosition", it->get_position()->get_yesterday_position_manual());
					child.put("UseManualPosition", it->get_position()->get_use_manual_position());
					child.put("YstPositionType", it->get_position()->get_yesterday_position_type());
				}
				else
				{
					child.put("ManualPosition", 0);
					child.put("UseManualPosition", true);
					child.put("YstPositionType",0);
				}
				children.push_back(std::make_pair("", child));
			}
			pt_root.add_child("dailylist",children);
			boost::property_tree::write_json(strFile, pt_root);
			return true;
		}
	}
}
