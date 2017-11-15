#include "forex.h"
namespace terra
{
	namespace instrument
	{

		forex::forex(std::string & code) :financialinstrument(code)
		{
			set_type(AtsType::InstrType::Forex);
		}


		forex::~forex()
		{
		}
	}
}