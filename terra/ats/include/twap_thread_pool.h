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
			int max_order_size=10;//最大下单
			int unit=1;//下单单位
			boost::posix_time::ptime btime = date_time_publisher_gh::get_instance()->now();
			boost::posix_time::ptime etime = btime + boost::posix_time::seconds(100);
			boost::posix_time::ptime hit_time = etime;
			double start_price=0.0;
			double stop_price=0.0;
			double max_price_offset = 0.0;
		};

		typedef std::map<int, order*, std::greater<int>> time_order_map;//按时间和orderid记录未撤单的order

		class twap_task :public terra::marketaccess::orderpassing::iorderobserver
		{
		public:
			void set_task(std::shared_ptr<twap_task_data> &task);
			void add_task(twap_task *task);
			virtual void add_order_cb(order *order);//下单回调,下单成功后其他模块调用该函数
			virtual void update_order_cb(order *order);//撤单回调
			virtual void inactive_order_cb(order *order);

			virtual void add_exec_cb(exec *exec);//成交回调

			virtual void clear_orders();
			virtual void clear_execs();

			twap_task();
			~twap_task();

			void process();

			bool checkunAckOrder();//简单是否有order处于waitsrv状态
			bool check_start_status();//检查start状态
			void do_Hitter_Trader();//主动打单交易

			void killall();//撤掉全部未成交的order
			void kill_by_time(OrderWay::type _type, int ack, int max);//按时间撤单
			int kill_bad_price();//撤掉价格较远的单

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
			AtsType::InstrType::type _type;//交易品类型
			bool isfin;
			bool is_start;
			//目标:在etime时间点到来之前，累积成交量达到qty
			int m_qty;//目标成交量
			boost::posix_time::ptime btime;
			boost::posix_time::ptime etime;
			boost::posix_time::ptime hit_time;

			boost::posix_time::ptime killalltime;
			boost::posix_time::ptime cancel_time;//上次撤单时间
			int killallnum;
			//boost::posix_time::time_duration total_time;//总体时间
			//boost::posix_time::time_duration t;//自变量初始值
			//boost::posix_time::time_duration killalltime;
			//boost::posix_time::ptime t_begin;//task启动时间

			bool iskillall;//是否执行了撤销全部order的操作
			atomic<float> total_exe;//总成交(q(t))单位
			atomic<int> total_exe_unit;
			//atomic<float> m_Nack;//用于记录撤单回调中的撤单数量
			atomic<int> m_Nack_unit;
			float total_sent;//总成交+总ACK
			
		private:
			bool SendOrders(int qty, int _type, OrderWay::type _way);
			
			std::atomic_flag m_lock;//自旋锁
			

			//float Future1Pos;//当前成交量
			int onum;
			int m_unit;//
			int min_order_size;
			int max_order_size;//最大下单
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

			double k;//twap理论时间函数的斜率：f(t) = kt,t^[0,total_time];
			double kl;//twap下限曲线的斜率，f2(t) = kl(t-total_time) + qty;令成交量函数为为q(t),当q(t)<f2(t)时触发主动交易
			//下单时如q(t)>f3(t),令f(x) = q(t),自变量t时间平移至x
			double kh;//twap上限曲线的斜率，f3(x) = kh(t-total_time) + qty;下单时确保下单量+q(t)<=f3(t)(下单量小于minsize时例外);

			double alpha;//kl = (1+alpha)k,kh=(1-alpha)k
			OrderWay::type o_type;//交易类型，分为买卖两种
			double max_price_offset;//允许的最大价格偏移量
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
	//		LockFreeWorkQueue<twap_task> m_task_queue;//无锁工作队列，缓存twap_thread_task
	//		std::unordered_set<twap_task*> m_task_map;//用于存储所有需要存储的twap_thread_task
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
	//		void dispatch();//把成交任务下给负荷最小的线程
	//		void push_task(ats_instrument *instr, iorderobserver *observer, int qty, int time);//外部调用该API进行交易
	//		void push_task(std::shared_ptr<twap_task_data>&task);//外部调用该API进行交易
	//		void active_twap();
	//		void stop_twap();
	//	private:
	//		vector<std::shared_ptr<twap_thread>>  m_data;//线程池 
	//		std::thread mthread;//运行dispatch线程
	//		tbb::concurrent_queue<std::shared_ptr<twap_task_data>> m_task_queue;//无锁队列，用于缓存twap_task
	//		bool is_init;
	//		int pool_size = 1;//线程池大小
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