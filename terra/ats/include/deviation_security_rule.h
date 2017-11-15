#ifndef __DEVIATION_SECURITY_RULE2_H__
#define __DEVIATION_SECURITY_RULE2_H__

#include <list>
#include <string>
#include "iorder_security_rule.h"
#include "feeditem.h"
namespace terra
{
	namespace ats
	{
		namespace security
		{
			enum deviation_type
			{
				DEV_LAST_CLOSE_BIDASK,
				DEV_LAST_CLOSE,
				DEV_LAST
			};

			class deviation_security_rule : public iorder_security_rule
			{
			public:
				deviation_security_rule(const char* mnemo, AtsType::InstrType::type t, deviation_type deviationType);
				virtual ~deviation_security_rule() {}

				virtual const char* get_description() { return m_description.c_str(); }

				virtual bool is_order_safe(order* o, terra::feedcommon::feed_item* f, char* pszReason);


				void add_deviation(double min, double step);
				double get_deviation(double price);


			protected:
				deviation_type m_deviationType;
				std::list<std::pair<double, double> > m_deviations;

				std::string m_description;

			private:
				static bool compare_deviations(std::pair<double, double> first, std::pair<double, double> second);
			};
		}
	}
}
#endif // __DEVIATION_SECURITY_RULE_H__
