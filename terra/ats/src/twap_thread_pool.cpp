#include "twap_thread_pool.h"
#include "atsconfig.h"
#include <sstream>
//#define Pool_Size 6
namespace terra
{
	namespace ats
	{
		twap_task::twap_task()
		{
			observer = nullptr;
			m_instr = nullptr;//����Ʒ�ֵ�ָ��
			iskillall = false;//�Ƿ�ִ���˳���ȫ��order�Ĳ���
			isfin = false;//task�Ƿ�ִ�����
			is_start = false;

			//Future1Pos = 0.0;//�ֲ���
			//m_Nack = 0.0;//������
			m_Nack_unit = 0;//������*unit
			m_lock.clear();

			onum = 5;//�µ���
			//min_limit_order_size = 1;//������С�µ���λ
			min_order_size = 5;//Ĭ�ϵ���С�����µ���λ����ʵ���µ���Ҫ��ʵ�����
			max_order_size = 10;//����µ���λ
			m_unit = 1;
			killallnum = 0;

			m_qty = 0;
			//total_time = boost::posix_time::seconds(0);
			//t = boost::posix_time::seconds(0);//�Ա�����ʼֵ
			//killalltime = boost::posix_time::seconds(0);
			alpha = 0.3;//�ٶ�����ɽ�����f(t)��б��ΪK��������������ֱ�ߣ�б��Ϊ��1+alpha���ͣ�1-alpha��,�������߿��Ժ�������Χ��һ�������䣬���ǵ�ʵ�ʳɽ�����Ҫ�����������������

			total_exe = 0.0;//�ܳɽ�(q(t))
			total_exe_unit = 0;//�ܳɽ�*unit
			total_sent = 0.0;//�Ѿ����͵�order���������ܳɽ����Ѿ�ACK��order
			max_price_offset = 0.001;
		}

		twap_task::~twap_task()
		{

		}

		void twap_task::set_task(std::shared_ptr<twap_task_data> &task)
		{
			this->m_instr = task->instr;
			this->observer = task->observer;

			if (m_instr == nullptr || observer == nullptr)
			{
				cout << "instr or observer should not be null " << endl;
				loggerv2::error("wap_thread_task::set_task,instr or observer should not be null");
				return;
			}


			this->m_qty = abs(task->qty);
			if (task->qty > 0)
				this->o_type = OrderWay::Buy;
			else
				this->o_type = OrderWay::Sell;
			this->_type = task->_type;

			this->alpha = task->alpha;
			this->max_order_size = task->max_order_size;
			this->min_order_size = task->min_order_size;

			this->btime = task->btime;
			auto now = date_time_publisher_gh::get_instance()->now();
			if (this->btime < now)
				this->btime = now;
			this->etime = task->etime;
			this->killalltime = task->btime;
			this->cancel_time = task->btime;
			this->hit_time = task->hit_time;

			this->m_unit = task->unit;
			this->start_price = task->start_price;
			this->stop_price = task->stop_price;
			this->max_price_offset = task->max_price_offset;
		}

		void twap_task::add_task(twap_task *task)
		{
			if (this->etime < task->etime)
				this->etime = task->etime;

			if (this->o_type == task->o_type)
				this->m_qty += task->m_qty;
			else
			{
				int res = this->m_qty - this->total_exe;//ʣ��res��λҪ����

				this->m_qty = abs(res - task->m_qty);
				if (res < task->m_qty)
					this->o_type = task->o_type;

			}
		}

		bool twap_task::check_start_status()
		{
			if (isfin)//�����Ѿ�����
				return false;

			boost::posix_time::ptime time_point = date_time_publisher_gh::get_instance()->now();//��ȡ��ǰʱ��
			if (time_point < btime)//��û�е���ʼ����ʱ��
				return false;

			if (is_start == false)
			{
				if (o_type == OrderWay::Buy)
				{
					if (math2::is_zero(start_price) || m_instr->get_feed_item()->get_bid_price() <= start_price)
					{
						is_start = true;
					}
					else
						return false;
				}
				else
				{
					if (math2::is_zero(start_price) || m_instr->get_feed_item()->get_ask_price() >= start_price)
					{
						is_start = true;
					}
					else
						return false;
				}
			}

			if (time_point > hit_time || abs(total_exe) >= abs(m_qty))
			{
				isfin = true;
				stringstream ss;
				ss << "intsr:" << m_instr->get_trade_item()->getCode() << " target" << m_qty << " high:" << high << " low:" << low << " tot_sent:" << total_sent
					<< " tot_exe:" << total_exe << " onum:" << onum << " reason:time over or trading over" << endl;

				loggerv2::info(ss.str().c_str());
			}

			if (math2::not_zero(stop_price) && o_type == OrderWay::Buy && m_instr->get_feed_item()->get_bid_price() > stop_price)
			{
				isfin = true;
				stringstream ss;
				ss << "intsr:" << m_instr->get_trade_item()->getCode() << " target" << m_qty << " high:" << high << " low:" << low << " tot_sent:" << total_sent
					<< " tot_exe:" << total_exe << " onum:" << onum << " reason:stop_price" << endl;

				loggerv2::info(ss.str().c_str());
			}

			if (math2::not_zero(stop_price) && o_type == OrderWay::Sell && m_instr->get_feed_item()->get_ask_price() < stop_price)
			{
				isfin = true;
				stringstream ss;
				ss << "intsr:" << m_instr->get_trade_item()->getCode() << " target" << m_qty << " high:" << high << " low:" << low << " tot_sent:" << total_sent
					<< " tot_exe:" << total_exe << " onum:" << onum << " reason:stop_price" << endl;

				loggerv2::info(ss.str().c_str());
			}

			if (isfin)
			{
				killall();
				return false;
			}

			return true;
		}

		void twap_task::do_Hitter_Trader()
		{
			boost::posix_time::ptime time_point = date_time_publisher_gh::get_instance()->now();
			if ((time_point - killalltime).total_milliseconds() > 2000 * 10)
			{
				//if (killallnum <= 1)
				iskillall = false;
				killalltime = time_point;
			}
			if (iskillall == false)
			{
				killall();
				iskillall = true;
				loggerv2::info("set iskillall to true");
				killalltime = time_point;
				++killallnum;
				return;
			}

			int _type = AtsTradingType::Hitter;
			int onum = m_qty - total_sent;

			if (onum > max_order_size)
				onum = max_order_size;


			if (onum > 0)
			{
				stringstream ss;
				ss << "intsr:" << m_instr->get_trade_item()->getCode() << " target" << m_qty<< " tot_sent:" << total_sent
					<< " tot_exe:" << total_exe << " onum:" << onum << endl;

				loggerv2::info(ss.str().c_str());
				if (SendOrders(onum, _type, o_type))
					total_sent += onum;
			}

			
		}

		void twap_task::process()//twap��������
		{
			if (check_start_status() == false)
				return;

			if (!checkunAckOrder())//������ڵȴ��е�order��wait
				return;

			boost::posix_time::ptime time_point = date_time_publisher_gh::get_instance()->now();
			int Nack = m_Nack_unit;
			m_Nack_unit = m_Nack_unit - Nack;
			total_sent -= (float)Nack / m_unit;

			if (time_point > etime && time_point < hit_time)//���еڶ��׶ε�������ģʽ
			{
				do_Hitter_Trader();
				return;
			}

			int _type = AtsTradingType::Unkown;
			boost::posix_time::time_duration total_dur = (etime - btime);
			boost::posix_time::time_duration time_dur = (time_point - btime);//�����ȥ��ʱ��

			k = (double)m_qty / total_dur.total_seconds();//twap����ʱ�亯����б�ʣ�f(t) = kt,t^[0,total_time];
			kl = k*(1 + alpha);//twap�������ߵ�б�ʣ�f2(t) = kl(t-total_time) + qty;��ɽ�������ΪΪq(t),��q(t)<f2(t)ʱ������������
			kh = k*(1 - alpha);//twap�������ߵ�б�ʣ�f3(x) = kh(t-total_time) + qty;�µ�ʱȷ���µ���+q(t)<=f3(t)(�µ���С��minsizeʱ����);

			//if (m_Nack>0)
			//	loggerv2::info("twap_task::process,m_Nack:%ld",m_Nack);

			//�����Ա���t������۳ɽ����Լ�������
			low = kl*(time_dur.total_seconds() - total_dur.total_seconds()) + m_qty;
			high = kh*(time_dur.total_seconds() - total_dur.total_seconds()) + m_qty;
			theory = k*time_dur.total_seconds();

			if (kill_bad_price() != 0)//�����۸񲻺õĵ�
				return;

			if (low > 0 && iskillall == false)
			{
				int ack_num = total_sent - total_exe;
				int max_ack_num = ceil(high) - floor(low);
				kill_by_time(o_type, ack_num, max_ack_num);//�����߼�,��������Ack��Ϊceli(high)-floor(low),��ǰ��Ack��Ϊtotal_sent - total_exe
			}

			onum = min_order_size;

			if (total_exe < low)
			{
				_type = AtsTradingType::Hitter;

				onum = (int)(low - total_exe);

				if (onum + total_sent > m_qty || total_sent + min_order_size >= m_qty)//ȷ�����ܵ����۳ɽ�������m_qty
					onum = m_qty - total_sent;

				if (onum<min_order_size&&iskillall == false)//����µ���С����Сֵ&&iskillallΪfalse&&m_qty - total_sent>min_order_size���µ�
					onum = 0;

				if (0 == onum&&total_sent<low)
				{
					_type = AtsTradingType::Contrib;
					int ack_num = total_sent - total_exe;
					int max_ack_num = ceil(high) - floor(low);

					onum = max_ack_num - ack_num;
					if (onum < min_order_size&&ceil(high)<m_qty)
						return;
				}
			}
			else
			{
				if (total_sent > high + min_order_size || iskillall)
						return;

				_type = AtsTradingType::Contrib;
				onum = ceil(high) - total_sent;

				if (onum < min_order_size&&ceil(high)<m_qty)
					onum = 0;
			}

			if (onum > max_order_size)
				onum = max_order_size;

			if (onum > 0)
			{
				stringstream ss;
				ss << "intsr:" << m_instr->get_trade_item()->getCode() <<" target"<<m_qty<< " high:" << high << " low:" << low << " tot_sent:" << total_sent
					<< " tot_exe:" << total_exe << " onum:" << onum << endl;

				loggerv2::info(ss.str().c_str());
				if (SendOrders(onum, _type, o_type))
					total_sent += onum;
			}
		}
			

		bool twap_task::SendOrders(int qty, int _type, OrderWay::type _way)
		{
			if (qty <= 0)
				return false;
			if (m_instr->get_feed_item()->is_bid_ask_active() == false)
			{
				return false;
			}

			if (_way == OrderWay::Sell)
				qty = -qty;

			OrderRestriction::type orderRestriction = OrderRestriction::None;
			OrderOpenClose::type openClose = OrderOpenClose::Undef;

			double price = qty > 0 ? m_instr->get_feed_item()->get_ask_price() : m_instr->get_feed_item()->get_bid_price();
			if (_type == AtsTradingType::Contrib)
			{
				price = qty > 0 ? m_instr->get_feed_item()->get_bid_price() : m_instr->get_feed_item()->get_ask_price();
			}

			OrderWay::type way = qty > 0 ? OrderWay::Buy : OrderWay::Sell;


			return m_instr->create_order(price, way, abs(qty)*m_unit, this, _type, orderRestriction, openClose);

		}

		bool twap_task::checkunAckOrder()
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
			for (auto &it : TotalOrders)
			{
				auto status = it->get_status();
				if (status == OrderStatus::Undef || status == OrderStatus::WaitServer || status == OrderStatus::WaitMarket)
				{
					m_lock.clear(std::memory_order_release);
					return false;
				}
			}

			m_lock.clear(std::memory_order_release);
			return true;
		}

		void twap_task::add_order_cb(order *order)
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;

			ptime tnow = date_time_publisher_gh::get_instance()->now();
			lwtp tp = ptime_to_lwtp(tnow);
			order->set_last_time(tp);

			double price = order->get_price();

			if (order->get_way() == OrderWay::Buy)
			{
				auto it = TotalBuyOrder.find(price);
				if (it == TotalBuyOrder.end())
				{
					time_order_map tmap;
					tmap.insert(std::make_pair(order->get_id(), order));
					TotalBuyOrder.insert(std::make_pair(price, tmap));
				}
				else
				{
					it->second.insert(std::make_pair(order->get_id(), order));
				}
			}
			else
			{
				auto it = TotalSellOrder.find(price);
				if (it == TotalSellOrder.end())
				{
					time_order_map tmap;
					tmap.insert(std::make_pair(order->get_id(), order));
					TotalSellOrder.insert(std::make_pair(price, tmap));
				}
				else
				{
					it->second.insert(std::make_pair(order->get_id(), order));
				}
			}

			TotalOrders.push_back(order);

			m_lock.clear(std::memory_order_release);
			if (observer != nullptr)
				observer->add_order_cb(order);
		}

		void twap_task::update_order_cb(order *order)
		{
			auto status = order->get_status();
			loggerv2::info("twap_task::update_order_cb");
			order->dump_info();
			if (/*status == OrderStatus::Nack ||*/ status == OrderStatus::Reject || status == OrderStatus::Cancel)
			{
				if (order->get_instrument()->getCode() == m_instr->get_trade_item()->getCode())
				{
					int cancle_num = abs(order->get_quantity() - order->get_exec_quantity());
					m_Nack_unit = m_Nack_unit + cancle_num;
					//m_Nack = m_Nack + ((float)cancle_num) / m_unit;
					loggerv2::info("twap_task::update_order_cb,cancle_num:%d", cancle_num);
				}
				else
				{
					stringstream ss;
					ss << "error update,order code is:" << order->get_instrument()->getCode() << " instr code is:" << m_instr->get_trade_item()->getCode() 
						<<" order id is:"<<order->get_id()<< endl;
					cout << ss.str();

					loggerv2::error(ss.str().c_str());
				}
			}


			if (observer != nullptr)
				observer->update_order_cb(order);
		}

		void twap_task::inactive_order_cb(order *order)
		{
			;
		}

		void twap_task::add_exec_cb(exec *exec)
		{
			total_exe_unit = total_exe_unit + exec->getQuantity();
			//double qty = ((float)abs(exec->getQuantity())) / m_unit;

			//loggerv2::info("Turtle::AddExecCB,num:%d", total_exe_unit);

			if (exec->getTradeItem()->getCode() == m_instr->get_trade_item()->getCode())
			{
				//Future1Pos += qty;
				total_exe = (float)total_exe_unit / m_unit;
			}
			if (observer != nullptr)
				observer->add_exec_cb(exec);
		}

		void twap_task::clear_orders()
		{
			;
		}

		void twap_task::clear_execs()
		{
			;
		}

		void twap_task::killall()
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
			for (auto &it : TotalOrders)
			{
				auto status = it->get_status();
				if (status == OrderStatus::Ack)
					it->Cancel();
			}
			TotalOrders.clear();
			TotalBuyOrder.clear();
			TotalSellOrder.clear();

			m_lock.clear(std::memory_order_release);
		}

		int twap_task::kill_bad_price()
		{
			while (m_lock.test_and_set(std::memory_order_acquire))
				;
			int cancel_num = 0;

			if (o_type == OrderWay::Buy)
			{
				auto it = TotalBuyOrder.begin();//���ȳ����۸��ϱȽϲ�ĵ�
				for (; it != TotalBuyOrder.end();)
				{
					double order_price = it->first;
					double instr_bid_pirce = m_instr->get_feed_item()->get_bid_price();//��һ��
					double offset = (instr_bid_pirce - order_price);//ƫ��
					if (offset <= max_price_offset*instr_bid_pirce)//����۸�ƫ��С����������ֵ�򲻴���
						break;
					
					for (auto itr = it->second.begin(); itr != it->second.end();)//��������۸��ȫ����
					{
						auto status = itr->second->get_status();
						if (status == OrderStatus::Ack)
						{
							itr->second->Cancel();
							cancel_num += itr->second->get_book_quantity();
							//it->second.erase(itr++);
						}
						else if (status == OrderStatus::Nack || status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
							it->second.erase(itr++);
						else
							++itr;
					}

					if (it->second.size() == 0)
						TotalBuyOrder.erase(it++);
					else
						++it;
				}
			}
			else
			{
				auto it = TotalSellOrder.begin();//���ȳ����۸��ϱȽϲ�ĵ�
				for (; it != TotalSellOrder.end();)
				{
					double order_price = it->first;
					double instr_ask_pirce = m_instr->get_feed_item()->get_ask_price();
					double offset = (order_price - instr_ask_pirce);
					if (offset <= max_price_offset*order_price)
						break;

					for (auto itr = it->second.begin(); itr != it->second.end();)//��������۸��ȫ����
					{
						auto status = itr->second->get_status();
						if (status == OrderStatus::Ack)
						{

							itr->second->Cancel();
							cancel_num += itr->second->get_book_quantity();
							//it->second.erase(itr++);

						}
						else if (status == OrderStatus::Nack || status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
							it->second.erase(itr++);
						else
							++itr;
					}

					if (it->second.size() == 0)
						TotalSellOrder.erase(it++);
					else
						++it;
				}
				
			}
			m_lock.clear(std::memory_order_release);
			return cancel_num;
		}

		void twap_task::kill_by_time(OrderWay::type _type, int ack, int max)
		{
			if (max < 0 || max >= ack)
				return;

			int cancel_num = ack - max;

			//auto time_now = date_time_publisher_gh::get_instance()->now();
			//if ((time_now - cancel_time).total_seconds() < 1)//��ֹƵ������
			//return;

			while (m_lock.test_and_set(std::memory_order_acquire))
				;
			if (_type == OrderWay::Buy)
			{
				auto it = TotalBuyOrder.begin();//���ȳ����۸��ϱȽϲ�ĵ�
				for (; it != TotalBuyOrder.end() && cancel_num>0;)
				{
					for (auto itr = it->second.begin(); itr != it->second.end();)//�۸���ͬ������£����ȳ������ڵĵ�
					{
						auto status = itr->second->get_status();
						if (status == OrderStatus::Ack)
						{
							if (cancel_num >= itr->second->get_book_quantity())
							{
								if (itr->second->get_price() < m_instr->get_feed_item()->get_ask_price())
								{
									itr->second->Cancel();
									cancel_num -= itr->second->get_book_quantity();
									//it->second.erase(itr++);
								}
							}
							else
								++itr;
						}
						else if (status == OrderStatus::Nack || status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
							it->second.erase(itr++);
						else
							++itr;
					}

					if (it->second.size() == 0)
						TotalBuyOrder.erase(it++);
					else
						++it;
				}
			}
			else //sell
			{
				auto it = TotalSellOrder.begin();//���ȳ����۸��ϱȽϲ�ĵ�
				for (; it != TotalSellOrder.end() && cancel_num > 0;)
				{
					for (auto itr = it->second.begin(); itr != it->second.end();)//�۸���ͬ������£����ȳ������ڵĵ�
					{
						auto status = itr->second->get_status();
						if (status == OrderStatus::Ack)
						{
							if (cancel_num >= itr->second->get_book_quantity())
							{
								if (itr->second->get_price() > m_instr->get_feed_item()->get_bid_price())
								{
									itr->second->Cancel();
									cancel_num -= itr->second->get_book_quantity();
									//it->second.erase(itr++);
								}

							}
							else
								++itr;
						}
						else if (status == OrderStatus::Nack || status == OrderStatus::Reject || status == OrderStatus::Cancel || status == OrderStatus::Exec)
							it->second.erase(itr++);
						else
							++itr;
					}

					if (it->second.size() == 0)
						TotalSellOrder.erase(it++);
					else
						++it;
				}
			}
			m_lock.clear(std::memory_order_release);

			cancel_time = date_time_publisher_gh::get_instance()->now();
		}

		bool twap_task::is_nearly_complete()
		{
			return total_sent + min_order_size > m_qty;
		}
	}
}
