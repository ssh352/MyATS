#ifndef __ys_FEED_V2_H__
#define __ys_FEED_V2_H__
#include "es_source.h"
#include "TapQuoteAPI.h"
#include "TapAPIError.h"
using namespace terra::feedcommon;
namespace feed
{
	namespace es
	{
		class es_connection : public ITapQuoteAPINotify, public feed_connection
		{
		public:
			es_connection(es_source* pSource);
			virtual ~es_connection();

			virtual void init();
			virtual void cleanup();
			virtual void create(){}
			bool subscribe_item(feed_item* item) override;
			bool unsubscribe_item(feed_item* item) override;

			virtual void TAP_CDECL OnDisconnect(TAPIINT32 reasonCode);
			virtual void TAP_CDECL OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo *info);
			virtual void TAP_CDECL OnAPIReady();
			virtual void TAP_CDECL OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole *info);
			virtual void TAP_CDECL OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract *info);
			virtual void TAP_CDECL OnRtnQuote(const TapAPIQuoteWhole *info);

			virtual void TAP_CDECL OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo *info);
			virtual void TAP_CDECL OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo *info);
		private:
			string get_CommodityNo(string code);
			string get_ContractNo(string code);
			string get_Strike(string code);
		private:
			es_source*    m_pSource;
			ITapQuoteAPI* m_pUserApi;
			TAPIUINT32    m_nRequestId=0;
		};
	}
}
#endif // __ys_feed_V2_H__

