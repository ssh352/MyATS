#ifndef __ABSTRACT_DATABSE_H_V2_	
#define __ABSTRACT_DATABSE_H_V2_	

#include "boost/property_tree/ptree.hpp"
#include <vector>


namespace terra
{
	namespace common
	{

		class abstract_database
		{
		public:
			abstract_database(){};
			abstract_database(const char* connString)
			{
				connString = connString;
			};
			virtual ~abstract_database(){};
			virtual bool init_connection() = 0;
			virtual bool open_database() = 0;
			virtual bool close_databse() = 0;
			virtual std::vector<boost::property_tree::ptree>* get_table(const char* command) = 0;
			virtual char* get_scalar(const char* command) = 0;
			virtual int executeNonQuery(const char* command) = 0;
			virtual int executeTransaction(const char* command) = 0;
			void set_connectString(const char* conS){ connString = conS; };
			static std::string get_item(std::string codes, std::string channel);

		protected:
			std::string connString;

		};




	}
}




#endif