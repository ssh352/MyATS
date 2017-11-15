#include "total_price_data.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include <defaultdatetimepublisher.h>

namespace terra
{
	namespace ats
	{
		void price_data::push_data(double price, ptime &uptime)
		{
			//m_price.push_back(price);

			if (m_kline_current.open_price == 0)
			{
				m_kline_current.open_price = price;
				m_kline_current.lowest = price;
				m_kline_current.highest = price;
				m_kline_current.btime = uptime;
			}
			if (m_kline_current.lowest > price)
			{
				m_kline_current.lowest = price;
				//m_event = KlineEvent::NewLowest;
			}
			if (m_kline_current.highest < price)
			{
				m_kline_current.highest = price;
				//m_event = KlineEvent::NewHighes;
			}

			auto interval = uptime - m_kline_current.btime;
			if (interval.total_seconds() > m_Kline_T)//生成新的30min kline
			{
				m_kline_current.close_price = price;
				m_kline_current.etime = uptime;


				while (m_lock.test_and_set(std::memory_order_acquire))
					;
				if (m_Kline.size() > 0)
				{
					double m_dA = m_kline_current.highest - m_kline_current.lowest;
					double m_dB = m_kline_current.highest - m_Kline.back().close_price;
					double m_dC = m_Kline.back().close_price - m_kline_current.lowest;

					//double max1 = (m_dA > m_dB) ? m_dA : m_dB;
					//double max = (max1 > m_dC) ? max1 : m_dC;

					m_kline_current.TR = max(max(m_dA, m_dB), m_dC);
					m_kline_current.Signal = m_Kline.back().Signal;
				}
				m_Kline.push_back(m_kline_current);
				while (m_Kline.size() > m_Kline_num)
					m_Kline.erase(m_Kline.begin());

				m_lock.clear(std::memory_order_release);

				m_kline_current.make_new_kline();
				//m_kline_close_price.push_back(price);

				//m_event = KlineEvent::NewKline;
				for (auto &func : m_handler)
				{
					func(m_code);
				}

				if (m_today_first_kline.open_price == 0)
				{
					m_today_first_kline = m_kline_current;
				}
			}
		}

		void price_data::get_temp_kline(list<Kline> &m_list)
		{
			m_list.clear();
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
			m_list = m_Kline;
			m_lock.clear(std::memory_order_release);
		}

		double price_data::get_high(int n)
		{
			int len = m_Kline.size();
			if (len < 1)
				return 0;
			auto CurrentPrice = get_end_it();
			--CurrentPrice;
			double res = CurrentPrice->highest;
			for (int i = 1; i < n&&i < len - 1; ++i)
			{
				if (CurrentPrice->highest > res)
					res = CurrentPrice->highest;
				--CurrentPrice;
			}
			return res;
		}

		double price_data::get_low(int n)
		{
			int len = m_Kline.size();
			if (len < 1)
				return 0;
			auto CurrentPrice = get_end_it();
			--CurrentPrice;
			double res = CurrentPrice->lowest;
			for (int i = 1; i < n&&i < len - 1; ++i)
			{
				if (CurrentPrice->highest < res)
					res = CurrentPrice->highest;
				--CurrentPrice;

			}
			return res;
		}
		std::list<Kline>::iterator price_data::get_end_it()
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
			std::list<Kline>::iterator it;
			if (m_Kline.size() == 0)
				it = m_Kline.end();
			else
				it = --(m_Kline.end());
			m_lock.clear(std::memory_order_release);
			return it;

		}

		void price_data::save(std::string atsDirectory)
		{
			std::string todir = atsDirectory + "/kline/";
			boost::filesystem::path fp(todir);
			if (!boost::filesystem::exists(fp))
			{
				boost::filesystem::create_directory(fp);
			}


			std::list<Kline> m_list;
			get_temp_kline(m_list);
			if (m_list.size() == 0)
				return;

			std::string fname = fp.string() + m_code + ".csv";
			std::ofstream f1(fname.data(), std::ios::out);
			if (!f1.bad())
			{
				for (auto &itr : m_list)
				{
					f1 << itr.open_price << "," << itr.highest << "," << itr.lowest << "," << itr.close_price << ",";
					std::string b = to_iso_extended_string(itr.btime);
					std::string e = to_iso_extended_string(itr.etime);

					f1 << b << "," << e << ",";
					f1 << itr.TR << "," << itr.Signal << std::endl;
				}


				f1.close();

			}
		}
		//
		//void total_price_data::save(std::string atsDirectory)
		//{
		//	//std::string todir = ats_config::get_instance()->get_daily_directory() + "/kline/";
		//	std::string todir = atsDirectory + "/kline/";
		//	boost::filesystem::path fp(todir);
		//	if (!boost::filesystem::exists(fp))
		//	{
		//		boost::filesystem::create_directory(fp);
		//	}
		//
		//
		//	for (auto &it : m_price_data)
		//	{
		//		std::list<Kline> m_list;
		//		it.second->get_temp_kline(m_list);
		//		if (m_list.size() == 0)
		//			continue;
		//
		//		std::string fname = fp.string() + it.first + ".csv";
		//		std::ofstream f1(fname.data(), std::ios::out);
		//		if (!f1.bad())
		//		{
		//			for (auto &itr : m_list)
		//			{
		//				f1 << itr.open_price << "," << itr.highest << "," << itr.lowest << "," << itr.close_price << ",";
		//				std::string b = to_iso_extended_string(itr.btime);
		//				std::string e = to_iso_extended_string(itr.etime);
		//
		//				f1 << b << "," << e << ",";
		//				f1<< itr.TR << "," << itr.Signal <<std::endl;
		//			}
		//			
		//		}
		//		f1.close();
		//
		//	}
		//}
		void price_data::load(std::string atsDirectory)
		{

			Kline kline;
			string kp = atsDirectory + "/kline/";
			fs::path kline_path(kp);
			if (!boost::filesystem::exists(kline_path))
			{
				boost::filesystem::create_directory(kline_path);
				return;
			}


			std::string fname = kline_path.string() + m_code + ".csv";

			std::ifstream f1(fname.data(), std::ios::in);
			if (!f1.bad())
			{
				string buf;
				vector<string> vec;
				int line = 0;
				auto Kline_List = m_Kline;
				int max_num = m_Kline_num;

				while (getline(f1, buf))
				{
					++line;
					boost::split(vec, buf, boost::is_any_of(","));
					if (vec.size() < 8)
						continue;

					kline.open_price = boost::lexical_cast<double>(vec[0]);
					kline.highest = boost::lexical_cast<double>(vec[1]);
					kline.lowest = boost::lexical_cast<double>(vec[2]);
					kline.close_price = boost::lexical_cast<double>(vec[3]);
					kline.btime = from_iso_extended_string(vec[4]);
					kline.etime = from_iso_extended_string(vec[5]);
					kline.TR = boost::lexical_cast<double>(vec[6]);
					kline.Signal = boost::lexical_cast<int>(vec[7]);


					if (m_today_first_kline.open_price == 0)
					{
						auto d1 = date_time_publisher_gh::get_instance()->today();
						auto d2 = kline.btime.date();
						if (d1 == d2)
						{
							m_today_first_kline = kline;
							if (m_Kline.size() > 1)
							{
								auto last = m_Kline.end();
								m_yesterday_last_kline = *(--last);
							}
						}
					}
					if (Kline_List.size() > max_num)
						Kline_List.erase(Kline_List.begin());
					Kline_List.push_back(kline);
				}
				f1.close();
				m_kline_current = kline;
			}
		}
		//
		//void total_price_data::load(std::string atsDirectory)
		//{
		//	/*std::string todir = ats_config::get_instance()->get_daily_directory();
		//	boost::filesystem::path todir_path(todir);
		//	fs::path fp = todir_path.parent_path();
		//
		//	if (!boost::filesystem::exists(fp))
		//	{
		//	boost::filesystem::create_directory(fp);
		//	}
		//
		//	Kline kline;
		//	vector<string> FilePathVector;
		//
		//
		//	fs::directory_iterator end_iter;
		//	for (fs::directory_iterator iter(fp); iter != end_iter; ++iter)
		//	{
		//	if (fs::is_directory(iter->status()))
		//	{
		//	FilePathVector.push_back(iter->path().string());
		//	}
		//
		//	}
		//
		//	for (auto &it : FilePathVector)
		//	{*/
		//	Kline kline;
		//	string kp = atsDirectory + "/kline/";
		//	fs::path kline_path(kp);
		//	if (!boost::filesystem::exists(kline_path))
		//	{
		//		boost::filesystem::create_directory(kline_path);
		//		//continue;
		//	}
		//
		//	fs::directory_iterator end_it;
		//	for (fs::directory_iterator diter(kline_path); diter != end_it; ++diter)
		//	{
		//		if (fs::is_regular_file(diter->status()))
		//		{
		//			std::string fname = diter->path().string();
		//
		//			int instr_len = fname.size() - kp.size() - 4;
		//
		//			if (instr_len <= 0)
		//				continue;
		//
		//			std::string instr_code = fname.substr(kp.size(), fname.size() - kp.size() - 4);//通过文件名获取交易品种
		//
		//			auto hash_it = m_price_data.find(instr_code);
		//			if (hash_it == m_price_data.end())
		//			{
		//				continue;
		//			}
		//			std::ifstream f1(fname.data(), std::ios::in);
		//			if (!f1.bad())
		//			{
		//				string buf;
		//				vector<string> vec;
		//				int line = 0;
		//				auto Kline_List = hash_it->second->m_Kline;
		//				int max_num = hash_it->second->m_Kline_num;
		//
		//				while (getline(f1, buf))
		//				{
		//					++line;
		//					boost::split(vec, buf, boost::is_any_of(","));
		//					if (vec.size() < 8)
		//						continue;
		//
		//					kline.open_price = boost::lexical_cast<double>(vec[0]);
		//					kline.highest = boost::lexical_cast<double>(vec[1]);
		//					kline.lowest = boost::lexical_cast<double>(vec[2]);
		//					kline.close_price = boost::lexical_cast<double>(vec[3]);
		//					kline.btime = from_iso_extended_string(vec[4]);
		//					kline.etime = from_iso_extended_string(vec[5]);
		//					kline.TR = boost::lexical_cast<double>(vec[6]);
		//					kline.Signal = boost::lexical_cast<int>(vec[7]);
		//
		//
		//					if (hash_it->second->m_today_first_kline.open_price == 0)
		//					{
		//						auto d1 = date_time_publisher_gh::get_instance()->today();
		//						auto d2 = kline.btime.date();
		//						if (d1 == d2)
		//						{
		//							hash_it->second->m_today_first_kline = kline;
		//							if (hash_it->second->m_Kline.size() > 1)
		//							{
		//								auto last = hash_it->second->m_Kline.end();
		//								hash_it->second->m_yesterday_last_kline = *(--last);
		//							}
		//						}
		//					}
		//					if (Kline_List.size() > max_num)
		//						Kline_List.erase(Kline_List.begin());
		//					Kline_List.push_back(kline);
		//					//hash_it->second->m_kline_close_price.push_back(kline.close_price);
		//				}
		//				f1.close();
		//				hash_it->second->m_kline_current = kline;
		//			}
		//
		//
		//		}
		//
		//	}
		//}

		void price_data_container::save(std::string dir)
		{
			for (auto& it : m_price_data)
			{
				it.second->save(dir);
			}
		}

		void terra::ats::price_data_container::load(std::string dir)
		{
			for (auto& it : m_price_data)
			{
				it.second->load(dir);
			}
		}
	}
}