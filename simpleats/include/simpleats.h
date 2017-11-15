#ifndef _SIMPLE_ATS_V2_H_
#define _SIMPLE_ATS_V2_H_
#pragma once
#include "abstractworkflowtask.h"
#include "abstractats.h"
#include "feedcommon.h"
#include "underlying.h"
#include "AtsType_types.h"
#include "order.h"
#include "atsinstrument.h"
using namespace terra::instrument;
using namespace terra::ats;
using namespace terra::marketaccess::orderpassing;
using namespace AtsType;
namespace simpleats
	{
		class SimpleTradingType :public AtsTradingType
		{
		public:
			static const int MaxTradingType = 4;
		};
	    class ia_future;
		class simple_ats : public abstract_ats
		{
		public:
			simple_ats(std::vector<std::string> & underlyingNames, const std::string& name, std::vector<std::string> &feedsources, std::vector<std::string> &connections, const string& stocks);
			virtual ~simple_ats();
		public:
			virtual void initialize_workflow();
			virtual void do_pricing();
			virtual void dump_intra_day();
			void kill_pending_orders();
			void kill_pending_orders(int tradingType);
			bool should_kill_pending_order(order * o);
			void kill_pending_orders_for_instrument(ats_instrument * instrument, int tradingType);
		public:
			map_ex<string, underlying*>  underlying_map;
			vector<ia_future*>           all_futures;
		};
		class simple_ats_factory
		{
		private:
			simple_ats_factory(){}
			~simple_ats_factory(){}
			static simple_ats_factory * g_pSimpleAtsFactory;
		public:
			static simple_ats_factory * get_instance()
			{
				if (g_pSimpleAtsFactory == nullptr)
				{
					g_pSimpleAtsFactory = new simple_ats_factory();
				}
				return g_pSimpleAtsFactory;
			}
		public:
			//map_ex<string, simple_ats*>  m_SimpleAtsMap;			
		public:
			//list<string>         get_split_arr(string str);
			//std::vector<string>  get_split_map(string str);
			simple_ats * create_automaton(const string& automatonName, const string& underlyingName, const string& feedsourcesStr, const string& connectionsStr, const std::string& stocks);
		};
	}
#endif //_SIMPLE_ATS_V2_H_


