#ifndef _IB_FEED_UPDATE_
#define _IB_FEED_UPDATE_
#include "EWrapper.h"

namespace feed
{
	struct ib_feed_update
	{

	public:
		ib_feed_update(){}
		ib_feed_update(int tickId, TickType tickType, double price, int qty)
			:
			m_tickId(tickId),
			m_ticktype(tickType),
			m_price(price),
			m_qty(qty)
		{

		}

	public:
		int m_tickId;
		TickType m_ticktype;
		double m_price;
		int m_qty;
	};



	struct  ib_contract_info
	{
	public:
		ib_contract_info(std::string l,std::string e, std::string s, std::string c)
			:localSymbol(l.c_str()),
			exchange(e.c_str()),
			secType(s.c_str()),
			currency(c.c_str())
		{

		}

	public:
		std::string localSymbol;
		std::string exchange;
		std::string secType;
		std::string currency;
		std::string expire;
	};





}

#endif