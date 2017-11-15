#include "simpleats.h"
#include "pricingtask.h"
#include "phasetask.h"
#include "atsmanager.h"
#include "instrumentcommon.h"
#include "underlying.h"
#include "iaoption.h"
#include "iafuture.h"
#include "maturity.h"
#include "strikeparity.h"
#include "futureclass.h"
#include "future.h"
#include "dumpintradaytask.h"
#include "pnltask.h"
#include "kill_pending_orders_task.h"
#include <stockclass.h>
#include "stock.h"
//
#include <forexclass.h>
#include "forex.h"
//
namespace simpleats
	{
		simple_ats_factory* simple_ats_factory::g_pSimpleAtsFactory = nullptr;

		simple_ats::simple_ats(std::vector<std::string> & underlyingNames, const std::string& name, std::vector<std::string> &feedsources, std::vector<std::string> &connections, const string& stocks) :abstract_ats(name, feedsources, connections)
		{			
			for (auto & itUnderlying : underlyingNames)
			{
				string & strUnderlying = itUnderlying;
				underlying* pUnderlying = referential::get_instance()->get_underlying_map().get_by_key(strUnderlying);
				if (pUnderlying != nullptr)
				{
					this->underlying_map.add(strUnderlying, pUnderlying);
					//1.option
					map_ex<string, optionclass*> & tmpOptionClassMap = pUnderlying->get_option_class_list();										
					for (auto & itOptionClass : tmpOptionClassMap)
					{
						optionclass * pOptionClass = itOptionClass.second;
						loggerv2::info("simple_ats::simple_ats underlying:%s, optionClass:%s", pUnderlying->get_name().c_str(), pOptionClass->get_name().c_str());
						if (pOptionClass != nullptr)
						{
							map_ex<string, maturity*>::iterator itMaturity = pOptionClass->get_maturity_map().begin();
							for (auto & itMaturity : pOptionClass->get_maturity_map())
							{
								maturity * pMaturity = itMaturity.second;								
								for (auto & itStrike : pMaturity->get_strike_parity_list())
								{
									strikeparity * pStrikeParity = (itStrike.second);
									if (pStrikeParity->get_call() != nullptr)
									{
										ia_option * pCall = new ia_option(pStrikeParity->get_call(), name, feedsources, connections, SimpleTradingType::MaxTradingType);
										this->AtsInstrumentList.push_back(pCall);
									}
									if (pStrikeParity->get_put() != nullptr)
									{
										ia_option * pPut = new ia_option(pStrikeParity->get_put(), name, feedsources, connections, SimpleTradingType::MaxTradingType);
										this->AtsInstrumentList.push_back(pPut);
									}									
								}								
							}
						}						
					}
					//2.future					
					for (auto & it_future_class : pUnderlying->get_future_class_list())
					{
						futureclass * pFutureclass = it_future_class.second;						
						for (auto & it_future : pFutureclass->get_future_container())
						{
							future * pFuture = it_future.second;
							ia_future * iafuture = new ia_future(pFuture, name, feedsources, connections, SimpleTradingType::MaxTradingType);
							this->AtsInstrumentList.push_back(iafuture);	
							this->all_futures.push_back(iafuture);
						}												
					}

					etf *petf = pUnderlying->get_etf();
					if (petf != nullptr)
					{
						ia_future * iafuture = new ia_future(petf, name, feedsources, connections, SimpleTradingType::MaxTradingType);
						this->AtsInstrumentList.push_back(iafuture);
						this->all_futures.push_back(iafuture);
					}
				}				
			}

			stockclass* pStocks = referential::get_instance()->get_stock_class_map().get_by_key(stocks);
			if (pStocks != nullptr)
			{
				for (auto& it : pStocks->get_stock_instrument_map())
				{
					stock * pSotck = it.second;
					ia_future * iafuture = new ia_future(pSotck, name, feedsources, connections, SimpleTradingType::MaxTradingType);
					this->AtsInstrumentList.push_back(iafuture);
					this->all_futures.push_back(iafuture);
				}
			}
			
			//
			forexclass* pForexClass = referential::get_instance()->get_forex_class_map().get_by_key(stocks);
			if (pForexClass != nullptr)
			{
				for (auto& it : pForexClass->get_forex_instrument_map())
				{
					forex * pForex = it.second;
					ia_future * iafuture = new ia_future(pForex, name, feedsources, connections, SimpleTradingType::MaxTradingType);
					this->AtsInstrumentList.push_back(iafuture);
					this->all_futures.push_back(iafuture);
				}
			}
			//
			
			on_name_changed();

			std::vector<string> v;
			v.push_back("PnlYesterday");
			v.push_back("PnlToday");
			v.push_back("PnlTotal");			
			m_intra_data = new atsintradata(name, v);
			abstract_ats::load_config();
			abstract_ats::load_config_daily();
		}
		simple_ats::~simple_ats()
		{
			
		}
		void simple_ats::initialize_workflow()
		{
			/*_atsWorkflow.FastWorkflow.register_workflow(new pricing_task(this));
			_atsWorkflow.FastWorkflow.register_workflow(new kill_pending_orders_task(this));

			_atsWorkflow.SlowWorkflow.register_workflow(new phase_task(this));
			_atsWorkflow.SlowWorkflow.register_workflow(new dump_intraday_task(this));
			_atsWorkflow.SlowWorkflow.register_workflow(new pnl_task(this));*/
			
			ats_work_flow.register_workflow(new pricing_task(this));
			ats_work_flow.register_workflow(new kill_pending_orders_task(this));

			ats_work_flow.register_workflow(new phase_task(this));
			ats_work_flow.register_workflow(new dump_intraday_task(this));
			ats_work_flow.register_workflow(new pnl_task(this));
		}
		void simple_ats::do_pricing()
		{
			abstract_ats::do_pricing();			
			for (auto & v : this->all_futures)
			{
				//((future *)v->get_instrument())->compute_actu();
			}
		}
		void simple_ats::dump_intra_day()
		{
			
			if (m_intra_data != nullptr)
			{
				m_intra_data->TimeSerie.push_back(date_time_publisher_gh::get_instance()->now());
				double PnlYesterday = 0;
				double PnlToday     = 0;
				double PnlTotal     = 0;
				if (this->get_auto_status() == true)
				{
					PnlYesterday = get_yesterday_pnl_bary();
					PnlToday     = get_today_pnl_bary();
					PnlTotal     = get_yesterday_pnl_bary() + get_today_pnl_bary() - (get_fees_exchange() + get_fees_broker());
				}				
				m_intra_data->Datas["PnlYesterday"].push_back(PnlYesterday);
				m_intra_data->Datas["PnlToday"].push_back(PnlToday);
				m_intra_data->Datas["PnlTotal"].push_back(PnlTotal);

				m_intra_data->on_update();

				string line = to_iso_extended_string(date_time_publisher_gh::get_instance()->now()) + "," + math2::round_ex(PnlYesterday, 2) + "," + math2::round_ex(PnlToday, 2) + "," + math2::round_ex(PnlTotal, 2);
				//this->m_stream_writer.write(line);
			}
		}
		void simple_ats::kill_pending_orders()
		{
			if (this->get_auto_status() == false)
			{
				for (int i = 0; i < SimpleTradingType::MaxTradingType; i++)
				{
					if (i == AtsTradingType::Hitter)
						continue;
					kill_pending_orders(i);
				}
			}
		}
		void simple_ats::kill_pending_orders(int tradingType)
		{
			for (auto & it : this->AtsInstrumentList)
			{
				kill_pending_orders_for_instrument(it, tradingType);
			}
		}
		bool simple_ats::should_kill_pending_order(order * o)
		{
			if (o == nullptr)
				return false;
			if (o->get_status() != OrderStatus::Ack && o->get_status() != OrderStatus::Nack)
				return false;
			switch (o->get_trading_type())
			{
			case AtsTradingType::Manual:
				return false;
			case AtsTradingType::Hitter:
				return (o->get_restriction() == OrderRestriction::None);			
			default:
				break;
			}
			return true;
		}
		void simple_ats::kill_pending_orders_for_instrument(ats_instrument * instrument, int tradingType)
		{
			if (instrument == nullptr)
				return;
			if (tradingType == AtsTradingType::Contrib)
			{

			}
			else
			{
				order_map* m = instrument->get_order_map_array()[tradingType];
				if (m != nullptr)
				{
					for (auto & it : *m)
					{
						if (should_kill_pending_order(it.second))
						{
							it.second->Cancel(); 
						}
					}
				}
			}
		}
		//list<string> simple_ats_factory::get_split_arr(string str)
		//{
		//	list<string> arr;
		//	std::vector<std::string> strs;
		//	boost::split(strs, str, boost::is_any_of("|"));
		//	for (std::string& fullcode : strs)
		//	{
		//		arr.push_back(fullcode);
		//	}
		//	return arr;
		//}		
		//std::vector<string> simple_ats_factory::get_split_map(string str)
		//{
		//	std::vector<string> vect;
		//	std::vector<std::string> strs;
		//	boost::split(strs, str, boost::is_any_of("|"));
		//	for (std::string& fullcode : strs)
		//	{				
		//		vect.push_back(fullcode);
		//	}
		//	return vect;
		//}
		simple_ats * simple_ats_factory::create_automaton(const string& automatonName, const string& underlyingName, const string& feedsourcesStr, const string& connectionsStr, const std::string& stocks)
		{
			vector<string> un;
			boost::split(un, underlyingName, boost::is_any_of("|"));
			vector<string> feed;
			boost::split(feed, feedsourcesStr, boost::is_any_of("|"));
			vector<string> con;
			boost::split(con, connectionsStr, boost::is_any_of("|"));
			simple_ats * pAts = new simple_ats(un, automatonName, feed, con,stocks);
			//if (m_SimpleAtsMap.contain_key(automatonName) == false)
			//{
			//	m_SimpleAtsMap.add(automatonName, pAts);
			//}
			return pAts;
		}
	}