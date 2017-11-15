#include "tradeitem.h"
#include "connection.h"
#include <etf.h>

namespace terra
{
	namespace marketaccess
	{
		namespace orderpassing
		{
			tradeitem::tradeitem(terra::instrument::financialinstrument *instr, std::string &ConnectionName, connection *con, std::string &Code, std::string &tradingCode, AtsType::InstrType::type instrType, std::string mktname, bool m_bKey_with_exchange)
			{
				m_strCode = Code;
				m_tradingCode = tradingCode;
				m_strConnectionName = ConnectionName;
				m_Instrument = instr;
				m_instrType = instrType;

				//compute_keys();
				//compute_full_name();
				m_key = m_strCode + "@" + m_strConnectionName;
				if (m_bKey_with_exchange)
					m_secondKey = m_tradingCode + "." + mktname + "@" + m_strConnectionName;
				else
					m_secondKey = m_tradingCode + "@" + m_strConnectionName;
				m_strName = m_strCode;
				//Add(this);
				//setConnection(con);
				m_Connection = con;
				m_lastSynchroTimePoint = lw_min_time;

				if (m_instrType == InstrType::ETF)
				{
					etf_unitisize = static_cast<instrument::etf*>(instr)->get_unit_size();
				}
				else
					etf_unitisize = 1;

				m_cancel_forbid = false;

				if (mktname == "SHFE" || mktname == "SH" || mktname == "SZ" || mktname == "SSE" || mktname == "SZE")
					m_close_order = CLOSE_PRIORITY::FIX;
				else
					m_close_order = CLOSE_PRIORITY::YST_FIRST;

				memset(m_log_str, 0, 512);

			}

			tradeItem_order_book* tradeitem::get_order_book()
			{
				return &m_orderbook;
			}

			//void tradeitem::compute_keys()
			//{
			//	m_key = m_strCode + "@" + m_strConnectionName;;
			//	m_secondKey = m_tradingCode + "@" + m_strConnectionName;
			//}

			//void tradeitem::compute_full_name()
			//{
			//	m_strName = m_strCode;
			//}

			int tradeitem::get_sellable_position()
			{
				if (get_instr_type() == AtsType::InstrType::Stock)
				{
					if (get_frozen_long_position() > 0)
						return get_yst_long_position() - get_pending_short_close_qty() - get_closed_position() + get_open_position() - get_frozen_long_position() + get_today_purred_qty();
					else
						return get_yst_long_position() - get_pending_short_close_qty() - get_closed_position() + get_today_purred_qty();
				}

				else
				{
					return 0;
				}
			}

			double tradeitem::get_point_value()
			{
				if (m_Instrument != nullptr)
				{
					return m_Instrument->get_point_value();
				}
				return 0.0;
			}

			std::string tradeitem::get_ISIN()
			{
				if (m_Instrument != nullptr)
				{
					return m_Instrument->get_isin();
				}
				return "";
			}

			void tradeitem::to_string()
			{

				loggerv2::info("Instrument DumpInfo :"
					" instr[%s]"
					" connection[%s]"

					" tot_long_position[%d]"
					" tot_short_position[%d]"

					" today_long_position[%d]"
					" today_short_position[%d]"

					" yst_long_position[%d]"
					" yst_short_position[%d]"

					" pending_long_close_qty[%d]"
					" pending_short_close_qty[%d]"

					" pending_long_close_today_qty[%d]"
					" pending_short_close_today_qty[%d]"


					" covered_sell_open_position[%d]"
					" pending_covered_sell_open_qty[%d]"
					" pending_covered_sell_close_qty[%d]"
					" frozen_long_position[%d]"

					" today_purred_qty[%d]"

					//" last_resynchro_time[%s]"
					,
					this->getName().c_str(),
					this->get_connection_name().c_str(),

					this->get_tot_long_position(),
					this->get_tot_short_position(),

					this->get_today_long_position(),
					this->get_today_short_position(),

					this->get_yst_long_position(),
					this->get_yst_short_position(),

					this->get_pending_long_close_qty(),
					this->get_pending_short_close_qty(),

					this->get_pending_long_close_today_qty(),
					this->get_pending_short_close_today_qty(),

					this->get_covered_sell_open_position(),
					this->get_pending_covered_sell_open_qty(),
					this->get_pending_covered_sell_close_qty(),
					this->get_frozen_long_position(),

					this->get_today_purred_qty()

					//to_iso_extended_string(this->get_last_sychro_timepoint()).c_str()
					);
			}
			void tradeitem::dumpinfo()
			{
				snprintf(m_log_str, 512, "tradeitem_Dump:instr[%s],connection[%s],tot_Long_pos[%d],tot_Short_pos[%d],td_Long_pos[%d],td_Short_pos[%d],"
					"yst_Long_pos[%d],yst_Short_pos[%d],Pd_Long_close[%d],Pd_Short_close[%d],Pd_Long_close_today[%d],Pd_short_close_today_[%d],"
					"covered_sell_open[%d],Pd_covered_sell_open[%d],Pd_covered_sell_close[%d] frozen_long_position[%d],today_purred[%d],should_Pd_long[%d],should_Pd_short[%d], "
					"yst_comb_long[%d], yst_comb_short[%d]",//last_resynchro_time[%s]\n",

					this->getName().c_str(),
					this->get_connection_name().c_str(),
					this->get_tot_long_position(),
					this->get_tot_short_position(),
					this->get_today_long_position(),
					this->get_today_short_position(),
					this->get_yst_long_position(),
					this->get_yst_short_position(),

					this->get_pending_long_close_qty(),
					this->get_pending_short_close_qty(),

					this->get_pending_long_close_today_qty(),
					this->get_pending_short_close_today_qty(),

					this->get_covered_sell_open_position(),
					this->get_pending_covered_sell_open_qty(),
					this->get_pending_covered_sell_close_qty(),
					this->get_frozen_long_position(),

					this->get_today_purred_qty(),
					this->should_pending_long_close_qty,
					this->should_pending_short_close_qty,
					this->get_yst_comb_long(),
					this->get_yst_comb_short());
					//this->get_today_comb_long(),
					//this->get_today_comb_short();
				//to_iso_extended_string(this->get_last_sychro_timepoint()).c_str());

				BOOST_LOG_TRIVIAL(info) << m_log_str;



			}

		}
	}
}
