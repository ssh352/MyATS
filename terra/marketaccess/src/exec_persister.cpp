#include "exec_persister.h"
#include "tradeItem_gh.h"
#include "portfolio_gh.h"
#include "exec_gh.h"
#include "position.h"
#include "string_tokenizer.h"
#include "AtsType_types.h"
#include "terra_logger.h"
#include "connection.h"

using namespace terra::common;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			exec_persister* exec_persister::ms_pInstance = NULL;

			exec_persister* exec_persister::instance()
			{
				if (ms_pInstance == NULL)
					ms_pInstance = new exec_persister();
				return ms_pInstance;
			}

			exec_persister::exec_persister()
			{
				m_directoryName = "";
				m_fullName = "";
				m_isAlive = true;
				
				for (auto it = _OrderWay_VALUES_TO_NAMES.begin(); it != _OrderWay_VALUES_TO_NAMES.end(); ++it)
				{
					_OrderWay_name_to_value.insert(std::make_pair(it->second, it->first));
				}
				//for (auto it = _TradingType_VALUES_TO_NAMES.begin(); it != _TradingType_VALUES_TO_NAMES.end(); ++it)
				//{
				//	_TradingType_name_TO_value.insert(std::make_pair(it->second, it->first));
				//}
				for (auto it = _OrderOpenClose_VALUES_TO_NAMES.begin(); it != _OrderOpenClose_VALUES_TO_NAMES.end(); ++it)
				{
					_OrderOpenClose_name_TO_value.insert(std::make_pair(it->second, it->first));
				}
			}

			exec_persister::~exec_persister()
			{
				
			}

			void exec_persister::set_directory_name(const char* pszDirectoryName)
			{
				m_directoryName = pszDirectoryName;
				m_fullName = m_directoryName + "/deals.csv";
			}

			bool exec_persister::is_alive()
			{
				bool b;
				writeLock wlock(m_mutex);
				{
					b = m_isAlive;
				}
				wlock.unlock();
				return b;
			}

			void exec_persister::is_alive(bool b)
			{
				writeLock wlock(m_mutex);
				{
					m_isAlive = b;
				}
				wlock.unlock();
			}

			void exec_persister::add_exec(exec* e)
			{
				if (e == NULL)
					return;

				//writeLock wlock(m_mutex);
				//{
					m_queue.Push(e);
				//}
				///wlock.unlock();
			}


			bool exec_persister::load(std::string &connection_name)
			{
				boost::filesystem::path p(m_fullName.c_str());
				if (boost::filesystem::exists(p))
				{
					boost::filesystem::ifstream stream(p);
					if (stream.bad() || stream.fail())
						return false;

					loggerv2::info("exec_persister::load - file [%s]", m_fullName.c_str());

					std::string line;
					while (stream.good())
					{
						std::getline(stream, line);
						if (line.length() == 0 || line[0] == '#')
							continue;

						//loggerv2::info("loading exec %s", line.c_str());

						exec* e = string_to_exec(line, connection_name);
						if (e != NULL)
						{
							
							if (exec_gh::get_instance().GetBook().contains(e->getReference().c_str()))
							{
								loggerv2::error("exec_persister::string_to_exec: dup Reference:%s", e->getReference().c_str());
								delete e;
								continue;
							}
							e->is_persisted(true);
							load_exec(e);
							m_exec_vec.push_back(line);
						}
					}
					stream.close();
				}
				else
				{
					loggerv2::error("exec_persister::load - file [%s] fail", m_fullName.c_str());
					return false;
				}
				return true;
			}

			exec* exec_persister::string_to_exec(std::string & pszLine, std::string &connection_name)
			{
				std::vector<std::string> tokenizer;
				boost::split(tokenizer, pszLine, boost::is_any_of(","));
				int n = tokenizer.size();
				if (n < 15)
				{
					loggerv2::error("exec_persister::string_to_exec: line not properly formatted, expecting 13 or 14 fields");
					return NULL;
				}

				if (boost::trim_right_copy(tokenizer[14]) != connection_name)
					return NULL;

				int orderId = atoi(tokenizer[1].data());
				int quantity = atoi(tokenizer[2].data());
				double price = atof(tokenizer[3].data());

				AtsType::OrderWay::type way = AtsType::OrderWay::Undef;
				auto iway = _OrderWay_name_to_value.find(tokenizer[4]);
				if (iway != _OrderWay_name_to_value.end())
					way = (AtsType::OrderWay::type)iway->second;
				if (way == AtsType::OrderWay::Undef)
				{
					loggerv2::error("exec_persister::string_to_exec: unknown order_way [%s] for exec [%s]", tokenizer[4].data(), tokenizer[8].data());
					return NULL;
				}

				int tradingType = atoi(tokenizer[5].data());
				if (tradingType == 0)
				{
					loggerv2::error("exec_persister::string_to_exec: unknown trading_type [%s] for exec [%s]", tokenizer[5].data(), tokenizer[8].data());
					return NULL;
				}

				std::string porf = tokenizer[6];
				if (porf == "UNKNOWN")
				{
					loggerv2::error("exec_persister::string_to_exec: Portfolio is unknown");
					return NULL;
				}

				AtsType::OrderOpenClose::type ord_oc = AtsType::OrderOpenClose::Undef;
				auto itr2 = _OrderOpenClose_name_TO_value.find(tokenizer[9]);
				if (itr2 != _OrderOpenClose_name_TO_value.end())
					ord_oc = (AtsType::OrderOpenClose::type)itr2->second;

				string trade_name = std::string(tokenizer[0]) + "@" + connection_name;
				tradeitem* i = tradeitem_gh::get_instance().container().get_by_key(trade_name.c_str());
				if (i == NULL)
				{
					loggerv2::error("exec_persister::string_to_exec: instrument [%s] not defined for exec [%s]", tokenizer[0].data(), tokenizer[8].data());
					return NULL;
				}
				// 合约代码,orderid,成交量，成交价格，买卖，tradingtype，portfolio， 时间, 成交流水号 ,  开平标志，投机套保标志,date,account
				std::string Reference = tokenizer[8];
				if (Reference.back() == 'b' || Reference.back() == 's')
					Reference.erase(Reference.end()-1);

				return new exec(orderId, i, way, tradingType, tokenizer[6].data(), Reference, quantity, price, tokenizer[7].data(), ord_oc, (OrderHedge)atoi(tokenizer[10].data()), atoi(tokenizer[12].data()));
			}

			void exec_persister::load_exec(exec* e)
			{
				if (e == NULL)
					return;

				portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(e->getPortfolioName());
				if (pPortfolio == NULL)
				{
					pPortfolio = new portfolio(e->getPortfolioName().c_str());
					portfolio_gh::get_instance().container().add(pPortfolio);
				}

				position* pPosition = pPortfolio->get_position(e->getTradeItem());
				if (pPosition == NULL)
					return;

				pPosition->add_exec(e);

				// add exec to the GH (so that later we can check if it has been serialized already)
				exec_gh::get_instance().GetBook().add(e);
				// Serialize exec
				//add_deal(pDeal);
			}

			bool exec_persister::start()
			{
				// create OutputFile
				boost::filesystem::path p(m_fullName.c_str());
				boost::filesystem::create_directories(p.parent_path());
				//m_ofstream.open(p, std::ios::out | std::ios::trunc);
				m_ofstream.open(p, std::ios::app);

				if (m_ofstream.bad() || m_ofstream.fail() || !m_ofstream)
				{
					loggerv2::error("exec_persister::start - error opening output file...");
					printf_ex("exec_persister::start - error opening output file...\n");
					//return false;
				}


				// start Persister Thread
				//RTThread::SetName("exec_persister");
				//SetPriority = ThreadPriority.BelowNormal;
				std::thread t1(boost::bind(&exec_persister::Process, this));
				m_thread.swap(t1);

				return true;
			}

			bool exec_persister::stop()
			{
				// Terminate Thread
				is_alive(false);
				m_thread.join();

				// close File
				m_ofstream.close();

				return true;
			}

			void exec_persister::Process()
			{
				while (is_alive())
				{
					sleep_by_milliseconds(1000);

					//if (!m_queue.m_queue.read_available()>0)
					//{
					//	int elementPop = 0;
					//	while (m_queue.m_queue.read_available()>0 && elementPop < 500)
					//	{
					//		persist_exec(m_queue.Pop());
					//		++elementPop;
					//	}
					//	m_ofstream.flush();
					//}

					int elementPop = 0;
					exec* e = m_queue.Pop();
					while (e != nullptr && elementPop<500)
					{
						persist_exec(e);
						++elementPop;
						e = m_queue.Pop();
					}
				}


				// Thread is closing: flush remaining messages
				//writeLock wlock2(m_mutex);
				exec* e = m_queue.Pop();
				while (e != nullptr)
				{
					persist_exec(e);
					e = m_queue.Pop();
				}
				m_ofstream.flush();
				//wlock2.unlock();
			}

			void exec_persister::persist_exec(exec* e)
			{
				if (e == nullptr)
					return;
				char szLine[256];
				if (exec_to_string(e, szLine))
				{
					if (m_ofstream.good())
					{
					m_ofstream << szLine << std::endl;
				}
					//
					if (m_ofstream.bad() || m_ofstream.fail() || !m_ofstream)
					{
						loggerv2::error("exec_persister::persist_exec,%s",szLine);					
					}
					//
				}

				/*//if (m_referencesSerialized.find(pDeal->get_reference()) == m_referencesSerialized.end())
				if (m_execSerialized.find(e->get_reference()) == m_execSerialized.end())
				{
				if (exec_to_string(e, szLine))
				{
				//loggerv2::info("Persist exec %s", pDeal->get_reference());

				m_ofstream << szLine << std::endl;
				//m_referencesSerialized.insert(pDeal->get_reference());
				m_execSerialized[e->get_reference()] = e;
				}
				}*/
			}

			bool exec_persister::exec_to_string(exec* e, char* szLine)
			{
				// 合约代码,orderid,成交量，，成交价格，，买卖，tradingtype，portfolio， 时间, 成交流水号 ,  开平标志，投机套保标志,交易所

				/*sprintf(szLine, "%s;%d;%d;%.*f;%s;%d;%s;%s;%s",
				  e->get_instrument()->get_isin_code(),
				  e->get_order_id(),
				  e->get_quantity(),
				  e->get_instrument()->get_precision(), e->get_price(),
				  order_way_to_string(e->get_order_way()),
				  e->get_trading_type(),
				  e->get_portfolio_name(),
				  e->get_time(),
				  e->get_reference()
				  );*/

				//terra::common::date_time date(time(NULL))/*.set_time(0, 0, 1)*/; //set time to 00:00:01 of today.
				//std::string sdate = date.get_string(terra::common::date_time::FN2);
				//std::string sdate = to_iso_string(day_clock::local_day());
				//std::string Oway = "Undef"; 
				//auto it = _OrderWay_VALUES_TO_NAMES.find(e->getWay());
				//if (it != _OrderWay_VALUES_TO_NAMES.end())
				//	Oway = it->second;

				//std::string Trtpye = "Unknown";
				//auto it2 = _TradingType_VALUES_TO_NAMES.find(e->getTradingType());
				//if (it2 != _TradingType_VALUES_TO_NAMES.end())
				//	Trtpye = it2->second;

				//std::string oc = "Undef";
				//auto it3 = _OrderOpenClose_VALUES_TO_NAMES.find(e->getOpenClose());
				//if (it3 != _OrderOpenClose_VALUES_TO_NAMES.end())
				//	oc = it3->second;

				sprintf(szLine, "%s,%d,%d,%f,%s,%d,%s,%s,%s,%s,%d,%s,%s,%d,%s",
					e->getTradeItem()->getCode().c_str(),
					e->getOrderId(),
					e->getQuantity(),
					//e->getTradeItem()->get_precision(), 
					e->getPrice(),
					_OrderWay_VALUES_TO_NAMES.at(e->getWay()),
					e->getTradingType(),
					e->getPortfolioName().c_str(),
					e->get_time().c_str(),
					e->getReference().c_str(),
					_OrderOpenClose_VALUES_TO_NAMES.at(e->getOpenClose()),
					e->get_hedge(),
					e->getTradeItem()->getInstrument()->get_exchange().c_str(),//e->getTradeItem()->get_market()->get_name(),
					to_iso_string(day_clock::local_day()).c_str(),
					e->get_account_num(),
					e->getTradeItem()->getConnection()->getName().c_str()
					);
				std::string str = szLine;
				m_exec_vec.push_back(str);
				return true;
			}
		}
	}
}
