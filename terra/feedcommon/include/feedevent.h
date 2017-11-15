#ifndef _FEED_EVENT_H_
#define _FEED_EVENT_H_
#include "feedenum.h"
#pragma once
namespace terra
{
	namespace feedcommon
	{
		/*
		disuse
		*/
		class feed_field_update_event_args
		{
		public:
			feed_field_update_event_args(){}
			feed_field_update_event_args(feed_field field, int depth, double value);
			~feed_field_update_event_args();
		public:
			feed_field Field;
			int Depth;
			double Value;
		};
		class feed_state_change_event_args
		{
		public:
			feed_state_change_event_args(){}
			feed_state_change_event_args(bool up){ Up = up; }
			~feed_state_change_event_args(){}
		public:
			bool Up;
		};
	}
}
#endif//_FEED_EVENT_H_

