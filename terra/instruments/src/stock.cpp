#include "stock.h"
#include "AtsType_types.h"
namespace terra
{
	namespace instrument
	{

		stock::stock(std::string & code) :financialinstrument(code)
		{
			set_type(AtsType::InstrType::Stock);
		}


		stock::~stock()
		{
		}
	}
}
