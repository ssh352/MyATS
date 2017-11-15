#ifndef _FEED_SOURCE_FACTORY_V2_H_
#define _FEED_SOURCE_FACTORY_V2_H_
#pragma once
#include "feedcommon.h"
#include "feedsource.h"
using namespace terra::feedcommon;
namespace terra
{
	namespace ats
	{
		class feed_source_factory
		{
		private:
			feed_source_factory(){}
			static feed_source_factory * g_FeedSourceFactory;
		public:
			static feed_source_factory * get_instance()
			{
				if (g_FeedSourceFactory == nullptr)
				{
					g_FeedSourceFactory = new feed_source_factory();
				}
				return g_FeedSourceFactory;
			}
		public:
			int initialize_feed_sources(string feedSourceFile, string dbFile);//[m]				
			feed_source * get_feed_source(string strSourceName);//[m]
		};
	}
}
#endif //_FEED_SOURCE_FACTORY_V2_H_


