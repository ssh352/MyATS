#include "quant_proxy.h"
#include "tlhelp32.h"
#include <stdio.h>
#include <string.h>
#include <WtsApi32.h>
#include <psapi.h>
#include "terra_logger.h"
#include "lts_file_connection.h"
#include "string_tokenizer.h"
#include "SecurityFtdcUserApiStruct.h"
using namespace terra::common;
namespace lts_file
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

	quant_proxy::quant_proxy()
	{

	}


	quant_proxy::~quant_proxy()
	{
	}

	bool lts_quant_proxy::connect()
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

			if (strcmp(pe.szExeFile, "winner.exe") == 0)//找到了进程
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
					printf_ex("err:Find the multiple winner.exe instance---!!!");
					loggerv2::error("Find the multiple winner.exe instance---!!!");
					//CloseHandle(hSnapshot);
					//return false;
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
	bool lts_quant_proxy::disconnect()
	{
		m_hwnd = 0;
		if (m_intProvider.is_alive() == true)
		{
			printf_ex("m_intProvider.stop\n");
			m_intProvider.stop();
		}
		return true;
	}
	void lts_quant_proxy::start()
	{
		std::thread t(boost::bind(&lts_quant_proxy::post_keyboard_msg, this));
		t.detach();
	}
	void lts_quant_proxy::stop()
	{		
		
	}
	void lts_quant_proxy::post_keyboard_msg()
	{
		sleep_by_milliseconds(1000);
		int n = 0;
		while (m_hwnd != 0)
		{
			//1.下单
			req_order_insert();
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F2, 1);//发送键盘消息给盈佳客户端
			sleep_by_milliseconds(500);
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F8, 1);
			sleep_by_milliseconds(500);			
			//2.下单回执
			read_rsp();	
			//3.撤单和成交回执
			req_real_cancel_trade(n);
		}
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
	void lts_quant_proxy::read_rsp()
	{
		string id = "";
		boost::lockfree::spsc_queue<string, boost::lockfree::capacity<2048> > tmp_queue;
		while (m_queue.pop(id)&&id.length()>0)
		{			
			string tmp_id = id;
			boost::filesystem::path p;
			p.clear();
			p.append(this->m_strOrderPath);
			//tmp_id.append("_CJHB.csv");
			tmp_id.append("_WTHB.csv");
			p.append(tmp_id);
			string filename = p.string();						
			loggerv2::info("lts_quant_proxy::read_rsp the id %s,%s\n", id.c_str(), filename.c_str());
			if (!boost::filesystem::exists(p))
			{
				//printf_ex("lts_quant_proxy::read_rsp didn't find the %s\n",filename.c_str());
				loggerv2::info("lts_quant_proxy::read_rsp didn't find the %s\n", filename.c_str());
				tmp_queue.push(id);				
				continue;
			}
			boost::filesystem::ifstream stream;
			stream.open(filename.c_str());
			string_tokenizer<1024> tokenizer;
			const char* szSeparators = ",";
			std::string line;
			int i = 0;
			while (stream.good())
			{
				std::getline(stream, line);
				if (line.length() == 0 || line[0] == '#')
					continue;
				tokenizer.break_line(line.c_str(), szSeparators);
				if (i == 0)
				{
					i++;
					continue;
				}
				i++;
				/*
				///报单
				struct CSecurityFtdcOrderField
				{
				///经纪公司代码
				TSecurityFtdcBrokerIDType	BrokerID;
				///投资者代码
				TSecurityFtdcInvestorIDType	InvestorID;
				///合约代码
				TSecurityFtdcInstrumentIDType	InstrumentID;
				///报单引用
				TSecurityFtdcOrderRefType	OrderRef;
				///用户代码
				TSecurityFtdcUserIDType	UserID;
				///交易所代码
				TSecurityFtdcExchangeIDType	ExchangeID;
				///报单价格条件
				TSecurityFtdcOrderPriceTypeType	OrderPriceType;
				///买卖方向
				TSecurityFtdcDirectionType	Direction;
				///组合开平标志
				TSecurityFtdcCombOffsetFlagType	CombOffsetFlag;
				///组合投机套保标志
				TSecurityFtdcCombHedgeFlagType	CombHedgeFlag;
				///价格
				TSecurityFtdcStockPriceType	LimitPrice;
				///数量
				TSecurityFtdcVolumeType	VolumeTotalOriginal;
				///有效期类型
				TSecurityFtdcTimeConditionType	TimeCondition;
				///GTD日期
				TSecurityFtdcDateType	GTDDate;
				///成交量类型
				TSecurityFtdcVolumeConditionType	VolumeCondition;
				///最小成交量
				TSecurityFtdcVolumeType	MinVolume;
				///触发条件
				TSecurityFtdcContingentConditionType	ContingentCondition;
				///止损价
				TSecurityFtdcPriceType	StopPrice;
				///强平原因
				TSecurityFtdcForceCloseReasonType	ForceCloseReason;
				///自动挂起标志
				TSecurityFtdcBoolType	IsAutoSuspend;
				///业务单元
				TSecurityFtdcBusinessUnitType	BusinessUnit;
				///请求编号
				TSecurityFtdcRequestIDType	RequestID;
				///本地报单编号
				TSecurityFtdcOrderLocalIDType	OrderLocalID;
				///会员代码
				TSecurityFtdcParticipantIDType	ParticipantID;
				///客户代码
				TSecurityFtdcClientIDType	ClientID;
				///合约在交易所的代码
				TSecurityFtdcExchangeInstIDType	ExchangeInstID;
				///交易所交易员代码
				TSecurityFtdcTraderIDType	BranchPBU;
				///安装编号
				TSecurityFtdcInstallIDType	InstallID;
				///报单提交状态
				TSecurityFtdcOrderSubmitStatusType	OrderSubmitStatus;
				///账户代码
				TSecurityFtdcAccountIDType	AccountID;
				///报单提示序号
				TSecurityFtdcSequenceNoType	NotifySequence;
				///交易日
				TSecurityFtdcDateType	TradingDay;
				///报单编号
				TSecurityFtdcOrderSysIDType	OrderSysID;
				///报单来源
				TSecurityFtdcOrderSourceType	OrderSource;
				///报单状态
				TSecurityFtdcOrderStatusType	OrderStatus;
				///报单类型
				TSecurityFtdcOrderTypeType	OrderType;
				///今成交数量
				TSecurityFtdcVolumeType	VolumeTraded;
				///剩余数量
				TSecurityFtdcVolumeType	VolumeTotal;
				///报单日期
				TSecurityFtdcDateType	InsertDate;
				///委托时间
				TSecurityFtdcTimeType	InsertTime;
				///激活时间
				TSecurityFtdcTimeType	ActiveTime;
				///挂起时间
				TSecurityFtdcTimeType	SuspendTime;
				///最后修改时间
				TSecurityFtdcTimeType	UpdateTime;
				///撤销时间
				TSecurityFtdcTimeType	CancelTime;
				///最后修改交易所交易员代码
				TSecurityFtdcTraderIDType	ActiveTraderID;
				///结算会员编号
				TSecurityFtdcParticipantIDType	ClearingPartID;
				///序号
				TSecurityFtdcSequenceNoType	SequenceNo;
				///前置编号
				TSecurityFtdcFrontIDType	FrontID;
				///会话编号
				TSecurityFtdcSessionIDType	SessionID;
				///用户端产品信息
				TSecurityFtdcProductInfoType	UserProductInfo;
				///状态信息
				TSecurityFtdcErrorMsgType	StatusMsg;
				///用户强评标志
				TSecurityFtdcBoolType	UserForceClose;
				///操作用户代码
				TSecurityFtdcUserIDType	ActiveUserID;
				///经纪公司报单编号
				TSecurityFtdcSequenceNoType	BrokerOrderSeq;
				///相关报单
				TSecurityFtdcOrderSysIDType	RelativeOrderSysID;
				///营业部编号
				TSecurityFtdcBranchIDType	BranchID;
				///成交金额
				TSecurityFtdcMoneyType	TradeAmount;
				///是否ETF
				TSecurityFtdcBoolType	IsETF;
				///合约类型
				TSecurityFtdcInstrumentTypeType	InstrumentType;
				};
				*/
				CSecurityFtdcOrderField orderField;
				memset(&orderField, 0, sizeof(CSecurityFtdcOrderField));
				//strcpy(orderField.InsertDate, tokenizer[0]);
				//strcpy(orderField.InsertTime, tokenizer[1]);
				strcpy(orderField.InsertTime, tokenizer[12]);
				strcpy(orderField.OrderRef, tokenizer[2]);		
				//strcpy(orderField.InstrumentID, tokenizer[3]);
				strcpy(orderField.InstrumentID, tokenizer[0]);				
				//if (strcmp(tokenizer[5], "买入") == 0)
				
				//if (strcmp(tokenizer[5], "0") == 0)
				//{
				//	orderField.Direction = SECURITY_FTDC_D_Buy;
				//}
				//else
				//{
				//	orderField.Direction = SECURITY_FTDC_D_Sell;
				//}
				orderField.Direction = tokenizer[5][0];

				//orderField.VolumeTraded = atoi(tokenizer[6]);
				//strcpy(orderField.LimitPrice, tokenizer[7]);
				orderField.VolumeTraded = atoi(tokenizer[7]);
				strcpy(orderField.LimitPrice, tokenizer[9]);

				orderField.OrderSubmitStatus = tokenizer[11][0];
				//orderField.OrderStatus = tokenizer[10][0];
				orderField.OrderStatus = tokenizer[14][0];

				//strcpy(orderField.UserID, tokenizer[11]);
				strcpy(orderField.UserID, tokenizer[16]);
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
				case '3':///未成交还在队列中
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_InsertSubmitted;
					break;
				case '5':
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_CancelSubmitted;
					break;				
				default:
					//printf_ex("didn't do with the order status:%c\n", orderField.OrderStatus);
					continue;
					break;
				}								
				loggerv2::info("lts_quant_proxy::read_rsp m_con->OnRtnOrderAsync %s\n", orderField.InstrumentID);
				m_con->OnRtnOrderAsync(&orderField);
			}
			stream.close();
		}	
		while (tmp_queue.pop(id))
		{
			m_queue.push(id);
			//printf_ex("lts_quant_proxy::read_rsp reinesert the id:%s\n", id.c_str());
			loggerv2::info("lts_quant_proxy::read_rsp reinesert the id:%s\n", id.c_str());
		}
	}

	/*
	1.输入文件格式为csv文件，包括下面几列：证券代码,证券名称,方向,数量,价格,备注
	2016110100002.csv
	2.11000613,180ETF购6月2905A,卖平,1,买1,11000613_卖平
	11000614,180ETF购6月2954A,买开,1,卖1,11000614_买开
	11000615,180ETF购6月3052A,卖开,1,买1,11000615_卖开
	20000808,中国平安沽4月2250,买平,1,卖1,20000808_买平
	*/
	bool lts_quant_proxy::ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description)
	{

		//2.
		string direction = "none";
		switch (way)
		{
		case OrderWay::Buy:
		{
			//direction = "买入";		
			direction = "0";
			break;
		}
		case OrderWay::Sell:
		{
			//direction = "卖开";
			direction = "1";
			break;
		}
		default:
			break;
		}
		//
		string CombOffsetFlag;
		switch (openClose)
		{
		case OrderOpenClose::Open:
			CombOffsetFlag = "0";
			break;		
		default:
			CombOffsetFlag = "1";
			break;
		}
		//
		string OrderPriceType;
		switch (priceMode)
		{
		case OrderPriceMode::Limit:
			OrderPriceType = "2";
			break;
		default:
			OrderPriceType = "3";
			break;
		}
		//
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		//sprintf(buffer, "%s,%s,%s,%d,%f,%s\n", feedCode.c_str(), name.c_str(), direction.c_str(), quantity, price, description.c_str());
		sprintf(buffer, "%s,%s,%s,%s,%d,%f,%s,%s\n", feedCode.c_str(), name.c_str(), direction.c_str(), CombOffsetFlag.c_str(), quantity, price,OrderPriceType.c_str(),description.c_str());
		m_order_queue.push(buffer);

		return true;
	}
	/*撤单f2*/
	bool lts_quant_proxy::cancel(int orderRef)
	{

		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d\n",orderRef);
		m_order_queue.push(buffer);

		return true;
	}
	/*下单f2
	2016110100008.csv
	*/
	bool lts_quant_proxy::req_order_insert()
	{
		if (m_order_queue.empty() == true)
			return false;
		char        buffer[256];
		memset(buffer, 0, sizeof(buffer));
		string id = this->get_request_id();
		sprintf(buffer, "%s.csv", id.c_str());
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
		string order;
		int i = 0;
		while (m_order_queue.pop(order) && order.length() > 0)
		{
			stream << order;
			i++;
		}
		stream.close();
		if (i > 0)
		{					
			m_queue.push(id);
		}
#if 0
		if (m_hwnd != 0)
		{
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F2, 1);
			//printf_ex("lts_quant_proxy::req_order_insert VK_F2\n");
			return true;
		}
#endif
		return false;
	}
	/*成交回报f8
	2016110100008_CJHB.csv
	*/
	bool lts_quant_proxy::req_trade()
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
	bool lts_quant_proxy::req_all_trades()
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
	bool lts_quant_proxy::rtn_all_trades()
	{
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
	}
	/*新增持仓资金导出（F10）
	1.20161101_position.csv
	2.20161101_account.csv
	*/
	bool lts_quant_proxy::req_account_position()
	{
		if (m_hwnd != 0)
		{
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F10, 1);
			return true;
		}
		return false;
	}
	string  lts_quant_proxy::get_request_id()
	{
		date d(day_clock::local_day());
		string & now = to_iso_string(d);
		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer,"%s%05d",now.c_str(),m_intProvider.get_next_int());
		loggerv2::info("lts_quant_proxy::get_request_id:%s\n",buffer);		
		return buffer;
	}
	void lts_quant_proxy::init_user_info(char * user_info_file)
	{		
		boost::filesystem::path p;
		p.append(user_info_file);
		p.append("id.ini");
		intProvider_file = p.string();		
		m_intProvider.set_filename(intProvider_file);
		m_intProvider.start(0);
	}
	/*撤单（F9）导出*/
	bool lts_quant_proxy::req_real_cancel_trade(int &n)
	{
		if (m_hwnd != 0)
		{
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F9, 1);
			sleep_by_milliseconds(200);
			rtn_real_cancel_trade(n);
			return true;
		}
		return false;
	}
	bool lts_quant_proxy::rtn_real_cancel_trade(int &n)
	{
		date d(day_clock::local_day());
		string & now = to_iso_string(d);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%s_orders.csv", now.c_str());
		boost::filesystem::path p;
		p.append(this->m_strOrderPath);
		p.append(buffer);
		string filename = p.string();
		//to do ...
		if (!boost::filesystem::exists(p))
			return false;
		boost::filesystem::ifstream stream;
		stream.open(filename.c_str());
		string_tokenizer<1024> tokenizer;
		const char* szSeparators = ",";
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
			if (i <= n)
				continue;
			tokenizer.break_line(line.c_str(), szSeparators);
			if (tokenizer[11][0] == '5')
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
				orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_CancelSubmitted;
				//
				if (strlen(orderField.UserID)<1)
				{
					memset(orderField.UserID, 0, sizeof(orderField.UserID));
					strcpy(orderField.UserID, m_con->get_user_id(atoi(orderField.OrderRef)).c_str());
				}
				//
				loggerv2::info("5.lts_quant_proxy::rtn_real_cancel_trade m_con->OnRtnOrderAsync %s,orderRef:%s\n", orderField.InstrumentID, orderField.OrderRef);
				m_con->OnRtnOrderAsync(&orderField);
			}	
			else if ((tokenizer[11][0] == '0' || tokenizer[11][0] == '1') && strlen(tokenizer[1]) > 0 && atof(tokenizer[7]) > 0)
			{
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
					loggerv2::info("0.lts_quant_proxy::rtn_real_cancel_trade m_con->OnRtnTradeAsync %s,orderRef:%s\n", tradeField.InstrumentID, tradeField.OrderRef);
					m_con->OnRtnTradeAsync(&tradeField);
				}
			}
			else if (tokenizer[11][0] == '3' || tokenizer[11][0] == '1')
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
					break;
				case '3':///未成交还在队列中,如果没有成交量，系统会认为是cancel
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_InsertSubmitted;
					break;
				case '5':
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_Accepted;
					break;
				default:
					continue;
					break;
				}
				//printf_ex("lts_quant_proxy::rtn_real_cancel_trade m_con->OnRtnOrderAsync %s,orderField.VolumeTraded:%d\n", orderField.InstrumentID, orderField.VolumeTraded);
				//
				if (strlen(orderField.UserID)<1)
				{
					memset(orderField.UserID, 0, sizeof(orderField.UserID));
					strcpy(orderField.UserID, m_con->get_user_id(atoi(orderField.OrderRef)).c_str());
				}
				//
				loggerv2::info("3.lts_quant_proxy::rtn_real_cancel_trade m_con->OnRtnOrderAsync %s,orderRef:%s\n", orderField.InstrumentID, orderField.OrderRef);
				m_con->OnRtnOrderAsync(&orderField);				
			}
		}
		stream.close();
		n = i;
		return true;
	}
}
