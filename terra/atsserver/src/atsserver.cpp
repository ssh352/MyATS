#include "atsserver.h"
#include "connection.h"
#include "atsinstrument.h"
#include "derivclassbase.h"
#include "atsmanager.h"
#include "feedsource.h"
#include "atsconfig.h"
#include "connection_gh.h"
#include "AtsType_types.h"
#include "time_order_container.h"
#include "price_order_container.h"
#include "urlDEcode.h"

#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/secblock.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

using namespace terra::ats;
using namespace terra::feedcommon;
using namespace terra::marketaccess::orderpassing;
using namespace AtsType;
using namespace CryptoPP;
namespace terra
{
	namespace atsserver
	{
		//unsigned char key[] = { 0x08, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };//AES::DEFAULT_KEYLENGTH  
		unsigned char key[] = "lingwang_test12_lingwang_test12";
		unsigned char iv[] = "lingwang_test12";
		//unsigned char iv[] = { 0x08, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x02, 0x03, 0x03, 0x03, 0x03 };
		ats_server* ats_server::g_AtsServer = nullptr;

		ats_server::ats_server()
		{
			is_verify = true;

		}
		ats_server::~ats_server()
		{
		}
		void ats_server::dokill()
		{
			int i = 5;
			for (; i > 0; --i)
			{
				cout << "verify fail,pleas contact lingwang,srv wiil be killed after " << i << " sec" << endl;
				sleep_by_milliseconds(1000);
			}

			exit(1);
		}
		void ats_server::init(pub_ats_event_handler pub)
		{
			task = new publish_task();
			task->handler = pub;
			publish_work_flow.register_workflow(task);
			publish_work_flow.set_sleep_timer(ats_config::get_instance()->get_gui_timer());
			publish_work_flow.set_name("zmq_publish_thread");


			kill_time = ats_config::get_instance()->get_shutdown_time();
			m_enable_stop = ats_config::get_instance()->get_enable_auto_stop();
		}
		void ats_server::start_publish_work_flow()
		{
			this->publish_work_flow.run();
		}
		void ats_server::stop_publish_work_flow()
		{
			this->publish_work_flow.stop();
		}
		AtsMsg ats_server::create_ats_msg(abstract_ats * ats)
		{
			AtsMsg msg;
			if (ats == nullptr)
				return msg;
			msg.Name = ats->get_name();
			msg.__set_YesterdayPositionType(ats->get_yesterday_position_type());
			ats->get_intra_data()->update_event_handler.push_back(boost::bind(&publish_task::intradata_update_event, task, _1, _2, _3));
			return msg;
		}
		AtsInstrumentMsg ats_server::create_ats_instrument_msg(ats_instrument * instrument, bool has_depth)
		{
			AtsInstrumentMsg msg;
			if (instrument == nullptr || instrument->get_instrument() == nullptr)
			{
				loggerv2::error("ats_server::create_ats_instrument_msg instrument name is null!\n");
				printf_ex("ats_server::create_ats_instrument_msg instrument name is null!\n");
				return msg;
			}

			msg.Position = create_position_msg(instrument->get_position());

			msg.Code = instrument->get_instrument()->get_code();
			msg.FeedItem = create_feed_msg(instrument, has_depth);
			for (int i = 0; i < instrument->max_trading_type; i++)
			{
				std::map<int32_t, OrderMsg> map;
				msg.OrderContainer.insert(make_pair(i, map));
			}
			msg.SizeToSend = 0;
			if (instrument->get_instrument()->get_class() != nullptr)
				msg.InstrumentClassName = instrument->get_instrument()->get_class()->get_name();
			return msg;
		}
		PositionMsg ats_server::create_position_msg(position * pPosition)
		{
			PositionMsg msg;
			if (pPosition == nullptr)
				return msg;
			msg.Instrument = pPosition->get_instrument()->getCode();
			msg.Connection = pPosition->get_instrument()->getConnection()->getName();
			msg.Portfolio = pPosition->get_portfolio_name();
			msg.UseManualPosition = pPosition->get_use_manual_position();
			msg.__set_YstPositionType(pPosition->get_yesterday_position_type());
			msg.__set_YesterdayPrice(pPosition->get_yesterday_price());
			msg.__set_YesterdayPriceLocal(pPosition->get_yesterday_price());

			msg.__set_YesterdayPriceExternal(pPosition->get_yesterday_price_external());
			msg.__set_YesterdayPositionLocal(pPosition->get_yesterday_position_local());
			msg.__set_YesterdayPositionManual(pPosition->get_yesterday_position_manual());
			msg.__set_YesterdayPositionExternal(pPosition->get_yesterday_position_external());
			msg.__set_YesterdayPosition(pPosition->get_yesterday_position());
			msg.__set_TodayRedPosition(pPosition->m_nTodayRedPosition);
			msg.__set_TodayPurPosition(pPosition->m_TodayPurPosition);

			return msg;
		}
		FeedMsg ats_server::create_feed_msg(ats_instrument * instrument, bool has_depth)
		{
			FeedMsg msg;
			msg.__isset.MaxDepth = true;
			msg.__isset.BidDepths = true;
			msg.__isset.AskDepths = true;
			msg.__isset.BidQtys = true;
			msg.__isset.AskQtys = true;
			//msg.MaxDepth = feed_limits::MAXDEPTH;
			if (has_depth)
				msg.MaxDepth = feed_limits::MAXDEPTH;
			else
				msg.MaxDepth = 0;

			msg.BidDepths.resize(msg.MaxDepth);
			msg.AskDepths.resize(msg.MaxDepth);
			msg.BidQtys.resize(msg.MaxDepth);
			msg.AskQtys.resize(msg.MaxDepth);
			if (instrument == nullptr || instrument->get_instrument() == nullptr)
			{
				loggerv2::error("ats_server::create_feed_msg instrument name is null!\n");
				printf_ex("ats_server::create_feed_msg instrument name is null!\n");
				return msg;
			}
			msg.__isset.Code = true;
			msg.__set_Code(instrument->get_instrument()->get_code());
			return msg;
		}
		UnderlyingMsg ats_server::create_underlying_msg(underlying * underlying)
		{
			UnderlyingMsg msg;
			if (underlying == nullptr)
			{
				return msg;
			}
			msg.Name = underlying->get_name();
			msg.Rate = underlying->get_rate();

			for (auto &it : underlying->get_days_off_list())
			{
				//date_time_ex & datetime = it.second;
				msg.DaysOffList.push_back(to_iso_extended_string(it.second));
			}

			for (auto &it : underlying->get_future_class_list())
			{
				msg.FutureClassList.push_back(create_instrument_class_msg((instrumentclass *)it.second));
			}

			for (auto &it : underlying->get_option_class_list())
			{
				msg.OptionClassList.push_back(create_instrument_class_msg((instrumentclass *)it.second));

			}
			return msg;
		}
		InstrumentClassMsg ats_server::create_instrument_class_msg(instrumentclass * pInstrumentclass)
		{
			InstrumentClassMsg msg;
			if (pInstrumentclass == nullptr)
				return msg;
			msg.__isset.Maturities = true;
			msg.__isset.MaturityTime = true;
			msg.Name = pInstrumentclass->get_name();
			msg.Currency = pInstrumentclass->get_currency_name();
			msg.PointValue = pInstrumentclass->get_point_value();
			msg.FeesStruct = create_fees_struct_msg(pInstrumentclass);
			derivclassbase* pDerivclassbase = dynamic_cast<derivclassbase*>(pInstrumentclass);
			if (pDerivclassbase != nullptr)
			{
				msg.MaturityTime = to_simple_string(pDerivclassbase->get_maturity_time());
				for (auto & v : pDerivclassbase->get_maturity_map())
				{
					msg.Maturities.push_back(create_maturity_msg(v.second));
				}
			}
			return msg;
		}
		FeesStructMsg ats_server::create_fees_struct_msg(instrumentclass * pInstrumentclass)
		{
			FeesStructMsg msg;
			if (pInstrumentclass != nullptr)
			{
				msg.ClassName = pInstrumentclass->get_name();
				msg.FeesFixExchange = pInstrumentclass->get_fees_fix_exchange();
				msg.FeesFixBroker = pInstrumentclass->get_fees_fix_broker();
				msg.FeesFloatBroker = pInstrumentclass->get_fees_float_broker();
				msg.FeesFloatExchange = pInstrumentclass->get_fees_float_exchange();
				msg.FeesSellAmount = pInstrumentclass->get_fees_sell_amount();
				msg.NotCloseToday = pInstrumentclass->get_not_close_today();
			}
			return msg;
		}
		MaturityMsg ats_server::create_maturity_msg(maturity *pMaturity)
		{
			MaturityMsg msg;
			if (pMaturity == nullptr)
				return msg;
			msg.MaturityStr = to_iso_extended_string(pMaturity->get_maturity_datetime().date());
			msg.OffDays = pMaturity->get_off_days();
			msg.OpenDays = pMaturity->get_open_days();
			msg.MaturityTime = to_simple_string(pMaturity->get_maturity_datetime().time_of_day());
			return msg;
		}
		void ats_server::update_abstract_ats_msg(AtsMsg & msg, abstract_ats * ats)
		{
			if (ats == nullptr)
				return;
			msg.__set_FeesBroker(ats->get_fees_broker());
			msg.__set_AutoStatus(ats->get_auto_status());
			msg.__set_FeesExchange(ats->get_fees_exchange());
			msg.__set_TodayPnlBary(ats->get_today_pnl_bary());
			msg.__set_TodayPnlLast(ats->get_today_pnl_last());
			msg.__set_TodayPnlMid(ats->get_today_pnl_mid());
			msg.__set_YesterdayPnlBary(ats->get_yesterday_pnl_bary());
			msg.__set_TotalPnl(ats->get_yesterday_pnl_bary() + ats->get_today_pnl_bary() - ats->get_fees_broker() - ats->get_fees_exchange());
			msg.__set_YesterdayPnlLast(ats->get_today_pnl_last());
			msg.__set_YesterdayPnlMid(ats->get_today_pnl_mid());
		}
		void ats_server::update_ats_instrument_msg(AtsInstrumentMsg & msg, ats_instrument* instrument, bool has_depth)
		{
			if (instrument->get_position() != nullptr)
			{
				update_position_msg(msg.Position, instrument->get_position());
			}
			if (instrument->get_feed_item() != nullptr)
			{
				update_feed_msg(msg.FeedItem, instrument->get_feed_item(), has_depth);
			}

			msg.__set_ExchangeFees(instrument->get_exchange_fees());
			msg.__set_BrokerFees(instrument->get_broker_fees());
			msg.__set_YesterdayPnlBarycenter(instrument->get_yesterday_phl_bary_center());
			msg.__set_TodayPnlBarycenter(instrument->get_today_phl_bary_center());


			msg.SizeToSend = instrument->size_to_send;

			if (instrument->get_market_maker_item_bid() != nullptr)
			{
				msg.MarketMakerItemBid.clear();
				unordered_map_ex<double, time_order_container*>  map;
				instrument->get_market_maker_item_bid()->get_map(map);
				for (auto & v : map)
				{
					msg.MarketMakerItemBid.insert(make_pair(v.first, v.second->get_quantity()));
				}
			}
			if (instrument->get_market_maker_item_ask() != nullptr)
			{
				msg.MarketMakerItemAsk.clear();
				unordered_map_ex<double, time_order_container*>  map;
				instrument->get_market_maker_item_ask()->get_map(map);
				for (auto & v : map)
				{
					msg.MarketMakerItemAsk.insert(make_pair(v.first, v.second->get_quantity()));
				}
			}
		}
		void ats_server::update_position_msg(PositionMsg & msg, position * position)
		{
			msg.YesterdayPosition = position->get_yesterday_position();
			msg.YesterdayPrice = position->get_yesterday_price();
			msg.TodayPosition = position->get_today_position();
			msg.TotalPosition = position->get_total_position();
			msg.TodayBuyPosition = position->get_today_buy_position();
			msg.TodayBuyPrice = position->get_today_buy_price();
			msg.TodaySellPosition = position->get_today_sell_position();
			msg.TodaySellPrice = position->get_today_sell_price();
			msg.TodayRedPosition = position->m_nTodayRedPosition;
			msg.TodayPurPosition = position->m_TodayPurPosition;
			msg.YesterdayPositionExternal = position->get_yesterday_position_external();
			msg.__set_YstPositionType(position->get_yesterday_position_type());
			msg.AccoutTotalPosition = position->get_instrument()->get_tot_long_position() - position->get_instrument()->get_tot_short_position();
		}
		void ats_server::update_feed_msg(FeedMsg & msg, feed_item * item, bool has_depth)
		{
			msg.__isset.Bid = true;
			msg.__isset.Ask = true;
			msg.__isset.AskQtys = true;
			msg.__isset.BidQtys = true;
			msg.__isset.Close = true;
			msg.__isset.Last = true;
			msg.__isset.DailyVolume = true;
			msg.__isset.FeedSourceName = true;
			msg.__isset.Perf = true;
			msg.FeedSourceName = item->get_feed_source_name();
			msg.Code = item->get_code();
			msg.Bid = item->get_bid_price();
			msg.Ask = item->get_ask_price();
			msg.BidQuantity = item->get_bid_quantity();
			msg.AskQuantity = item->get_ask_quantity();
			msg.Last = item->get_last_price();
			msg.LastQuantity = item->get_last_quantity();
			msg.LastOrClose = item->last_or_close();
			msg.Mid = item->mid();
			msg.Close = item->get_close_price();
			msg.Settlement = item->get_settlement();
			msg.UpperLimit = item->get_upper_limit();
			msg.LowerLimit = item->get_lower_limit();
			msg.Perf = item->get_perf();
			msg.DailyVolume = item->get_daily_volume();
			msg.isBiddAskActive = item->is_bid_ask_active();
			msg.isSuspended = item->is_suspended();
			if (has_depth)
			{
				for (int i = 0; i < msg.MaxDepth; i++)
				{
					msg.BidDepths[i] = item->market_bid(i);
					msg.AskDepths[i] = item->market_ask(i);
					msg.BidQtys[i] = item->market_bid_qty(i);
					msg.AskQtys[i] = item->market_ask_qty(i);
				}
			}
			msg.MarketTime = item->market_time;
		}

		void ats_server::update_maturity_msg(MaturityMsg& msg, maturity* item)
		{
			msg.OpenDays = item->get_open_days();
			msg.Actu = item->get_actu();
		}

		publish_task::publish_task(lwdur delay) :itask(delay)
		{
			open_buy = 0;
			close_buy = 0;
			open_sell = 0;
			close_sell = 0;
		}

		void publish_task::execute()
		{
			try{
				pub_feed_status();
				pub_connection_status();
				pub_logs();
				pub_orders();
				pub_quotes();
				pub_execs();
				pub_intra_data();
				if (handler)
				{
					handler();
				}
			}
			catch (exception& e)
			{
				cout << "publish_task exception: " << e.what() << endl;
				loggerv2::error("exception exception : :%s", e.what());
			}

			if (ats_server::get_instance()->m_enable_stop == false)
				return;

			auto tm = second_clock::local_time();
			if (tm > ats_server::get_instance()->kill_time)
			{
				for (auto &it : ats_server::get_instance()->AbstractATSMap)
				{
					it.second->stop_automaton(true);
				}

				int i = 5;
				for (; i > 0; --i)
				{
					cout << "time over,srv wiil be killed after " << i << " sec" << endl;
					sleep_by_milliseconds(1000);
				}

				exit(1);
			}
		}
		void publish_task::pub_feed_status()
		{

			for (auto &it : *feed_source_container::get_instance())
			{
				feed_source * pSource = it.second;
				FeedSourceMsg msg;
				msg.Activated = pSource->get_activated();
				msg.Name = pSource->get_name();
				msg.Status = pSource->get_status();
				ats_manager::get_instance()->net_mq_publisher->publish<FeedSourceMsg>("FeedSource", msg);
			}
		}
		void publish_task::pub_connection_status()
		{
			for (auto &it : connection_gh::get_instance().container().get_map())
			{
				connection* conn = it.second;
				ConnectionMsg msg;
				msg.Name = conn->getName();
				msg.Ack = conn->get_statistics().get_ack();
				msg.Status = conn->getStatus();
				msg.Can = conn->get_statistics().get_can();
				msg.Exe = conn->get_statistics().get_exe();
				msg.NAck = conn->get_statistics().get_nack();
				msg.New = conn->get_statistics().get_new();
				msg.TradingAllowed = conn->getTradingAllowed();
				//
				msg.RiskDegree     = conn->get_RiskDegree();
				//
				ats_manager::get_instance()->net_mq_publisher->publish<ConnectionMsg>("Connection", msg);
			}
		}
		void publish_task::pub_logs()
		{

			log_record_vector logvec;
			loggerv2::get_instance()->get_queue(logvec);
			ListLogMsg logs;
			for (auto &record : logvec)
			{
				//log_record * record = (it);				
				LogMsg msg;
				msg.TimeStamp = record->time;
				msg.Message = urlDE::get_instance().UrlEncode(record->msg);
				//ats_manager::get_instance()->net_mq_publisher->publish<LogMsg>("Log", msg);
				logs.Logs.push_back(msg);
			}
			if (logs.Logs.size() > 0)
			{
				ats_manager::get_instance()->net_mq_publisher->publish<ListLogMsg>("Logs", logs);
			}
		}
		void publish_task::pub_orders()
		{

			//std::list<order*> order_list;
			auto queue = m_order_data.get_add_order();
			int i = 0;
			ListOrderMsg orders;
			for (; i < 1000 && queue->read_available(); ++i)
			{

				order *temp = queue->Pop();
				OrderMsg msg;
				update_order_msg(msg, temp);

				orders.Orders.push_back(msg);
			}
			if (orders.Orders.size()>0)
			{
				ats_manager::get_instance()->net_mq_publisher->publish<ListOrderMsg>("Orders", orders);
			}

			//ats_manager::get_instance()->net_mq_publisher->publish<OrderMsg>("Order", msg);
		}

		void publish_task::pub_quotes()
		{

			//std::list<order*> order_list;
			auto queue = m_order_data.get_add_quote();
			int i = 0;
			ListQuoteMsg quotes;
			for (; i < 1000 && queue->read_available(); ++i)
			{

				quote *temp = queue->Pop();
				QuoteMsg msg;
				msg.Code = temp->get_instrument()->getCode();
				msg.Id = temp->get_id();
				msg.TimeStamp = lwtp_to_simple_time_string(temp->get_last_time());
				msg.__set_Status(temp->get_status());
				msg.__set_Portfolio(temp->get_portfolio());
				msg.__set_LastReason(urlDE::get_instance().UrlEncode(temp->get_lastreason()));
				msg.FQR_ID = temp->get_FQR_ID();

				msg.TradingType = temp->get_trading_type();
				if (temp->get_bid_order() != nullptr)
				{
					update_order_msg(msg.BuyOrder, temp->get_bid_order());
				}
				if (temp->get_ask_order() != nullptr)
				{

					update_order_msg(msg.SellOrder, temp->get_ask_order());
				}
				
				quotes.Quotes.push_back(msg);
			}
			if (quotes.Quotes.size() > 0)
			{
				ats_manager::get_instance()->net_mq_publisher->publish<ListQuoteMsg>("Quotes", quotes);
			}
			//ats_manager::get_instance()->net_mq_publisher->publish<QuoteMsg>("Quote", msg);
		}
		void publish_task::pub_execs()
		{

			//std::list<exec*> exec_list;
			auto queue = m_order_data.get_exe_order();
			int i = 0;
			ListExecMsg execs;
			for (; i < 1000 && queue->read_available(); ++i)
			{
				ExecMsg msg;
				exec *temp = queue->Pop();
				msg.__set_Way(temp->getWay());
				msg.__set_TradingType(temp->getTradingType());
				if (temp->getOpenClose() == OrderOpenClose::Open)
				{
					if (temp->getWay() == OrderWay::Buy)
						open_buy += temp->getQuantity();
					if (temp->getWay() == OrderWay::Sell)
						open_sell += temp->getQuantity();
				}

				if (temp->getOpenClose() == OrderOpenClose::Close || temp->getOpenClose() == OrderOpenClose::CloseToday)
				{
					if (temp->getWay() == OrderWay::Buy)
						close_buy += temp->getQuantity();
					if (temp->getWay() == OrderWay::Sell)
						close_sell += temp->getQuantity();
				}
				msg.__set_OpenClose(to_string(open_buy) + "|" + to_string(open_sell) + "|" + to_string(close_buy) + "|" + to_string(close_sell));
				msg.__set_Reference(temp->getReference());
				msg.__set_Quantity(temp->getQuantity());
				msg.__set_Price(temp->getPrice());
				msg.__set_Portfolio(temp->getPortfolioName());
				msg.__set_Time(temp->getTime());
				msg.__set_Id(temp->getOrderId());
				msg.__set_Code(temp->getTradeItem()->getCode());
				execs.Execs.push_back(msg);

			}
			if (execs.Execs.size() > 0)
			{
				ats_manager::get_instance()->net_mq_publisher->publish<ListExecMsg>("Execs", execs);
			}
		}

		void publish_task::intradata_update_event(terra::ats::atsintradata* source, ptime lastTs, map<string, double>& lastData)
		{
			std::shared_ptr<IntraDataMsg>intra_data_msg(new IntraDataMsg());
			intra_data_msg->name = source->get_name();
			intra_data_msg->time = to_iso_extended_string(lastTs);
			intra_data_msg->datas = lastData;
			//std::cout << "counted " << intra_data_msg.use_count() <<" before push"<<  std::endl;
			instrDataQueue.push(intra_data_msg);
			//std::cout << "counted " << intra_data_msg.use_count() << " after push" << std::endl;

		}

		bool ats_server::CheckAesMsg(const AESDataMsg& msg)
		{
			return true;
			CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption  Decryptor1(key, 32, iv);
			CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption  Decryptor2(key, 32, iv);
			CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption  Decryptor3(key, 32, iv);
			std::string Hexdate, HexNum, HexTime, strDate, strNum, strEndTime, strTime;
			//bool res;
			try
			{
				StringSource(msg.date,
					true,
					new CryptoPP::HexDecoder(new StringSink(Hexdate))
					);

				StringSource(msg.num,
					true,
					new CryptoPP::HexDecoder(new StringSink(HexNum))
					);

				StringSource(msg.time_t,
					true,
					new CryptoPP::HexDecoder(new StringSink(HexTime))
					);

				CryptoPP::StringSource(Hexdate,
					true,
					new StreamTransformationFilter(Decryptor1,
					new StringSink(strDate),
					BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING,
					true)
					);

				StringSource(HexNum,
					true,
					new StreamTransformationFilter(Decryptor2,
					new StringSink(strNum),
					BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING,
					true)
					);

				StringSource(HexTime,
					true,
					new StreamTransformationFilter(Decryptor3,
					new StringSink(strTime),
					BlockPaddingSchemeDef::BlockPaddingScheme::ZEROS_PADDING,
					true)
					);

				//ptime now = second_clock::local_time();
				strEndTime = strDate.substr(0, 10);
				date mdate(from_simple_string(strEndTime));
				ptime mtime(time_from_string(strTime.substr(0, 19)));
				//boost::posix_time::time_duration dur(0, 10, 0, 0);
				if (date_time_publisher_gh::get_instance()->today() > mdate)
				{
					is_verify = false;
					return false;
				}

				int num = atoi(strNum.c_str());
				if (num <= 0)
				{
					is_verify = false;
					return false;
				}

				if (abs((date_time_publisher_gh::get_instance()->now() - mtime).total_seconds()) > 10 * 60)
				{
					is_verify = false;
					return false;
				}
			}
			catch (std::exception &ex)
			{
				cout << ex.what() << endl;
				is_verify = false;
				return false;
			}
			is_verify = true;

			return is_verify;
		}

		void publish_task::pub_intra_data()
		{

			//int consumed = 0;
			while (instrDataQueue.empty() == false)
			{
				std::shared_ptr<IntraDataMsg>msg;

				if (instrDataQueue.try_pop(msg))
				{
					if (msg.get() != nullptr)
						ats_manager::get_instance()->net_mq_publisher->publish<IntraDataMsg>("IntraData", *msg);
				}

				//++consumed;
			}

			//std::cout << "Consumed " << consumed << "intradata " << std::endl;
		}

		void publish_task::update_order_msg(OrderMsg &msg, order * temp)
		{
			msg.__set_Way(temp->get_way());
			msg.__set_TradingType(temp->get_trading_type());
			msg.__set_Status(temp->get_status());
			std::string dtime = lwtp_to_simple_time_string(temp->get_last_time());
			msg.__set_TimeStamp(dtime);
			msg.__set_Quantity(temp->get_quantity());
			msg.__set_Price(temp->get_price());
			msg.__set_Portfolio(temp->get_portfolio());
			msg.__set_LastReason(urlDE::get_instance().UrlEncode(temp->get_lastreason()));
			msg.__set_Id(temp->get_id());
			msg.__set_ExecQty(temp->get_exec_quantity());
			msg.__set_ExecPrice(temp->get_exec_price());
			msg.__set_Code(temp->get_instrument()->getCode());
			msg.__set_BookQty(temp->get_book_quantity());
			msg.__set_Active(temp->get_active());
			msg.__set_Action(temp->get_last_action());
			msg.OpenClose = temp->get_open_close();
			if (temp->get_status() == OrderStatus::Ack)
			{
				acks[temp->get_instrument()->getCode()] = acks[temp->get_instrument()->getCode()] + 1;
			}
			if (temp->get_status() == OrderStatus::Cancel)
			{
				cancels[temp->get_instrument()->getCode()] = cancels[temp->get_instrument()->getCode()] + 1;
			}

			msg.Ack_Cancel_Ratio = to_string(acks[temp->get_instrument()->getCode()]) + "_" + to_string(cancels[temp->get_instrument()->getCode()]);
		}

	}
}