#ifndef _PRICE_DATA_H_
#define _PRICE_DATA_H_

//#include "LockFreeArray.h"
#include <map>
#include <atomic>
#include "singleton.hpp"
#include <memory>
#include "common.h"
#include <list>


using namespace std;
using namespace terra::common;
namespace fs = boost::filesystem;
//#define KLINE_T 1800//30 min Kline
namespace terra
{
	namespace ats
	{
		template<typename T>
		class list_ex :public std::list<T>
		{
		public:

			list_ex()
			{
				m_lock.clear();
			}

			void push_value(T t1)
			{
				while (m_lock.test_and_set(std::memory_order_acquire))
					;
				std::list<T>::push_back(t1);
				m_lock.clear(std::memory_order_release);
			}

			void rm_iterator(typename list_ex<T>::iterator &it)
			{
				while (m_lock.test_and_set(std::memory_order_acquire))
					;
				erase(it);
				m_lock.clear(std::memory_order_release);
			}

			T get_back()
			{
				while (m_lock.test_and_set(std::memory_order_acquire))
					;

				T res = *(this->rbegin());

				m_lock.clear(std::memory_order_release);

				return res;
			}

			T average()
			{
				while (m_lock.test_and_set(std::memory_order_acquire))
					;

				T res;
				double sum = 0;
				int i = 0;
				for (auto it : *this)
				{
					++i;
					sum += it;
				}

				res = (T)(sum / i);

				m_lock.clear(std::memory_order_release);

				return res;
			}
			std::atomic_flag m_lock;
		};

		//enum KlineEvent
		//{
		//	None = 0,
		//	NewKline,
		//	NewHighes,
		//	NewLowest
		//};

		class Kline
		{
		public:
			Kline()
			{
				open_price = 0;
				close_price = 0;
				highest = 0;
				lowest = 0;
				TR = 0.0;
				Signal = 0;
				btime = boost::posix_time::from_iso_string("19700101T000000");
				etime = btime;
			}
			~Kline()
			{

			}
			void make_new_kline()
			{
				open_price = close_price;
				close_price = close_price;
				highest = close_price;
				lowest = close_price;

				btime = etime;
			}
			double open_price;
			double close_price;
			double highest;
			double lowest;

			ptime btime;
			ptime etime;

			double TR;
			int Signal;
		};


		class price_data
		{
			typedef std::function<void(std::string &code)> ProcessHandler;
		public:
			price_data()
			{
				m_lock.clear();
				//m_event = KlineEvent::None;
				m_Kline_T = 1800;
				m_Kline_num = 55;
			}
			~price_data()
			{

			}

			unsigned int m_Kline_T;
			unsigned int m_Kline_num;
			//KlineEvent m_event;
			void push_data(double price, ptime &uptime);
			void get_temp_kline(list<Kline> &m_list);
			std::list<Kline>::iterator get_end_it();//由于K线的list是一直增长的，为了线程安全，获取list最后一个元素的迭代器时请使用该函数
			
			Kline m_kline_current;
			Kline m_today_first_kline;//本个交易日首个K线.
			Kline m_yesterday_last_kline;//上个交易日最后一个K线.
			//lock_free_array<double> m_price;//保存今日的tick价格数据，不在磁盘备份
			//lock_free_array<double> m_kline_close_price;//保存K线的收盘价格

			std::list<Kline> m_Kline;//K线数据
			//list<Kline> m_Kline_temp;//写入磁盘的k线数据
			std::list<ProcessHandler> m_handler;
			
			std::atomic_flag m_lock;
			double get_high(int n);
			double get_low(int n);

			void setCode(std::string &code){ m_code = code; }
			void setHandler(ProcessHandler handler){ m_handler.push_back(handler);}
			void save(std::string dir);
			void load(std::string dir);
		private:
			std::string m_code;
		};

		//class total_price_data :public SingletonBase<total_price_data>
		//{

		//public:
		//	unordered_map_ex<string, std::shared_ptr<price_data>>* get_hash_map(){ return &m_price_data; }
		//	void save(std::string dir);
		//	void load(std::string dir);

		//private:
		//	unordered_map_ex<string, std::shared_ptr<price_data>>  m_price_data;

		//};
		class price_data_container
		{

		public:
			unordered_map_ex<string, std::shared_ptr<price_data>>* get_hash_map(){ return &m_price_data; }
			void save(std::string dir);
			void load(std::string dir);

		private:
			unordered_map_ex<string, std::shared_ptr<price_data>>  m_price_data;

		};
	}
}
#endif //_PRICE_DATA_H_
