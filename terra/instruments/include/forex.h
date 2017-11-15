#ifndef _FOREX_H_
#define _FOREX_H_
#include "financialinstrument.h"
namespace terra
{
	namespace instrument
	{
		class forex :
			public financialinstrument
		{
		public:
			forex(std::string & code);
			~forex();
		};

	}
}
#endif