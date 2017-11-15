#ifndef __SQLITE_CLIENT_H_V2_
#define __SQLITE_CLIENT_H_V2_

#include <vector>
#include "abstract_database.h"
#include "sqlite3.h"
#include "boost/property_tree/ptree.hpp"
namespace terra
{
	namespace common
	{
		class SQLiteClient : public abstract_database
		{


		public:
			virtual bool init_connection();

			virtual bool open_database();

			virtual bool close_databse();

			virtual std::vector<boost::property_tree::ptree>* get_table(const char* command);

			virtual char* get_scalar(const char* command);

			virtual int executeNonQuery(const char* command);

			virtual int executeTransaction(const char* command);
		private:
			sqlite3* pDB;
			std::vector<boost::property_tree::ptree>* res;
			static int callbackPTree(void *data, int argc, char **argv, char **azColName);
		};

	}
}


#endif