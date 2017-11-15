#ifndef _CONVERSE_H_
#define _CONVERSE_H_
#pragma once
#include <float.h>
#include <common.h>

//#include "instrumentcommon.h"
namespace terra
{
	namespace instrument
	{
		class converse
		{
		public:
			converse()
			{
				reset();
			}
			~converse(){}
		public:
			void reset()
			{
				this->m_bid_qty = 0;
				this->m_bid = 0.0;
				this->m_bid_strike = 0.0;

				this->m_ask_qty = 0;
				this->m_ask = DBL_MAX;
				this->m_ask_strike = 0.0;
			}
			void compute_mid()
			{
				m_mid = !common::math2::eq(m_ask, DBL_MAX) ? (m_bid + m_ask) / 2 : DBL_MAX;
			}
		protected:
			double m_bid;
			int    m_bid_qty;
			double m_bid_strike;

			double m_ask;
			int    m_ask_qty;
			double m_ask_strike;

			double m_mid;
		public:
			double get_bid() { return m_bid; }
			int    get_bid_qty(){ return m_bid_qty; }
			double get_bid_strike(){ return m_bid_strike; }

			double get_ask() { return m_ask; }
			int    get_ask_qty(){ return m_ask_qty; }
			double get_ask_strike(){ return m_ask_strike; }

			void set_bid(double value){ m_bid = value; }
			void set_bid_qty(int value){ m_bid_qty = value; }
			void set_bid_strike(double value){ m_bid_strike = value; }

			void set_ask(double value){ m_ask = value; }
			void set_ask_qty(int value){ m_ask_qty = value; }
			void set_ask_strike(double value){ m_ask_strike = value; }

			double get_mid(){ return m_mid; }
		};
	}
}
#endif //_CONVERSE_H_

