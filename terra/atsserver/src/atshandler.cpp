#include "atshandler.h"
#include "atsmanager.h"
#include "feedsource.h"
#include "connection_gh.h"
#include "order_gh.h"
#include "portfolio_gh.h"
#include "tradeItem_gh.h"
#include "position.h"
#include "atsserver.h"
#include "atsinstrument.h"
#include "defaultdatetimepublisher.h"
#include "underlying.h"
#include "futureclass.h"
#include "stockclass.h"
#include "common.h"
#include "exec_persister.h"
#include "heartbeatcheck.h"
using namespace terra::ats;
using namespace terra::feedcommon;
using namespace terra::marketaccess::orderpassing;
using namespace terra::instrument;
namespace terra
{
	namespace atsserver
	{

		ats_handler::ats_handler()
		{
		}


		ats_handler::~ats_handler()
		{
		}

		void ats_handler::ForceSubScribe(const std::string& atsName, const std::string& instrumentCode)
		{
			printf_ex("ats_handler::ForceSubScribe atsName:%s\n", atsName.c_str());
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				auto instr = it->find(instrumentCode);
				if (instr != nullptr)
				{
					instr->stop_feed();
					instr->start_feed();
				}
			}
		}

		void ats_handler::StartAutomaton(const std::string& atsName)
		{
			if (ats_server::get_instance()->is_verify == false)
			{
				printf_ex("ats_handler::StartAutomaton fail,is_verify = false");
				return;
			}
			printf_ex("ats_handler::StartAutomaton atsName:%s\n", atsName.c_str());
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				loggerv2::error("ats_handler::StartAutomaton atsName:%s", atsName.c_str());
				it->start_automaton();
			}
		}

		void ats_handler::StopAutomaton(const std::string& atsName)
		{
			printf_ex("ats_handler::StopAutomaton atsName:%s\n", atsName.c_str());
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				loggerv2::error("ats_handler::StopAutomaton atsName:%s", atsName.c_str());
				it->stop_automaton(true);
			}
		}

		void ats_handler::StartAutomatonByList(const std::vector<std::string> & atsName)
		{
			if (ats_server::get_instance()->is_verify == false)
			{
				printf_ex("ats_handler::StartAutomaton fail,is_verify = false");
				return;
			}

			for (auto itr : atsName)
			{
				auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(itr);
				if (it != nullptr)
				{
					it->start_automaton();
				}
			}
		}

		void ats_handler::CaculateATS(const std::string& atsName)
		{
			printf_ex("ats_handler::CaculateATS atsName:%s\n", atsName.c_str());
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				it->do_pricing();
				it->compute_pnl();
			}
		}

		void ats_handler::Start(const std::string& atsName)
		{
			printf_ex("ats_handler::Start atsName:%s\n", atsName.c_str());
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				it->start();
				it->start_workflow();
			}
		}

		void ats_handler::Stop(const std::string& atsName)
		{
			printf_ex("ats_handler::Stop atsName:%s\n", atsName.c_str());
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				it->stop();
			}
		}

		void ats_handler::SaveConfig(const std::string& atsName)
		{
			printf_ex("ats_handler::SaveConfig atsName:%s\n", atsName.c_str());
			loggerv2::info("ats_handler::SaveConfig");
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				it->save_config();
			}
		}

		void ats_handler::SaveConfigDaily(const std::string& atsName)
		{
			printf_ex("ats_handler::SaveConfigDaily atsName:%s\n", atsName.c_str());
			loggerv2::info("ats_handler::SaveConfigDaily");
			auto it = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (it != nullptr)
			{
				it->save_config_daily();
			}
		}
		void ats_handler::StartPublish()
		{
			if (!atsserver::ats_server::get_instance()->is_verify)
				ats_server::get_instance()->dokill();
			printf_ex("ats_handler::StartPublish\n");
			ats_server::get_instance()->start_publish_work_flow();
		}

		int64_t ats_handler::HeartBeat()
		{
			try
			{
				heart_beat_check::LastHeartBeatTime = date_time_publisher_gh::get_instance()->now();
				printf_ex("ats_handler::HeartBeat %s \n", to_iso_extended_string(heart_beat_check::LastHeartBeatTime).c_str());
				loggerv2::info("ats_handler::HeartBeat ");

				for (auto &it : ats_server::get_instance()->AbstractATSMap)
				{
					cout << "ats_handler::HeartBeat,Ats " << it.second->get_name() << " is working" << endl;

					if (!ats_server::get_instance()->publish_work_flow.m_thread.joinable())
						loggerv2::error("ats_handler::HeartBeat,publish work flow has down");
				}


				return date_time_publisher_gh::get_instance()->get_time_of_day().total_milliseconds();
			}
			catch (exception& e)
			{
				cout << "Standard exception: " << e.what() << endl;
				loggerv2::error("HeartBeat::Standard exception:%s", e.what());
			}
			return date_time_publisher_gh::get_instance()->get_time_of_day().total_milliseconds();
		}

		void ats_handler::SetLocalTime(const int64_t ticks)
		{
			printf_ex("ats_handler::SetLocalTime\n");
			date_time_publisher_gh::get_instance()->set_local_date_time(ticks);
		}

		void ats_handler::CloseAll()
		{
			printf_ex("ats_handler::CloseAll\n");
			ats_server::get_instance()->stop_publish_work_flow();
			ats_manager::get_instance()->close_all();
		}

		void ats_handler::GetAllFeedSources(std::vector<FeedSourceMsg> & _return)
		{
			printf_ex("ats_handler::GetAllFeedSources\n");
			return;//abandon
		}
		void ats_handler::GetAllConnections(std::vector<ConnectionMsg> & _return)
		{
			printf_ex("ats_handler::GetAllConnections\n");
			return;//abandon
		}
		void ats_handler::FeedSourceRelease(const std::string& feedsourcename)
		{
			printf_ex("ats_handler::FeedSourceRelease\n");
			feed_source * pSource = feed_source_container::get_instance()->get_by_key(feedsourcename);
			if (pSource != nullptr)
			{
				if (pSource->get_status() == AtsType::FeedSourceStatus::Up)
				{
					pSource->release_source();
					pSource->update_state(AtsType::FeedSourceStatus::Down, "close feed by mannul");
				}
				else
				{
					pSource->re_init_source();
					pSource->update_state(AtsType::FeedSourceStatus::Up, "up feed by mannul");
				}
			}
		}
		bool ats_handler::SetFeedActived(const std::string& feedsourcename, const bool activated)
		{
			printf_ex("ats_handler::SetFeedActived\n");
			feed_source * pSource = feed_source_container::get_instance()->get_by_key(feedsourcename);
			if (pSource->get_status() == FeedSourceStatus::Down)
			{
				//pSource->init_source();
				pSource->re_init_source();
				pSource->update_state(AtsType::FeedSourceStatus::Up, "up feed by mannul");
			}

			return false;
		}
		bool ats_handler::SetConnectionTradingAllowed(const std::string& connectionname, const bool allowed)
		{
			printf_ex("ats_handler::SetConnectionTradingAllowed\n");
			connection* pConnection = connection_gh::get_instance().container().get_by_name(connectionname.c_str());
			if (pConnection != nullptr)
			{
				pConnection->setTradingAllowed(allowed);
				return pConnection->getTradingAllowed();
			}
			return false;
		}
		void ats_handler::ConnectionConnect(const std::string& connectionname, const bool toConnect)
		{
			printf_ex("ats_handler::ConnectionConnect %s", connectionname.c_str());
			connection* pConnection = connection_gh::get_instance().container().get_by_name(connectionname.c_str());
			if (pConnection != nullptr)
			{
				if (toConnect == true)
				{
					if (pConnection->getStatus() == ConnectionStatus::Connected)
					{
						return;
					}
					int maxTimeout = 180;//180 seconds					
					//
					pConnection->connect();
					//					
					if (pConnection->getRequestInstruments() == true)
					{
						while (pConnection->get_is_last() == false && maxTimeout > 0)
						{
							sleep_by_milliseconds(1000);
							maxTimeout--;
						}
						pConnection->setRequestInstruments(false);
						loggerv2::info("ats_handler::ConnectionConnect:feed begin reload the dico file,maxTimeout:%d!\n", maxTimeout);
						auto dbfile = referential::get_instance()->get_db_file();
						referential::get_instance()->load_option(dbfile);
						referential::get_instance()->load_future(dbfile);
						referential::get_instance()->load_stocks(dbfile);
						referential::get_instance()->load_etfs(dbfile);
						for (auto &it : *feed_source_container::get_instance())
						{
							it.second->load_database();
						}
						for (auto &it : connection_gh::get_instance().container().get_map())
						{
							connection* conn = it.second;
							conn->load_instruments(conn->getName(), conn->getConfigFile(), nullptr);
						}
						printf_ex("ats_handler::ConnectionConnect:feed end reload the dico file!\n");
					}
					else
					{
						printf_ex("ats_handler::ConnectionConnect didn't need reload the dico file!\n");
					}
					//
				}
				else
				{
					pConnection->disconnect();
				}
			}
		}
		bool ats_handler::CancelOrder(const int32_t id)
		{
			printf_ex("ats_handler::CancelOrder\n");
			order * pOrder = order_gh::get_instance().GetBook()->get_by_id(id);
			if (pOrder != nullptr && pOrder->get_binding_quote()==nullptr)
			{
				pOrder->Cancel();
			}
			return true;
		}
		void ats_handler::CancelAllOrder()
		{
			printf_ex("ats_handler::CancelAllOrder\n");
			order_gh::get_instance().GetBook()->killall();
		}
		int32_t ats_handler::SetYesterdayPositionLocal(const PositionMsg& positionMsg, const int32_t yesterdayPositionLocal)
		{
			printf_ex("ats_handler::SetYesterdayPositionLocal\n");
			portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(positionMsg.Portfolio.c_str());
			tradeitem* pTradeItem = tradeitem_gh::get_instance().container().get_by_key(positionMsg.Instrument, positionMsg.Connection);
			if (pPortfolio != nullptr)
			{
				position* pPosition = pPortfolio->get_position(pTradeItem);
				if (pPosition != nullptr)
				{
					pPosition->set_yesterday_position_local(yesterdayPositionLocal);
					return pPosition->get_yesterday_position_local();
				}
			}
			return 0;
		}

		double ats_handler::SetYesterdayPriceLocalLocal(const PositionMsg& positionMsg, const double yesterdayPriceLocal)
		{
			printf_ex("ats_handler::SetYesterdayPriceLocalLocal\n");
			portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(positionMsg.Portfolio.c_str());
			tradeitem* pTradeItem = tradeitem_gh::get_instance().container().get_by_key(positionMsg.Instrument, positionMsg.Connection);
			if (pPortfolio != nullptr)
			{
				position* pPosition = pPortfolio->get_position(pTradeItem);
				if (pPosition != nullptr)
				{
					pPosition->set_yesterday_price_local(yesterdayPriceLocal);
					return pPosition->get_yesterday_price_local();
				}
			}
			return 0.0;
		}

		int32_t ats_handler::SetYesterdayPositionManual(const PositionMsg& positionMsg, const int32_t yesterdayPositionManual)
		{
			printf_ex("ats_handler::SetYesterdayPositionManual\n");
			portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(positionMsg.Portfolio.c_str());
			tradeitem* pTradeItem = tradeitem_gh::get_instance().container().get_by_key(positionMsg.Instrument, positionMsg.Connection);
			if (pPortfolio != nullptr)
			{
				position* pPosition = pPortfolio->get_position(pTradeItem);
				if (pPosition != nullptr)
				{
					pPosition->set_yesterday_position_manual(yesterdayPositionManual);
					return pPosition->get_yesterday_position_manual();
				}
			}
			return 0;
		}

		int32_t ats_handler::SetYesterdayPositionExternal(const PositionMsg& positionMsg, const int32_t yesterdayPositionExternal)
		{
			printf_ex("ats_handler::SetYesterdayPositionExternal\n");
			portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(positionMsg.Portfolio.c_str());
			tradeitem* pTradeItem = tradeitem_gh::get_instance().container().get_by_key(positionMsg.Instrument, positionMsg.Connection);
			if (pPortfolio != nullptr)
			{
				position* pPosition = pPortfolio->get_position(pTradeItem);
				if (pPosition != nullptr)
				{
					pPosition->set_yesterday_position_external(yesterdayPositionExternal);
					return pPosition->get_yesterday_position_external();
				}
			}
			return 0;
		}

		bool ats_handler::SetUseManualPosition(const PositionMsg& positionMsg, const bool useManualPosition)
		{
			printf_ex("ats_handler::SetUseManualPosition\n");
			portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(positionMsg.Portfolio.c_str());
			tradeitem* pTradeItem = tradeitem_gh::get_instance().container().get_by_key(positionMsg.Instrument, positionMsg.Connection);
			if (pPortfolio != nullptr)
			{
				position* pPosition = pPortfolio->get_position(pTradeItem);
				if (pPosition != nullptr)
				{
					pPosition->set_use_manual_position(useManualPosition);
					return pPosition->get_use_manual_position();
				}
			}
			return false;
		}
		::AtsType::YesterdayPositionType::type ats_handler::SetYstPositionType(const PositionMsg& positionMsg, const  ::AtsType::YesterdayPositionType::type ystPositionType)
		{
			printf_ex("ats_handler::SetYstPositionType\n");
			portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(positionMsg.Portfolio.c_str());
			tradeitem* pTradeItem = tradeitem_gh::get_instance().container().get_by_key(positionMsg.Instrument, positionMsg.Connection);
			if (pPortfolio != nullptr)
			{
				position* pPosition = pPortfolio->get_position(pTradeItem);
				if (pPosition != nullptr)
				{
					pPosition->set_yesterday_position_type(ystPositionType);
					return pPosition->get_yesterday_position_type();
				}
			}
			return ::AtsType::YesterdayPositionType::Local;
		}

		::AtsType::YesterdayPositionType::type ats_handler::SetAtsYstPositionType(const AtsMsg& ats, const  ::AtsType::YesterdayPositionType::type ystPositionType)
		{
			printf_ex("ats_handler::SetAtsYstPositionType\n");
			abstract_ats * pAbstractATS = ats_server::get_instance()->AbstractATSMap.get_by_key(ats.Name);
			if (pAbstractATS != nullptr)
			{
				pAbstractATS->set_yesterday_position_type(ystPositionType);
				return pAbstractATS->get_yesterday_position_type();
			}
			return ::AtsType::YesterdayPositionType::Local;
		}

		bool ats_handler::CreateManualOrder(const double price, const  ::AtsType::OrderWay::type way, const int32_t quantity, const std::string& atsName, const std::string& atsInstrumentCode, const int32_t tradingtype, const  ::AtsType::OrderRestriction::type orderrestriction, const  ::AtsType::OrderOpenClose::type openclose, const  ::AtsType::OrderPriceMode::type priceMode)
		{
			printf_ex("ats_handler::CreateManualOrder\n");
			abstract_ats * pAbstractATS = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (pAbstractATS != nullptr)
			{
				ats_instrument * pInstrument = pAbstractATS->find(atsInstrumentCode);
				if (pInstrument != nullptr)
				{
					return pInstrument->create_order(price, way, quantity, pInstrument, tradingtype, orderrestriction, openclose, priceMode);
				}
			}
			return false;
		}

		void ats_handler::GetFeesStruct(FeesStructMsg& _return, const std::string& className)
		{
			printf_ex("ats_handler::GetFeesStruct\n");
			instrumentclass* pClass = referential::get_instance()->get_instrument_class_map().get_by_key(className);
			if (pClass != nullptr)
			{
				_return.FeesFixExchange = pClass->get_fees_fix_exchange();
				_return.FeesFixBroker = pClass->get_fees_fix_broker();
				_return.FeesFloatBroker = pClass->get_fees_float_broker();
				_return.FeesFloatExchange = pClass->get_fees_float_exchange();
				_return.FeesSellAmount = pClass->get_fees_sell_amount();
				_return.NotCloseToday = pClass->get_not_close_today();
			}
		}

		void ats_handler::UpdateFeesStruct(FeesStructMsg& _return, const FeesStructMsg& feesStruct)
		{
			printf_ex("ats_handler::UpdateFeesStruct\n");
			instrumentclass* pClass = referential::get_instance()->get_instrument_class_map().get_by_key(feesStruct.ClassName);
			if (pClass != nullptr)
			{
				pClass->set_fees_fix_broker(feesStruct.FeesFixBroker);
				pClass->set_fees_fix_exchange(feesStruct.FeesFixExchange);
				pClass->set_fees_float_broker(feesStruct.FeesFloatBroker);
				pClass->set_fees_float_exchange(feesStruct.FeesFloatExchange);
				pClass->set_fees_sell_amount(feesStruct.FeesSellAmount);
				pClass->set_not_close_today(feesStruct.NotCloseToday);

				_return.FeesFixExchange = pClass->get_fees_fix_exchange();
				_return.FeesFixBroker = pClass->get_fees_fix_broker();
				_return.FeesFloatBroker = pClass->get_fees_float_broker();
				_return.FeesFloatExchange = pClass->get_fees_float_exchange();
				_return.FeesSellAmount = pClass->get_fees_sell_amount();
				_return.NotCloseToday = pClass->get_not_close_today();
			}
		}

		double ats_handler::TickUpPrice(const AtsInstrumentMsg& instrument, const double price, const int32_t ticks)
		{
			printf_ex("ats_handler::TickUpPrice\n");
			financialinstrument* pInstr = referential::get_instance()->get_instrument_map().get_by_key(instrument.Code);
			double tempPrice = price;
			if (pInstr != nullptr)
			{
				tempPrice = pInstr->tick_up(price, ticks);
			}
			return tempPrice;
		}

		double ats_handler::TickDownPrice(const AtsInstrumentMsg& instrument, const double price, const int32_t ticks)
		{
			printf_ex("ats_handler::TickDownPrice\n");
			financialinstrument* pInstr = referential::get_instance()->get_instrument_map().get_by_key(instrument.Code);
			double tempPrice = price;
			if (pInstr != nullptr)
			{
				tempPrice = pInstr->tick_down(price, ticks);
			}
			return tempPrice;
		}

		void ats_handler::KillAll(const std::string& atsName, const  int32_t tradingtype)
		{
			printf_ex("ats_handler::KillAll\n");
			order_gh::get_instance().GetBook()->killall(atsName, tradingtype);
		}

		void ats_handler::GetTradingPeriodManager(TradingPeriodManagerMsg& _return, const std::string& atsName)
		{
			abstract_ats * pAbstractATS = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (pAbstractATS != nullptr)
			{
				_return.ShiftEodTime = to_simple_string(pAbstractATS->TradingPeriodManager.get_shift_eod_time());
				_return.ShiftPriceTime = to_simple_string(pAbstractATS->TradingPeriodManager.get_shift_price_time());
				_return.ShiftEodTimeN = to_simple_string(pAbstractATS->TradingPeriodManager.get_shift_eod_time_n());
				TradingPeriodMsg msg;
				for (auto & v : pAbstractATS->TradingPeriodManager.get_trading_period_list())
				{
					msg.AutoStopInterval = (*v).get_auto_stop_interval();
					msg.StartTime = to_simple_string((*v).get_start_time());
					msg.StopTime = to_simple_string((*v).get_stop_time());
					msg.Phase = (*v).get_trading_phase();
					_return.TradingPeriodList.push_back(msg);
				}
			}
			return;//abandon
		}

		void ats_handler::SetTradingPeriodManager(const std::string& atsName, const TradingPeriodManagerMsg& tradingperiodManager)
		{
			abstract_ats * pAbstractATS = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (pAbstractATS != nullptr)
			{
				pAbstractATS->TradingPeriodManager.clear();
				for (auto & v : tradingperiodManager.TradingPeriodList)
				{
					time_duration start(duration_from_string(v.StartTime));
					time_duration stop(duration_from_string(v.StopTime));

					AtsType::TradingPhase::type pha = v.Phase;
					pAbstractATS->TradingPeriodManager.add_trading_period(start, stop, pha, v.AutoStopInterval);
				}
				time_duration span(duration_from_string(tradingperiodManager.ShiftPriceTime));
				pAbstractATS->TradingPeriodManager.set_shift_price_time(span);

				time_duration span2(duration_from_string(tradingperiodManager.ShiftEodTime));
				pAbstractATS->TradingPeriodManager.set_shift_eod_time(span2);

				time_duration span3(duration_from_string(tradingperiodManager.ShiftEodTimeN));
				pAbstractATS->TradingPeriodManager.set_shift_eod_time_n(span3);

				//boost::filesystem::path p;
				//p.clear();
				//p.append(ats_config::get_instance()->get_referential_path());
				//string strFile = "tradingphase.json";
				//p.append(strFile);
				//strFile = p.string();
				pAbstractATS->save_config();
			}
			return;//abandon
		}

		void ats_handler::SaveReferential(const std::string& atsName, const TradingPeriodManagerMsg& tradingPeriodManager, const UnderlyingMsg& underlying, const InstrumentClassMsg& StockClass)
		{
			try
			{
				SetTradingPeriodManager(atsName, tradingPeriodManager);

				terra::instrument::underlying* pUnderlying = referential::get_instance()->get_underlying_map().get_by_key(underlying.Name);
				FeesStructMsg msg;
				if (pUnderlying != nullptr)
				{
					pUnderlying->set_rate(underlying.Rate);
					pUnderlying->get_days_off_list().clear();
					for (auto & v : underlying.DaysOffList)
					{
						/*	date_time_ex dateTime;
							dateTime.set_date(v);*/
						date dt(from_simple_string(v));
						pUnderlying->add_day_off(dt);
					}
					pUnderlying->compute_days_off();
					for (auto & v : underlying.FutureClassList)
					{
						futureclass* pFuture = pUnderlying->get_future_class_list().get_by_key(v.Name);
						if (pFuture != nullptr)
						{
							time_duration span(duration_from_string(v.MaturityTime));
							pFuture->set_maturity_time(span);
							this->UpdateFeesStruct(msg, v.FeesStruct);
						}
					}
					for (auto & v : underlying.OptionClassList)
					{
						optionclass* pOption = pUnderlying->get_option_class_list().get_by_key(v.Name);
						if (pOption != nullptr)
						{
							time_duration span(duration_from_string(v.MaturityTime));
							pOption->set_maturity_time(span);
							this->UpdateFeesStruct(msg, v.FeesStruct);
						}
					}
					referential::get_instance()->save_underlying(pUnderlying);
				}
				this->UpdateFeesStruct(msg, StockClass.FeesStruct);
				stockclass* pStock = referential::get_instance()->get_stock_class_map().get_by_key(StockClass.Name);
				if (pStock != nullptr)
				{
					referential::get_instance()->save_instrument_class(pStock);
				}
			}
			catch (std::exception &ex)
			{
				std::cout << "SaveReferential error:" << ex.what() << std::endl;
				loggerv2::error("SaveReferential error:%s", ex.what());
			}
			return;//abandon
		}

		bool ats_handler::CheckAesData(const AESDataMsg& msg)
		{
			bool res = ats_server::get_instance()->CheckAesMsg(msg);
			if (!res)
			{

				std::thread t1(std::bind(&ats_server::dokill, ats_server::get_instance()));
				t1.detach();
			}
			return res;
		}

		void ats_handler::SendExternalPosition(const std::vector<std::string> & positions)
		{
			//std::map<std::string, tradeitem *> tr_map;//生成一个tradeitem code和tradeitem*的map，当一个code对应多个connection时这个办法是错误的。
			//auto trade_map = tradeitem_gh::get_instance().container().get_map();
			//for (auto itr = trade_map.begin(); itr != trade_map.end(); ++itr)
			//{
			//	tr_map[itr->second->getCode()] = itr->second;
			//}
			auto pos_hashmap = portfolio_gh::get_instance().get_postion_external();

			for (auto it = positions.begin(); it != positions.end(); ++it)
			{
				vector<std::string>vec;
				boost::split(vec, *it, boost::is_any_of(","));

				if (vec.size() < 3)
					continue;
				if (0 == vec[0].size()*vec[1].size()*vec[2].size())
					continue;
				if (vec[0].at(0) == '#')
					continue;
				/*auto res = tr_map.find(vec[1]);
				if (res == tr_map.end())
				continue;

				portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(vec[0]);
				if (pPortfolio == NULL)
				{
				pPortfolio = new portfolio(vec[0].c_str());
				portfolio_gh::get_instance().container().add(pPortfolio);
				}

				tradeitem *i = res->second;

				position* pPosition = pPortfolio->get_position(i);
				if (pPosition == NULL)
				return;

				int pos = boost::lexical_cast<int>(vec[2]);

				pPosition->set_yesterday_position_local(pos);*/
				auto itr = pos_hashmap->find(vec[1]);
				if (itr == pos_hashmap->end())
				{
					std::unordered_map<string, string> hmap;
					hmap[vec[0]] = vec[2];
					(*pos_hashmap)[vec[1]] = hmap;
				}
				else
				{
					auto ite = itr->second.find(vec[0]);
					if (ite == itr->second.end())
					{
						itr->second.insert(std::make_pair(vec[0], vec[2]));
					}
					else
					{
						ite->second = vec[2];
					}
				}
			}
		}

		void ats_handler::RequestDeals(std::vector<std::string> & _return)
		{
			auto it = exec_persister::instance()->m_exec_vec.begin();
			for (; it != exec_persister::instance()->m_exec_vec.end(); ++it)
			{
				_return.push_back(*it);
			}
		}

		bool ats_handler::CreateManualQuote(const std::string& atsName, const std::string& atsInstrumentCode, const double bidprice, const int32_t bidquantity, const double askprice, const int32_t askquantity, const int32_t tradingtype, const ::AtsType::OrderOpenClose::type bidopenclose, const ::AtsType::OrderOpenClose::type askopenclose)
		{
			printf_ex("ats_handler::CreateManualQuote\n");
			abstract_ats * pAbstractATS = ats_server::get_instance()->AbstractATSMap.get_by_key(atsName);
			if (pAbstractATS != nullptr)
			{
				ats_instrument * pInstrument = pAbstractATS->find(atsInstrumentCode);
				if (pInstrument != nullptr)
				{
					return pInstrument->create_quote(bidprice, bidquantity, askprice, askquantity,pInstrument, tradingtype, bidopenclose, askopenclose);
				}
			}
			return false;
		}

		bool ats_handler::CancelQuote(const int32_t id)
		{

			printf_ex("ats_handler::CancelOrder\n");
			quote * pQuote = quote_gh::get_instance().GetBook()->get_by_id(id);
			if (pQuote != nullptr)
			{
				pQuote->Cancel();
			}
			return true;





		}

		void ats_handler::CancelAllQuote()
		{
			printf_ex("ats_handler::CancelAllOrder\n");
			quote_gh::get_instance().GetBook()->killall();
		}

		
	}
}
