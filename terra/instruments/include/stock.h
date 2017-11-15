#ifndef _STOCK_H_
#define _STOCK_H_
#pragma once
#include "financialinstrument.h"
namespace terra
{
	namespace instrument
	{
		class stock : public financialinstrument
		{
		public:
			stock(std::string & code);
			~stock();
		};
	}
}
#endif //_STOCK_H_


