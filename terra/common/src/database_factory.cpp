#include "database_factory.h"
#include "abstract_database.h"
#include "boost/algorithm/string.hpp"
#include "sqliteclient.h"
namespace terra
{
	namespace common
	{

		abstract_database* database_factory::create(const char* dbType)
		{
			abstract_database *pDatabase;
			if (!strcmp(dbType, "sqlite"))
			{
				pDatabase = new SQLiteClient;
				return pDatabase;
			}
			else
			{
				return nullptr;
			}
		}

		abstract_database* database_factory::create(const char* dbType, const char* connString)
		{
			abstract_database * pDataBase = create(dbType);
			pDataBase->set_connectString(connString);
			return pDataBase;
		}
	}
}

