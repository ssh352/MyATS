#include "feedsource.h"
#include "feeditem.h"
#include "abstract_database.h"
#include "database_factory.h"
#include <boost/algorithm/string.hpp>
#include "nn.h"
#include "pubsub.h"
#include "io_service_gh.h"
#include "reqrep.h"
namespace terra
{
	namespace feedcommon
	{
		feed_source_container * feed_source_container::g_feed_source_container = nullptr;
		//
		void tick_snapshot::update(void* pMsg, int size)
		{
			if (sizeof(m_buffer) > size)
			{
				memcpy_lw(m_buffer, pMsg, size);
				m_size = size;
			}
			else
			{
				printf_ex("tick_snapshot::update %s,err,for the max len:%d,but in fact the len:%d\n",m_code.c_str(),sizeof(m_buffer),size);
			}
		}
		//
		feed_source::feed_source(const string& type, const string& name, const  string& server, const string& port, const string& brokerid, const string& username, const string& passwd, const string& db, string pub, string url, string req_url)
		{
			_server = server;
			_port = port;
			_brokerid = brokerid;
			_username = username;
			_passwd = passwd;
			_db = db;
			_activated = false;
			_needUpdate = true;
			m_status = AtsType::FeedSourceStatus::Down;
			m_pConnection = nullptr;
			Type = type;
			Name = name;

			vector<string> un;
			boost::split(un, Name, boost::is_any_of("@"));
			if (un.size()==2) 
			{
				//printf_ex("source name changed from %s to %s\n",Name.c_str(),un[0].c_str());
				printf_ex("code type changed from %s to %s\n",type.c_str(),un[1].c_str());
				//Name = un[0];
				Type = un[1];
			}

			//m_status_changed_callback = nullptr;
			m_decoder_callback = nullptr;
			feed_source_container::get_instance()->add(Name, this);
			//
			m_strPub = pub;
			m_strUrl = url;
			m_strUrl_REQREP = req_url;
			//
		}
		


		feed_source::~feed_source()
		{
			release_source();
			//if (m_pConnection)
			//{
			//	m_pConnection->cleanup();
			//	delete m_pConnection;
			//	m_pConnection = nullptr;
			//}
			//is_process = false;
			//m_thread.join();
		}
		void feed_source::init_source()
		{
			this->init_pub_sub();
		}

		void feed_source::sub_callback()
		{
			while (1)
			{
				char *buf = NULL;
				int bytes = nn_recv(m_reqrep, &buf, NN_MSG, 0);
				if (bytes >= 0)
				{					
					std::string code = string(buf,bytes);
					vector<string> un;
					boost::split(un,code, boost::is_any_of("|"));
					AtsType::InstrType::type type = AtsType::InstrType::Undef;
					if (un.size() > 1)
					{
						code = un[0];
						type = (AtsType::InstrType::type)atoi(un[1].c_str());
					}
					printf_ex("feed_source::sub_callback rcv:%s,pub,%s\n",code.c_str(),this->get_name().c_str());
					loggerv2::info("feed_source::sub_callback rcv:%s,pub,%s\n", code.c_str(), this->get_name().c_str());

					feed_item *item = get_feed_item(code);
					if (item == nullptr)
					{
						//printf_ex("feed_source::sub_callback:no such item:%s\n", code.c_str());
						//continue;
						item = this->create_feed_item(code, type);
					}
					m_pConnection->subscribe_item(item);

					//std::string str = std::string("sub ") + buf + std::string(" fin");
					//nn_send(m_reqrep, str.data(), str.size(), 0);
					//nn_freemsg(buf);
					//
					tbb::concurrent_hash_map<string, tick_snapshot*>::const_accessor ra;
					if (m_tick_snapshot_map.find(ra, code))
					{						
						if (m_pub_handle > -1)
						{
							int rc = nn_send(m_pub_handle, ra->second->m_buffer,ra->second->m_size, 0);
						}
					}
					//
				}
				
			}
		}

		void feed_source::init_pub_sub()
		{
			if (m_strUrl.length() > 0)
			{
				char buffer[64];
				memset(buffer, 0, sizeof(buffer));
				strcat(buffer, m_strUrl_REQREP.c_str());
#ifdef Linux
				buffer[strlen(buffer) - 1] = 0;
#endif
				if (m_strPub == "pub")
				{
					m_pub_handle = nn_socket(AF_SP, NN_PUB);
					m_reqrep = nn_socket(AF_SP, NN_REP);
					int rc = nn_bind(m_pub_handle, m_strUrl.c_str());
					loggerv2::info("feed_source::init_source,nn_bind url:%s,%d(%s),%s,%s", m_strUrl.c_str(), rc, nn_strerror(nn_errno()), this->get_name().c_str(), m_strPub.c_str());
					printf_ex("feed_source::init_source,nn_bind url:%s,%d(%s),%s,%s\n", m_strUrl.c_str(), rc, nn_strerror(nn_errno()), this->get_name().c_str(), m_strPub.c_str());
					int res = nn_bind(m_reqrep, buffer);
					loggerv2::info("feed_source::init_source,nn_bind req url:%s,%d(%s),%s,%s", buffer, rc, nn_strerror(nn_errno()), this->get_name().c_str(),m_strPub.c_str());
					printf_ex("feed_source::init_source,nn_bind req url:%s,%d(%s),%s,%s\n", buffer, rc, nn_strerror(nn_errno()), this->get_name().c_str(), m_strPub.c_str());
					if (rc < 0||res<0)
					{
						nn_close(m_pub_handle);
						m_pub_handle = -1;

						loggerv2::info("feed_source::init_source,nn_bind fail,rc:%d,res:%d,pub,%s", rc, res, this->get_name().c_str());
						printf_ex("feed_source::init_source,nn_bind fail,rc:%d,res:%d,pub,%s\n", rc, res, this->get_name().c_str());						
						//exit(1);
					}

					std::thread process_sub(std::bind(&feed_source::sub_callback, this));
					m_process_sub.swap(process_sub);
				}
				else if (m_strPub == "sub")
				{

					m_sub_handle = nn_socket(AF_SP, NN_SUB);
					m_reqrep = nn_socket(AF_SP, NN_REQ);
					if (m_sub_handle >= 0 && m_reqrep>=0)
					{
						if (nn_setsockopt(m_sub_handle, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) < 0)
						{
							loggerv2::info("feed_source::init_source,set_sockopt:%s fail", m_strUrl.c_str());
							printf_ex("feed_source::init_source,set_sockopt:%s fail\n", m_strUrl.c_str());
							//exit(1);
						}
						int rc = nn_connect(m_sub_handle, m_strUrl.c_str());
						loggerv2::info("feed_source::init_source,nn_connect url:%s,%d(%s),%s,%s", m_strUrl.c_str(), rc, nn_strerror(nn_errno()), this->get_name().c_str(), m_strPub.c_str());
						printf_ex("feed_source::init_source,nn_connect url:%s,%d(%s),%s,%s\n", m_strUrl.c_str(), rc, nn_strerror(nn_errno()), this->get_name().c_str(), m_strPub.c_str());

						int res = nn_connect(m_reqrep, buffer);
						printf_ex("feed_source::init_source,nn_connect req url:%s,%d(%s),%s,%s\n",buffer, res, nn_strerror(nn_errno()), this->get_name().c_str(), m_strPub.c_str());
						if (rc >= 0&&res>=0)
						{
							
							update_state(AtsType::FeedSourceStatus::Up, "");
							//m_receiver_thread = std::thread(&feed_source::start_receiver, this);
#ifdef __linux__
							io_service_gh::get_instance().add_nanomsg_rcv_handler(std::bind(&feed_source::start_receiver, this));
#else
							boost::asio::high_resolution_timer* timer = new boost::asio::high_resolution_timer(*(io_service_gh::get_instance().get_io_service(io_service_type::feed)), std::chrono::milliseconds(5));
							timer->async_wait(boost::bind(&feed_source::process_loop, this, boost::asio::placeholders::error, timer));
#endif
						}
						else
						{
							nn_close(m_sub_handle);
							m_sub_handle = -1;
						}
					}
				}
			}
		}

		void feed_source::process_loop(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		{
			start_receiver();
			t->expires_from_now(std::chrono::microseconds(15));
			t->async_wait(boost::bind(&feed_source::process_loop, this, boost::asio::placeholders::error, t));
			return;
		}

		bool feed_source::publish_msg(void* pMsg, int size, const string & code)
		{			
			if (m_pub_handle > -1)
			{
				//
				tbb::concurrent_hash_map<string, tick_snapshot*>::const_accessor ra;
				if (!m_tick_snapshot_map.find(ra, code))
				{
					//
					tick_snapshot* tick = new tick_snapshot();
					tick->m_code = code;
					tick->update(pMsg, size);
					ra.release();
					//
					tbb::concurrent_hash_map<string, tick_snapshot*>::accessor wa;
					m_tick_snapshot_map.insert(wa, code);
					wa->second = tick;
					wa.release();
					//
				}
				else
				{
					ra->second->update(pMsg, size);
				}
				//
				int rc = nn_send(m_pub_handle, pMsg, size, 0);
				if (rc >= 0)
				{
					//
				    //static int i = 0;
					//i++;
					//printf_ex("feed_source::publish_msg,%d,%s\n", i,this->get_name().c_str());
					//
					return true;
				}
				else
				{
					loggerv2::info("feed_source::publish_msg,nn_send:%d(%s)",rc, nn_strerror(nn_errno()));
				}
			}
			return false;
		}
		void feed_source::update_state(AtsType::FeedSourceStatus::type newState, const string& message)
		{
			m_status = newState;
			m_errMsg = message;
			if (this->Status_Changed_Handler)
			{
				Status_Changed_Handler(this);
			}
		}
		feed_item * feed_source::get_feed_item(string& feed_code)
		{
			tbb::concurrent_hash_map<string, feed_item*>::const_accessor ra;
			if (_feedersByFeedCode.find(ra, feed_code))
			{
				return ra->second;
			}
			else
				return nullptr;
		}
		bool feed_source::add_feed_item(feed_item * fitem)
		{
			if (fitem == nullptr)
				return false;
			tbb::concurrent_hash_map<string, feed_item*>::const_accessor ra;
			if (!_feedersByFeedCode.find(ra, fitem->get_feed_code()))
			{
				ra.release();
				tbb::concurrent_hash_map<string, feed_item*>::accessor wa;
				_feedersByFeedCode.insert(wa, fitem->get_feed_code());
				wa->second = fitem;
				wa.release();
				loggerv2::info("feed_source::add_feed_item,ok,%s\n", fitem->get_feed_code().c_str());
				return true;
			}
			cout << "hash size is:" << _feedersByFeedCode.size() << endl;
			/*if (_feedersByFeedCode.find(feed_item->get_code()) == _feedersByFeedCode.end())
			{
				_feedersByFeedCode[feed_item->get_feed_code()] = feed_item;
				return true;
			}*/
			return false;
		}
		bool feed_source::remove_feed_item(feed_item ** ppfeed_item)
		{
			if (ppfeed_item == nullptr || *ppfeed_item == nullptr)
				return false;
			for (auto & it : _feedersByFeedCode)
			{
				if (it.second == *ppfeed_item)
				{
					_feedersByFeedCode.erase(it.first);
					delete (*ppfeed_item);
					*ppfeed_item = nullptr;
					return true;
				}
			}
			return false;
		}
		void feed_source::remove_all_item()
		{
			for (auto & it : _feedersByFeedCode)
			{
				//_feedersByFeedCode.erase(it.first);
				un_subscribe(it.second);
				delete it.second;
			}
			//
			_feedersByFeedCode.clear();
			//
		}

		bool feed_source::resubscribe_all()
		{
			//cout << "feed_source::resubscribe_all hash size is:" << _feedersByFeedCode.size() << endl;
			printf_ex("feed_source::resubscribe_all,size:%d,%s\n", _feedersByFeedCode.size(), this->get_name().c_str());
			loggerv2::info("feed_source::resubscribe_all,size:%d,%s", _feedersByFeedCode.size(),this->get_name().c_str());
			for (auto & it : _feedersByFeedCode)
			{
				it.second->subscribe();
				m_pConnection->subscribe_item(it.second);
			}
			return true;
		}

		bool feed_source::unsubscribe_all()
		{
			cout << "feed_source::unsubscribe_all hash size is:" << _feedersByFeedCode.size() << endl;
			loggerv2::info("feed_source::unsubscribe_all,size:%d,%s", _feedersByFeedCode.size(), this->get_name().c_str());
			for (auto & it : _feedersByFeedCode)
			{
				it.second->un_subscribe();
				m_pConnection->unsubscribe_item(it.second);
			}
			return true;
		}

		bool feed_source::subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || m_pConnection == nullptr|| get_status() == AtsType::FeedSourceStatus::Down)
			{
				loggerv2::error("subscribe feed fail,feed_itme or connection invailable,or feedsourceStatus is Down");
				return false;
			}
			if (feed_item->is_subsribed() == true)
				return true;
			add_feed_item(feed_item);
			feed_item->subscribe();
			
			if (m_strPub == "sub")
			{
				std::string code = feed_item->get_code();
				//code+InstrType
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%s|%d", code.c_str(), feed_item->get_type());
				//
				int rc = nn_send(m_reqrep, buffer, strlen(buffer), 0);
				printf_ex("feed_source::subscribe:%s,sub,%s,%d\n",buffer,this->get_name().c_str(),rc);
				loggerv2::info("feed_source::subscribe:%s,sub,%s,%d\n", buffer, this->get_name().c_str(), rc);

				//char *buf = nullptr;
				//int bytes = nn_recv(m_reqrep, &buf, NN_MSG, 0);
				//printf_ex("feed_source::subscribe nn_recv:%s,sub\n", buf);
				//nn_freemsg(buf);
				return true;
			}
			else
				return m_pConnection->subscribe_item(feed_item);
		}
		bool  feed_source::un_subscribe(feed_item * feed_item)
		{
			if (feed_item == nullptr || m_pConnection == nullptr || get_status() == AtsType::FeedSourceStatus::Down)
				return false;
			feed_item->un_subscribe();
			//std::string sFeedCode = m_code2FeedCodeMap[feed_item->get_code()];
			//if (sFeedCode.empty())
			//{
			//	return false;
			//}
			return m_pConnection->unsubscribe_item(feed_item);
		}
		void feed_source::post(feed_item * feed_item)
		{
			if (feed_item == nullptr)
				return;
			double perf = 0.0;
			if (feed_item->get_close_price() > 0 && feed_item->get_last_price()>0)
			{
				perf = (feed_item->get_last_price() - feed_item->get_close_price()) / feed_item->get_close_price() * 100;
			}
			feed_item->set_perf(perf);
			if (m_decoder_callback != nullptr)
			{
				m_decoder_callback(get_name(), feed_item);
			}
			feed_item->on_feed_item_update_event();
		}
		void feed_source::release_source()
		{
			if (m_pConnection != nullptr)
			{
				m_pConnection->cleanup();
				//delete m_pConnection;
				//m_pConnection = nullptr;
			}
			//stop_process();
			//
			if (m_pub_handle > -1)
			{
				nn_close(m_pub_handle);
				m_pub_handle = -1;
			}
			if (m_sub_handle > -1)
			{
				nn_close(m_sub_handle);
				m_sub_handle = -1;
			}
			//
		}

		void feed_source::re_init_source()
		{
			if (m_pConnection != nullptr)
			{
				m_pConnection->init();
				//
				this->init_pub_sub();
				//
				//delete m_pConnection;
				//m_pConnection = nullptr;
				this->resubscribe_all();
			}
		}

		void feed_source::load_database()
		{
			abstract_database* db = database_factory::create("sqlite", get_database_name().c_str());

			if (db->open_database())
			{
				loggerv2::info("connect to database %s", get_database_name().c_str());
			}

			std::array<std::string, 5> sInst = { "Stocks", "ETFs", "Options", "Futures","Forex"};

			for (auto s : sInst)
			{
				std::string sCmd = "select Code, FeedCodes from " + s + " where FeedCodes like '%@" + Type + "%' ";
				std::vector<boost::property_tree::ptree>* res = db->get_table(sCmd.c_str());
				printf_ex("feed_source %s: get %s number %d\n",this->get_name().c_str(),s.c_str(),(int)res->size());

				for (std::vector<boost::property_tree::ptree>::iterator it = res->begin(); it != res->end(); ++it)
				{
					std::string temp = abstract_database::get_item((*it).get("FeedCodes", ""), Type);
					std::string code = (*it).get("Code", "");
					m_feedCode2CodeMap.insert(make_pair(temp, code));
					m_code2FeedCodeMap.insert(make_pair(code, temp));
				}
				delete res;
			}

			db->close_databse();
			loggerv2::info("%s - %d instruments loaded from database", get_name().c_str(), m_feedCode2CodeMap.size());
		}

		void feed_source::process_depth(int i, int bidQty, double bidPrice, double askPrice, int askQuantity, feed_item * feed_item)
		{
			if (feed_item == nullptr || i < 0 || i >= feed_item->get_max_depth())
				return;
			//if (feed_item->BidLevels[i] != nullptr)
			{
				//if (math2::not_zero(bidPrice))
				{
					feed_item->set_market_bid(i, bidPrice);
					feed_item->set_market_bid_qty(i, bidQty);
				}				
			}
			//if (feed_item->AskLevels[i] != nullptr)
			{
				//if (math2::not_zero(askPrice))
				{
					feed_item->set_market_ask(i, askPrice);
					feed_item->set_market_ask_qty(i, askQuantity);
				}				
			}
			if (i == 0 && feed_item->get_implicit_pre_open() == false)
			{
				//if (math2::not_zero(bidPrice))
				{
					feed_item->set_bid_price(bidPrice);
					feed_item->set_bid_quantity(bidQty);
				}				
				//if (math2::not_zero(askPrice))
				{
					feed_item->set_ask_price(askPrice);
					feed_item->set_ask_quantity(askQuantity);
				}				
			}
		}

		//void feed_source::init_process()
		//{
		//	is_alive(true);
		//	std::thread t(std::bind(&feed_source::set_kernel_timer_thread, this));
		//	m_thread.swap(t);
		//}

		//void  feed_source::set_kernel_timer_thread()
		//{
		//	boost::asio::high_resolution_timer t(io, std::chrono::microseconds(20));
		//	t.async_wait(boost::bind(&feed_source::process_loop, this, boost::asio::placeholders::error, &t));
		//	io.run();
		//}
		//void feed_source::process_loop(const boost::system::error_code&, boost::asio::high_resolution_timer* t)
		//{

		//	while (is_alive())
		//	{
		//		process();
		//		t->expires_at(t->expires_at() + std::chrono::microseconds(40));
		//		t->async_wait(boost::bind(&feed_source::process_loop, this, boost::asio::placeholders::error, t));
		//		return;

		//	}
		//}

		//void feed_source::stop_process()
		//{
		//	m_isAlive = false;
		//	m_thread.join();
		//}


	}
}