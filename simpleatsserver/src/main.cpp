#include "feedsourcefactory.h"
#include "common.h"
#include "instrumentcommon.h"
#include "feedcommon.h"
#include "feeditem.h"
#include "feedsource.h"
#include "atsconfig.h"
#include "atsmanager.h"
#include "atsinstrument.h"
#include "simpleats.h"
#include "simpleatsserver.h"
#include "socketconnection.h"

using namespace terra::feedcommon;
using namespace terra::ats;
using namespace terra::instrument;
using namespace simpleatsserver;
using namespace terra::common;
/*
1.feed connection
*/
zmq::context_t   g_zmq_context(4);
int main(int argc, char *argv[])
{	
	//1.配置读取
	ats_config::get_instance()->init();
	ats_config::get_instance()->load(ats_config::get_instance()->get_config_file());
	
	ats_manager::get_instance()->initialize_logger();

	//2.配置与client通信的ZMQ接口,用于发布feed/order/log等动态信息
	ats_manager::get_instance()->net_mq_publisher = new socket_connection(g_zmq_context);
	ats_manager::get_instance()->net_mq_publisher->init(ats_config::get_instance()->get_ats_publish_port().c_str());
	
	//3.读取合约信息
	ats_manager::get_instance()->initialize_referential();
	ats_manager::get_instance()->initialize_feed_sources();
	ats_manager::get_instance()->initialize_order_passing();
	ats_manager::get_instance()->initialize_position();
	ats_manager::get_instance()->initialize_security();

	//4.配置与client通信的thrift接口,用于接收client的合约标的请求信息
	simple_ats_server::get_instance()->init_thrift();

	//5.启动feed行情处理引擎
	ats_manager::get_instance()->active_feed();

	ats_manager::get_instance()->active_io_service();

	ats_manager::get_instance()->active_twap_thread_pool();
	//6.启动thrift服务
	simple_ats_server::get_instance()->start_thrift();
	return 0;
}