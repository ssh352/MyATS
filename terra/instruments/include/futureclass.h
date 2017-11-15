#ifndef _FUTURE_CLASS_H_
#define _FUTURE_CLASS_H_
#pragma once
#include "derivclassbase.h"
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class future;
		class futureclass : public derivclassbase
		{
		public:
			futureclass(underlying * pUnderlying, std::string & className, int pointValue, currency * pCurrency);
			~futureclass();
		public:
			void add(future * pFuture);
		protected:
			map_ex<date, future*> m_refFutureContainer;
		public:
			map_ex<date, future*> & get_future_container(){ return m_refFutureContainer; }
		};
	}
}
#endif //_FUTURE_CLASS_H_
