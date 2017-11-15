#ifndef _FUTURE_SPREAD_H_
#define _FUTURE_SPREAD_H_
#pragma once
#include "abstractderivative.h"
namespace terra
{
	namespace instrument
	{
		class futurespread :public abstractderivative
		{
		public:
			futurespread(std::string & code);
			~futurespread();
		public:
			virtual void on_maturity_time_changed();
		};
	}
}
#endif //_FUTURE_SPREAD_H_


