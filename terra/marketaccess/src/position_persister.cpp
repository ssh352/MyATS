#include "position_persister.h"
#include "position.h"
#include "tradeitem.h"
#include "tradeItem_gh.h"
#include "portfolio_gh.h"
#include "terra_logger.h"
#include "string_tokenizer.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/tokenizer.hpp>
using namespace terra::common;
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			void position_persister::load(const char* filename)
			{
				boost::filesystem::path p(filename);
				if (!boost::filesystem::exists(p))
				{
					loggerv2::error("position_persister::load - file [%s] does not exist", filename);
					return;
				}

				//
				// TEMP, find  better way to do it...
				//
				time_t t = boost::filesystem::last_write_time(p);
				//date_time dt(t);

				
				//ptime d = from_time_t(t);
				//ptime d = from_time_t(boost::filesystem::last_write_time(p));
				//if (d.date() != day_clock::local_day())
				//date d1 = d.date();
				//date d1(dt.get_year(), dt.get_month(), dt.get_day());
				date d1 = from_time_t(t).date();
				date d2 = day_clock::local_day();
				loggerv2::info("d1 %s", to_iso_string(d1).c_str()); // nok, gmt
				loggerv2::info("d2 %s", to_iso_string(d2).c_str()); // ok

				if (d1 != d2)
				{
					loggerv2::error("position_persister::load - file [%s] is obsolete", filename /*File.GetLastWriteTime(filename).Date*/);
					return;
				}

				boost::filesystem::ifstream stream;
				stream.open(p);
				if (stream.bad() || stream.fail())
					return;

				std::string line;
				terra::common::string_tokenizer<1024> tokenizer;
				const char* szSeparators = "\",";
				//typedef boost::tokenizer<boost::char_separator<char> > tokenizer2;
				//boost::char_separator<char> separators("\",;\t ");
				//tokenizer2 tokens(line);

				while (stream.good())
				{
					std::getline(stream, line);
					if (line.length() == 0 || line[0] == '#')
						continue;

					tokenizer.break_line(line.c_str(), szSeparators);
					tokenizer.remove_empty_tokens();
					//tokens.assign(line, separators);

					int n = tokenizer.size();
					if (n < 3)
						continue;

					std::string ptf_name = tokenizer[0];
					std::string isin = tokenizer[1];


					tradeitem* instr = tradeitem_gh::get_instance().container().get_by_second_key(isin.c_str());
					if (instr == NULL)
						continue;


					portfolio* ptf = portfolio_gh::get_instance().container().get_by_name(ptf_name.c_str());
					if (ptf == NULL)
					{
						ptf = new portfolio(ptf_name.c_str());
						portfolio_gh::get_instance().container().add(ptf);
					}

					position* p = ptf->get_position(instr);

					double d = atof(tokenizer[2]) /*/ i->get_quantity_factor()*/;
					p->set_yesterday_position_external(d);

					loggerv2::info("position_persister::load - ptf [%s] [%s] [%.0f]", ptf_name.c_str(), isin.c_str(), d);
				}
				stream.close();
			}
		}
	}

}