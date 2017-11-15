#include <vector>
#include "terra_logger.h"
#include "referential.h"
#include "database_factory.h"
#include "tickrule.h"
#include "future.h"
#include "futureclass.h"
#include "stock.h"
#include "stockclass.h"
#include "currency.h"
#include "index.h"
#include "forex.h"
#include "forexclass.h"
#include <boost/property_tree/json_parser.hpp>  
#include <boost/filesystem.hpp>
using namespace terra::common;
using namespace boost::property_tree;
namespace terra
{
	namespace instrument
	{
		referential::~referential()
		{

			for (auto &it : m_currency_map)
			{
				delete it.second;
			}


			for (auto &it : m_future_instrument_map)
			{
				delete it.second;
			}

			for (auto &it : m_future_class_map)
			{
				delete it.second;
			}


			for (auto &it : m_stock_instrument_map)
			{
				delete it.second;
			}

			for (auto &it : m_stock_class_map)
			{
				delete it.second;
			}


			for (auto &it : m_etf_instrument_map)
			{
				delete it.second;
			}


			for (auto &it : m_option_instrument_map)
			{
				delete it.second;
			}

			for (auto &it : m_option_class_map)
			{
				delete it.second;
			}


			for (auto &it : m_index_map)
			{
				delete it.second;
			}

			for (auto &it : m_index_map)
			{
				delete it.second;
			}
		}
		bool referential::load(std::string strConfigPath, std::string jsonFile, std::string dbFile)
		{
			m_strDbFile = dbFile;
			boost::filesystem::path p(dbFile);
			if (!boost::filesystem::exists(p))
			{
				loggerv2::error("referential::load dbfile:%s not exist!", dbFile.c_str());
				return false;
			}
			m_strConfigPath = strConfigPath;
			load_currencies(jsonFile);
			m_str_currency_file = jsonFile;
			load_forex(dbFile);
			load_future(dbFile);
			load_stocks(dbFile);
			load_etfs(dbFile);
			load_option(dbFile);
			load_indexes(dbFile);
			load_etf_components(dbFile);
			load_index_components(dbFile);
			for (auto & iter : m_underlying_map)
			{
				load_underlying(iter.second);
			}
			for (auto & iter : m_stock_class_map)
			{
				load_instrument_class(iter.second);
			}
			return true;
		}
		bool referential::load_instrument_class(instrumentclass * pClass)
		{
			if (pClass == nullptr)
				return false;
			string strFileName = this->get_config_path() + "/" + pClass->get_name() + ".json";
			boost::filesystem::path p(strFileName);
			if (!boost::filesystem::exists(p))
			{
				loggerv2::error("referential::load_instrument_class file:%s not exist!", strFileName.c_str());
				return false;
			}
			pClass->load(strFileName);
			return true;
		}
		bool referential::load_underlying(underlying * pUnderlying)
		{
			if (pUnderlying == nullptr)
				return false;
			string strFileName = this->get_config_path() + "/" + pUnderlying->get_name() + ".json";

			boost::filesystem::path p(strFileName);
			if (!boost::filesystem::exists(p))
			{
				loggerv2::error("referential::load_underlying file:%s not exist!", strFileName.c_str());
				return false;
			}

			pUnderlying->load(strFileName);


			for (auto &it : pUnderlying->get_option_class_list())
			{
				loggerv2::info("referential::load_underlying underlying:%s, optionclass:%s", pUnderlying->get_name().c_str(), it.second->get_name().c_str());
				load_instrument_class(it.second);
			}

			for (auto &it : pUnderlying->get_future_class_list())
			{
				loggerv2::info("referential::load_underlying underlying:%s, pfutureclass:%s", pUnderlying->get_name().c_str(), it.second->get_name().c_str());
				load_instrument_class(it.second);
			}
			return true;
		}

		bool referential::load_currencies(std::string & jsonFile)
		{
			boost::filesystem::path p(jsonFile);
			if (!boost::filesystem::exists(p))
			{
				loggerv2::info("referential::load_currencies file:%s not exist!", jsonFile.c_str());
				return false;
			}

			boost::property_tree::ptree root,target;
			boost::property_tree::read_json(jsonFile, root);
			try
			{
				target = root.get_child("List");
			}
			catch (std::exception &ex)
			{
				cout << ex.what() << endl;
				target = root;
			}


			for (auto& it : target)
			{
				string  strName = it.second.get<string>("Name");
				currency * pCurrency = new currency(strName, it.second.get<double>("ToReference", 1));
				m_currency_map[strName] = pCurrency;
			}

			return true;
		}

		bool referential::save_currencies()
		{
			boost::filesystem::path p(m_str_currency_file);
			if (!boost::filesystem::exists(p))
			{
				loggerv2::info("referential::load_currencies file:%s not exist!", m_str_currency_file.c_str());
				return false;
			}

			boost::property_tree::ptree pt_root;
			boost::property_tree::ptree child,children;
			for (auto & v : this->m_currency_map)
			{
				child.clear();
				child.put("Name", v.first);
				child.put("ToReference", v.second->get_to_reference());
				children.push_back(std::make_pair("", child));
			}
			pt_root.add_child("List", children);

			boost::property_tree::write_json(m_str_currency_file, pt_root);

			return true;
		}

		bool referential::load_future(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					std::string sCmd = "select * from Futures join InstrumentClass on Futures.InstrumentClass=InstrumentClass.ClassName";
					std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
					if (res != nullptr)
					{
						for (auto& it : *res)
						{
							std::string code = it.get("Code", "");
							if (m_financial_instrument_map.contain_key(code) == true)
								continue;
							//1.create the option
							future * pFutureInstr = new future(code);
							if (pFutureInstr)
							{
								update_common_instrument(pFutureInstr, it);
								//std::string strMaturity = it.get("Maturity", "");
								//pFutureInstr->get_maturity().setDate(strMaturity);							
								pFutureInstr->set_maturity(it.get("Maturity", ""));
								this->m_future_instrument_map[pFutureInstr->get_code()] = pFutureInstr;
								m_financial_instrument_map.add(pFutureInstr->get_code(), pFutureInstr);

								//2.create the underlying class if the underlying not exist														
								std::string strUnderLying = it.get("Underlying", "");
								underlying * pUnderlying = this->m_underlying_map.get_by_key(strUnderLying);
								if (pUnderlying == nullptr)
								{
									// Underlying(string name)
									pUnderlying = new underlying(strUnderLying);
									this->m_underlying_map[pUnderlying->get_name()] = pUnderlying;
								}

								//3.create the option instrument class if the option class not exist
								std::string  strinstrumentclass = it.get("InstrumentClass", "");
								futureclass * pfutureclass = this->m_future_class_map.get_by_key(strinstrumentclass);
								if (pfutureclass == nullptr)
								{
									std::string strCurrency = it.get("Currency", "");
									currency * pCurrency = this->m_currency_map.get_by_key(strCurrency);

									//std::string strPonitValue = it.get("PointValue", "");
									int iPointValue = it.get<int>("PointValue", 1);
									pFutureInstr->set_point_value(iPointValue);
									pfutureclass = new futureclass(pUnderlying, strinstrumentclass, iPointValue, pCurrency);
									this->m_future_class_map[pfutureclass->get_name()] = pfutureclass;
									m_instrument_class_map.add(pfutureclass->get_name(), pfutureclass);

									std::string strtickrule = it.get("TickRule", "");
									tickrule * ptickrule = new tickrule(strtickrule);
									pfutureclass->set_tick_rule(ptickrule);
								}
								///4.create the relation 
								pUnderlying->append(pfutureclass);
								pFutureInstr->set_class(pfutureclass);
								pfutureclass->add(pFutureInstr);
							}
						}
						delete res;
						res = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::load_future Can't open database: %s", dbFile.c_str());
			}
			return false;
		}
		bool referential::load_stocks(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					//std::string sCmd = "select * from Stocks join Instrumentclass on Stocks.InstrumentClass=InstrumentClass.ClassName where InstrumentClass='ChinaStock'";
					std::string sCmd = "select * from Stocks join Instrumentclass on Stocks.InstrumentClass=InstrumentClass.ClassName where InstrumentClass=InstrumentClass.ClassName";
					std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
					if (res != nullptr)
					{
						for (auto& it : *res)
						{
							std::string code = it.get("Code", "");
							if (m_financial_instrument_map.contain_key(code) == true)
								continue;
							//1.create the option
							stock * pStockInstr = new stock(code);
							if (pStockInstr)
							{
								update_common_instrument(pStockInstr, it);
								this->m_stock_instrument_map[pStockInstr->get_code()] = pStockInstr;
								m_financial_instrument_map.add(pStockInstr->get_code(), pStockInstr);

								//3.create the option instrument class if the option class not exist
								std::string  strinstrumentclass = it.get("InstrumentClass", "");
								stockclass * pStockClass = this->m_stock_class_map.get_by_key(strinstrumentclass);
								if (pStockClass == nullptr)
								{
									std::string strCurrency = it.get("Currency", "");
									currency * pCurrency = this->m_currency_map.get_by_key(strCurrency);

									//std::string strPonitValue = it.get("PointValue", "");
									int iPointValue = it.get<int>("PointValue", 1);
									pStockInstr->set_point_value(iPointValue);
									pStockClass = new stockclass(strinstrumentclass, iPointValue, pCurrency);
									this->m_stock_class_map[pStockClass->get_name()] = pStockClass;

									std::string strtickrule = it.get("TickRule", "");
									tickrule * ptickrule = new tickrule(strtickrule);
									pStockClass->set_tick_rule(ptickrule);
								}
								///4.create the relation 							
								pStockInstr->set_class(pStockClass);
								pStockClass->add(pStockInstr);
							}
						}
						delete res;
						res = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::loadStock Can't open database: %s", dbFile.c_str());
			}
			return false;
		}

		bool referential::load_forex(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					std::string sCmd = "select * from Forex join Instrumentclass on Forex.InstrumentClass=InstrumentClass.ClassName";
					std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
					if (res != nullptr)
					{
						for (auto& it : *res)
						{
							std::string code = it.get("Code", "");
							if (m_financial_instrument_map.contain_key(code) == true)
								continue;
							//1.create the option
							forex * pForexInstr = new forex(code);
							if (pForexInstr)
							{
								update_common_instrument(pForexInstr, it);
								this->m_forex_instrument_map[pForexInstr->get_code()] = pForexInstr;
								m_financial_instrument_map.add(pForexInstr->get_code(), pForexInstr);

								//3.create the option instrument class if the option class not exist
								std::string  strinstrumentclass = it.get("InstrumentClass", "");
								forexclass * pInstClass = this->m_forex_class_map.get_by_key(strinstrumentclass);
								if (pInstClass == nullptr)
								{
									std::string strCurrency = it.get("Currency", "");
									currency * pCurrency = this->m_currency_map.get_by_key(strCurrency);

									//std::string strPonitValue = it.get("PointValue", "");
									int iPointValue = it.get<int>("PointValue", 1);
									pForexInstr->set_point_value(iPointValue);
									pInstClass = new forexclass(strinstrumentclass, iPointValue, pCurrency);
									this->m_forex_class_map[pInstClass->get_name()] = pInstClass;

									std::string strtickrule = it.get("TickRule", "");
									tickrule * ptickrule = new tickrule(strtickrule);
									pInstClass->set_tick_rule(ptickrule);
								}
								///4.create the relation 							
								pForexInstr->set_class(pInstClass);
								pInstClass->add(pForexInstr);
							}
						}
						delete res;
						res = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::loadForex Can't open database: %s", dbFile.c_str());
			}
			return false;
		}


		bool referential::load_etfs(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					std::string sCmd = "select * from ETFs join InstrumentClass on ETFs.InstrumentClass=InstrumentClass.ClassName";
					std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
					if (res != nullptr)
					{
						for (auto& it : *res)
						{
							std::string code = it.get("Code", "");
							if (m_financial_instrument_map.contain_key(code) == true)
								continue;
							//1.create the option
							etf * pETFInstr = new etf(code);
							if (pETFInstr)
							{
								update_common_instrument(pETFInstr, it);
								this->m_etf_instrument_map[pETFInstr->get_code()] = pETFInstr;
								m_financial_instrument_map[pETFInstr->get_code()] = pETFInstr;
								//2.create the underlying class if the underlying not exist														
								std::string strUnderLying = pETFInstr->get_code();
								underlying * pUnderlying = this->m_underlying_map.get_by_key(strUnderLying);
								if (pUnderlying == nullptr)
								{
									// Underlying(string name)
									pUnderlying = new underlying(strUnderLying);
									this->m_underlying_map[pUnderlying->get_name()] = pUnderlying;
								}

								//3.create the option instrument class if the option class not exist
								std::string  strinstrumentclass = it.get("InstrumentClass", "");
								stockclass * pStockClass = this->m_stock_class_map.get_by_key(strinstrumentclass);
								if (pStockClass == nullptr)
								{
									std::string strCurrency = it.get("Currency", "");
									currency * pCurrency = this->m_currency_map.get_by_key(strCurrency);

									//std::string strPonitValue = it.get("PointValue", "");
									int iPointValue = it.get<int>("PointValue", 1);
									pETFInstr->set_point_value(iPointValue);
									pStockClass = new stockclass(strinstrumentclass, iPointValue, pCurrency);
									this->m_stock_class_map[pStockClass->get_name()] = pStockClass;

									m_instrument_class_map.add(pStockClass->get_name(), pStockClass);
									std::string strtickrule = it.get("TickRule", "");
									tickrule * ptickrule = new tickrule(strtickrule);
									pStockClass->set_tick_rule(ptickrule);
								}
								///4.create the relation 				
								pUnderlying->set(pETFInstr);
								pETFInstr->set_class(pStockClass);
							}
						}
						delete res;
						res = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::loadETF Can't open database: %s", dbFile.c_str());
			}
			return false;
		}
		void referential::update_common_instrument(financialinstrument * pInstrument, boost::property_tree::ptree& it)
		{
			if (pInstrument == nullptr)
				return;
			pInstrument->set_exchange(it.get("Exchange", ""));
			pInstrument->set_isin(it.get("ISIN", ""));
			pInstrument->set_ric(it.get("RIC", ""));
			std::string strCurrency = it.get("Currency", "");
			currency * pCurrency = this->m_currency_map.get_by_key(strCurrency);
			pInstrument->set_currency(pCurrency);
			std::string strFeedCodes = it.get("FeedCodes", "");
			std::string strTradingCodes = it.get("ConnectionCodes", "");
			pInstrument->set_feed_codes(strFeedCodes);
			pInstrument->set_trading_code(strTradingCodes);
			string strTickRule = it.get("TickRule", "");
			pInstrument->set_tick_rule(new tickrule(strTickRule));
		}
		/*
		1.根据option记录，创建option类
		2.然后根据option记录中的instrumentclass创建class类
		3.再根据option记录中的UnderLying字段创建underlying类
		4.同时建立option类，class类和underlying类的关系
		*/
		bool referential::load_option(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					std::string sCmd = "select *, Options.pointValue as instrumentPointValue, Instrumentclass.pointValue as classPointValue from Options join InstrumentClass on Options.InstrumentClass = InstrumentClass.ClassName";
					std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
					if (res != nullptr)
					{
						for (auto& it : *res)
						{
							std::string code = it.get("Code", "");
							if (this->m_financial_instrument_map.contain_key(code) == true)
								continue;
							//1.create the option
							option * pOptionInstr = new option(code);
							if (pOptionInstr)
							{
								update_common_instrument(pOptionInstr, it);

								//std::string strMaturity = it.get("Maturity", "");
								//pOptionInstr->get_maturity().setDate(strMaturity);
								pOptionInstr->set_maturity(it.get("Maturity", ""));
								//std::string strPonitValue = it.get("instrumentPointValue", "");
								pOptionInstr->set_point_value(it.get<int>("instrumentPointValue", 1));
								//std::string strStrike = it.get("Strike", "");
								pOptionInstr->set_strike(it.get<double>("Strike", 0));
								/*std::string strCallType = it.get("CallPut", "");
								if (strCallType == "C") pOptionInstr->set_type(InstrType::Call);
								if (strCallType == "P")	pOptionInstr->set_type(InstrType::Put);*/
								it.get("CallPut", "") == "C" ? pOptionInstr->set_type(AtsType::InstrType::Call) : pOptionInstr->set_type(AtsType::InstrType::Put);

								this->m_option_instrument_map[pOptionInstr->get_code()] = pOptionInstr;
								m_financial_instrument_map.add(pOptionInstr->get_code(), pOptionInstr);

								//2.create the underlying class if the underlying not exist														
								std::string strUnderLying = it.get("Underlying", "");
								underlying * pUnderlying = this->m_underlying_map.get_by_key(strUnderLying);
								if (pUnderlying == nullptr)
								{
									// Underlying(string name)
									pUnderlying = new underlying(strUnderLying);
									this->m_underlying_map[pUnderlying->get_name()] = pUnderlying;
								}

								//3.create the option instrument class if the option class not exist
								std::string  strinstrumentclass = strUnderLying + "_" + it.get("InstrumentClass", "");
								optionclass * poptionclass = this->m_option_class_map.get_by_key(strinstrumentclass);
								if (poptionclass == nullptr)
								{
									//std::string strCurrency = it.get("Currency", "");
									currency * pCurrency = this->m_currency_map.get_by_key(it.get("Currency", ""));

									//std::string strPonitValue = it.get<int>("classPointValue", 1);
									//int iPointValue = it.get<int>("classPointValue", 1);
									poptionclass = new optionclass(pUnderlying, strinstrumentclass, it.get<int>("classPointValue", 1), pCurrency);
									this->m_option_class_map[poptionclass->get_name()] = poptionclass;
									m_instrument_class_map.add(poptionclass->get_name(), poptionclass);

									std::string strtickrule = it.get("TickRule", "");
									tickrule * ptickrule = new tickrule(strtickrule);
									poptionclass->set_tick_rule(ptickrule);
								}
								///4.create the relation 
								pUnderlying->append(poptionclass);
								pOptionInstr->set_class(poptionclass);
								poptionclass->add(pOptionInstr);
							}
						}
						delete res;
						res = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::load_option Can't open database: %s", dbFile.c_str());
			}
			return false;
		}

		//void referential::create_index(abstract_database* db, string & strUnderLyingName)
		//{
		//	if (db == nullptr)
		//		return;
		//	underlying * pUnderLying = nullptr;
		//	if (this->m_underlying_map.contain_key(strUnderLyingName) == true)
		//	{
		//		pUnderLying = this->m_underlying_map.get_by_key(strUnderLyingName);
		//	}
		//	else
		//	{
		//		//loggerv2::error("referential::create_index didn't find the underlying: %s", strUnderLyingName.c_str());
		//		return;
		//	}
		//	index * pIndex = new index(pUnderLying, strUnderLyingName);
		//	this->m_index_map[strUnderLyingName] = pIndex;
		//	pUnderLying->set(pIndex);

		//	std::string sCmd = "select * from Indexes where Underlying like '" + strUnderLyingName + "'";
		//	std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
		//	double divisor = 1;
		//	if (res != nullptr)
		//	{
		//		for (auto& it : *res)
		//		{
		//			std::string strCode = it.get("Code", "");
		//			double nbShare = it.get<double>("NShare", 1);
		//			if (strCode == "divisor")
		//			{
		//				divisor = nbShare;
		//			}
		//			else
		//			{
		//				if (this->m_stock_instrument_map.contain_key(strCode) == true)
		//				{
		//					stock * pStock = this->m_stock_instrument_map[strCode];
		//					pIndex->add(pStock, nbShare);
		//				}
		//			}
		//		}
		//	}
		//	pIndex->set_divisor(divisor);
		//	delete res;
		//	res = nullptr;
		//	//
		//}
		bool referential::load_indexes(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					std::string sCmd = "select * from Indexes join InstrumentClass on Indexes.InstrumentClass=InstrumentClass.ClassName";
					std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
					if (res != nullptr)
					{
						for (auto& it : *res)
						{
							std::string code = it.get("Code", "");
							//1.create the option
							index * pIndex = new index(code);
							if (pIndex)
							{
								update_common_instrument(pIndex, it);
								this->m_index_map[pIndex->get_code()] = pIndex;
								m_financial_instrument_map[pIndex->get_code()] = pIndex;
								//2.create the underlying class if the underlying not exist														
								std::string strUnderLying = it.get("Underlying", "");;
								underlying * pUnderlying = this->m_underlying_map.get_by_key(strUnderLying);
								if (pUnderlying == nullptr)
								{
									// Underlying(string name)
									pUnderlying = new underlying(strUnderLying);
									this->m_underlying_map[pUnderlying->get_name()] = pUnderlying;
								}

								//3.create the option instrument class if the option class not exist
								std::string  strinstrumentclass = it.get("InstrumentClass", "");
								stockclass * pStockClass = this->m_stock_class_map.get_by_key(strinstrumentclass);
								if (pStockClass == nullptr)
								{
									std::string strCurrency = it.get("Currency", "");
									currency * pCurrency = this->m_currency_map.get_by_key(strCurrency);

									//std::string strPonitValue = it.get("PointValue", "");
									int iPointValue = it.get<int>("PointValue", 1);
									pIndex->set_point_value(iPointValue);
									pStockClass = new stockclass(strinstrumentclass, iPointValue, pCurrency);
									this->m_stock_class_map[pStockClass->get_name()] = pStockClass;

									std::string strtickrule = it.get("TickRule", "");
									tickrule * ptickrule = new tickrule(strtickrule);
									pStockClass->set_tick_rule(ptickrule);
								}
								///4.create the relation 				
								pUnderlying->set(pIndex);
								pIndex->set_underlying(pUnderlying);
								pIndex->set_class(pStockClass);
							}
						}
						delete res;
						res = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::loadETF Can't open database: %s", dbFile.c_str());
			}
			return false;
		}

		bool referential::load_index_components(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database())
				{
					std::string sCmd = "select distinct(Underlying) from IndexesComponents";
					std::vector<boost::property_tree::ptree>* resUnd = db->get_table(sCmd.c_str());
					if (resUnd != nullptr)
					{
						for (auto& it : *resUnd)
						{
							std::string strUnderlying = it.get("Underlying", "");

							//create_index(db, strUnderlying);
							//if (db == nullptr)
							//	return;
							underlying * pUnderLying = nullptr;
							if (this->m_underlying_map.contain_key(strUnderlying) == true)
							{
								pUnderLying = this->m_underlying_map.get_by_key(strUnderlying);
							}
							else
							{
								//loggerv2::error("referential::create_index didn't find the underlying: %s", strUnderLyingName.c_str());
								continue;
							}
							index * pIndex = pUnderLying->get_index();


							std::string sCmd = "select * from IndexesComponents where Underlying like '" + strUnderlying + "'";
							std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
							double divisor = 1;
							if (res != nullptr)
							{
								for (auto& it : *res)
								{
									std::string strCode = it.get("Code", "");
									double nbShare = it.get<double>("NShare", 1);
									if (strCode == "divisor")
									{
										divisor = nbShare;
									}
									else
									{
										if (this->m_stock_instrument_map.contain_key(strCode) == true)
										{
											stock * pStock = this->m_stock_instrument_map[strCode];
											pIndex->add(pStock, nbShare);
										}
									}
								}
							}
							pIndex->set_divisor(divisor);
		
							delete res;
							res = nullptr;
						}
						delete resUnd;
						resUnd = nullptr;
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::load_index Can't open database: %s", dbFile.c_str());
			}
			return false;
		}

		void referential::create_etf_component(abstract_database* db, etf * pETF)
		{
			if (pETF == nullptr || db == nullptr)
			{
				return;
			}
			std::string sCmd = "select * from ETFComponents where ETF like '" + pETF->get_code() + "'";
			std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
			if (res != nullptr)
			{
				for (auto& it : *res)
				{
					std::string strCode = it.get("Code", "");
					//std::string strShares = it.get("Shares", "");
					//std::string strCashReplaceAmt = it.get("CashReplaceAmt", "");
					//std::string strMustCashReplace = it.get("MustCashReplace", "");
					if (strCode == "Cash")
					{
						pETF->set_cash_diff_per_unit(it.get<double>("CashReplaceAmt", 0));
					}
					else if (strCode == "UnitSize")
					{
						pETF->set_unit_size(it.get<int>("Shares", 0));
					}
					else
					{
						stock * pStock = nullptr;
						if (this->m_stock_instrument_map.contain_key(strCode) == true)
						{
							pStock = m_stock_instrument_map.get_by_key(strCode);
							pETF->add(new etfcomponent(pStock, it.get<int>("Shares", 0), it.get<int>("MustCashReplace", 0), it.get<double>("CashReplaceAmt", 0)));
						}
						else
						{
							loggerv2::error("referential::create_etf_component didn't find the code: %s", strCode.c_str());
						}

					}
				}
			}
			delete res;
			res = nullptr;
		}
		bool referential::load_etf_components(std::string & dbFile)
		{
			abstract_database* db = database_factory::create("sqlite", dbFile.c_str());
			if (db)
			{
				if (db->open_database() == true)
				{
					for (auto& it : this->m_etf_instrument_map)
					{
						create_etf_component(db, it.second);
					}
					db->close_databse();
				}
				delete db;
				db = nullptr;
			}
			else
			{
				loggerv2::error("referential::load_etf_components Can't open database: %s", dbFile.c_str());
			}
			return false;
		}
		void referential::save_underlying(underlying* pUnderlying)
		{
			if (pUnderlying == nullptr)
				return;
			boost::filesystem::path p;
			p.clear();
			p.append(get_referential_directory());
			string strFile = pUnderlying->get_name() + ".json";
			p.append(strFile);
			strFile = p.string();
			pUnderlying->save(strFile);
			for (auto & v : pUnderlying->get_future_class_list())
			{
				this->save_instrument_class(v.second);
			}
			for (auto & v : pUnderlying->get_option_class_list())
			{
				this->save_instrument_class(v.second);
			}
		}
		void referential::save_instrument_class(instrumentclass * pClass)
		{
			if (pClass == nullptr)
				return;
			boost::filesystem::path p;
			p.clear();
			p.append(get_referential_directory());
			string strFile = pClass->get_name() + ".json";
			p.append(strFile);
			strFile = p.string();
			pClass->save(strFile);
		}
		void referential::show()
		{
			loggerv2::info("referential::show enter");
			loggerv2::info("referential::show m_currency_map.size:%d", m_currency_map.size());
			for (auto & iter : m_currency_map)
			{
				loggerv2::info("referential::show m_currency_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_future_instrument_map.size:%d", m_future_instrument_map.size());
			for (auto & iter : m_future_instrument_map)
			{
				loggerv2::info("referential::show m_future_instrument_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_future_class_map.size:%d", m_future_class_map.size());
			for (auto & iter : m_future_class_map)
			{
				loggerv2::info("referential::show m_future_class_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_stock_instrument_map.size:%d", m_stock_instrument_map.size());
			for (auto & iter : m_stock_instrument_map)
			{
				loggerv2::info("referential::show m_stock_instrument_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_stock_class_map.size:%d", m_stock_class_map.size());
			for (auto & iter : m_stock_class_map)
			{
				loggerv2::info("referential::show m_stock_class_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_etf_instrument_map.size:%d", m_etf_instrument_map.size());
			for (auto & iter : m_etf_instrument_map)
			{
				loggerv2::info("referential::show m_instrument_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_option_instrument_map.size:%d", m_option_instrument_map.size());
			for (auto & iter : m_option_instrument_map)
			{
				loggerv2::info("referential::show m_option_instrument_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_option_class_map.size:%d", m_option_class_map.size());
			for (auto & iter : m_option_class_map)
			{
				loggerv2::info("referential::show m_option_class_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_index_map.size:%d", m_index_map.size());
			for (auto & iter : m_index_map)
			{
				loggerv2::info("referential::show m_index_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show m_underlying_map.size:%d", m_underlying_map.size());
			for (auto & iter : m_underlying_map)
			{
				loggerv2::info("referential::show m_underlying_map----------------------------------");
				iter.second->show();
			}
			loggerv2::info("referential::show end");
		}





		referential* referential::g_instance = nullptr;

	}
}
