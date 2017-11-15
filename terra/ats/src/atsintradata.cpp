#include "atsintradata.h"
#include "boost/algorithm/string.hpp"
namespace terra
{
	namespace ats
	{
		void atsintradata::load_data(string fileName)
		{
			std::ifstream f1(fileName.data(), std::ios::in);
			if (f1.is_open())
			{
				std::string buf;
				while (getline(f1, buf))
				{
					if (buf.empty() || boost::starts_with(buf, "#"))
						continue;
					std::vector<std::string> exChgs;
					boost::split(exChgs, buf, boost::is_any_of(","));
					try
					{
						if (exChgs.size() >= 2)
						{
							/*	date_time date;
								date.set_date(exChgs[0], "FN2");*/
							TimeSerie.push_back(from_iso_extended_string(exChgs[0]));
							for (unsigned int i = 1; i < Datas.size() + 1; i++)
							{
								Datas[m_DataCols[i - 1]].push_back(boost::lexical_cast<double>(exChgs[i]));
							}
						}
					}
					catch (std::exception &ex)
					{

					}
				}
				f1.close();
			}
			on_data_loaded();
		}
		void atsintradata::on_update()
		{
			if (update_event_handler.size() > 0)
			{
				map<string, double> lastData;
				for (auto & it : Datas)
				{
					lastData.emplace(it.first, it.second.back());
				}
				//update_event_handler(this, this->TimeSerie.back(), lastData);
				for (auto &it : update_event_handler)
				{
					it(this, this->TimeSerie.back(), lastData);
				}

			}
		}
		void atsintradata::on_data_loaded()
		{
			if (load_event_handler.size() > 0)
			{
				for (auto &it : load_event_handler)
				{
					it(this);
				}
				//load_event_handler(this);
			}
		}
	}
}