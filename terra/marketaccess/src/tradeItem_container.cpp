#include "tradeItem_container.h"
#include "tradeitem.h"
#include "terra_logger.h"

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			bool tradeitem_container::add(tradeitem* pInstrument)
			{
				if (pInstrument == NULL)
					return false;

				// key (tradingCode@marketId)
				if (m_instrumentsByKey.find(pInstrument->get_key()) != m_instrumentsByKey.end())
				{
					loggerv2::error("tradeitem_container::add - key [%s] already exist", pInstrument->get_key());
					return false;
				}
				m_instrumentsByKey[pInstrument->get_key()] = pInstrument;


				// key (tradingCode)
				if (m_instrumentsBySecondKey.find(pInstrument->get_second_key()) == m_instrumentsBySecondKey.end())
				{
					m_instrumentsBySecondKey[pInstrument->get_second_key()] = pInstrument;
				}
				else
				{
					loggerv2::error("tradeitem_container::add - second_key [%s] already exist for [%s]",
						pInstrument->get_second_key(),
						m_instrumentsBySecondKey[pInstrument->get_second_key()]->get_key());
				}


				//if (m_instrumentsByPosKey.find(pInstrument->get_pos_key()) == m_instrumentsByPosKey.end())
				//{
				// m_instrumentsByPosKey[pInstrument->get_pos_key()] = pInstrument;
				//}
				//else
				//{
				// loggerv2::error("tradeitem_container::add - pos_key [%s] already exist for [%s]",
				//	 pInstrument->get_pos_key(),
				//	 m_instrumentsByPosKey[pInstrument->get_pos_key()]->get_key());
				//}



				return true;
			}

			bool tradeitem_container::remove(tradeitem* pInstrument)
			{
				if (pInstrument == NULL)
					return false;

				if (m_instrumentsByKey.find(pInstrument->get_key()) == m_instrumentsByKey.end())
					return false;

				m_instrumentsByKey.erase(pInstrument->get_key());
				m_instrumentsBySecondKey.erase(pInstrument->get_second_key());
				//m_instrumentsByIsin.erase(pInstrument->get_isin_code());

				return true;
			}

			bool tradeitem_container::contains(tradeitem* pInstrument)
			{
				return (m_instrumentsByKey.find(pInstrument->get_key()) != m_instrumentsByKey.end());
			}

			void tradeitem_container::clear()
			{
				m_instrumentsByKey.clear();
				m_instrumentsBySecondKey.clear();
			}

			tradeitem* tradeitem_container::get_by_key(const char* key)
			{
				instrument_by_key::iterator it = m_instrumentsByKey.find(key);
				return it != m_instrumentsByKey.end() ? (*it).second : NULL;
			}

			tradeitem* tradeitem_container::get_by_key(std::string code, std::string name)
			{
				std::string key = code + "@" + name;
				instrument_by_key::iterator it = m_instrumentsByKey.find(key);
				return it != m_instrumentsByKey.end() ? (*it).second : NULL;
			}

			tradeitem* tradeitem_container::get_by_second_key(const char* secondKey)
			{
				instrument_by_key::iterator it = m_instrumentsBySecondKey.find(secondKey);
				return it != m_instrumentsBySecondKey.end() ? (*it).second : NULL;
			}

			//tradeitem* tradeitem_container::get_by_isin_code(const char* isinCode)
			//{
			//   instrument_by_key::iterator it = m_instrumentsByIsin.find(isinCode);
			//   return it != m_instrumentsByIsin.end() ? (*it).second : NULL;
			//}

			/*void tradeitem_container::dump()
			{
			for (instrument_by_key::iterator it = m_instrumentsByKey.begin(); it != m_instrumentsByKey.end(); it++)
			{
			//std::string mnemo = (*it).second->get_mnemo();
			//if (mnemo == "KS200")
			std::cout << (*it).second->get_trading_code() << std::endl;
			}
			}*/
		}
	}

}