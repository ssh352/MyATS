#include "order_security_controller.h"
#include "feedsource.h"
#include "terra_logger.h"
#include "string_tokenizer.h"
#include "quantity_security_rule.h"
#include "max_nominal_security_rule.h"
#include "frequency_security_rule.h"
#include "deviation_security_rule.h"
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace terra
{
	namespace ats
	{
		namespace security
		{
			bool order_security_controller::control(ats_instrument* ats_instr, order* o, char* pszReason)
			{
				//return true;

				// check rules
				std::list<iorder_security_rule*>& rules = m_rules[o->get_instrument()->getCode()][o->get_instrument()->get_instr_type()];
				if (rules.size() == 0)
				{
					snprintf(pszReason, 256, "no security rules defined for mnemo[%s] instr[%c]",
						o->get_instrument()->getCode().data(),
						_InstrType_VALUES_TO_NAMES.at(o->get_instrument()->get_instr_type())
						);
					return false;
				}

				std::string feedsource = o->get_instrument()->getInstrument()->get_trading_codes().begin()->first;
				// get feedItem
				terra::feedcommon::feed_item* feedItem = ats_instr->get_feed_item();

				if (feedItem == NULL)
				{
					snprintf(pszReason, 256, "order_security_controller - feed_item %s not found", feedsource.c_str());
					return false;
				}


				// check each rule
				for(iorder_security_rule* rule:rules)
				{
					if (!rule->is_order_safe(o, feedItem, pszReason))
						return false;
				}
				return true;
			}

			void order_security_controller::load(const char* filename)
			{
				boost::filesystem::path p(filename);
				if (!boost::filesystem::exists(p))
				{
					loggerv2::error("order_security_controller::load - file [%s] does not exist", filename);
					return;
				}

				boost::filesystem::ifstream stream;
				stream.open(p);
				if (stream.bad() || stream.fail())
					return;

				std::string line;
				terra::common::string_tokenizer<1024> tokenizer;

				while (stream.good())
				{
					std::getline(stream, line);
					if (line.length() == 0 || line[0] == '#')
						continue;

					tokenizer.break_line(line.c_str(), ' ');
					tokenizer.remove_empty_tokens();

					int n = tokenizer.size();
					if (n < 7)
						continue;

					std::string instr = tokenizer[0];
					AtsType::InstrType::type type1 = AtsType::InstrType::Undef;
					AtsType::InstrType::type type2 = AtsType::InstrType::Undef;
					if (instr == "FUTURE")
						type1 = AtsType::InstrType::Future;
					else if (instr == "STOCK")
						type1 = AtsType::InstrType::Stock;
					else if (instr == "OPTION")
					{
						type1 = AtsType::InstrType::Call;
						type2 = AtsType::InstrType::Put;
					}
					else
						continue;
					const char* mnemo = tokenizer[1];


					// quantity_security_rule
					m_rules[mnemo][type1].push_back(new quantity_security_rule(mnemo, type1, atoi(tokenizer[2])));
					if (type2 != AtsType::InstrType::Undef)
						m_rules[mnemo][type2].push_back(new quantity_security_rule(mnemo, type2, atoi(tokenizer[2])));


					// nominal_security_rule
					m_rules[mnemo][type1].push_back(new max_nominal_security_rule(mnemo, type1, atof(tokenizer[3])));
					if (type2 != AtsType::InstrType::Undef)
						m_rules[mnemo][type2].push_back(new max_nominal_security_rule(mnemo, type2, atof(tokenizer[3])));


					// frequency - keep the same rule for call / put
					frequency_security_rule* freqRule = new frequency_security_rule(mnemo, type1, atoi(tokenizer[4]), atoi(tokenizer[5]));
					m_rules[mnemo][type1].push_back(freqRule);
					if (type2 != AtsType::InstrType::Undef)
						m_rules[mnemo][type2].push_back(freqRule);


					// deviation_security_rule
					deviation_security_rule* devRule1 = new deviation_security_rule(mnemo, type1, DEV_LAST_CLOSE_BIDASK);
					for (unsigned int i = 7; i < tokenizer.size(); i = i + 2)
						devRule1->add_deviation(atof(tokenizer[i - 1]), atof(tokenizer[i]));
					m_rules[mnemo][type1].push_back(devRule1);

					if (type2 != AtsType::InstrType::Undef)
					{
						deviation_security_rule* devRule2 = new deviation_security_rule(mnemo, type2, DEV_LAST_CLOSE_BIDASK);
						for (unsigned int i = 7; i < tokenizer.size(); i = i + 2)
							devRule2->add_deviation(atof(tokenizer[i - 1]), atof(tokenizer[i]));
						m_rules[mnemo][type2].push_back(devRule2);
					}
				}
				stream.close();
			}
		}
	}
}
