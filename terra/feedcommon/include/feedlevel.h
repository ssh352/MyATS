#ifndef _V2_FEED_LEVEL_H_
#define _V2_FEED_LEVEL_H_
#pragma once
namespace terra
{
	namespace feedcommon
	{
		class feed_level
		{
		public:
			feed_level();
			~feed_level();
		public:
			int Qty;
			double Price;
		};
	}
}
#endif//_V2_FEED_LEVEL_H_


