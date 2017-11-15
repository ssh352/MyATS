#ifndef _CURRENCY_H_
#define _CURRENCY_H_
#pragma once
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		/*
		refer to
		1.currency.json
		2.http://www.cppblog.com/wanghaiguang/archive/2013/12/26/205020.html
		3.D:\codes\Terra\Instrument\Referential.cs load_currencies
		4.json format
		[
		{
		"Name": "CNY",
		"ToReference": 1.0
		}
		]
		*/
		class currency
		{
		public:
			currency(std::string & name, double toReference)
				:m_strName(name), m_ToReference(toReference)
			{

			}
			~currency(){}
		public:
			std::string to_string()  { return m_strName; }
			string get_name(){ return m_strName; }
			double get_to_reference() { return m_ToReference; }
			void set_to_reference(double v){ m_ToReference = v; }
			void show()
			{
				loggerv2::info("currency::show enter");
				loggerv2::info("currency::show name:%s", m_strName.c_str());
				loggerv2::info("currency::show ToReference:%f", m_ToReference);
				loggerv2::info("currency::show end");
			}
		private:
			std::string  m_strName;
			double       m_ToReference;
		};
	}
}
#endif //_CURRENCY_H_


