#ifndef ATS_DAILY_DATA_V2_H_
#define ATS_DAILY_DATA_V2_H_
#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "common.h"
#include "AtsType_types.h"
using namespace AtsType;
namespace terra
{
	namespace ats
	{
		class abstract_ats;
		class instrument_daily_data
		{
		public:
			string Code;
			int  ManualPosition;
			bool UseManualPosition;
			YesterdayPositionType YstPositionType;
		};
		class ats_daily_data
		{
		public:
			ats_daily_data(abstract_ats * ats);
			~ats_daily_data();
		protected:
			abstract_ats * m_ats;
		public:
			virtual bool load_daily();
			virtual bool save_daily();
		};
	}
}
#endif //ATS_DAILY_DATA_V2_H_


