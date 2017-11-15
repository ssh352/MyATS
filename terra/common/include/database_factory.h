#ifndef __DATABASE_FACTORY_H_V2_
#define __DATABASE_FACTORY_H_V2_


namespace terra
{
	namespace common
	{
		class abstract_database;
		class database_factory
		{
		public:
			database_factory();
			~database_factory();
			static abstract_database* create(const char* dbType);
			static abstract_database* create(const char* dbType, const char* connString);

		};

	}
}


#endif