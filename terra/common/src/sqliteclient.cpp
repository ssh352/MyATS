

extern "C" {
#include "sqlite3.h"
}
#include "sqliteclient.h"
#include "boost/property_tree/ptree.hpp"
using namespace boost::property_tree;
#include "terra_logger.h"
namespace terra
{
	namespace common
	{
		static int callbackScalar(void *data, int argc, char **argv, char **azColName)
		{
			return 1;
		}

		int SQLiteClient::callbackPTree(void *data, int argc, char **argv, char **azColName)
		{
			int i;
			ptree point;
			for (i = 0; i < argc; i++)
			{
				point.put(const_cast<const char*>(azColName[i]), argv[i] == 0 ? "NUL" : argv[i]);
			}
			((SQLiteClient*)data)->res->push_back(point);
			//SQLiteClient::res.push_back(point);
			return 0;
		}

		bool SQLiteClient::init_connection()
		{
			return true;
		}

		bool SQLiteClient::open_database()
		{
			int rc = sqlite3_open(connString.c_str(), &pDB);
			sqlite3_exec(pDB, "PRAGMA synchronous = OFF; ", 0, 0, 0);
			if (rc)
			{
				loggerv2::error("Can't open database: %s", sqlite3_errmsg(pDB));
				return false;
			}
			else
				return true;
		}

		bool SQLiteClient::close_databse()
		{
			if (pDB)
			{
				sqlite3_close(pDB);
				return true;
			}
			else
				return false;

		}

		std::vector<boost::property_tree::ptree>* SQLiteClient::get_table(const char* command)
		{
			/*delete table;
			table = new ptree;*/
			//int rc;
			char * err_msg = NULL;
			res = new std::vector<boost::property_tree::ptree>;
			if (sqlite3_exec(pDB, command, SQLiteClient::callbackPTree, this, &err_msg) != SQLITE_OK)
			{
				loggerv2::error("failed to exec %s, error msg:%s", command, err_msg);
				sqlite3_free(err_msg);
				
			}

			return res;


		}

		char* SQLiteClient::get_scalar(const char* command)
		{
			throw std::logic_error("The method or operation is not implemented.");
		}

		int SQLiteClient::executeNonQuery(const char* command)
		{
			char * err_msg = NULL;
			if (sqlite3_exec(pDB, command, NULL, NULL, &err_msg) != SQLITE_OK)
			{
				loggerv2::error("failed to exec %s, error msg:%s", command, err_msg);
				sqlite3_free(err_msg);
				return 0;
			}
			else
			{
				return 1;
			}
		}

		int SQLiteClient::executeTransaction(const char* command)
		{
			sqlite3_exec(pDB, "begin;", 0, 0, 0);
			sqlite3_exec(pDB, command, 0, 0, 0);
			sqlite3_exec(pDB, "commit;", 0, 0, 0);
			return 0;
		}

	}
}
