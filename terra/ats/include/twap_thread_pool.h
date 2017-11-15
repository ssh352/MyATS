#ifndef TWAP_THREAD_POOL_H
#define TWAP_THREAD_POOL_H

#include "LockFreeWorkQueue.h"
#include "atsinstrument.h"
#include "iorderobserver.h"

#include "order.h"
#include "exec.h"
#include "singleton.hpp"

#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <map>
#include <unordered_set>
#include "tbb/concurrent_queue.h"

using namespace std;
using namespace terra::common;
using namespace terra::ats;
using namespace terra::marketaccess::orderpassing;

namespace terra
{
	namespace ats
	{
		class twap_task_data
		{
		public:
			ats_instrument *instr=nullptr;
			iorderobserver *observer=nullptr;
			int qty=0;
			AtsType::InstrType::type _type;
			double alpha = 0.3;
			int min_order_size=5;
			int max_order_size=10;//����µ�
			int unit=1;//�µ���λ
			boost::posix_time::ptime btime = date_time_publisher_gh::get_instance()->now();
			boost::posix_time::ptime etime = btime + boost::posix_time::seconds(100);
			boost::posix_time::ptime hit_time = etime;
			double start_price=0.0;
			double stop_price=0.0;
			double max_price_offset = 0.0;
		};

		typedef std::map<int, order*, std::greater<int>> time_order_map;//��ʱ���orderid��¼δ������order

		class twap_task :public terra::marketaccess::orderpassing::iorderobserver
		{
		public:
			void set_task(std::shared_ptr<twap_task_data> &task);
			void add_task(twap_task *task);
			virtual void add_order_cb(order *order);//�µ��ص�,�µ��ɹ�������ģ����øú���
			virtual void update_order_cb(order *order);//�����ص�
			virtual void inactive_order_cb(order *order);

			virtual void add_exec_cb(exec *exec);//�ɽ��ص�

			virtual void clear_orders();
			virtual void clear_execs();

			twap_task();
			~twap_task();

			void process();

			bool checkunAckOrder();//���Ƿ���order����waitsrv״̬
			bool check_start_status();//���start״̬
			void do_Hitter_Trader();//�����򵥽���

			void killall();//����ȫ��δ�ɽ���order
			void kill_by_time(OrderWay::type _type, int ack, int max);//��ʱ�䳷��
			int kill_bad_price();//�����۸��Զ�ĵ�

			ats_instrument * get_ats_instrument(){ return m_instr; }
			boost::posix_time::ptime& get_begin_time(){ return btime; }
			boost::posix_time::ptime& get_end_time(){ return etime; }
			float get_total_exec(){ return total_exe; }
			float get_total_sent(){ return total_sent; }
			

			bool get_is_finished(){ return isfin; }
			void stop_task(){ isfin = true; }

			bool get_is_started(){ return is_start; }
			bool get_is_killall(){ return iskillall; }
			int get_qty_order(){ return onum; }
			int get_min_order_size(){ return min_order_size; }
			int get_max_order_size(){ return max_order_size; }
			int get_lot_qty(){ return m_unit; }
			int get_target_qty(){ return m_qty; }
			double get_alpha(){ return alpha; }
			
			OrderWay::type get_way(){ return o_type; }
			double get_max_price_offset(){ return max_price_offset; }
			bool is_nearly_complete();
		private:
			ats_instrument *m_instr;
			iorderobserver *observer;
			AtsType::InstrType::type _type;//����Ʒ����
			bool isfin;
			bool is_start;
			//Ŀ��:��etimeʱ��㵽��֮ǰ���ۻ��ɽ����ﵽqty
			int m_qty;//Ŀ��ɽ���
			boost::posix_time::ptime btime;
			boost::posix_time::ptime etime;
			boost::posix_time::ptime hit_time;

			boost::posix_time::ptime killalltime;
			boost::posix_time::ptime cancel_time;//�ϴγ���ʱ��
			int killallnum;
			//boost::posix_time::time_duration total_time;//����ʱ��
			//boost::posix_time::time_duration t;//�Ա�����ʼֵ
			//boost::posix_time::time_duration killalltime;
			//boost::posix_time::ptime t_begin;//task����ʱ��

			bool iskillall;//�Ƿ�ִ���˳���ȫ��order�Ĳ���
			atomic<float> total_exe;//�ܳɽ�(q(t))��λ
			atomic<int> total_exe_unit;
			//atomic<float> m_Nack;//���ڼ�¼�����ص��еĳ�������
			atomic<int> m_Nack_unit;
			float total_sent;//�ܳɽ�+��ACK
			
		private:
			bool SendOrders(int qty, int _type, OrderWay::type _way);
			
			std::atomic_flag m_lock;//������
			

			//float Future1Pos;//��ǰ�ɽ���
			int onum;
			int m_unit;//
			int min_order_size;
			int max_order_size;//����µ�
			double start_price = 0.0;
			double stop_price = 0.0;
			
			std::list<order*> TotalOrders;
			std::map<double, time_order_map> TotalBuyOrder;
			std::map<double, time_order_map, std::greater<double>> TotalSellOrder;
			std::list<order*> TotalHitter;
			std::list<order*> TotalSpread;

			double theory;//f(t)
			double high;//f3(x)
			double low;//f2(t)

			double k;//twap����ʱ�亯����б�ʣ�f(t) = kt,t^[0,total_time];
			double kl;//twap�������ߵ�б�ʣ�f2(t) = kl(t-total_time) + qty;��ɽ�������ΪΪq(t),��q(t)<f2(t)ʱ������������
			//�µ�ʱ��q(t)>f3(t),��f(x) = q(t),�Ա���tʱ��ƽ����x
			double kh;//twap�������ߵ�б�ʣ�f3(x) = kh(t-total_time) + qty;�µ�ʱȷ���µ���+q(t)<=f3(t)(�µ���С��minsizeʱ����);

			double alpha;//kl = (1+alpha)k,kh=(1-alpha)k
			OrderWay::type o_type;//�������ͣ���Ϊ��������
			double max_price_offset;//��������۸�ƫ����
		};
		

	//	class twap_thread
	//	{
	//	public:
	//		twap_thread();
	//		~twap_thread();
	//		
	//		void process();
	//		void push_twap_thread_task(std::shared_ptr<twap_task_data> &task);

	//		std::thread mthread;
	//		LockFreeWorkQueue<twap_task> m_task_queue;//�����������У�����twap_thread_task
	//		std::unordered_set<twap_task*> m_task_map;//���ڴ洢������Ҫ�洢��twap_thread_task
	//		std::list<twap_task*> m_dead_list;
	//		int task_num;
	//		bool m_bactive=true;
	//	};


	//	class twap_thread_pool //:public SingletonBase<twap_thread_pool>
	//	{
	//	public:
	//		twap_thread_pool(){ is_init = false; }
	//		~twap_thread_pool(){}
	//		void init();
	//		void dispatch();//�ѳɽ������¸�������С���߳�
	//		void push_task(ats_instrument *instr, iorderobserver *observer, int qty, int time);//�ⲿ���ø�API���н���
	//		void push_task(std::shared_ptr<twap_task_data>&task);//�ⲿ���ø�API���н���
	//		void active_twap();
	//		void stop_twap();
	//	private:
	//		vector<std::shared_ptr<twap_thread>>  m_data;//�̳߳� 
	//		std::thread mthread;//����dispatch�߳�
	//		tbb::concurrent_queue<std::shared_ptr<twap_task_data>> m_task_queue;//�������У����ڻ���twap_task
	//		bool is_init;
	//		int pool_size = 1;//�̳߳ش�С
	//		bool m_bactive = true;
	//	};

	//	inline void twap_thread_pool::active_twap()
	//	{
	//		if (m_bactive == true)
	//			return;
	//		for (int i = 0; i < pool_size; ++i)
	//		{
	//			m_data[i]->m_bactive = true;
	//		}
	//		m_bactive = true;
	//	}

	//	inline void twap_thread_pool::stop_twap()
	//	{
	//		if (m_bactive == false)
	//			return;
	//		for (int i = 0; i < pool_size; ++i)
	//		{
	//			m_data[i]->m_bactive = false;
	//		}
	//		m_bactive = false;
	//	}
	}
}

#endif