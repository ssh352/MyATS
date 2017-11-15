#include "gx_file_quant_proxy.h"
#include "tlhelp32.h"
#include <stdio.h>
#include <string.h>
#include <WtsApi32.h>
#include <psapi.h>
#include "terra_logger.h"
#include "gx_file_connection.h"
#include "string_tokenizer.h"
//#include "SecurityFtdcUserApiStruct.h"
using namespace terra::common;
namespace gx_file
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

	bool gx_file_quant_proxy::connect()
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
			//loggerv2::error("pe.szExeFile:%s!!!", pe.szExeFile);
			if (strcmp(pe.szExeFile, "TradeStationAgentForms.exe") == 0)//找到了进程
			{
				if (bFound == false)
				{
					bFound = true;
					m_hwnd = (int)GetProcessMainWnd(pe.th32ProcessID);
					//获取进程路径				
					//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
					//char path[MAX_PATH + 1];
					//memset(path, 0, sizeof(path));
					//GetModuleFileNameEx(hProcess, NULL, path, MAX_PATH + 1);
					//m_strAppPath = path;

					//boost::filesystem::path p;
					//p.clear();
					//p.append(path);
					//boost::filesystem::path parentPath = p.parent_path();
					//m_strAppPath = parentPath.string();

					if (this->m_con)
					{
					boost::filesystem::path p;
					p.append(this->m_con->get_order_dir());
					p.append("files.txt");					
					m_strOrderPath = p.string();
					}

					if (boost::filesystem::exists(this->m_con->get_order_dir()))
					{
						printf_ex("gx_quant_proxy::connect %s,id:%d,m_strOrderPath:%s\n", pe.szExeFile, pe.th32ProcessID, this->m_con->get_order_dir().c_str());
					}
					else
					{
						bFound = false;
					}
					//loggerv2::error("lts_quant_proxy::connect %s,id:%d,path:%s\n", pe.szExeFile, pe.th32ProcessID, path);
					printf_ex("gx_quant_proxy::connect bFound:%d\n", bFound);
				}
				else
				{
					//关闭其他winner进程,只保留一个
					printf_ex("err:Find the multiple TradeStationAgentForms.exe instance!!!");
					loggerv2::error("Find the multiple TradeStationAgentForms.exe instance!!!");
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
			loggerv2::error("The TradeStationAgentForms.exe didn't start!!!");
		}

		return bFound;
	}
	bool gx_file_quant_proxy::disconnect()
	{
		m_hwnd = 0;
		if (m_intProvider.is_alive() == true)
		{
			printf_ex("m_intProvider.stop\n");
			m_intProvider.stop();
		}
		return true;
	}
	void gx_file_quant_proxy::start()
	{
		std::thread t(boost::bind(&gx_file_quant_proxy::post_keyboard_msg, this));
		t.detach();
	}
	void gx_file_quant_proxy::stop()
	{		
		
	}
	void gx_file_quant_proxy::post_keyboard_msg()
	{
		sleep_by_milliseconds(1000);
		//int n1 = 0;
		int n2 = 0;
		//int n3 = 0;
		while (m_hwnd != 0)
		{
			//1.下单
			req_order_insert();
			sleep_by_milliseconds(500);	
			//2.撤单和下单回执
			req_real_cancel_trade();
			//3.成交回执
			req_real_trade(n2);
			req_account_position();
		}
	}	
	/*
	local_entrust_no,fund_account,exchange_type,stock_code,entrust_bs,entrust_prop,entrust_price,entrust_amount,client_filed1,clientfield2
	1,               23076653,    1,            601866,    1,         0,           1.53,         200,           client_field1,
	100001,600001.SH,200,BUY,开仓,LIMIT,4.8
	100002,000001.SZ,200,SELL,平仓,MARKET
	100004,CANCLE,100001
	END
	*/
	int gx_file_quant_proxy::ReqOrderInsert(const string & code, const string & exchange, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description)
	{
		int local_entrust_no    = get_local_entrust_no();
		string  exchange_type   = (exchange == "SSE") ? "SH" : "SZ";
		string  stock_code      = code;
		string  entrust_bs      = (way == OrderWay::Buy) ? "BUY" : "SELL";
		double entrust_price    = price;
		int    entrust_amount   = quantity;
		string open_close       = (openClose == OrderOpenClose::Close) ? "平仓" : "开仓";
		string limit_market     = (priceMode == OrderPriceMode::Limit) ? "LIMIT" : "MARKET";
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));		
		if (priceMode == OrderPriceMode::Limit)
		{
			sprintf(buffer, "%d,%s.%s,%d,%s,%s,%s,%f\n", local_entrust_no, stock_code.c_str(), exchange_type.c_str(), entrust_amount, entrust_bs.c_str(), open_close.c_str(), limit_market.c_str(), entrust_price);
		}
		else
		{
			sprintf(buffer, "%d,%s.%s,%d,%s,%s,%s\n", local_entrust_no, stock_code.c_str(), exchange_type.c_str(), entrust_amount, entrust_bs.c_str(), open_close.c_str(), limit_market.c_str());
		}
		m_order_queue.push(buffer);
		return local_entrust_no;
	}
	/*撤单f2*/
	//100004,CANCLE,100001
	bool gx_file_quant_proxy::cancel(int orderRef)
	{
		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));
		int local_entrust_no = get_local_entrust_no();
		sprintf(buffer, "%d,CANCEL,%d\n",local_entrust_no,orderRef);
		m_order_queue.push(buffer);
		return true;
	}
	/*批量下单f2
	100001,600001.SH,200,BUY,开仓,LIMIT,4.8
	100002,000001.SZ,200,SELL,平仓,MARKET
	100004,CANCLE,100001
	END
	*/
	bool gx_file_quant_proxy::req_order_insert()
	{
		if (m_order_queue.empty() == true)
			return false;		
		string filename = m_strOrderPath;		
		boost::filesystem::ofstream stream;
		stream.open(filename.c_str());
		string order;
		int i = 0;
		while (m_order_queue.pop(order) && order.length() > 0)
		{
			stream << order;
			i++;
		}
		stream << "END";
		stream.close();		
		if (i > 0)
		{
			loggerv2::info("gx_quant_proxy::ReqOrderInsert filename:%s,i:%d\n", filename.c_str(),i);		
		}
		return true;
	}
	/*
	Position.CSV
	Account,Stock,Quantity,QuantityAvailable
	410001174092,000001.SZ,100,100
	410001174092,000002.SZ,299,299
	*/
	bool gx_file_quant_proxy::req_account_position()
	{
		boost::filesystem::path p;
		p.append(this->m_con->get_result_dir());
		p.append("Position.CSV");
		string filename = p.string();
		//to do ...
		if (!boost::filesystem::exists(p))
			return false;
		//string_tokenizer<1024> tokenizer;
		//const char* szSeparators = ",";
		boost::filesystem::ifstream stream;
		stream.open(filename.c_str());
		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0)
				continue;
			if (line[0] == '#')
			{
				i++;
				continue;
			}
			i++;
			//if (i <= n)
				//continue;
			if (line.length()>10)
			{
				//m_con->OnRspQryPosition(line);
				vector<string>::iterator result = find(m_position_line_vector.begin(), m_position_line_vector.end(), line);
				if (result == m_position_line_vector.end())
				{
					m_con->OnRspQryPosition(line);
					m_position_line_vector.push_back(line);
				}
			}			
		}
		stream.close();
		//n = i;
		return true;
	}
	/*
	order_10笔.20161004110504100.csv
	*/
	string  gx_file_quant_proxy::get_request_id()
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
		loggerv2::info("gx_quant_proxy::get_request_id:%s\n",buffer);		
		return buffer;
	}
	int gx_file_quant_proxy::get_local_entrust_no()
	{
		return m_intProvider.get_next_int();
	}
	void gx_file_quant_proxy::init_user_info(char * user_info_file)
	{		
		boost::filesystem::path p;
		p.append(user_info_file);
		p.append("id.ini");
		intProvider_file = p.string();		
		m_intProvider.set_filename(intProvider_file);
		m_intProvider.start(0);
	}
	/*撤单（F9）导出*/
	bool gx_file_quant_proxy::req_real_cancel_trade()
	{
		if (m_hwnd != 0)
		{
			rtn_real_cancel_trade();
			return true;
		}
		return false;
	}
	/*
	c:\Order.CSV:
	OrderDate,OrderTime,FilledDate,FilledTime,OrderID,Stock,    buy/sell,OrderType,OrderPrice,OrderNumber,EnterNumber,NodealNumber,MatchPrice,OrderStatue,Market,Account
	20170727, 09:21:44, 20170727,  09:21:44,  100001, 600001.SH,buy,     limit,    4.8,       200,        0,          200,         0.0,       rejected,   SH,    410001174092
	20170727,09:21:44,20170727,09:21:44,100002,000001.SZ,sell,market,0.0,200,0,200,0.0,rejected,SZ,410001174092
	*/
	bool gx_file_quant_proxy::rtn_real_cancel_trade()
	{
		boost::filesystem::path p;
		p.append(this->m_con->get_result_dir());
		p.append("Order.CSV");
		string filename = p.string();
		//to do ...
		if (!boost::filesystem::exists(p))
			return false;
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";		
		boost::filesystem::ifstream stream;
		stream.open(filename.c_str());		
		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0)
				continue;			
			if (i == 0 || line[0] == '#')
			{
				i++;
				continue;
			}
			i++;			
			//if (i <= n)
				//continue;						
			if (line.length()>10)
			{
				vector<string>::iterator result = find(m_ack_line_vector.begin(), m_ack_line_vector.end(), line);
				if (result == m_ack_line_vector.end())
				{
					m_con->OnRtnOrderAsync(line);
					m_ack_line_vector.push_back(line);
				}
			}
			//
			//tokenizer.break_line(line.c_str(), szSeparators);
			//string OrderStatus = tokenizer[13];
			//if (OrderStatus == "received" || OrderStatus == "partiallyfilled")
			//{
			//	i--;
			//}
			//
		}
		stream.close();
		//n = i;
		return true;
	}
	/*
	资金账号0,    买/卖1,委托期限2,委托数量3,委托日期4,委托时间5,成交数量6,成交日期7,成交时间8,成交价格9,委托编号10,                   委托状态11,委托状态明细12,成交价格13,股票代码14,委托类型15,委托ID16
	410001174092,buy,  gfd,     100,     20170727,11:16:21,100,     20170727,11:16:21,36.450,  0_41-0001-1740-92_2-0170-727_-3001-5593,filled, filled,      36.45,   002032.sz,limit,  3
	410001174092,buy,  gfd,     100,     20170727,11:16:43,100,     20170727,11:16:43,36.410,  0_41-0001-1740-92_2-0170-727_-3001-5594,filled, filled,      36.41,   600009.sh,limit,  4
	410001174092,buy,  aut,     100,     20170727,13:19:13,100,     20170727,13:19:13,4.920,   0_41-0001-1740-92_2-0170-727_-3001-5655,filled, filled,      4.92,    000005.sz,limit,  5
	*/
	bool gx_file_quant_proxy::req_real_trade(int &n)
	{
		boost::filesystem::path p;
		p.append(this->m_con->get_result_dir());
		p.append("Match.CSV");
		string filename = p.string();
		//to do ...
		if (!boost::filesystem::exists(p))
			return false;
		//string_tokenizer<1024> tokenizer;
		//const char* szSeparators = ",";
		boost::filesystem::ifstream stream;
		stream.open(filename.c_str());
		std::string line;
		int i = 0;
		while (stream.good())
		{
			std::getline(stream, line);
			if (line.length() == 0)
				continue;
			if (line[0] == '#')
			{
				i++;
				continue;
			}
			i++;
			if (i <= n)
				continue;

			if (line.length()>10)
			{
				m_con->OnRtnTradeAsync(line);
			}
			//
			//tokenizer.break_line(line.c_str(), szSeparators);
			//string OrderStatus = tokenizer[13];
			//if (OrderStatus == "received" || OrderStatus == "partiallyfilled")
			//{
			//	i--;
			//}
			//
		}
		stream.close();
		n = i;
		return true;
	}
}
