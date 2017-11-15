#include "quant_proxy.h"
#include "tlhelp32.h"
#include <stdio.h>
#include <string.h>
#include <WtsApi32.h>
#include <psapi.h>
#include "terra_logger.h"
#include "ht_file_connection.h"
#include "string_tokenizer.h"
#include "defaultdatetimepublisher.h"
//#include "SecurityFtdcUserApiStruct.h"
using namespace terra::common;
namespace ht_file
{
	typedef struct tagWNDINFO
	{
		DWORD dwProcessId;
		HWND  hWnd;
	} WNDINFO, *LPWNDINFO;

	BOOL CALLBACK MyEnumProc(HWND hWnd, LPARAM lParam)//枚举所有进程
	{
		DWORD dwProcId;
		GetWindowThreadProcessId(hWnd, &dwProcId);
		LPWNDINFO pInfo = (LPWNDINFO)lParam;
		if (dwProcId == pInfo->dwProcessId)
		{
			pInfo->hWnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	HWND GetProcessMainWnd(DWORD dwProcessId)//获取给定进程ID的窗口handle
	{
		WNDINFO wi;
		wi.dwProcessId = dwProcessId;
		wi.hWnd = NULL;
		EnumWindows(MyEnumProc, (LPARAM)&wi);
		return wi.hWnd;
	}

	bool ht_quant_proxy::connect()
	{
		PROCESSENTRY32 pe;
		memset(&pe, 0, sizeof(pe));
		pe.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		Process32First(hSnapshot, &pe);
		bool bFound = false;
		std::string intProvider_file;
		do
		{

			if (strcmp(pe.szExeFile, "matic.exe") == 0)//找到了进程
			{
				if (bFound == false)
				{
					bFound = true;
					m_hwnd = (int)GetProcessMainWnd(pe.th32ProcessID);
					//获取进程路径				
					HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
					char path[MAX_PATH + 1];
					memset(path, 0, sizeof(path));
					GetModuleFileNameEx(hProcess, NULL, path, MAX_PATH + 1);
					m_strAppPath = path;

					boost::filesystem::path p;
					p.clear();
					p.append(path);
					boost::filesystem::path parentPath = p.parent_path();
					m_strAppPath = parentPath.string();

					if (this->m_con)
					{
					boost::filesystem::path p;
					p.append(this->m_strAppPath);
					p.append("files");
					p.append(this->m_con->get_stock_account_name());
					m_strOrderPath = p.string();
					}

					if (boost::filesystem::exists(m_strOrderPath))
					{
						printf_ex("lts_quant_proxy::connect %s,id:%d,m_strOrderPath:%s,m_strAppPath:%s\n", pe.szExeFile, pe.th32ProcessID, m_strOrderPath.c_str(), m_strAppPath.c_str());
					}
					else
					{
						bFound = false;
					}
					//loggerv2::error("lts_quant_proxy::connect %s,id:%d,path:%s\n", pe.szExeFile, pe.th32ProcessID, path);
					printf_ex("lts_quant_proxy::connect bFound:%d\n", bFound);
				}
				else
				{
					//关闭其他winner进程,只保留一个
					printf_ex("err:Find the multiple winner.exe instance!!!");
					loggerv2::error("Find the multiple winner.exe instance!!!");
					CloseHandle(hSnapshot);
					return false;
				}
			}
		} while (Process32Next(hSnapshot, &pe));
		CloseHandle(hSnapshot);
		if (bFound == true)
		{
			this->start();
			//
			//this->req_all_trades();
		}
		else
		{
			loggerv2::error("The winner.exe didn't start!!!");
		}

		return bFound;
	}
	bool ht_quant_proxy::disconnect()
	{
		m_hwnd = 0;
		/*if (m_intProvider.is_alive() == true)
		{
			printf_ex("m_intProvider.stop\n");
			m_intProvider.stop();
		}*/
		return true;
	}
	void ht_quant_proxy::start()
	{
		std::thread t(boost::bind(&ht_quant_proxy::post_keyboard_msg, this));
		t.detach();
	}
	void ht_quant_proxy::stop()
	{		
		
	}
	void ht_quant_proxy::post_keyboard_msg()
	{
		sleep_by_milliseconds(1000);

		while (m_hwnd != 0)
		{
			//1.下单
			req_order_insert();
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F2, 1);//发送键盘消息给盈佳客户端
			sleep_by_milliseconds(500);
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F8, 1);
			sleep_by_milliseconds(500);			
			//2.下单回执
			if (is_restart)
			{
				read_rsp(is_restart);
				is_restart = false;
			}
			read_rsp();
			//3.撤单和成交回执
			read_cancel();
			read_trader();
		}
	}

	

	/*
	local_entrust_no,fund_account,exchange_type,stock_code,entrust_bs,entrust_prop,entrust_price,entrust_amount,client_filed1,clientfield2
	1,               23076653,    1,            601866,    1,         0,           1.53,         200,           client_field1,
	*/
	bool ht_quant_proxy::ReqOrderInsert(int id,const string & code, const string & exchange, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description)
	{
		int local_entrust_no = id;
		string fund_account  = m_con->get_account();
		int  exchange_type   = (exchange == "SSE") ? 1 : 2;
		string stock_code    = code;
		int    entrust_bs    = (way == OrderWay::Buy) ? 1 : 2;
		int    entrust_prop  = 0;
		double entrust_price = price;
		int    entrust_amount= quantity;
		string client_field1 = description;
		string client_field2 = "";
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d,%s,%d,%s,%d,%d,%f,%d,%s,%s", local_entrust_no, fund_account, exchange_type, stock_code, entrust_bs, entrust_prop, entrust_price, entrust_amount, client_field1.c_str(), client_field2.c_str());
		m_order_queue.push(buffer);
		return true;
	}
	/*撤单f2*/
	bool ht_quant_proxy::cancel(int orderRef)
	{
		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d\n",orderRef);
		m_order_queue.push(buffer);
		return true;
	}
	/*下单f2
	2016110100008.csv
	客户程序将委托组合写入报单文件（.csv）中，写完后移动到委托文件目录下，
	其中报单文件名格式：ORDER_xxx.yyyyMMddHHmmssSSS.csv，撤单文件名格式：CANCEL_xxx.yyyyMMddHHmmssSSS.csv，xxx为客户自定义字段，yyyyMMddHHmmssSSS表示下单时间，不符合文件名规则文件不处理
	*/
	bool ht_quant_proxy::req_order_insert()
	{
		if (m_order_queue.empty() == true)
			return false;
		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));

		string id = this->get_request_id();

		sprintf(buffer, "ORDER_%s.%s.csv", id.c_str(),id.c_str());

		boost::filesystem::path p;
		p.append(this->m_strOrderPath);
		p.append(buffer);
		string filename = p.string();
		loggerv2::info("lts_quant_proxy::ReqOrderInsert filename:%s\n", filename.c_str());
		if (boost::filesystem::exists(p))
		{
			//printf_ex("lts_quant_proxy::ReqOrderInsert already exists filename:%s\n", filename.c_str());
			loggerv2::error("lts_quant_proxy::ReqOrderInsert already exists filename:%s\n", filename.c_str());
			return false;
		}
		boost::filesystem::ofstream stream;
		stream.open(filename.c_str());
		stream << "local_entrust_no,fund_account,exchange_type,stock_code,entrust_bs,entrust_prop,entrust_price,entrust_amount,client_filed1,clientfield2";
		string order;
		int i = 0;
		while (m_order_queue.pop(order) && order.length() > 0)
		{
			stream << order;
			i++;
		}
		stream.close();

		return true;
	}
	/*成交回报f8
	2016110100008_CJHB.csv
	*/
	bool ht_quant_proxy::req_trade()
	{
		if (m_hwnd != 0)
		{
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F8, 1);
			//printf_ex("lts_quant_proxy::req_trade VK_F8\n");
			return true;
		}
		return false;
	}
	/*所有成交回报（F9）导出
	20161101_orders.csv
	F9导出时增加 TradeID字段
	*/
	bool ht_quant_proxy::req_all_trades()
	{
		if (m_hwnd != 0)
		{
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F9, 1);
			sleep_by_milliseconds(200);
			rtn_all_trades();
			return true;
		}
		return false;
	}
	/*
	交易日[0],时间[1],报单号[2],证券代码[3],证券名称[4],方向[5],本次成交数量[6],本次成交价格[7],成交编号[8],累计成交数量[9],累计成交价格[10],状态[11],备注[12],源文件[13]
	20161102,14:44:32,13138,000001,平安银行,买入,0,0.0000,,0,0.0000,a,平安银行_买,2016110200004.csv
	20161102,14:44:32,13141,159901,深100ETF,买入,0,0.0000,,0,0.0000,a,100etf_买,2016110200004.csv
	1/0:trade
	3/5:order
	*/
	bool ht_quant_proxy::rtn_all_trades()
	{
#if 0
		date d(day_clock::local_day());
		string & now = to_iso_string(d);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer,"%s_orders.csv",now.c_str());
		boost::filesystem::path p;
		p.append(this->m_strOrderPath);
		p.append(buffer);
		string filename = p.string();
		//to do ...
		if (!boost::filesystem::exists(p))
			return false;
		boost::filesystem::ifstream stream;
		stream.open(filename.c_str(),std::ifstream::in);
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0)
				break;
			if (/*line.length() == 0 ||*/line[0] == '#')
				continue;			
			if (i == 0)
			{
				i++;
				continue;
			}
			i++;	
			tokenizer.break_line(line.c_str(), szSeparators);
			if (tokenizer[11][0] == 'a')
			{
				continue;
			}
			else if (tokenizer[11][0] == '3' || tokenizer[11][0] == '5')
			{
				CSecurityFtdcOrderField orderField;
				memset(&orderField, 0, sizeof(CSecurityFtdcOrderField));
				strcpy(orderField.InsertDate, tokenizer[0]);
				strcpy(orderField.InsertTime, tokenizer[1]);
				strcpy(orderField.OrderRef, tokenizer[2]);
				strcpy(orderField.InstrumentID, tokenizer[3]);

				if (strcmp(tokenizer[5], "买入") == 0)
				{
					orderField.Direction = SECURITY_FTDC_D_Buy;
					strcpy(orderField.CombOffsetFlag, "0");
				}
				else
				{
					orderField.Direction = SECURITY_FTDC_D_Sell;
					strcpy(orderField.CombOffsetFlag, "1");
				}
				orderField.VolumeTraded = atoi(tokenizer[6]);
				strcpy(orderField.LimitPrice, tokenizer[7]);
				orderField.OrderStatus = tokenizer[11][0];
				strcpy(orderField.UserID, tokenizer[12]);
				/*
				/////////////////////////////////////////////////////////////////////////
				///TFtdcOrderStatusType是一个报单状态类型
				/////////////////////////////////////////////////////////////////////////
				///全部成交
				#define SECURITY_FTDC_OST_AllTraded '0'
				///部分成交还在队列中
				#define SECURITY_FTDC_OST_PartTradedQueueing '1'
				///部分成交不在队列中
				#define SECURITY_FTDC_OST_PartTradedNotQueueing '2'
				///未成交还在队列中
				#define SECURITY_FTDC_OST_NoTradeQueueing '3'
				///未成交不在队列中
				#define SECURITY_FTDC_OST_NoTradeNotQueueing '4'
				///撤单
				#define SECURITY_FTDC_OST_Canceled '5'
				///未知
				#define SECURITY_FTDC_OST_Unknown 'a'
				///尚未触发
				#define SECURITY_FTDC_OST_NotTouched 'b'
				///已触发
				#define SECURITY_FTDC_OST_Touched 'c'
				*/
				switch (orderField.OrderStatus)
				{
				case '1':///部分成交还在队列中
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_InsertSubmitted;
					if (orderField.VolumeTraded > 0)
					{
						//printf_ex("lts_quant_proxy::rtn_all_trades m_con->OnRtnOrderAsync %s,orderField.VolumeTraded:%d\n", orderField.InstrumentID, orderField.VolumeTraded);
						m_con->OnRtnOrderAsync(&orderField);						
					}
					continue;
					break;
				case '3':///未成交还在队列中,如果没有成交量，系统会认为是cancel
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_InsertSubmitted;
					break;
				case '5':
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_Accepted/*SECURITY_FTDC_OSS_CancelSubmitted*/;
					break;
				default:
					break;
				}								
				//printf_ex("lts_quant_proxy::rtn_all_trades m_con->OnRtnOrderAsync %s,orderField.VolumeTraded:%d\n", orderField.InstrumentID, orderField.VolumeTraded);
				m_con->OnRtnOrderAsync(&orderField);				
				continue;
			}
			CSecurityFtdcTradeField tradeField;
			memset(&tradeField, 0, sizeof(CSecurityFtdcTradeField));
			strcpy(tradeField.TradeDate, tokenizer[0]);
			strcpy(tradeField.TradingDay, tokenizer[0]);
			strcpy(tradeField.TradeTime, tokenizer[1]);
			strcpy(tradeField.OrderRef, tokenizer[2]);
			strcpy(tradeField.InstrumentID, tokenizer[3]);

			if (strcmp(tokenizer[5], "买入") == 0)
			{
				tradeField.Direction = SECURITY_FTDC_D_Buy;				
			}
			else
			{
				tradeField.Direction = SECURITY_FTDC_D_Sell;				
			}
			tradeField.Volume = atoi(tokenizer[6]);			
			strcpy(tradeField.Price, tokenizer[7]);
			strcpy(tradeField.TradeID, tokenizer[8]);
			strcpy(tradeField.UserID, tokenizer[12]);
			if (strlen(tradeField.TradeTime) > 0 && atof(tradeField.Price)>0)
			{
				m_con->OnRtnTradeAsync(&tradeField);
			}
		}
		stream.close();
#endif
		return true;
	}
	/*新增持仓资金导出（F10）
	1.20161101_position.csv
	2.20161101_account.csv
	*/
	bool ht_quant_proxy::req_account_position()
	{
		if (m_hwnd != 0)
		{
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F10, 1);
			return true;
		}
		return false;
	}
	/*
	order_10笔.20161004110504100.csv
	*/
	string  ht_quant_proxy::get_request_id()
	{
		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));
#if 0
		date d(day_clock::local_day());
		string & now = to_iso_string(d);		
		sprintf(buffer,"%s%05d",now.c_str(),m_intProvider.get_next_int());
#else
		lwtp tp = get_lwtp_now();
		auto tt = std::chrono::system_clock::to_time_t(tp);
		struct tm* ptm = localtime(&tt);				
		sprintf(buffer, "%04d%02d%02d%02d%02d%02d%03d",
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec,0);
#endif
		loggerv2::info("ht_quant_proxy::get_request_id:%s\n",buffer);		
		return buffer;
	}
	int ht_quant_proxy::get_local_entrust_no()
	{
		return 0;
		//return m_intProvider.get_next_int();
	}
	void ht_quant_proxy::init_user_info(char * user_info_file)
	{		
		boost::filesystem::path p;
		p.append(user_info_file);
		p.append("id.ini");
		intProvider_file = p.string();		
		//m_intProvider.set_filename(intProvider_file);
		//m_intProvider.start(0);
	}
	/*撤单（F9）导出*/
	
	void ht_quant_proxy::read_cancel(bool isold)
	{
		string rtn_name;
		boost::filesystem::path p;
		p.clear();
		p.append(this->m_strOrderPath);

		auto pt = to_iso_string(date_time_publisher_gh::get_instance()->now());
		auto str = pt.substr(0, 8);
		string fn = "result_cancel." + str + ".csv";
		p.append(fn);

		rtn_name = p.string();

		loggerv2::info("lts_quant_proxy::read_rsp the %s\n", rtn_name.c_str());
		if (!boost::filesystem::exists(p))
		{
			loggerv2::error("lts_quant_proxy::read_rsp didn't find the %s\n", rtn_name.c_str());
			return;
		}

		boost::filesystem::ifstream stream;
		stream.open(rtn_name.c_str());

		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0 || line[0] == '#')
				continue;
			vector<std::string> vec;
			boost::split(vec, line, boost::is_any_of(","));

			if (i == 0 || i<read_line_cancel_rtn || vec.size()<7)
			{
				++i;
				continue;
			}
			++i;
			++read_line_cancel_rtn;

			result_cancel orderField;
			orderField.file_name = vec[0];
			orderField.local_withdraw_no = vec[1];
			orderField.fund_account = vec[2];
			orderField.entrust_no = vec[3];
			orderField.entrust_no_old = vec[4];
			orderField.result = vec[5];
			orderField.remark = vec[6];

			int ret = atoi(orderField.result.data());
			switch (ret)
			{
			case 0://ack

				break;

			default:
				return;
			}

			m_con->OnRtnCancelOrderAsyn(orderField);
		}
		stream.close();
		return;
	}

	void ht_quant_proxy::read_trader(bool isold)
	{
		string rtn_name;
		boost::filesystem::path p;
		p.clear();
		p.append(this->m_strTraderPath);

		auto pt = to_iso_string(date_time_publisher_gh::get_instance()->now());
		auto str = pt.substr(0, 8);
		string fn = "ALL_DEAL." + str + ".csv";
		p.append(fn);

		rtn_name = p.string();

		loggerv2::info("lts_quant_proxy::read_rsp the %s\n", rtn_name.c_str());
		if (!boost::filesystem::exists(p))
		{
			loggerv2::error("lts_quant_proxy::read_rsp didn't find the %s\n", rtn_name.c_str());
			return;
		}

		boost::filesystem::ifstream stream;
		stream.open(rtn_name.c_str());

		const char* szSeparators = ",";
		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0 || line[0] == '#')
				continue;
			vector<std::string> vec;
			boost::split(vec, line, boost::is_any_of(","));

			if (i == 0 || i<read_line_trader_rtn || vec.size()<22)
			{
				++i;
				continue;
			}
			++i;
			++read_line_trader_rtn;

			result_trader orderField;

			orderField.id = vec[0];
			orderField.date = vec[1];
			orderField.exchange_type = vec[2];
			orderField.fund_account = vec[3];
			orderField.stock_account = vec[4];
			orderField.stock_code = vec[5];
			orderField.entrust_bs = vec[6];
			orderField.business_price = vec[7];
			orderField.business_amount = vec[8];
			orderField.business_time = vec[9];
			orderField.real_type = vec[10];
			orderField.real_status = vec[11];
			orderField.entrust_no = vec[12];
			orderField.business_balance = vec[13];
			orderField.stock_name = vec[14];
			orderField.position_str = vec[15];
			orderField.entrust_prop = vec[16];
			orderField.business_no = vec[17];
			orderField.serial_no = vec[18];
			orderField.business_times = vec[19];
			orderField.report_no = vec[20];
			orderField.orig_order_id = vec[21];

			m_con->OnRtnTradeAsync(orderField);
		}
		stream.close();
		return;
	}

	/*
	交易日[0],时间[1], 报单号[2],证券代码[3],证券名称[4],方向[5],本次成交数量[6],本次成交价格[7],累计成交数量[8],累计成交价格[9],状态[10],备注[11],      源文件[12]
	20170413, 16:39:02,2245,     019543,     16国债15,   买入,   111,            0.1000,         111,            1.0000,         0,       1    0186a41 1,2017041300004.csv
	其中方向和状态为LTS API中对应的Direction 和OrderStatus字段的枚举值
	新版:
	_WTHB.csv
	证券代码[0],交易所[1],报单号[2],报单引用[3],本地报单号[4],方向[5],开平[6],委托数量[7],剩余数量[8],委托价格[9],价格类型[10],提交状态[11],委托时间[12],撤单时间[13],委托状态[14],状态信息[15],备注[16]
	019548,     SSE,      1,        1,          71,           0,      0,      12,         12,         10.6000,    2,           0,           09:01:48,    ,            a,           报单已提交,  1    0 c3521 1
	*/
	void ht_quant_proxy::read_rsp(bool isold)
	{
		string rtn_name;
		boost::filesystem::path p;
		p.clear();
		p.append(this->m_strOrderPath);

		auto pt = to_iso_string(date_time_publisher_gh::get_instance()->now());
		auto str = pt.substr(0, 8);
		string fn = "result_order." + str + ".csv";
		p.append(fn);

		rtn_name = p.string();

		loggerv2::info("lts_quant_proxy::read_rsp the %s\n", rtn_name.c_str());
		if (!boost::filesystem::exists(p))
		{
			loggerv2::error("lts_quant_proxy::read_rsp didn't find the %s\n", rtn_name.c_str());
			return;
		}

		boost::filesystem::ifstream stream;
		stream.open(rtn_name.c_str());
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0 || line[0] == '#')
				continue;
			vector<std::string> vec;
			boost::split(vec, line, boost::is_any_of(","));

			if (i == 0 || i<read_line_order_rtn || vec.size()<7)
			{
				++i;
				continue;
			}
			++i;
			++read_line_order_rtn;
			result_orde orderField;
			orderField.file_name = vec[0];
			orderField.local_entrust_no = vec[1];
			orderField.batch_no = vec[2];
			orderField.entrust_no = vec[3];
			orderField.fund_account = vec[4];
			orderField.result = vec[5];
			orderField.remark = vec[6];

			int ret = atoi(orderField.result.data());
			switch (ret)
			{
			case 0://ack

				break;

			default:
				return;
			}

			m_con->OnRtnOrderAsync(orderField);
		}
		stream.close();

	}
}
