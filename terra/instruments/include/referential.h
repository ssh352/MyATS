#ifndef _REFERENTIAL_H_
#define _REFERENTIAL_H_
#pragma once
#include "etf.h"
#include "underlying.h"
#include "optionclass.h"
#include "option.h"
#include "currency.h"
#include "abstract_database.h"
#include "financialinstrument.h"
using namespace terra::common;
namespace terra
{
	namespace instrument
	{
		class future;
		class futureclass;
		class stock;
		class stockclass;
		class index;
		class etf;
		class forex;
		class forexclass;
		/*
		refer to
		1.D:\codes\Terra\Instrument\Referential.cs
		*/
		class referential
		{
		private:
			referential(){}
			~referential();
			static referential* g_instance;
		public:
			static referential * get_instance()
			{
				if (g_instance == nullptr)
				{
					g_instance = new referential();
				}
				return g_instance;
			}
			bool load(std::string configPath, std::string jsonFile, std::string dbFile);
			void save_underlying(underlying* pUnderlying);
			void save_instrument_class(instrumentclass * pClass);
			void show();
			void set_referential_directory(string value){ m_strConfigPath = value; }
			string get_referential_directory(){ return m_strConfigPath; }
			unordered_map_ex<std::string, underlying*>  &  get_underlying_map(){ return m_underlying_map; }
			unordered_map_ex<std::string, optionclass*> &  get_optionclass_map(){ return m_option_class_map; }
			unordered_map_ex<std::string, option*>      &  get_option_map(){ return m_option_instrument_map; }
			unordered_map_ex<std::string, futureclass*> &  get_future_class_map(){ return m_future_class_map; }
			unordered_map_ex<std::string, future*>      &  get_future_map(){ return m_future_instrument_map; }
			unordered_map_ex<std::string, stockclass*>  &  get_stock_class_map(){ return m_stock_class_map; }
			unordered_map_ex<std::string, stock*>       &  get_stock_map(){ return m_stock_instrument_map; }
			unordered_map_ex<std::string, forexclass*>  &  get_forex_class_map(){ return m_forex_class_map; }
			unordered_map_ex<std::string, forex*>       &  get_forex_map(){ return m_forex_instrument_map; }
			unordered_map_ex<std::string, etf*>         &  get_etf_map(){ return m_etf_instrument_map; }
			unordered_map_ex<std::string, currency*>    &  get_currency_map(){ return m_currency_map; }
			unordered_map_ex<std::string, index*>       &  get_index_map(){ return m_index_map; }
			unordered_map_ex<std::string, instrumentclass*> & get_instrument_class_map(){ return m_instrument_class_map; }
			unordered_map_ex<std::string, financialinstrument*> & get_instrument_map(){ return m_financial_instrument_map; }
		public:
			bool load_currencies(std::string & jsonFile);
			bool save_currencies();
			bool load_future(std::string & dbFile);
			bool load_stocks(std::string & dbFile);
			bool load_forex(std::string & dbFile);
			bool load_etfs(std::string & dbFile);
			bool load_option(std::string & dbFile);
			bool load_indexes(std::string & dbFile);
			bool load_index_components(std::string & dbFile);
			bool load_etf_components(std::string & dbFile);
			void update_common_instrument(financialinstrument * pInstrument, boost::property_tree::ptree& it);
			bool load_underlying(underlying * pUnderlying);
			bool load_instrument_class(instrumentclass * pClass);
			string get_db_file(){ return m_strDbFile; }
			std::string & get_config_path()
			{
				return m_strConfigPath;
			}
			//void create_index(abstract_database* db, string & strUnderLyingName);
			void create_etf_component(abstract_database* db, etf * pETF);
		protected:
			string                                   m_strDbFile;
			std::string                              m_strConfigPath;
			std::string m_str_currency_file;
			unordered_map_ex<std::string, underlying*>          m_underlying_map;
			unordered_map_ex<std::string, optionclass*>         m_option_class_map;
			unordered_map_ex<std::string, option*>              m_option_instrument_map;

			unordered_map_ex<std::string, futureclass*>         m_future_class_map;
			unordered_map_ex<std::string, future*>              m_future_instrument_map;

			unordered_map_ex<std::string, stockclass*>          m_stock_class_map;
			unordered_map_ex<std::string, stock*>               m_stock_instrument_map;
			unordered_map_ex<std::string, forexclass*>          m_forex_class_map;
			unordered_map_ex<std::string, forex*>               m_forex_instrument_map;
			unordered_map_ex<std::string, etf*>                 m_etf_instrument_map;

			unordered_map_ex<std::string, currency*>            m_currency_map;


			unordered_map_ex<std::string, index*>               m_index_map;

			unordered_map_ex<std::string, instrumentclass*>     m_instrument_class_map;
			unordered_map_ex<std::string, financialinstrument*> m_financial_instrument_map;
		};
	}
}
#endif //_REFERENTIAL_H_


