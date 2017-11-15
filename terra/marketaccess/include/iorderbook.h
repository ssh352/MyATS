//#ifndef __IORDERBOOK2_H__
//#define __IORDERBOOK2_H__
//
//#include "order.h"
//
//
//namespace terra
//{
//	namespace marketaccess
//	{
//		namespace orderpassing
//		{
//			class iorderbook
//			{
//				//IOrderBook(){};
//				//virtual ~IOrderBook(){};
//			public:
//				virtual bool add(order * order) = 0;
//				virtual bool remove(order * order) = 0;
//				virtual bool contains(order * order) = 0;
//
//				virtual order * get_by_id(int id) = 0;
//
//				virtual int count() = 0;
//
//				virtual int get_nb_open_orders() = 0;
//
//				virtual void killall() = 0;
//				virtual void killall(std::string portfolio)=0;
//				virtual void killall(std::string portfolio,int Type)=0;
//			};
//
//		}
//	}
//}
//
//
//#endif