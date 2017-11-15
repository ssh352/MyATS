#ifndef _FUTURE_H_
#define _FUTURE_H_
#pragma once
#include "abstractderivative.h"
#include "futureclass.h"
namespace terra
{
	namespace instrument
	{
		class future : public abstractderivative
		{
		public:
			future(std::string & code);
			~future();
		
		public:
			double Actu;

		public:
		
			void compute_actu();
			
			double actu_inv()
			{
				return math2::not_zero(Actu) ? 1 / Actu : 0;
			}	
			futureclass * get_future_class()
			{
				return (futureclass *)get_class();
			}	
			//virtual void on_maturity_time_changed();
		};
	}
}
#endif //_FUTURE_H_


