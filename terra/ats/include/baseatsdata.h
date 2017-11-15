#ifndef _BASE_ATS_DATA_H_
#define _BASE_ATS_DATA_H_
#pragma once
#include "common.h"
namespace terra
{
	namespace ats
	{
		class abstract_ats;
	    class instrument_data
		{
		public:
			string code;
			int YesterdayPosLocal;
			double YesterdayPrcLocal;
		};
		class base_ats_data
		{
		public:
			base_ats_data(abstract_ats * pAts);
			~base_ats_data();
		protected:
			abstract_ats * m_ats;
		public:
			virtual void save(std::string atsDirectory);
			virtual void load(std::string atsDirectory);

			void save_trading_periods(std::string atsDirectory);
			void load_trading_periods(std::string atsDirectory);
			void load_yst_position();
		};
	}
}
#endif//_BASE_ATS_DATA_H_
