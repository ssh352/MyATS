#ifndef __FEMAS_SOURCE_V2_H__
#define __FEMAS_SOURCE_V2_H__

//#include "femas_decoder.h"
#include "USTPFtdcUserApiDataType.h"
#include "feedsource.h"
struct CUstpFtdcDepthMarketDataField;
using namespace terra::common;
namespace feed
{
	namespace femas
	{
		typedef terra::common::lockfree_classpool_workqueue<CUstpFtdcDepthMarketDataField> outbound_queue;

		class femas_source : public terra::feedcommon::feed_source
		{
		public:
			femas_source(const string & sourceName, const string & hostname, const string & service, const string & brokerId, const string & user, const string & password, const string & db, string pub = "", string url = "", string req_url = "");
			//virtual ~femas_source();
		public:
			virtual void init_source();
			//virtual void release_source();
			void process_msg(CUstpFtdcDepthMarketDataField* pMsg);
			inline outbound_queue* get_queue() { return &m_queue; }
			//
			void start_receiver();
			//
		protected:
			void process() override;
			//virtual int  process_out_bound_msg_handler();
			void update_item(CUstpFtdcDepthMarketDataField* pMsg, terra::feedcommon::feed_item * feed_item);
		protected:
			//femas_decoder m_decoder;
			outbound_queue m_queue;
#ifdef Linux
			int efd;
			void  init_epoll_eventfd();
#endif
		};
	}
}
#endif //__FEMAS_SOURCE_V2_H__

