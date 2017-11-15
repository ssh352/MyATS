#ifndef _TDF_CONNECTION_H_
#define _TDF_CONNECTION_H_
#include "tdf_source.h"
extern "C" {
#include "TDFAPI.h"
}
//#include "TDFAPI.h"
namespace feed
{
	namespace tdf
	{
		class tdf_connection :public feed_connection
		{

		public:
			tdf_connection(tdf_source* pSource, unsigned int date, unsigned int time);
			~tdf_connection();
			void init() override;
			void cleanup() override
			{
				if (g_hTDF)
				{
					TDF_Close(g_hTDF);
				}
			}
			void create() override{};
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;
			static tdf_source* instance_source;
			vector<string> subscribe_vec;
		private:
			THANDLE g_hTDF;
		
			tdf_source* m_pSource;
			static void OnDataMsg(THANDLE hTdf, TDF_MSG* pMsgHead);
			static void OnSysMsg(THANDLE hTdf, TDF_MSG* pSysMsg);
			unsigned int m_date;
			unsigned int m_time;
			
		};


	
	}
}


#endif
