#ifndef _ATS_HANDLER_V2_H_
#define _ATS_HANDLER_V2_H_
#pragma once
//#ifdef _WWW
//#include "SimpleAtsOperationHandler.h"
//using namespace SimpleMsg;
//#endif
#include "AtsOperation.h"
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using boost::shared_ptr;
//#ifdef _WWW
//using namespace  ::SimpleMsg;
//#endif
using namespace  ::AtsGeneral;
namespace terra
{
	namespace atsserver
	{
		class ats_handler : virtual public AtsOperationIf
		{
		public:
			ats_handler();
			virtual ~ats_handler();
		public:

			virtual void ForceSubScribe(const std::string& atsName, const std::string& instrumentCode);

			virtual void StartAutomaton(const std::string& atsName);

			virtual void StartAutomatonByList(const std::vector<std::string> & atsName);

			virtual void StopAutomaton(const std::string& atsNamme);

			virtual void CaculateATS(const std::string& atsName);

			virtual void Start(const std::string& atsName);

			virtual void Stop(const std::string& atsName);

			virtual void SaveConfig(const std::string& atsName);

			virtual void SaveConfigDaily(const std::string& atsName);

			virtual void StartPublish();

			virtual int64_t HeartBeat();

			virtual void SetLocalTime(const int64_t ticks);

			virtual void CloseAll();
			virtual void GetAllFeedSources(std::vector<FeedSourceMsg> & _return);
			virtual void GetAllConnections(std::vector<ConnectionMsg> & _return);
			virtual void FeedSourceRelease(const std::string& feedsourcename);
			virtual bool SetFeedActived(const std::string& feedsourcename, const bool activated);
			virtual bool SetConnectionTradingAllowed(const std::string& connectionname, const bool allowed);
			virtual void ConnectionConnect(const std::string& connectionname, const bool toConnect);
			virtual bool CancelOrder(const int32_t id);
			virtual void CancelAllOrder();
			virtual int32_t SetYesterdayPositionLocal(const PositionMsg& position, const int32_t yesterdayPositionLocal);

			virtual double SetYesterdayPriceLocalLocal(const PositionMsg& position, const double yesterdayPriceLocal);

			virtual int32_t SetYesterdayPositionManual(const PositionMsg& position, const int32_t yesterdayPositionManual);

			virtual int32_t SetYesterdayPositionExternal(const PositionMsg& position, const int32_t yesterdayPositionExternal);

			virtual bool SetUseManualPosition(const PositionMsg& position, const bool useManualPosition);
			virtual ::AtsType::YesterdayPositionType::type SetYstPositionType(const PositionMsg& position, const  ::AtsType::YesterdayPositionType::type ystPositionType);

			virtual ::AtsType::YesterdayPositionType::type SetAtsYstPositionType(const AtsMsg& ats, const  ::AtsType::YesterdayPositionType::type ystPositionType);

			virtual bool CreateManualOrder(const double price, const  ::AtsType::OrderWay::type way, const int32_t quantity, const std::string& atsName, const std::string& atsInstrumentCode, const  int32_t tradingtype, const  ::AtsType::OrderRestriction::type orderrestriction, const  ::AtsType::OrderOpenClose::type openclose, const  ::AtsType::OrderPriceMode::type priceMode);

			virtual void GetFeesStruct(FeesStructMsg& _return, const std::string& className);

			virtual void UpdateFeesStruct(FeesStructMsg& _return, const FeesStructMsg& feesStruct);

			virtual double TickUpPrice(const AtsInstrumentMsg& instrument, const double price, const int32_t ticks);

			virtual double TickDownPrice(const AtsInstrumentMsg& instrument, const double price, const int32_t ticks);

			virtual void KillAll(const std::string& atsName, const int32_t  tradingtype);

			virtual void GetTradingPeriodManager(TradingPeriodManagerMsg& _return, const std::string& atsName);

			virtual void SetTradingPeriodManager(const std::string& atsName, const TradingPeriodManagerMsg& tradingperiodManager);

			virtual void SaveReferential(const std::string& atsName, const TradingPeriodManagerMsg& tradingPeriodManager, const UnderlyingMsg& underlying, const InstrumentClassMsg& StockClass);

			virtual bool CheckAesData(const AESDataMsg& msg);

			virtual void SendExternalPosition(const std::vector<std::string> & positions);
			virtual void RequestDeals(std::vector<std::string> & _return);

			virtual bool CreateManualQuote(const std::string& atsName, const std::string& atsInstrumentCode, const double bidprice, const int32_t bidquantity, const double askprice, const int32_t askquantity, const int32_t tradingtype, const ::AtsType::OrderOpenClose::type bidopenclose, const ::AtsType::OrderOpenClose::type askopenclose);

			virtual bool CancelQuote(const int32_t id);

			virtual void CancelAllQuote();

		};
	}
}
#endif //_ATS_HANDLER_V2_H_


