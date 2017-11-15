#ifndef __INSTRUMENT_CONTAINER2_H__
#define __INSTRUMENT_CONTAINER2_H__

#include <map>

//


namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			class tradeitem;

			class tradeitem_container
			{
			public:
				typedef std::map<std::string, tradeitem*> instrument_by_key;


			public:
				tradeitem_container() {}
				virtual ~tradeitem_container() {}

				bool add(tradeitem* pInstrument);
				bool remove(tradeitem* pInstrument);
				bool contains(tradeitem* pInstrument);
				void clear();

				int count() { return m_instrumentsByKey.size(); }

				//void dump();

				// key = tradingCode@A
				tradeitem* get_by_key(const char* key);

				tradeitem *get_by_key(std::string, std::string);

				// TO REMOVE (once we found a solution)...
				// used by ITS/HOA to rebuild orders
				tradeitem* get_by_second_key(const char* key);
				//

				tradeitem* get_by_pos_key(const char* isin);

				const instrument_by_key& get_map() { return m_instrumentsByKey; }
				//


			private:
				instrument_by_key m_instrumentsByKey;
				instrument_by_key m_instrumentsBySecondKey;
				instrument_by_key m_instrumentsByPosKey;
			};
		}
	}
}
#endif // __INSTRUMENT_CONTAINER_H__
