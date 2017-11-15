#ifndef _V2_FEED_COMMON_H_
#define _V2_FEED_COMMON_H_
#include "common.h"
#include "feedevent.h"

using namespace terra::common;
namespace terra
{
	namespace feedcommon
	{
		class feed_item;
		class feed_source;
		typedef void(*decoder_callback)(string strSourceName, feed_item * feed_item);
		
		
		typedef std::function<void(feed_item * feed_item)> feed_update_event_handler;
		typedef std::function<void(feed_item * feed_item, feed_state_change_event_args & e)> feed_state_change_event_handler;
		typedef std::function<void(feed_item * feed_item, feed_field_update_event_args & e)> feed_field_update_event_handler;

		typedef std::function<void(feed_source * source)> feed_source_change_event_handler;
	}
}
#endif//_V2_FEED_COMMON_H_
