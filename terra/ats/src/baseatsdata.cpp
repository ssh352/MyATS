#include "baseatsdata.h"
#include "string_tokenizer.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "atsinstrument.h"
#include "abstractats.h"
#include <position_persister.h>
#include "portfolio_gh.h"
#include "tradeItem_gh.h"
#include "referential.h"
using namespace terra::common;
using namespace terra::marketaccess::orderpassing;
namespace terra
{
	namespace ats
	{
		base_ats_data::base_ats_data(abstract_ats * m_ats) :m_ats(m_ats)
		{
		}


		base_ats_data::~base_ats_data()
		{
		}
		void base_ats_data::save(std::string atsDirectory)
		{
			save_trading_periods(atsDirectory);
			if (m_ats == nullptr)
				return;
			boost::filesystem::path p;
			p.clear();
			p.append(atsDirectory);
			string filename = "InstrumentDatas";
			filename.append(".csv");
			p.append(filename);

			string file = p.string();

			boost::filesystem::ofstream stream;
			stream.open(file.c_str());

			char buffer[256];
			
			for (auto & it : m_ats->AtsInstrumentList)
			{
				memset(buffer, 0, sizeof(buffer));
				if (it->get_instrument() != nullptr )
				{					
					if (it->get_position() != nullptr)
					{
						if (it->get_position()->get_yesterday_position_local() != 0 || math2::not_zero(it->get_position()->get_yesterday_price_local()) == true)
						{
							sprintf(buffer, "%s,%d,%s\n", it->get_instrument()->get_code().c_str(), it->get_position()->get_yesterday_position_local(), math2::round_ex(it->get_position()->get_yesterday_price_local(), 4).c_str());
							stream << buffer;
						}
					}				
				}
				
			}
			stream.close();
			terra::instrument::referential::get_instance()->save_currencies();
		}
		void base_ats_data::load(std::string atsDirectory)
		{				
			load_trading_periods(atsDirectory);
			if (m_ats == nullptr)
				return;
			boost::filesystem::path p;
			p.clear();
			p.append(atsDirectory);
			string filename = "InstrumentDatas";
			filename.append(".csv");
			p.append(filename);

			string file = p.string();

			boost::filesystem::ifstream stream;
			if (!boost::filesystem::exists(p))
				return;
			stream.open(file.c_str());
			string_tokenizer<1024> tokenizer;
			const char* szSeparators = ", ";
			std::string line;

			instrument_data data;

			while (stream.good())
			{
				std::getline(stream, line);
				if (line.length() == 0 || line[0] == '#')
					continue;

				tokenizer.break_line(line.c_str(), szSeparators);
				if (tokenizer.size() < 3)
					continue;

				data.code = tokenizer[0];
				data.YesterdayPosLocal = atoi(tokenizer[1]);
				data.YesterdayPrcLocal = atof(tokenizer[2]);

				ats_instrument * inst = m_ats->find(data.code);
				if (inst != nullptr && inst->get_position() != nullptr)
				{
					inst->get_position()->set_yesterday_position_local(data.YesterdayPosLocal);
					inst->get_position()->set_yesterday_price_local(data.YesterdayPrcLocal);		
					//inst->get_position()->set_yesterday_position_external(position_persister::);
				}
			}
			stream.close();
		}

		void base_ats_data::save_trading_periods(std::string atsDirectory)
		{
			std::string section;
			std::string filename;

			section = "tradingphase";
			filename = atsDirectory + "/" + section + ".json";
			m_ats->TradingPeriodManager.save(filename);
		}

		void base_ats_data::load_trading_periods(std::string atsDirectory)
		{
			std::string section;
			std::string filename;

			section = "tradingphase";
			filename = atsDirectory + "/" + section + ".json";
			try
			{
				m_ats->TradingPeriodManager.load(filename);
			}
			catch (std::exception &ex)
			{
				std::cout << "load_trading_periods error:" << ex.what() << std::endl;
				loggerv2::error("load_trading_periods error:%s", ex.what());
			}
		}

		void base_ats_data::load_yst_position()
		{
			auto pos_hmap = portfolio_gh::get_instance().get_postion_external();
			for (auto & it : m_ats->AtsInstrumentList)
			{
				std::string code = it->get_instrument()->get_code();
				auto itr = pos_hmap->find(code);
				if (itr == pos_hmap->end())
					continue;
				
				for (auto ite : itr->second)
				{
					std::string porfiolio = ite.first;

					portfolio* pPortfolio = portfolio_gh::get_instance().container().get_by_name(porfiolio);
					if (pPortfolio == NULL)
					{
						pPortfolio = new portfolio(porfiolio.c_str());
						portfolio_gh::get_instance().container().add(pPortfolio);
					}

					std::string tkey = code + "@" + it->get_connection_name();
					tradeitem* instr = tradeitem_gh::get_instance().container().get_by_key(tkey.c_str());
					if (instr == NULL)
						continue;

					position* pPosition = pPortfolio->get_position(instr);
					if (pPosition == NULL)
						return;

					int pos = boost::lexical_cast<int>(ite.second);
					//pPosition->set_yesterday_position_type(AtsType::YesterdayPositionType::External);
					pPosition->set_yesterday_position_external(pos);
				}
			}
		}
	}
}
