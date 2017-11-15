#include "count_task.h"
namespace terra
{
	namespace ats
	{

		count_task::count_task(abstract_ats * pAts,lwdur delay) :abstract_workflow_task(pAts, "Count", delay)
		{

		}


		
		void count_task::execute()
		{
			if (m_pT != nullptr)
			{

				//ptime last = date_time_publisher_gh::get_instance()->now();

				if (m_pT->print_screen)
				{
					++num_workflow;
					if (num_workflow >= last_num + 100)
					{
						last_num += 100;
						std::cout << "current phase: " << _TradingPhase_VALUES_TO_NAMES.at(m_pT->currentTradingPhase) << " task number: " << last_num << "  consume ms: " << total_ms << std::endl;
						total_ms = 0;
					}
				}
			}
		}
	}
}