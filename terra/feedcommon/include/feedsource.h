#ifndef _FEED_SOURCE_V2_H_
#define _FEED_SOURCE_V2_H_
#include "abstract_processor.h"
#include "feeditem.h"
#include "boost/asio.hpp"
#include "terra_logger.h"
#include <AtsType_types.h>
#include <functional> 
#include <lockfree_classpool_workqueue.h>
#include "tbb/concurrent_hash_map.h"

#pragma once
namespace terra
{
	namespace feedcommon
	{
		class feed_connection;
		//
		class tick_snapshot
		{
		public:
			string m_code;
			char   m_buffer[1024*5];
			int    m_size;
			void update(void* pMsg, int size);
		};
		//
		class feed_source :public abstract_processor
		{
		public:
			feed_source(const string& type, const string& name, const string& server, const string& port, const string& brokerid, const string& username, const string& passwd, const string& db, string pub="", string url="",string req_url="");
			virtual ~feed_source();
		protected:
			string Type;
			string           Name;
			decoder_callback m_decoder_callback;
		public:
			string& get_name(){ return Name; }
			string& get_type(){ return Type; }
			//feed_source_type get_type(){ return Type; }
			//string to_string(){ return Name; }
			void set_decoder_callback(decoder_callback callBack){ m_decoder_callback = callBack; }
			virtual void init_source();
			feed_source_change_event_handler Status_Changed_Handler;
			//
			void init_pub_sub();
			bool publish_msg(void* pMsg, int size,const string & code);
			virtual void start_receiver(){}
			bool is_sub(){ return (m_sub_handle>-1); }
			void process_loop(const boost::system::error_code&, boost::asio::high_resolution_timer* t);
			//
		public:
			//virtual void process() = 0;
			//virtual int  process_out_bound_msg_handler() = 0;
			void load_database();
		protected:
			string _server;
			string _port;
			string _brokerid;
			string _username;
			string _passwd;
			string _db;
			bool   _activated;
			bool   _needUpdate;
			AtsType::FeedSourceStatus::type                   m_status;
			string                             m_errMsg;

			feed_connection *                   m_pConnection;
			void process_depth(int i, int bidQty, double bidPrice, double askPrice, int askQuantity, feed_item * feed_item);

			void sub_callback();
			//void init_process();
			//inline bool is_alive() { return m_isAlive; }
			//inline void is_alive(bool b) { m_isAlive = b; }
			//
			string         m_strPub = "none";
			string         m_strUrl;
			string         m_strUrl_REQREP;
			int            m_pub_handle = -1;
			int            m_sub_handle = -1;
			int				m_reqrep = -1;
			std::thread m_process_sub;
			tbb::concurrent_hash_map<string, tick_snapshot*> m_tick_snapshot_map;
			//std::thread    m_receiver_thread;
			//
		public:
			string& get_strPub(){ return m_strPub; }
			string& get_database_name(){ return _db; }
			string& get_service_name(){ return _server; }
			string& get_port(){ return _port; }
			string& get_broker(){ return _brokerid; }
			string& get_user_name(){ return _username; }
			string& get_passwd(){ return _passwd; }
			void update_state(AtsType::FeedSourceStatus::type newState, const string& message);
			AtsType::FeedSourceStatus::type get_status(){ return m_status; }
			bool get_activated(){ return _activated; }
			feed_item *create_feed_item(string& code, AtsType::InstrType::type instrType)
			{
				feed_item* feed = get_feed_item(get_feedcode_by_code(code));

				if (feed == nullptr)
					feed = new feed_item(get_name(), code, get_feedcode_by_code(code), instrType);
				return feed;
			}
			feed_item * get_feed_item(string& code);
			string& get_code_by_feedcode(string& feedcode){ return m_feedCode2CodeMap[feedcode]; }
			string& get_feedcode_by_code(string& code){ return m_code2FeedCodeMap[code]; }
			bool add_feed_item(feed_item * feed_item);
			bool remove_feed_item(feed_item ** ppfeed_item);
			void remove_all_item();
			virtual bool resubscribe_all();
			virtual bool unsubscribe_all();
			virtual bool subscribe(feed_item * feed_item);
			virtual bool un_subscribe(feed_item * feed_item);
			void post(feed_item * feed_item);
			virtual void release_source();
			virtual void re_init_source();
			//void stop_process();

			tbb::concurrent_hash_map<string, feed_item*> & get_all_item(){ return _feedersByFeedCode; }
		private:
			bool m_isAlive;
			tbb::concurrent_hash_map<string, feed_item*>            _feedersByFeedCode;
			std::unordered_map<std::string, std::string> m_feedCode2CodeMap;
			std::unordered_map<std::string, std::string> m_code2FeedCodeMap;

			//std::thread m_thread;
			//boost::asio::io_service io;
			//void process_loop(const boost::system::error_code&e, boost::asio::high_resolution_timer* t);
			//void set_kernel_timer_thread();


		};
		//class feed_decoder
		//{
		//public:
		//	feed_decoder(){}
		//	virtual ~feed_decoder(){}
		//protected:
		//	void process_depth(int i, int bidQty, double bidPrice, double askPrice, int askQuantity, feed_item * feed_item);
		//	//void process_msg(feed_item * feed_item);
		//	//void post_msg(feed_item * feed_item);
		//};
		class feed_connection
		{
		public:
			feed_connection(){}
			virtual ~feed_connection(){}
		public:
			virtual void init() = 0;
			virtual void cleanup() = 0;
			virtual void create() = 0;
			virtual bool subscribe_item(feed_item * item) = 0;
			virtual bool unsubscribe_item(feed_item * item) = 0;
		public:
			bool m_bReconnected = false;
		};
		class feed_source_container : public map_ex<string, feed_source*>
		{
		private:
			feed_source_container(){}
			static feed_source_container * g_feed_source_container;
		public:
			static feed_source_container * get_instance()
			{
				if (g_feed_source_container == nullptr)
				{
					g_feed_source_container = new feed_source_container();
				}
				return g_feed_source_container;
			}
		};
	}
}
#endif //_FEED_SOURCE_V2_H_


