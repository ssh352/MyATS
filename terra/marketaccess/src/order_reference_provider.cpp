#include "order_reference_provider.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			/*order_reference_provider* order_reference_provider::ms_pInstance = NULL;

			order_reference_provider* order_reference_provider::instance()
			{
				if (ms_pInstance == NULL)
					ms_pInstance = new order_reference_provider();
				return ms_pInstance;
			}*/

			bool order_reference_provider::initialize(const char* filename, int startInt)
			{
				m_intProvider.set_filename(filename);
				//m_intProvider.set_name("order_reference_provider");
				m_intProvider.set_current_int(startInt);
				return m_intProvider.start(startInt);
			}

			void order_reference_provider::close()
			{
				m_intProvider.stop();
			}

			void order_reference_provider::set_current_int(int n)
			{
				m_intProvider.set_current_int(n);
			}

			int order_reference_provider::get_current_int()
			{
				return m_intProvider.get_current_int();
			}

			int order_reference_provider::get_next_int()
			{
				return m_intProvider.get_next_int();
			}
		}
	}

}