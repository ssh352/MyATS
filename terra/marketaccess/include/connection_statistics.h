#ifndef __CONNECTION_STA2_H__
#define __CONNECTION_STA2_H__

#include <map>
#include <vector>
#include <atomic>
namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class connection_statistics
			{
			public:
				connection_statistics()
				{
					for (int i = 0; i < 7; ++i)
					{
						m_statistics[i] = 0;
					}
				}
				virtual ~connection_statistics() {  }


				// incr
				inline void incr_new() { ++m_statistics[0]; }
				inline void incr_mod() { ++m_statistics[1]; }
				inline void incr_can() { ++m_statistics[2]; }

				inline void incr_rej() { ++m_statistics[3]; }
				inline void incr_ack() { ++m_statistics[4]; }
				inline void incr_nack() { ++m_statistics[5]; }

				inline void incr_exe() { ++m_statistics[6]; }


				// decr
				inline void decr_ack() { --m_statistics[4]; }


				// get
				inline int get_new() { return m_statistics[0]; }
				inline int get_mod() { return m_statistics[1]; }
				inline int get_can() { return m_statistics[2]; }

				inline int get_rej() { return m_statistics[3]; }
				inline int get_ack() { return m_statistics[4]; }
				inline int get_nack() { return m_statistics[5]; }

				inline int get_exe() { return m_statistics[6]; }


			private:
				std::atomic<int> m_statistics[7];
			};
		}
	}
}


#endif // __CONNECTION_STATISTICS_H__

