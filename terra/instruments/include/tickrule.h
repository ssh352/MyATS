#ifndef _TICK_RULE_H_
#define _TICK_RULE_H_
#pragma once
#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		///format:0_0.01
		//��С�䶯��λ:http://www.cffex.com.cn/sspz/sz50/hyb/
		class tickrule
		{
		public:
			tickrule(std::string & values);
			~tickrule();
		public:
			double tick_up(double price);
			double tick_down(double price);
			string get_name() { return m_strValues; }
			map_ex<double, double> & get_rule_values(){return m_ruleValues;}
		protected:
			std::string m_strValues;
			//����֤std::map��Ĭ�϶�key��������
			map_ex<double, double> m_ruleValues;
		};
	}
}
#endif //_TICK_RULE_H_

