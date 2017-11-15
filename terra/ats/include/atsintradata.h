#ifndef _ATS_INTRA_DATA_H_
#define _ATS_INTRA_DATA_H_
#include "common.h"
#include <istream>

#include <functional>
using namespace terra::common;
#pragma once



namespace terra
{
	namespace ats
	{
		class atsintradata
		{
			typedef std::function<void(terra::ats::atsintradata*)> DataLoadEventHandler;
			typedef std::function<void(terra::ats::atsintradata*, ptime lastTs, map<string, double>& lastData)> DataUpdateEventHandler;

		public:
			atsintradata(){}

			atsintradata(std::string atsName, std::vector<std::string> &colnames)
			{
				m_strName = atsName;
				m_DataCols = colnames;
			}
			~atsintradata(){}
			string& get_name(){ return m_strName; }
			std::list<ptime> TimeSerie;						
			std::list<DataLoadEventHandler> load_event_handler;
			std::list<DataUpdateEventHandler> update_event_handler;
			std::unordered_map<std::string, std::list<double>> Datas;

			std::vector<std::string>& get_DataCols(){ return m_DataCols; }
		protected:
			std::string m_strName;
			std::vector<std::string> m_DataCols;			
		public:
			void load_data(std::string fileName);
			void on_update();
			void on_data_loaded();
		};
	}
}
#endif //_ATS_INTRA_DATA_H_


