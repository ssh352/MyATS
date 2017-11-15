#ifndef __POSITION_PERSISTER2_H__
#define __POSITION_PERSISTER2_H__

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class position_persister
			{
			public:
				static void load(const char* filename);
			};
		}
	}
}

#endif // __POSITION_PERSISTER_H__
