#ifndef __RESYNCHRONIZATIONMODE2__H
#define __RESYNCHRONIZATIONMODE2__H
#include "orderdatadef.h"
#include <map>
#include <boost/noncopyable.hpp>

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class ResynchronizationModeTranslator:public boost::noncopyable
			{
			public:
				inline static ResynchronizationModeTranslator& get_instance() 
				{
					static ResynchronizationModeTranslator t;
					return t;
				}
			
				

				~ResynchronizationModeTranslator() { }

				ResynchronizationMode GetResynchronizationMode(::std::string val)
				{
					ResynchronizationMode m = None;
					auto it = resychroModeToStringArray.find(val);
					if (it != resychroModeToStringArray.end())
						return it->second;
					return m;
				}

				std::string GetString(ResynchronizationMode val)
				{
					std::string str = "none";

					auto it = stringToResynchroModeArray.find((int)val);
					if (it != stringToResynchroModeArray.end())
						return it->second;
					return str;
				}
			protected:
				ResynchronizationModeTranslator()
				{
					stringToResynchroModeArray[(int)(ResynchronizationMode::None)] = "none";
					stringToResynchroModeArray[(int)(ResynchronizationMode::Last)] = "last";
					stringToResynchroModeArray[(int)(ResynchronizationMode::Full)] = "full";

					resychroModeToStringArray["none"] = ResynchronizationMode::None;
					resychroModeToStringArray["last"] = ResynchronizationMode::Last;
					resychroModeToStringArray["full"] = ResynchronizationMode::Full;
				}

			private:
				std::map<std::string, ResynchronizationMode>resychroModeToStringArray;
				std::map<int, std::string>stringToResynchroModeArray;
			};
		}
	}
}
#endif