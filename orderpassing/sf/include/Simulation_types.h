/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Simulation_TYPES_H
#define Simulation_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>
#include "AtsType_types.h"


namespace Simulation {

class FeedMsg;

class OrderRtnMsg;

class OrderTradeRtnMsg;

class OrderCancelRtnMsg;

typedef struct _FeedMsg__isset {
  _FeedMsg__isset() : Code(false), MarketTime(false), Last(false), BidDepths(false), AskDepths(false), BidQtys(false), AskQtys(false), DailyVolume(false), TurnOver(false), OpenInterest(false), Close(false), Settlement(false), UpperLimit(false), LowerLimit(false) {}
  bool Code :1;
  bool MarketTime :1;
  bool Last :1;
  bool BidDepths :1;
  bool AskDepths :1;
  bool BidQtys :1;
  bool AskQtys :1;
  bool DailyVolume :1;
  bool TurnOver :1;
  bool OpenInterest :1;
  bool Close :1;
  bool Settlement :1;
  bool UpperLimit :1;
  bool LowerLimit :1;
} _FeedMsg__isset;

class FeedMsg {
 public:

  FeedMsg(const FeedMsg&);
  FeedMsg& operator=(const FeedMsg&);
  FeedMsg() : Code(), MarketTime(), Last(0), DailyVolume(0), TurnOver(0), OpenInterest(0), Close(0), Settlement(0), UpperLimit(0), LowerLimit(0) {
  }

  virtual ~FeedMsg() throw();
  std::string Code;
  std::string MarketTime;
  double Last;
  std::vector<double>  BidDepths;
  std::vector<double>  AskDepths;
  std::vector<int32_t>  BidQtys;
  std::vector<int32_t>  AskQtys;
  int64_t DailyVolume;
  double TurnOver;
  int64_t OpenInterest;
  double Close;
  double Settlement;
  double UpperLimit;
  double LowerLimit;

  _FeedMsg__isset __isset;

  void __set_Code(const std::string& val);

  void __set_MarketTime(const std::string& val);

  void __set_Last(const double val);

  void __set_BidDepths(const std::vector<double> & val);

  void __set_AskDepths(const std::vector<double> & val);

  void __set_BidQtys(const std::vector<int32_t> & val);

  void __set_AskQtys(const std::vector<int32_t> & val);

  void __set_DailyVolume(const int64_t val);

  void __set_TurnOver(const double val);

  void __set_OpenInterest(const int64_t val);

  void __set_Close(const double val);

  void __set_Settlement(const double val);

  void __set_UpperLimit(const double val);

  void __set_LowerLimit(const double val);

  bool operator == (const FeedMsg & rhs) const
  {
    if (!(Code == rhs.Code))
      return false;
    if (!(MarketTime == rhs.MarketTime))
      return false;
    if (!(Last == rhs.Last))
      return false;
    if (!(BidDepths == rhs.BidDepths))
      return false;
    if (!(AskDepths == rhs.AskDepths))
      return false;
    if (!(BidQtys == rhs.BidQtys))
      return false;
    if (!(AskQtys == rhs.AskQtys))
      return false;
    if (!(DailyVolume == rhs.DailyVolume))
      return false;
    if (!(TurnOver == rhs.TurnOver))
      return false;
    if (!(OpenInterest == rhs.OpenInterest))
      return false;
    if (__isset.Close != rhs.__isset.Close)
      return false;
    else if (__isset.Close && !(Close == rhs.Close))
      return false;
    if (__isset.Settlement != rhs.__isset.Settlement)
      return false;
    else if (__isset.Settlement && !(Settlement == rhs.Settlement))
      return false;
    if (__isset.UpperLimit != rhs.__isset.UpperLimit)
      return false;
    else if (__isset.UpperLimit && !(UpperLimit == rhs.UpperLimit))
      return false;
    if (__isset.LowerLimit != rhs.__isset.LowerLimit)
      return false;
    else if (__isset.LowerLimit && !(LowerLimit == rhs.LowerLimit))
      return false;
    return true;
  }
  bool operator != (const FeedMsg &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FeedMsg & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(FeedMsg &a, FeedMsg &b);

inline std::ostream& operator<<(std::ostream& out, const FeedMsg& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _OrderRtnMsg__isset {
  _OrderRtnMsg__isset() : orderId(false), IsSuccess(false), reason(false) {}
  bool orderId :1;
  bool IsSuccess :1;
  bool reason :1;
} _OrderRtnMsg__isset;

class OrderRtnMsg {
 public:

  OrderRtnMsg(const OrderRtnMsg&);
  OrderRtnMsg& operator=(const OrderRtnMsg&);
  OrderRtnMsg() : orderId(0), IsSuccess(0), reason() {
  }

  virtual ~OrderRtnMsg() throw();
  int32_t orderId;
  bool IsSuccess;
  std::string reason;

  _OrderRtnMsg__isset __isset;

  void __set_orderId(const int32_t val);

  void __set_IsSuccess(const bool val);

  void __set_reason(const std::string& val);

  bool operator == (const OrderRtnMsg & rhs) const
  {
    if (!(orderId == rhs.orderId))
      return false;
    if (!(IsSuccess == rhs.IsSuccess))
      return false;
    if (!(reason == rhs.reason))
      return false;
    return true;
  }
  bool operator != (const OrderRtnMsg &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const OrderRtnMsg & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(OrderRtnMsg &a, OrderRtnMsg &b);

inline std::ostream& operator<<(std::ostream& out, const OrderRtnMsg& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _OrderTradeRtnMsg__isset {
  _OrderTradeRtnMsg__isset() : orderId(false), ExeQuantity(false), price(false), tradertime(false), MatchID(false) {}
  bool orderId :1;
  bool ExeQuantity :1;
  bool price :1;
  bool tradertime :1;
  bool MatchID :1;
} _OrderTradeRtnMsg__isset;

class OrderTradeRtnMsg {
 public:

  OrderTradeRtnMsg(const OrderTradeRtnMsg&);
  OrderTradeRtnMsg& operator=(const OrderTradeRtnMsg&);
  OrderTradeRtnMsg() : orderId(0), ExeQuantity(0), price(0), tradertime(), MatchID() {
  }

  virtual ~OrderTradeRtnMsg() throw();
  int32_t orderId;
  int32_t ExeQuantity;
  double price;
  std::string tradertime;
  std::string MatchID;

  _OrderTradeRtnMsg__isset __isset;

  void __set_orderId(const int32_t val);

  void __set_ExeQuantity(const int32_t val);

  void __set_price(const double val);

  void __set_tradertime(const std::string& val);

  void __set_MatchID(const std::string& val);

  bool operator == (const OrderTradeRtnMsg & rhs) const
  {
    if (!(orderId == rhs.orderId))
      return false;
    if (!(ExeQuantity == rhs.ExeQuantity))
      return false;
    if (!(price == rhs.price))
      return false;
    if (!(tradertime == rhs.tradertime))
      return false;
    if (!(MatchID == rhs.MatchID))
      return false;
    return true;
  }
  bool operator != (const OrderTradeRtnMsg &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const OrderTradeRtnMsg & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(OrderTradeRtnMsg &a, OrderTradeRtnMsg &b);

inline std::ostream& operator<<(std::ostream& out, const OrderTradeRtnMsg& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _OrderCancelRtnMsg__isset {
  _OrderCancelRtnMsg__isset() : orderId(false), IsSuccess(false), reason(false), CancelNum(false) {}
  bool orderId :1;
  bool IsSuccess :1;
  bool reason :1;
  bool CancelNum :1;
} _OrderCancelRtnMsg__isset;

class OrderCancelRtnMsg {
 public:

  OrderCancelRtnMsg(const OrderCancelRtnMsg&);
  OrderCancelRtnMsg& operator=(const OrderCancelRtnMsg&);
  OrderCancelRtnMsg() : orderId(0), IsSuccess(0), reason(), CancelNum(0) {
  }

  virtual ~OrderCancelRtnMsg() throw();
  int32_t orderId;
  bool IsSuccess;
  std::string reason;
  int32_t CancelNum;

  _OrderCancelRtnMsg__isset __isset;

  void __set_orderId(const int32_t val);

  void __set_IsSuccess(const bool val);

  void __set_reason(const std::string& val);

  void __set_CancelNum(const int32_t val);

  bool operator == (const OrderCancelRtnMsg & rhs) const
  {
    if (!(orderId == rhs.orderId))
      return false;
    if (!(IsSuccess == rhs.IsSuccess))
      return false;
    if (!(reason == rhs.reason))
      return false;
    if (!(CancelNum == rhs.CancelNum))
      return false;
    return true;
  }
  bool operator != (const OrderCancelRtnMsg &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const OrderCancelRtnMsg & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(OrderCancelRtnMsg &a, OrderCancelRtnMsg &b);

inline std::ostream& operator<<(std::ostream& out, const OrderCancelRtnMsg& obj)
{
  obj.printTo(out);
  return out;
}

} // namespace

#endif