#include "feedevent.h"
namespace terra
{
	namespace feedcommon
	{
		feed_field_update_event_args::feed_field_update_event_args(feed_field field, int depth, double value)
		{
			Field = field;
			Depth = depth;
			Value = value;
		}


		feed_field_update_event_args::~feed_field_update_event_args()
		{
		}
	}
}
