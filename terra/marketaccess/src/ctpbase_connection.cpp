#include "ctpbase_connection.h"
//#include "LockFreeClassPool.h"
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <FastMemcpy.h>
#include <tradeItem_gh.h>
#include <referential.h>
#include "database_factory.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{

			ctpbase_connection::ctpbase_connection(bool checkSecurities) : connection(checkSecurities)
			{
				m_debug = false;
				m_last_time = get_lwtp_now();
				m_last_check_wait_time = m_last_time;
				//m_isAlive = true;
				m_bKey_with_exchange = true;
				m_outboundQueue.setHandler(std::bind(&ctpbase_connection::process_outbound_order, this, std::placeholders::_1));
				m_outquoteboundQueue.setHandler(std::bind(&ctpbase_connection::process_outbound_quote, this, std::placeholders::_1));
			}


			bool ctpbase_connection::init_config(const std::string& name, const std::string& strConfigFile)
			{
				m_sName = name;
				m_sConfigFile = strConfigFile;
				if (strConfigFile.length() < 1)
					return false;

				boost::filesystem::path p(strConfigFile);
				if (!boost::filesystem::exists(p))
				{
					printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
					return false;
				}
				boost::property_tree::ptree root;
				boost::property_tree::ini_parser::read_ini(strConfigFile, root);

				m_debug = root.get<bool>(name + ".debug", true);
				m_type = root.get<string>(name + ".type", "");
				m_code_type = root.get<string>(name + ".codetype", m_type);
				// stock
				m_sHostname = root.get<string>(name + ".hostname", "");

				m_sService = root.get<string>(name + ".service", "");

				m_sUsername = root.get<string>(name + ".username", "");

				m_sPassword = root.get<string>(name + ".password", "");

				m_sBrokerId = root.get<string>(name + ".broker_id", "");

				//
				m_sLoginId = root.get<string>(name + ".login_id", "");
				//

				m_sCurrentBizDate = getCurrentBizDate();
				loggerv2::info("%s::init_config current biz date is %s", m_sName.c_str(), m_sCurrentBizDate.c_str());
				m_bCloseToday = root.get<bool>(name + ".close_today", false);
				m_bRequestPosition = root.get<bool>(name + ".req_position", true);

				m_bRequestInstruments = root.get<bool>(name + ".req_instruments", false);

				m_cancel_num_warning = root.get<unsigned int>(name + ".cancel_num_warning", 200);
				m_cancel_num_ban = root.get<unsigned int>(name + ".cancel_num_ban", 225);
				return true;
			}
			void ctpbase_connection::process_idle()
			{
				lwtp now = get_lwtp_now();
				//tbb::concurrent_hash_map<int, order*>::accessor ra;
				std::list<order *> to_move_order_ids, to_notifier_order_ids, ot_order_ids;

				if (now - m_last_check_wait_time >= std::chrono::seconds(5))
				{
					auto check_lambda = [&to_move_order_ids, &now, &to_notifier_order_ids, &ot_order_ids](terra_safe_tbb_hash_map<int, order*>::const_accessor &ra)
					{
						order *o = ra->second;
						auto status = ra->second->get_status();
						if (status == OrderStatus::WaitServer && now - o->get_last_time() >= std::chrono::seconds(5))
						{
							loggerv2::error("ctpbase_connection::process_idle order roll back");
							o->dump_info();
							o->rollback();
							o->dump_info();
							o->set_last_time(now);
							to_notifier_order_ids.push_back(o);
						}
						if (o->get_last_action() == OrderAction::Created && status == OrderStatus::Nack)
						{
							to_move_order_ids.push_back(o);
							ot_order_ids.push_back(o);//此类order属于超时的异常单
						}
						if (status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
							to_move_order_ids.push_back(o);
					};

					m_activeOrders.for_each(check_lambda);

					for (auto& o : to_notifier_order_ids)
					{
						update_pending_order(o);
					}

					for (auto& o : to_move_order_ids)
					{
						int orderId = o->get_id();
						//m_activeOrders.unsafe_erase(orderId);
						m_activeOrders.erase(orderId);
						m_deadOrders.emplace(orderId, o);

						if (m_crossCheck)
						{
							//撤单/成交/rej/nack回调，移除order_book中的项
							o->get_instrument()->get_order_book()->remove_order(abs(o->get_id()));// , o->get_price(), o->get_book_quantity(), o->get_way());
						}
						update_pending_order(o);
						o->on_update_order();
					}
					m_last_check_wait_time = now;
				}


				for (auto& o : ot_order_ids)
				{

					tbb::concurrent_hash_map<int, order*>::accessor wa;
					m_overTimeOrders.insert(wa, o->get_id());//超时类异常单，理论上五秒后还是有可能拿到下单回报
					wa->second = o;
					wa.release();
				}


				if (m_bRequestPosition && now - m_last_time >= std::chrono::seconds(5) && m_status == AtsType::ConnectionStatus::Connected)
				{
					if (m_bRequestPosition)
						request_investor_full_positions();
					m_last_time = now;
					req_RiskDegree();
				}

				
			}



			std::string ctpbase_connection::getCurrentBizDate(){
				time_t     now = time(0);
				struct tm  tstruct;
				char       buf[9];
				tstruct = *localtime(&now);
				strftime(buf, sizeof(buf), "%Y%m%d", &tstruct);
				buf[8] = '\0';
				return buf;
			}



			ctpbase_connection::~ctpbase_connection()
			{
			}

			int ctpbase_connection::get_order_id(const char* psz, OrderWay::type way)
			{
				//std::string str = psz;
				//int len = str.size();
				int account, bidId, askId, portfolioId, tradingType;
				get_user_info(psz, account, bidId, askId, portfolioId, tradingType);
				if (way == OrderWay::Buy && bidId != 0)
				{
					return bidId;
				}
				else
				{
					return askId;
				}
				/*
								if (strlen(psz) < 15)
								return -1;

								if (psz[1] == ' '||psz[14]==' ')
								return -1;

								char cAcc[2 + 1],cOId[5 + 1];

								memcpy_lw(cAcc, psz, 2); cAcc[2] = '\0';
								memcpy_lw(cOId, psz + 7, 5); cOId[5] = '\0';

								int nAccount,orderid;
								nAccount = strtol(cAcc, NULL, 16);
								orderid = strtol(cOId, NULL, 16);

								if (nAccount != m_account_num)
								{
								orderid = orderid + 1000000 * nAccount;
								}


								return orderid;*/
			}

			void ctpbase_connection::get_user_info(const char* psz, int& nAccount, int& userOrderId, int& internalRef, int& nPortfolio, int& nTradeType)
			{
				loggerv2::info("%s::get_user_info psz info[%s]", getName().c_str(), psz);

				char cAcc[2 + 1], cUId[5 + 1], cIRef[5 + 1];

				memcpy_lw(cAcc, psz, 2); cAcc[2] = '\0';
				memcpy_lw(cUId, psz + 2, 5); cUId[5] = '\0';
				memcpy_lw(cIRef, psz + 7, 5); cIRef[5] = '\0';

				nAccount = strtol(cAcc, NULL, 16);
				userOrderId = strtol(cUId, NULL, 16);
				internalRef = strtol(cIRef, NULL, 16);

				sscanf(psz + 12, "%1d", &nTradeType);
				sscanf(psz + 13, "%2x", &nPortfolio);
				if (nAccount != m_account_num)
				{
					internalRef = internalRef + 1000000 * nAccount;
					if (userOrderId > 0)
						userOrderId = userOrderId + 1000000 * nAccount;
					nPortfolio = -1;
					nTradeType = 0;
				}
				loggerv2::info("%s::get_user_info nAccount %d, userOrderId %d, internalRef %d, nTradeType %d, nPortfolio %d", getName().c_str(), nAccount, userOrderId, internalRef, nTradeType, nPortfolio);
				return;
			}
			void ctpbase_connection::update_instr_on_ack_from_market_cb(order* o, int ackQty)
			{

				tradeitem* i = o->get_instrument();
				int nOrdQtyp = ackQty < 0 ? o->get_book_quantity() : ackQty;
				if (m_debug)
				{
					loggerv2::info("calling update_instr_on_ack_from_market_cb,id:%d,qty:%d", o->get_id(), nOrdQtyp);
					i->dumpinfo();
					o->dump_info();
				}
				switch (o->get_open_close())
				{
				case AtsType::OrderOpenClose::Close:

					switch (o->get_way())
					{
					case AtsType::OrderWay::Buy:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created)

							//i->set_pending_long_close_qty(i->get_pending_long_close_qty() + nOrdQtyp);
							i->add_pending_long_close_qty(nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled && i->get_pending_long_close_qty() >= nOrdQtyp)
							i->set_pending_long_close_qty(i->get_pending_long_close_qty() - nOrdQtyp);
					}


					break;
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::Exercise:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created)
							//i->set_pending_short_close_qty(i->get_pending_short_close_qty() + nOrdQtyp);
							i->add_pending_short_close_qty(nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled && i->get_pending_short_close_qty() >= nOrdQtyp)
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - nOrdQtyp);
					}

					break;
					case AtsType::OrderWay::Freeze:
					{
						//for underlying
						//should not receive openclose = close and way = freeze	
						//the way should be unfreeze

					}
					break;
					case AtsType::OrderWay::Unfreeze:
					{
						//release the frozen quantity.
						if (i->get_frozen_long_position() >= nOrdQtyp)
							i->set_frozen_long_position(i->get_frozen_long_position() - nOrdQtyp);
					}
					break;

					case AtsType::OrderWay::CoveredBuy: //备兑平仓 buy call option
					{
						if (o->get_last_action() == AtsType::OrderAction::Created)
						{
							i->set_pending_covered_sell_close_qty(i->get_pending_covered_sell_close_qty() + nOrdQtyp);
						}
						else if (o->get_last_action() == AtsType::OrderAction::Cancelled)
						{
							if (i->get_pending_covered_sell_close_qty() >= nOrdQtyp)
								i->set_pending_covered_sell_close_qty(i->get_pending_covered_sell_close_qty() - nOrdQtyp);
						}
					}
					break;
					default:
						break;
					}


					break;


				case AtsType::OrderOpenClose::CloseToday:


					switch (o->get_way())
					{
					case AtsType::OrderWay::Buy:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created)

							//i->set_pending_long_close_today_qty(i->get_pending_long_close_today_qty() + nOrdQtyp);
							i->add_pending_long_close_today_qty(nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled && i->get_pending_long_close_today_qty() >= nOrdQtyp)
							i->set_pending_long_close_today_qty(i->get_pending_long_close_today_qty() - nOrdQtyp);
					}


					break;
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::Exercise:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created)

							//i->set_pending_short_close_today_qty(i->get_pending_short_close_today_qty() + nOrdQtyp);
							i->add_pending_short_close_today_qty(nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled && i->get_pending_short_close_today_qty() >= nOrdQtyp)
							i->set_pending_short_close_today_qty(i->get_pending_short_close_today_qty() - nOrdQtyp);
					}

					break;
					default:
						break;


					}
					break;
				case OrderOpenClose::Open:
				{

					switch (o->get_way())
					{
					case AtsType::OrderWay::Freeze:
					{
						//this is the case that underlying is frozen
						//we increase the frozen quantity.	
						i->set_frozen_long_position(i->get_frozen_long_position() + nOrdQtyp);
					}
					break;
					case AtsType::OrderWay::CoveredSell:  //备兑开仓 short call option
					{
						if (o->get_last_action() == AtsType::OrderAction::Created)
						{
							i->set_pending_covered_sell_open_qty(i->get_pending_covered_sell_open_qty() + nOrdQtyp);
						}
						else if (o->get_last_action() == AtsType::OrderAction::Cancelled)
						{
							if (i->get_pending_covered_sell_open_qty() >= nOrdQtyp)
								i->set_pending_covered_sell_open_qty(i->get_pending_covered_sell_open_qty() - nOrdQtyp);
						}
					}
					break;
					case AtsType::OrderWay::ETFRed:
					{
						i->set_pending_covered_sell_open_qty(i->get_pending_covered_sell_open_qty() + nOrdQtyp);
					}
					break;
					case AtsType::OrderWay::Undef:
					case AtsType::OrderWay::Buy:
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::CoveredBuy:
					case AtsType::OrderWay::Unfreeze:
					case AtsType::OrderWay::PLEDGE_BOND_IN:
					case AtsType::OrderWay::PLEDGE_BOND_OUT:
					case AtsType::OrderWay::ETFPur:
					case AtsType::OrderWay::OFPur:
					case AtsType::OrderWay::OFRed:
					case AtsType::OrderWay::Exercise:
					defalut :
						break;
					}
				}
				break;
				case OrderOpenClose::Undef: break;

				default: break;
				}



				if (m_debug)
					i->dumpinfo();


			}

			void ctpbase_connection::update_instr_on_nack_from_market_cb(order* o, int actQty)
			{
				//order is already rolled back
				/*   if (m_debug)
				loggerv2::info("calling update_instr_on_nack_from_market_cb");*/
				if (o == nullptr)
					return;
				tradeitem* i = o->get_instrument();
				int nOrdQtyp = actQty < 0 ? o->get_book_quantity() : actQty;
				if (m_debug)
				{
					loggerv2::info("calling update_instr_on_nack_from_market_cb,id:%d,qty:%d", o->get_id(), nOrdQtyp);
					i->dumpinfo();
					o->dump_info();
				}
				switch (o->get_open_close())
				{
				case AtsType::OrderOpenClose::Close:

					switch (o->get_way())
					{
					case AtsType::OrderWay::Buy:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created  && i->get_pending_long_close_qty() >= nOrdQtyp)
							i->set_pending_long_close_qty(i->get_pending_long_close_qty() - nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled)
							i->set_pending_long_close_qty(i->get_pending_long_close_qty() + nOrdQtyp);
					}

					break;
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::Exercise:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created  && i->get_pending_short_close_qty() >= nOrdQtyp)
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled)
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() + nOrdQtyp);

					}
					break;
					case AtsType::OrderWay::Unfreeze: //解锁失败 
					{
						i->set_frozen_long_position(i->get_frozen_long_position() + nOrdQtyp);
					}
					break;
					case AtsType::OrderWay::CoveredBuy:
					{
						//备兑平仓 失败
						if (i->get_pending_covered_sell_close_qty() >= nOrdQtyp)
							i->set_pending_covered_sell_close_qty(i->get_pending_covered_sell_close_qty() - nOrdQtyp);
					}
					break;
					case AtsType::OrderWay::ETFRed:
					{
						if (i->get_pending_short_close_qty() >= nOrdQtyp)
						{
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - nOrdQtyp);
						}
					}
					break;
					default:
						break;


					}

					break;
				case AtsType::OrderOpenClose::CloseToday:

					switch (o->get_way())
					{
					case AtsType::OrderWay::Buy:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created  && i->get_pending_long_close_today_qty() >= nOrdQtyp)
							i->set_pending_long_close_today_qty(i->get_pending_long_close_today_qty() - nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled)
							i->set_pending_long_close_today_qty(i->get_pending_long_close_today_qty() + nOrdQtyp);
					}

					break;
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::Exercise:
					{
						if (o->get_last_action() == AtsType::OrderAction::Created  && i->get_pending_short_close_today_qty() >= nOrdQtyp)
							i->set_pending_short_close_today_qty(i->get_pending_short_close_today_qty() - nOrdQtyp);
						if (o->get_last_action() == AtsType::OrderAction::Cancelled)
							i->set_pending_short_close_today_qty(i->get_pending_short_close_today_qty() + nOrdQtyp);

					}
					break;
					default:
						break;


					}
					break;

				case OrderOpenClose::Open:
					switch (o->get_way())
					{
					case AtsType::OrderWay::Freeze: //锁仓失败
					{
						//for underlying	   
						if (i->get_frozen_long_position() >= nOrdQtyp)
						{
							i->set_frozen_long_position(i->get_frozen_long_position() - nOrdQtyp);
						}
					}
					break;

					case AtsType::OrderWay::CoveredSell: //备兑开仓 失败
					{
						if (i->get_pending_covered_sell_open_qty() >= o->get_quantity())
						{
							i->set_pending_covered_sell_open_qty(i->get_pending_covered_sell_open_qty() - o->get_quantity());
						}

					}
					break;

					default:
						break;
					}
					break;
				default: break;
				}

				if (m_debug)
					i->dumpinfo();
			}
			void ctpbase_connection::update_instr_on_cancel_from_market_cb(order* o, int cancelQty)
			{
				//called during resychro
				//order is already rolled back
				/* if (m_debug)
				loggerv2::info("calling update_instr_on_cancel_from_market_cb");
				*/
				if (o == nullptr)
					return;
				tradeitem* i = o->get_instrument();
				int nOrdQtyp = cancelQty < 0 ? o->get_book_quantity() : cancelQty;
				if (m_debug)
				{
					loggerv2::info("calling update_instr_on_cancel_from_market_cb,nOrdQtyp:%d",nOrdQtyp);
					i->dumpinfo();
					o->dump_info();
				}

				switch (o->get_open_close())
				{
				case AtsType::OrderOpenClose::Close:

					switch (o->get_way())
					{
					case AtsType::OrderWay::Buy:
					{
						if (i->get_pending_long_close_qty() >= nOrdQtyp)
							i->set_pending_long_close_qty(i->get_pending_long_close_qty() - nOrdQtyp);
					}

					break;
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::Exercise:
					{
						if (i->get_pending_short_close_qty() >= nOrdQtyp)
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - nOrdQtyp);

					}
					break;
					case AtsType::OrderWay::CoveredBuy:  //取消 备兑平仓
					{
						if (i->get_pending_covered_sell_close_qty() >= nOrdQtyp)
							i->set_pending_covered_sell_close_qty(i->get_pending_covered_sell_close_qty() - nOrdQtyp);
					}
					break;
					case AtsType::OrderWay::ETFRed:
					{
						if (i->get_pending_short_close_qty() >= nOrdQtyp)
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - nOrdQtyp);

					}
					break;
					case AtsType::OrderWay::Freeze:
					case AtsType::OrderWay::Unfreeze:
					{
						//will never happen. frozen position cannot be cancelled here. 
					}
					break;

					default:
						break;
					}
					break;


				case AtsType::OrderOpenClose::CloseToday:

					switch (o->get_way())
					{
					case AtsType::OrderWay::Buy:
					{
						if (i->get_pending_long_close_today_qty() >= nOrdQtyp)
							i->set_pending_long_close_today_qty(i->get_pending_long_close_today_qty() - nOrdQtyp);
					}

					break;
					case AtsType::OrderWay::Sell:
					case AtsType::OrderWay::Exercise:
					{
						if (i->get_pending_short_close_today_qty() >= nOrdQtyp)
							i->set_pending_short_close_today_qty(i->get_pending_short_close_today_qty() - nOrdQtyp);

					}
					break;
					default:
						break;
					}
					break;

				case OrderOpenClose::Open:

					switch (o->get_way())
					{

					case AtsType::OrderWay::CoveredSell: //取消备兑平仓
					{
						if (i->get_pending_covered_sell_open_qty() >= nOrdQtyp)
						{
							i->set_pending_covered_sell_open_qty(i->get_pending_covered_sell_open_qty() - nOrdQtyp);
						}
					}
					break;

					default:
						break;
					}


					break;
				default: break;
				}


				if (m_debug)
					i->dumpinfo();
			}


			void ctpbase_connection::update_instr_on_exec_from_market_cb(order* o, exec* e, bool onlyUpdatePending)
			{

				tradeitem* i = o->get_instrument();
				int execQty = e->getQuantity();
				if (m_debug)
				{

					loggerv2::info("calling update_instr_on_exec_from_market_cb,id:%d,onlyUpdatePending:%d", o->get_id(), onlyUpdatePending);
					i->dumpinfo();
					o->dump_info();
					e->dump_info();
				}
				switch (o->get_way())
				{
				case AtsType::OrderWay::Buy:
				{
					//i->set_today_long_position(i->get_today_long_position() + execQty);


					if (o->get_open_close() == AtsType::OrderOpenClose::Open)
					{
						if (!onlyUpdatePending)
						{
							i->set_tot_long_position(i->get_tot_long_position() + execQty);
							i->set_today_long_position(i->get_today_long_position() + execQty);
							i->set_open_position(i->get_open_position() + execQty);
							if (i->getInstrument()->get_not_close_today())
							{
								for (auto& it : m_activeOrders)
								{
									if (it.second->get_instrument()->getCode() == i->getCode() && (it.second->get_open_close() == AtsType::OrderOpenClose::Close || it.second->get_open_close() == AtsType::OrderOpenClose::CloseToday) && it.second->get_way() == OrderWay::Sell)
									{
										it.second->Cancel();
									}
								}
							}
						}
					}

					if (o->get_open_close() == AtsType::OrderOpenClose::Close)
					{
						if (i->get_pending_long_close_qty() >= execQty)
						{
							i->set_pending_long_close_qty(i->get_pending_long_close_qty() - execQty);

						}
						if (!onlyUpdatePending)
						{

							i->set_tot_short_position(i->get_tot_short_position() - execQty > 0 ? i->get_tot_short_position() - execQty : 0);
							//if (m_bCloseToday)


							if (i->get_close_order() == CLOSE_PRIORITY::FIX)
								i->set_yst_short_position(i->get_yst_short_position() - execQty > 0 ? i->get_yst_short_position() - execQty : 0);
							else
							{
								if (i->get_close_order() == CLOSE_PRIORITY::YST_FIRST)
								{
									int yst_closed_pos = min(i->get_yst_short_position(), execQty);
									i->set_yst_short_position(i->get_yst_short_position() - yst_closed_pos);
									i->set_today_short_position(i->get_today_short_position() - execQty + yst_closed_pos > 0 ? i->get_today_short_position() - execQty + yst_closed_pos : 0);
								}
							}

							/*if (o->get_open_close() == AtsType::OrderOpenClose::CloseToday)
							i->set_today_short_position(i->get_today_short_position() - execQty > 0 ? i->get_today_short_position() - execQty : 0);*/
						}

					}

					if (o->get_open_close() == AtsType::OrderOpenClose::CloseToday)
					{
						if (i->get_pending_long_close_today_qty() >= execQty)
						{
							i->set_pending_long_close_today_qty(i->get_pending_long_close_today_qty() - execQty);

						}
						if (!onlyUpdatePending)
						{
							i->set_tot_short_position(i->get_tot_short_position() - execQty > 0 ? i->get_tot_short_position() - execQty : 0);
							if (i->get_close_order() == CLOSE_PRIORITY::FIX)
								i->set_today_short_position(i->get_today_short_position() - execQty > 0 ? i->get_today_short_position() - execQty : 0);
							else
							{
								if (i->get_close_order() == CLOSE_PRIORITY::YST_FIRST)
								{
									int yst_closed_pos = min(i->get_yst_short_position(), execQty);
									i->set_yst_short_position(i->get_yst_short_position() - yst_closed_pos);
									i->set_today_short_position(i->get_today_short_position() - execQty + yst_closed_pos > 0 ? i->get_today_short_position() - execQty + yst_closed_pos : 0);
								}
							}

							/*if (o->get_open_close() == AtsType::OrderOpenClose::CloseToday)
							i->set_today_short_position(i->get_today_short_position() - execQty > 0 ? i->get_today_short_position() - execQty : 0);*/
						}

					}

				}

				break;
				case AtsType::OrderWay::Sell:
				{
					//i->set_today_short_position(i->get_today_short_position() + execQty);

					if (o->get_open_close() == AtsType::OrderOpenClose::Open)
					{
						if (!onlyUpdatePending)
						{
							i->set_tot_short_position(i->get_tot_short_position() + execQty);
							i->set_today_short_position(i->get_today_short_position() + execQty);
							i->set_closed_position(i->get_closed_position() + execQty);

							if (i->getInstrument()->get_not_close_today())
							{
								for (auto& it : m_activeOrders)
								{
									if (it.second->get_instrument()->getCode() == i->getCode() && (it.second->get_open_close() == AtsType::OrderOpenClose::Close || it.second->get_open_close() == AtsType::OrderOpenClose::CloseToday) && it.second->get_way() == OrderWay::Buy)
									{
										it.second->Cancel();
									}
								}
							}
						}

					}

					if (o->get_open_close() == AtsType::OrderOpenClose::Close)
					{
						if (i->get_pending_short_close_qty() >= execQty)
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - execQty);

						if (!onlyUpdatePending)
						{
							i->set_tot_long_position(i->get_tot_long_position() - execQty > 0 ? i->get_tot_long_position() - execQty : 0);
							//if (m_bCloseToday)

							/*  if (o->get_open_close() == AtsType::OrderOpenClose::CloseToday)
							i->set_today_long_position(i->get_today_long_position() - execQty > 0 ? i->get_today_long_position() - execQty : 0);*/

							if (i->get_close_order() == CLOSE_PRIORITY::FIX)
								i->set_yst_long_position(i->get_yst_long_position() - execQty > 0 ? i->get_yst_long_position() - execQty : 0);
							else
							{
								if (i->get_close_order() == CLOSE_PRIORITY::YST_FIRST)
								{
									int yst_closed_pos = min(i->get_yst_long_position(), execQty);
									i->set_yst_long_position(i->get_yst_long_position() - yst_closed_pos);
									i->set_today_long_position(i->get_today_long_position() - execQty + yst_closed_pos > 0 ? i->get_today_long_position() - execQty + yst_closed_pos : 0);
								}
							}
						}

					}
					if (o->get_open_close() == AtsType::OrderOpenClose::CloseToday)
					{
						if (i->get_pending_short_close_today_qty() >= execQty)
							i->set_pending_short_close_today_qty(i->get_pending_short_close_today_qty() - execQty);

						if (!onlyUpdatePending)
						{
							i->set_tot_long_position(i->get_tot_long_position() - execQty > 0 ? i->get_tot_long_position() - execQty : 0);

							if (i->get_close_order() == CLOSE_PRIORITY::FIX)
								i->set_today_long_position(i->get_today_long_position() - execQty > 0 ? i->get_today_long_position() - execQty : 0);
							else
							{
								if (i->get_close_order() == CLOSE_PRIORITY::YST_FIRST)
								{
									int yst_closed_pos = min(i->get_yst_long_position(), execQty);
									i->set_yst_long_position(i->get_yst_long_position() - yst_closed_pos);
									i->set_today_long_position(i->get_today_long_position() - execQty + yst_closed_pos > 0 ? i->get_today_long_position() - execQty + yst_closed_pos : 0);
								}
							}

							/*if (o->get_open_close() == AtsType::OrderOpenClose::CloseToday)
							i->set_today_long_position(i->get_today_long_position() - execQty > 0 ? i->get_today_long_position() - execQty : 0);*/
						}

					}


				}
				break;
				case AtsType::OrderWay::CoveredSell: //备兑开仓 成交
				{

					if (i->get_pending_covered_sell_open_qty() >= execQty)
						i->set_pending_covered_sell_open_qty(i->get_pending_covered_sell_open_qty() - execQty);

					//decrease the frozon position
					if (i->get_underlying_frozen_long_position() >= execQty*i->get_point_value())
						i->set_underlying_frozen_long_position(int(i->get_underlying_frozen_long_position() - execQty*i->get_point_value()));

					//update covered sell open qty
					i->set_covered_sell_open_position(i->get_covered_sell_open_position() + execQty);

				}
				break;
				case AtsType::OrderWay::CoveredBuy: //备兑平仓 成交
				{
					if (i->get_pending_covered_sell_close_qty() >= execQty)
						i->set_pending_covered_sell_close_qty(i->get_pending_covered_sell_close_qty() - execQty);

					//increase frozen position
					i->set_underlying_frozen_long_position(int(i->get_underlying_frozen_long_position() + execQty*i->get_point_value()));

					//decrease covered sell open qty
					if (i->get_covered_sell_open_position() >= execQty)
						i->set_covered_sell_open_position(i->get_covered_sell_open_position() - execQty);

				}
				break;
				case AtsType::OrderWay::ETFRed:
				{
					if (i->get_instr_type() == AtsType::InstrType::Stock) //qty will increase
					{
						i->set_tot_long_position(i->get_tot_long_position() + execQty);
						i->set_today_long_position(i->get_today_long_position() + execQty);
						i->set_open_position(i->get_open_position() + execQty);
						i->set_today_purred_qty(i->get_today_purred_qty() + execQty);
					}

					else if (i->get_instr_type() == AtsType::InstrType::ETF)
					{
						if (i->get_pending_short_close_qty() >= execQty)
						{
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - execQty);
						}

						//i->set_today_long_position(i->get_today_long_position() - execQty);
						//i->set_tot_long_position(i->get_tot_long_position() - execQty);

						i->set_today_short_position(i->get_today_short_position() + execQty);
						i->set_tot_short_position(i->get_tot_short_position() + execQty);

						if (i->get_today_purred_qty() >= execQty)
							i->set_today_purred_qty(i->get_today_purred_qty() - execQty);

					}

				}
				break;


				case AtsType::OrderWay::ETFPur: //to test
				{
					if (i->get_instr_type() == AtsType::InstrType::ETF)
					{
						i->set_tot_long_position(i->get_tot_long_position() + execQty);
						i->set_today_long_position(i->get_today_long_position() + execQty);
						i->set_open_position(i->get_open_position() + execQty);
						i->set_today_purred_qty(i->get_today_purred_qty() + execQty);
					}
					else if (i->get_instr_type() == AtsType::InstrType::Stock) //qty will decrease
					{
						if (i->get_pending_short_close_qty() >= execQty)
						{
							i->set_pending_short_close_qty(i->get_pending_short_close_qty() - execQty);
						}
						//i->set_today_long_position(i->get_today_long_position() - execQty);
						//i->set_tot_long_position(i->get_tot_long_position() - execQty);

						i->set_tot_short_position(i->get_tot_short_position() + execQty);
						i->set_today_short_position(i->get_today_short_position() + execQty);

						//if (i->get_today_purred_qty() >= execQty)
						//{
						//	i->set_today_purred_qty(i->get_today_purred_qty() - execQty);
						//}

						//todo compute sellable qty


					}
				}
				break;

				case AtsType::OrderWay::Freeze:
				case AtsType::OrderWay::Unfreeze: //for underlying only
				{
					//not going to happen.
				}
				break;
				default:
					break;
				}

				if (m_debug)
					i->dumpinfo();

			}

			bool ctpbase_connection::compute_userId(order* o, char* userID, int n)
			{
				//int nAccountNum = getAccountNum(o->get_account().c_str());
				int nAccountNum = o->get_account_num();
				int nUserOrderId = o->get_user_orderid();
				int nPortfolio = getPortfolioNum(o->get_portfolio().c_str());
				int nInternalRef = o->get_id();
				if (nPortfolio > 99 || nPortfolio <= 0)
				{
					loggerv2::error("nPortfolio :%d is bigger than 99", nPortfolio);
					return false;
				}
				if (o->get_trading_type() > 9)
				{
					loggerv2::error("tradetype :%d is bigger than 9", o->get_trading_type());
					return false;
				}
				snprintf(userID, n, "%2x%5x%5x%1d%2x", nAccountNum, nUserOrderId, nInternalRef, o->get_trading_type(), nPortfolio);
				return true;
			}

			bool ctpbase_connection::compute_userId(quote* o, char* userID, int n)
			{
				//int nAccountNum = getAccountNum(o->get_account().c_str());
				int nAccountNum = o->get_account_num();
				//int nUserOrderId = o->get_user_orderid();
				int bidOrderID = o->get_bid_order()->get_id();
				int nPortfolio = getPortfolioNum(o->get_portfolio().c_str());
				int askOrderID = o->get_ask_order()->get_id();
				if (nPortfolio > 99 || nPortfolio <= 0)
				{
					loggerv2::error("nPortfolio :%d is bigger than 99", nPortfolio);
					return false;
				}
				if (o->get_trading_type() > 9)
				{
					loggerv2::error("tradetype :%d is bigger than 9", o->get_trading_type());
					return false;
				}
				snprintf(userID, n, "%2x%5x%5x%1d%2x", nAccountNum, bidOrderID, askOrderID, o->get_trading_type(), nPortfolio);
				return true;
			}


			void ctpbase_connection::load_instruments_type(const std::string& name, const std::string& strConfigFile, terra::common::abstract_database* db, InstrType::type instType)
			{
				if (strConfigFile.length() < 1)
					return;

				boost::filesystem::path p(strConfigFile);
				if (!boost::filesystem::exists(p))
				{
					printf_ex("ats_config::load config_file:%s not exist!\n", strConfigFile.c_str());
					return;
				}
				boost::property_tree::ptree root;
				boost::property_tree::ini_parser::read_ini(strConfigFile, root);



				std::string cmd = "select Code,ConnectionCodes, Exchange, ISIN, TickRule, ClassName,";

				switch (instType)
				{
				case InstrType::Stock:
					cmd += (instType == InstrType::Stock) ? " PointValue from Stocks, InstrumentClass where Stocks.InstrumentClass=InstrumentClass.ClassName" : "";
					break;
				case InstrType::Option:
					cmd += (instType == InstrType::Option) ? "  Options.PointValue, Maturity,Underlying,Strike, CallPut from Options,InstrumentClass where Options.InstrumentClass=InstrumentClass.ClassName" : "";
					break;
				case InstrType::Future:
					cmd += (instType == InstrType::Future) ? " PointValue, Maturity from Futures, InstrumentClass where Futures.InstrumentClass=InstrumentClass.ClassName" : "";
					break;
				case InstrType::ETF:
					cmd += (instType == InstrType::ETF) ? " PointValue from ETFs, InstrumentClass where ETFs.InstrumentClass=InstrumentClass.ClassName" : "";
					break;
				case InstrType::Forex:
					cmd += (instType == InstrType::Forex) ? " PointValue from Forex, InstrumentClass where Forex.InstrumentClass=InstrumentClass.ClassName" : "";
					break;
				default:
					return;
					break;
				}



				std::string sStkExchgs = root.get<string>(name + "." + _InstrType_VALUES_TO_NAMES.at(instType), "");
				if (!sStkExchgs.empty())
				{

					std::vector<std::string> exChgs;
					boost::split(exChgs, sStkExchgs, boost::is_any_of("|"));
					std::string appendCmd = " and (Exchange=";
					while (!exChgs.empty())
					{
						appendCmd += '\'';
						appendCmd += exChgs.back();
						appendCmd += '\'';
						if (exChgs.size() > 1)
							appendCmd += " or Exchange=";
						exChgs.pop_back();
					}
					appendCmd += ")";
					cmd += appendCmd;
				}

				std::vector<boost::property_tree::ptree>* ptS = db->get_table(cmd.c_str());

				//create tradeitem now
				for (auto it : *ptS)//std::vector<boost::property_tree::ptree>::iterator it = ptS->begin(); it != ptS->end(); ++it)//此处不可使用for（auto it:cotain）这样的语言
				{
					std::string sInst = it.get("Code", "");
					std::string stemp = it.get("ConnectionCodes", "");
					terra::instrument::financialinstrument *cfi = instrument::referential::get_instance()->get_instrument_map().get_by_key(sInst);
					if (cfi == nullptr)
					{
						printf_ex("ctpbase_connection::load_instruments_type didn't find the %s\n", sInst.c_str());
						continue;
					}
					tradeitem* u = nullptr;

					if (stemp.empty())
						continue;
					std::string temp = abstract_database::get_item(stemp, m_code_type);
					if (temp.empty())
					{
						continue;
					}

					std::vector<std::string> tmpVec2;
					boost::split(tmpVec2, temp, boost::is_any_of("."));

					std::string tradingcde = tmpVec2[0];
					//tradingcde += "@XS2";

					std::string mktname = std::string(it.get("Exchange", ""));
					//std::string mktname = "SH";
					if (m_code_type == "XS")
					{
						if (mktname == "SSE")
							mktname = "SH";
						else if (mktname == "SZE")
							mktname = "SZ";
					}


					AtsType::InstrType::type iType = instType;
					if (instType == InstrType::Option)
					{
						//std::string sCallPut = it.get("CallPut", "");
						if (it.get("CallPut", "") == "C")
							iType = AtsType::InstrType::Call;
						else
							iType = AtsType::InstrType::Put;

						std::string sOptionClass = it.get("ClassName", "");
						std::size_t pos = sOptionClass.find("_");      // position of "live" in str
						std::string sUnd = sOptionClass.substr(pos + 1);

						std::string sUndCode = sUnd + "@" + getName();
						u = tradeitem_gh::get_instance().container().get_by_key(sUndCode.c_str());
					}

					/*std::string name =;*/
					string key = sInst + "@" + getName();
					if (tradeitem_gh::get_instance().container().get_by_key(key.c_str()) == nullptr)
					{
						tradeitem *i = new tradeitem(cfi, getName(), this, sInst, tradingcde, iType, mktname, m_bKey_with_exchange);
						i->setMarket(mktname);
						if (u)
							i->set_underlying(u);

						/*std::string secKey = tradingcde + "." + mktname + "@" + getName();
						i->set_second_key(secKey.c_str());*/

						tradeitem_gh::get_instance().container().add(i);
					}
				}
				loggerv2::info("%s load_instruments_type %s , tradeitem num %d", getName().c_str(), _InstrType_VALUES_TO_NAMES.at(instType), ptS->size());

			}

			void ctpbase_connection::load_instruments(const std::string& name, const std::string& ini, const char* sqlfile)
			{
				if (m_database == nullptr)
				{
					m_database = database_factory::create("sqlite", sqlfile);
				}
				m_database->open_database();
				load_instruments_type(name, ini, m_database, InstrType::Stock);
				load_instruments_type(name, ini, m_database, InstrType::ETF);
				load_instruments_type(name, ini, m_database, InstrType::Future);
				load_instruments_type(name, ini, m_database, InstrType::Option);
				//
				load_instruments_type(name, ini, m_database, InstrType::Forex);
				//
				m_database->close_databse();
			}

			void ctpbase_connection::process_outbound_order(order* o)
			{
				if (o != nullptr)
				{
					if (!is_busy() || AtsType::OrderAction::Cancelled == o->get_last_action())
					{
						switch (o->get_last_action())
						{
						case AtsType::OrderAction::Created:
							if (market_create_order_async(o, outReason) == 0)
							{
								on_nack_from_market_cb(o, outReason,false);
							}

							else
								process_should_pending(o);
							break;

						case AtsType::OrderAction::Cancelled:
							if (market_cancel_order_async(o, outReason) == 0)
							{
								on_nack_from_market_cb(o, outReason, false);
							}
							break;

						default:
							loggerv2::error("%s::process_outbound_msg_cb - unknown last_action %d", getName().c_str(), o->get_last_action());
							break;
						}
					}
					else
					{
						on_nack_from_market_cb(o, "Connection is Busy", false);
					}
				}

				lwtp now = get_lwtp_now();
				if (*m_send_it == lw_min_time || (now - *m_send_it) > std::chrono::milliseconds(m_maxSendInterval * 1000))
				{
					*m_send_it = now;
					m_send_it++;
					if (m_send_it == m_send_list.end())
					{
						m_send_it = m_send_list.begin();
					}

					m_isbusy = false;
				}
				else
				{
					*m_send_it = now;
					m_isbusy = true;
				}

			}

			void ctpbase_connection::process_outbound_quote(quote* q)
			{
				if (q != nullptr)
				{
					if (!is_busy() || AtsType::OrderAction::Cancelled == q->get_last_action())
					{
						switch (q->get_last_action())
						{
						case AtsType::OrderAction::Created:
							if (market_create_quote_async(q, outReason) == 0)
							{
								on_nack_quote_from_market_cb(q, outReason);
							}
							else
							{
								process_should_pending(q->get_ask_order());
								process_should_pending(q->get_bid_order());
							}
							break;

						case AtsType::OrderAction::Cancelled:
							if (market_cancel_quote_async(q, outReason) == 0)
							{
								on_nack_quote_from_market_cb(q, outReason);
							}
							break;

						default:
							loggerv2::error("%s::process_outbound_quote - unknown last_action %d", getName().c_str(), q->get_last_action());
							break;
						}
					}
					else
					{
						on_nack_quote_from_market_cb(q, "Connection is Busy");
					}
				}

				lwtp now = get_lwtp_now();
				if (*m_send_it == lw_min_time || (now - *m_send_it) > std::chrono::milliseconds(m_maxSendInterval * 1000))
				{
					*m_send_it = now;
					m_send_it++;
					if (m_send_it == m_send_list.end())
					{
						m_send_it = m_send_list.begin();
					}

					m_isbusy = false;
				}
				else
				{
					*m_send_it = now;
					m_isbusy = true;
				}

			}

			void ctpbase_connection::release()
			{
				stop_process();
			}




			void ctpbase_connection::cancel_num_warning(tradeitem* i)
			{
				loggerv2::warn("tradeitem:%s cancel num is more than warning lev", i->getCode().c_str());
				this->setTradingAllowed(false);
			}

			void ctpbase_connection::cancel_num_ban(tradeitem* i)
			{

				loggerv2::warn("tradeitem:%s cancel num is more than forbid lev", i->getCode().c_str());
				this->setTradingAllowed(false);
				i->set_cancel_forbid(true);

			}


		}
	}
}
