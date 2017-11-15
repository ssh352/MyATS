#ifndef __ORDER_DATA_DEF2_H__
#define __ORDER_DATA_DEF2_H__


namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			enum OrderHedge
			{
				Undef = 0,
				Hedge,
				Speculation,
				Spread,
				//H_MAX
			};

			enum OrderValidity
			{
				Active,
				Done
			};

			enum ResynchronizationMode
			{
				None,
				Last,
				Full
			};
			
		}
	}
}


#endif