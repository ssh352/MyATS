#include "abstract_database.h"
#include <boost/algorithm/string.hpp>
namespace terra
{
	namespace common
	{
		std::string abstract_database::get_item(std::string codes, std::string channel)
		{
			std::vector<std::string> strs;
			boost::split(strs, codes, boost::is_any_of("|"));
			for  (std::string&  fullcode : strs)
			{
				std::vector<std::string> strs1;
				boost::split(strs1, fullcode, boost::is_any_of("@"));
				if (strs1[1].compare(channel) == 0)
					return strs1[0];

			}
			return "";

		}
	}
}


