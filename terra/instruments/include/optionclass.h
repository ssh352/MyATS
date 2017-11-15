#ifndef _OPTION_CLASS_H_
#define _OPTION_CLASS_H_
#pragma once
#include "derivclassbase.h"
namespace terra
{
	namespace instrument
	{
		class option;
		class optionclass : public derivclassbase
		{
		public:
			optionclass(underlying * pUnderlying, std::string & className, int pointValue, currency * pCurrency);
			virtual ~optionclass();
		public:
			void add(option * pOption);
		};
	}
}
#endif //_OPTION_CLASS_H_


