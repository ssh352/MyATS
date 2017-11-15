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

	BOOL CALLBACK MyEnumProc(HWND hWnd, LPARAM lParam)//ö�����н���
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

	HWND GetProcessMainWnd(DWORD dwProcessId)//��ȡ��������ID�Ĵ���handle
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

			if (strcmp(pe.szExeFile, "winner.exe") == 0)//�ҵ��˽���
			{
				if (bFound == false)
				{
					bFound = true;
					m_hwnd = (int)GetProcessMainWnd(pe.th32ProcessID);
					//��ȡ����·��				
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
					//�ر�����winner����,ֻ����һ��
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
			//1.�µ�
			req_order_insert();
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F2, 1);//���ͼ�����Ϣ��ӯ�ѿͻ���
			sleep_by_milliseconds(500);
			::PostMessage((HWND)m_hwnd, WM_KEYDOWN, VK_F8, 1);
			sleep_by_milliseconds(500);			
			//2.�µ���ִ
			read_rsp();	
			//3.�����ͳɽ���ִ
			req_real_cancel_trade(n);
		}
	}

	/*
	������[0],ʱ��[1], ������[2],֤ȯ����[3],֤ȯ����[4],����[5],���γɽ�����[6],���γɽ��۸�[7],�ۼƳɽ�����[8],�ۼƳɽ��۸�[9],״̬[10],��ע[11],      Դ�ļ�[12]
	20170413, 16:39:02,2245,     019543,     16��ծ15,   ����,   111,            0.1000,         111,            1.0000,         0,       1    0186a41 1,2017041300004.csv
	���з����״̬ΪLTS API�ж�Ӧ��Direction ��OrderStatus�ֶε�ö��ֵ
	�°�:
	_WTHB.csv
	֤ȯ����[0],������[1],������[2],��������[3],���ر�����[4],����[5],��ƽ[6],ί������[7],ʣ������[8],ί�м۸�[9],�۸�����[10],�ύ״̬[11],ί��ʱ��[12],����ʱ��[13],ί��״̬[14],״̬��Ϣ[15],��ע[16]
	019548,     SSE,      1,        1,          71,           0,      0,      12,         12,         10.6000,    2,           0,           09:01:48,    ,            a,           �������ύ,  1    0 c3521 1
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
				///����
				struct CSecurityFtdcOrderField
				{
				///���͹�˾����
				TSecurityFtdcBrokerIDType	BrokerID;
				///Ͷ���ߴ���
				TSecurityFtdcInvestorIDType	InvestorID;
				///��Լ����
				TSecurityFtdcInstrumentIDType	InstrumentID;
				///��������
				TSecurityFtdcOrderRefType	OrderRef;
				///�û�����
				TSecurityFtdcUserIDType	UserID;
				///����������
				TSecurityFtdcExchangeIDType	ExchangeID;
				///�����۸�����
				TSecurityFtdcOrderPriceTypeType	OrderPriceType;
				///��������
				TSecurityFtdcDirectionType	Direction;
				///��Ͽ�ƽ��־
				TSecurityFtdcCombOffsetFlagType	CombOffsetFlag;
				///���Ͷ���ױ���־
				TSecurityFtdcCombHedgeFlagType	CombHedgeFlag;
				///�۸�
				TSecurityFtdcStockPriceType	LimitPrice;
				///����
				TSecurityFtdcVolumeType	VolumeTotalOriginal;
				///��Ч������
				TSecurityFtdcTimeConditionType	TimeCondition;
				///GTD����
				TSecurityFtdcDateType	GTDDate;
				///�ɽ�������
				TSecurityFtdcVolumeConditionType	VolumeCondition;
				///��С�ɽ���
				TSecurityFtdcVolumeType	MinVolume;
				///��������
				TSecurityFtdcContingentConditionType	ContingentCondition;
				///ֹ���
				TSecurityFtdcPriceType	StopPrice;
				///ǿƽԭ��
				TSecurityFtdcForceCloseReasonType	ForceCloseReason;
				///�Զ������־
				TSecurityFtdcBoolType	IsAutoSuspend;
				///ҵ��Ԫ
				TSecurityFtdcBusinessUnitType	BusinessUnit;
				///������
				TSecurityFtdcRequestIDType	RequestID;
				///���ر������
				TSecurityFtdcOrderLocalIDType	OrderLocalID;
				///��Ա����
				TSecurityFtdcParticipantIDType	ParticipantID;
				///�ͻ�����
				TSecurityFtdcClientIDType	ClientID;
				///��Լ�ڽ������Ĵ���
				TSecurityFtdcExchangeInstIDType	ExchangeInstID;
				///����������Ա����
				TSecurityFtdcTraderIDType	BranchPBU;
				///��װ���
				TSecurityFtdcInstallIDType	InstallID;
				///�����ύ״̬
				TSecurityFtdcOrderSubmitStatusType	OrderSubmitStatus;
				///�˻�����
				TSecurityFtdcAccountIDType	AccountID;
				///������ʾ���
				TSecurityFtdcSequenceNoType	NotifySequence;
				///������
				TSecurityFtdcDateType	TradingDay;
				///�������
				TSecurityFtdcOrderSysIDType	OrderSysID;
				///������Դ
				TSecurityFtdcOrderSourceType	OrderSource;
				///����״̬
				TSecurityFtdcOrderStatusType	OrderStatus;
				///��������
				TSecurityFtdcOrderTypeType	OrderType;
				///��ɽ�����
				TSecurityFtdcVolumeType	VolumeTraded;
				///ʣ������
				TSecurityFtdcVolumeType	VolumeTotal;
				///��������
				TSecurityFtdcDateType	InsertDate;
				///ί��ʱ��
				TSecurityFtdcTimeType	InsertTime;
				///����ʱ��
				TSecurityFtdcTimeType	ActiveTime;
				///����ʱ��
				TSecurityFtdcTimeType	SuspendTime;
				///����޸�ʱ��
				TSecurityFtdcTimeType	UpdateTime;
				///����ʱ��
				TSecurityFtdcTimeType	CancelTime;
				///����޸Ľ���������Ա����
				TSecurityFtdcTraderIDType	ActiveTraderID;
				///�����Ա���
				TSecurityFtdcParticipantIDType	ClearingPartID;
				///���
				TSecurityFtdcSequenceNoType	SequenceNo;
				///ǰ�ñ��
				TSecurityFtdcFrontIDType	FrontID;
				///�Ự���
				TSecurityFtdcSessionIDType	SessionID;
				///�û��˲�Ʒ��Ϣ
				TSecurityFtdcProductInfoType	UserProductInfo;
				///״̬��Ϣ
				TSecurityFtdcErrorMsgType	StatusMsg;
				///�û�ǿ����־
				TSecurityFtdcBoolType	UserForceClose;
				///�����û�����
				TSecurityFtdcUserIDType	ActiveUserID;
				///���͹�˾�������
				TSecurityFtdcSequenceNoType	BrokerOrderSeq;
				///��ر���
				TSecurityFtdcOrderSysIDType	RelativeOrderSysID;
				///Ӫҵ�����
				TSecurityFtdcBranchIDType	BranchID;
				///�ɽ����
				TSecurityFtdcMoneyType	TradeAmount;
				///�Ƿ�ETF
				TSecurityFtdcBoolType	IsETF;
				///��Լ����
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
				//if (strcmp(tokenizer[5], "����") == 0)
				
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
				///TFtdcOrderStatusType��һ������״̬����
				/////////////////////////////////////////////////////////////////////////
				///ȫ���ɽ�
				#define SECURITY_FTDC_OST_AllTraded '0'
				///���ֳɽ����ڶ�����
				#define SECURITY_FTDC_OST_PartTradedQueueing '1'
				///���ֳɽ����ڶ�����
				#define SECURITY_FTDC_OST_PartTradedNotQueueing '2'
				///δ�ɽ����ڶ�����
				#define SECURITY_FTDC_OST_NoTradeQueueing '3'
				///δ�ɽ����ڶ�����
				#define SECURITY_FTDC_OST_NoTradeNotQueueing '4'
				///����
				#define SECURITY_FTDC_OST_Canceled '5'
				///δ֪
				#define SECURITY_FTDC_OST_Unknown 'a'
				///��δ����
				#define SECURITY_FTDC_OST_NotTouched 'b'
				///�Ѵ���
				#define SECURITY_FTDC_OST_Touched 'c'
				*/
				switch (orderField.OrderStatus)
				{
				case '3':///δ�ɽ����ڶ�����
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
	1.�����ļ���ʽΪcsv�ļ����������漸�У�֤ȯ����,֤ȯ����,����,����,�۸�,��ע
	2016110100002.csv
	2.11000613,180ETF��6��2905A,��ƽ,1,��1,11000613_��ƽ
	11000614,180ETF��6��2954A,��,1,��1,11000614_��
	11000615,180ETF��6��3052A,����,1,��1,11000615_����
	20000808,�й�ƽ����4��2250,��ƽ,1,��1,20000808_��ƽ
	*/
	bool lts_quant_proxy::ReqOrderInsert(const string & feedCode, const string & name, OrderWay::type way, int quantity, double price, OrderRestriction::type restriction, OrderOpenClose::type openClose, OrderPriceMode::type priceMode, const string & description)
	{

		//2.
		string direction = "none";
		switch (way)
		{
		case OrderWay::Buy:
		{
			//direction = "����";		
			direction = "0";
			break;
		}
		case OrderWay::Sell:
		{
			//direction = "����";
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
	/*����f2*/
	bool lts_quant_proxy::cancel(int orderRef)
	{

		char  buffer[256];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d\n",orderRef);
		m_order_queue.push(buffer);

		return true;
	}
	/*�µ�f2
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
	/*�ɽ��ر�f8
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
	/*���гɽ��ر���F9������
	20161101_orders.csv
	F9����ʱ���� TradeID�ֶ�
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
	������[0],ʱ��[1],������[2],֤ȯ����[3],֤ȯ����[4],����[5],���γɽ�����[6],���γɽ��۸�[7],�ɽ����[8],�ۼƳɽ�����[9],�ۼƳɽ��۸�[10],״̬[11],��ע[12],Դ�ļ�[13]
	20161102,14:44:32,13138,000001,ƽ������,����,0,0.0000,,0,0.0000,a,ƽ������_��,2016110200004.csv
	20161102,14:44:32,13141,159901,��100ETF,����,0,0.0000,,0,0.0000,a,100etf_��,2016110200004.csv
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

				if (strcmp(tokenizer[5], "����") == 0)
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
				///TFtdcOrderStatusType��һ������״̬����
				/////////////////////////////////////////////////////////////////////////
				///ȫ���ɽ�
				#define SECURITY_FTDC_OST_AllTraded '0'
				///���ֳɽ����ڶ�����
				#define SECURITY_FTDC_OST_PartTradedQueueing '1'
				///���ֳɽ����ڶ�����
				#define SECURITY_FTDC_OST_PartTradedNotQueueing '2'
				///δ�ɽ����ڶ�����
				#define SECURITY_FTDC_OST_NoTradeQueueing '3'
				///δ�ɽ����ڶ�����
				#define SECURITY_FTDC_OST_NoTradeNotQueueing '4'
				///����
				#define SECURITY_FTDC_OST_Canceled '5'
				///δ֪
				#define SECURITY_FTDC_OST_Unknown 'a'
				///��δ����
				#define SECURITY_FTDC_OST_NotTouched 'b'
				///�Ѵ���
				#define SECURITY_FTDC_OST_Touched 'c'
				*/
				switch (orderField.OrderStatus)
				{
				case '1':///���ֳɽ����ڶ�����
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_InsertSubmitted;
					if (orderField.VolumeTraded > 0)
					{
						//printf_ex("lts_quant_proxy::rtn_all_trades m_con->OnRtnOrderAsync %s,orderField.VolumeTraded:%d\n", orderField.InstrumentID, orderField.VolumeTraded);
						m_con->OnRtnOrderAsync(&orderField);						
					}
					continue;
					break;
				case '3':///δ�ɽ����ڶ�����,���û�гɽ�����ϵͳ����Ϊ��cancel
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

			if (strcmp(tokenizer[5], "����") == 0)
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
	/*�����ֲ��ʽ𵼳���F10��
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
	/*������F9������*/
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

				if (strcmp(tokenizer[5], "����") == 0)
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

				if (strcmp(tokenizer[5], "����") == 0)
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

				if (strcmp(tokenizer[5], "����") == 0)
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
				///TFtdcOrderStatusType��һ������״̬����
				/////////////////////////////////////////////////////////////////////////
				///ȫ���ɽ�
				#define SECURITY_FTDC_OST_AllTraded '0'
				///���ֳɽ����ڶ�����
				#define SECURITY_FTDC_OST_PartTradedQueueing '1'
				///���ֳɽ����ڶ�����
				#define SECURITY_FTDC_OST_PartTradedNotQueueing '2'
				///δ�ɽ����ڶ�����
				#define SECURITY_FTDC_OST_NoTradeQueueing '3'
				///δ�ɽ����ڶ�����
				#define SECURITY_FTDC_OST_NoTradeNotQueueing '4'
				///����
				#define SECURITY_FTDC_OST_Canceled '5'
				///δ֪
				#define SECURITY_FTDC_OST_Unknown 'a'
				///��δ����
				#define SECURITY_FTDC_OST_NotTouched 'b'
				///�Ѵ���
				#define SECURITY_FTDC_OST_Touched 'c'
				*/
				switch (orderField.OrderStatus)
				{
				case '1':///���ֳɽ����ڶ�����
					orderField.OrderSubmitStatus = SECURITY_FTDC_OSS_InsertSubmitted;					
					break;
				case '3':///δ�ɽ����ڶ�����,���û�гɽ�����ϵͳ����Ϊ��cancel
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
