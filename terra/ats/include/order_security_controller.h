#ifndef __ORDER_SECURITY_CONTROLLER_H__
#define __ORDER_SECURITY_CONTROLLER_H__

#include <map>
#include <list>
#include <string>
#include "tradeitem.h"
#include "iorder_security_rule.h"
#include "singleton.hpp"
#include "atsinstrument.h"
namespace terra
{
	namespace ats
	{
		namespace security
		{
			
			class order_security_controller :public SingletonBase<order_security_controller>
			{
			public:
				void load(const char* filename);
				bool control(ats_instrument* ats_instr, order* o, char* pszReason);

			private:
				std::map<std::string, std::map<AtsType::InstrType::type, std::list<iorder_security_rule*> > > m_rules;

				friend class nominal_security_rule;
			};
		}
	}
}
#endif // __ORDER_SECURITY_CONTROLLER_H__
