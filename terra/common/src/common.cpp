#include "common.h"
#include "defaultdatetimepublisher.h"
#ifdef Linux
#include <thread>
#include<chrono>
#endif
namespace terra
{
	namespace common
	{
		void printf_ex(const char *format, ...)
		{			
			char          buf[1024];
			memset(buf, 0, sizeof(buf));
			va_list       ap;
			if (format == nullptr || strlen(format) < 1)
				return;						
			va_start(ap, format);
			vsprintf((char *)buf, format, ap);
			va_end(ap);

			printf("%s ", to_iso_extended_string(date_time_publisher_gh::get_instance()->now()).c_str());
			printf("%s", buf);			
		}

		void sleep_by_milliseconds(int m_millis)
		{
#ifdef Linux
			std::this_thread::sleep_for(std::chrono::milliseconds(m_millis));
#else
			Sleep(m_millis);
#endif

		}
	}
}
