#include "tdf_connection.h"
#include "boost/algorithm//string.hpp"

namespace feed
{
	namespace tdf
	{
		tdf_source* tdf_connection::instance_source = nullptr;
		tdf_connection::tdf_connection(tdf_source* pSource, unsigned int date, unsigned int time)
		{

			m_pSource = pSource;
			instance_source = m_pSource;
			m_date = date;
			m_time = time;
		}


		tdf_connection::~tdf_connection()
		{
		}

		void tdf_connection::init()
		{

			TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_INTERVAL, 10);
			TDF_SetEnv(TDF_ENVIRON_OPEN_TIME_OUT, 30);
			TDF_SetEnv(TDF_ENVIRON_MISSED_BEART_COUNT, 100);
			TDF_OPEN_SETTING settings = { 0 };
			strncpy(settings.szIp,  m_pSource->get_service_name().c_str(), sizeof(settings.szIp) - 1);
			strncpy(settings.szPort, m_pSource->get_port().c_str(), sizeof(settings.szPort) - 1);
			strncpy(settings.szUser, m_pSource->get_user_name().c_str(), sizeof(settings.szUser) - 1);
			strncpy(settings.szPwd, m_pSource->get_passwd().c_str(), sizeof(settings.szPwd) - 1);

			settings.nReconnectCount = 9999999;
			settings.nReconnectGap = 5;
			settings.pfnMsgHandler = OnDataMsg;
			settings.pfnSysMsgNotify = OnSysMsg;
			settings.nProtocol = 0;
			settings.szMarkets = "sh;sz";
			string stocklist = boost::join(subscribe_vec, ";");
			int nlen = stocklist.size();
			char* stocklist_chars = new char[nlen + 1];
			stocklist_chars[nlen] = 0;
			strncpy(stocklist_chars, stocklist.c_str(), nlen+1);
			settings.szSubScriptions = stocklist_chars;
			//settings.szSubScriptions = "";
			
			//settings.szSubScriptions = (boost::join(subscribe_vec, ";")).c_str();
			settings.nTypeFlags =  DATA_TYPE_TRANSACTION;
			settings.nDate = m_date;
			settings.nTime = m_time;
			
			loggerv2::info("tdf_source::init_source ip:%s szPort:%s szUser:%s szPwd:%s dummy instrument:%s", settings.szIp, settings.szPort, settings.szUser, settings.szPwd, settings.szSubScriptions);

			TDF_ERR nErr = TDF_ERR_SUCCESS;
			g_hTDF = TDF_Open(&settings, &nErr);
			if (g_hTDF)
			{
				loggerv2::info("tdf_source::init_source feed source up");
				m_pSource->update_state(AtsType::FeedSourceStatus::Up,"");
			}
			else
			{
				switch (nErr)
				{
				case TDF_ERR_UNKOWN:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_UNKOWN");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_UNKOWN");
				}
				break;
				case TDF_ERR_INITIALIZE_FAILURE:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_INITIALIZE_FAILURE");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_INITIALIZE_FAILURE");
				}
				break;
				case TDF_ERR_NETWORK_ERROR:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_NETWORK_ERROR");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_NETWORK_ERROR");
				}
				break;
				case TDF_ERR_INVALID_PARAMS:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_INVALID_PARAMS");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_INVALID_PARAMS");
				}
				break;
				case TDF_ERR_VERIFY_FAILURE:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_VERIFY_FAILURE");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_VERIFY_FAILURE");
				}
				break;
				case TDF_ERR_NO_AUTHORIZED_MARKET:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_NO_AUTHORIZED_MARKET");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_NO_AUTHORIZED_MARKET");
				}
				break;
				case TDF_ERR_NO_CODE_TABLE:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_NO_CODE_TABLE");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_NO_CODE_TABLE");
				}
				break;
				case TDF_ERR_SUCCESS:
				{
					loggerv2::info("tdf_source::init_source TDF_ERR_SUCCESS");
					m_pSource->update_state(AtsType::FeedSourceStatus::Down, "TDF_ERR_SUCCESS");
				}
				break;
				default:
					break;
				}
			}
		}
		bool tdf_connection::subscribe_item(feed_item* pItem)
		{
			//loggerv2::info("tdf_source::subscribe_item Code %s FeedCode %s", pItem->get_code().c_str(), pItem->get_feed_code().c_str());

			//TDF_SetSubscription(g_hTDF, pItem->get_feed_code().c_str(), SUBSCRIPTION_ADD);
			subscribe_vec.push_back(pItem->get_feed_code());
			return true;
		}

		bool tdf_connection::unsubscribe_item(feed_item* pItem)
		{
			return true;
		}



		void tdf_connection::OnSysMsg(THANDLE hTdf, TDF_MSG* pSysMsg)
		{
			if (!pSysMsg || !hTdf)
			{
				return;
			}
			//tdf_source* pSource = instance;
			/*if (!m_pSource)
			{
				return;
			}*/
			switch (pSysMsg->nDataType)
			{
			case MSG_SYS_DISCONNECT_NETWORK:

				instance_source->update_state(AtsType::FeedSourceStatus::Down, "network disconnect");
				break;
			case MSG_SYS_CONNECT_RESULT:
			{
				TDF_CONNECT_RESULT* pConnResult = (TDF_CONNECT_RESULT*)pSysMsg->pData;
				if (pConnResult && pConnResult->nConnResult)
				{
					loggerv2::info("tdf_source::init_source connect success");
				}
				else
				{
					loggerv2::error("tdf_source::init_source connect failed");
				}
			}
			break;
			case MSG_SYS_LOGIN_RESULT:
			{
				TDF_LOGIN_RESULT* pLoginResult = (TDF_LOGIN_RESULT*)pSysMsg->pData;
				if (pLoginResult && pLoginResult->nLoginResult)
				{
					loggerv2::info("login success!info:%s, nMarkets:%d.", pLoginResult->szInfo, pLoginResult->nMarkets);
					for (int i = 0; i < pLoginResult->nMarkets; i++)
					{
						loggerv2::info("tdf_source::OnSysMsg market:%s, dyn_date:%d.", pLoginResult->szMarket[i], pLoginResult->nDynDate[i]);
					}
					instance_source->update_state(AtsType::FeedSourceStatus::Up,"");
				}
				else
				{
					loggerv2::error("login failure, reason:%s\n", pLoginResult->szInfo);
					instance_source->update_state(AtsType::FeedSourceStatus::Down, pLoginResult->szInfo);
				}
			}
			break;
			case MSG_SYS_CODETABLE_RESULT:
				//{
				//								 TDF_CODE_RESULT* pCodeResult = (TDF_CODE_RESULT*)pSysMsg->pData;
				//								 if (pCodeResult)
				//								 {
				//									 Print("info:%s, number of markets:%d\n", pCodeResult->szInfo, pCodeResult->nMarkets);
				//									 for (int i = 0; i < pCodeResult->nMarkets; i++)
				//									 {
				//										 Print("market:%s, code_count:%d, code_date:%d\n", pCodeResult->szMarket[i], pCodeResult->nCodeCount[i], pCodeResult->nCodeDate[i]);
				//									 }
				//									 //获取代码表 
				//									 TDF_CODE* pCodeTable;
				//									 unsigned int nItems;

				//									 for (int i = 0; i < pCodeResult->nMarkets; i++)
				//									 {
				//										 TDF_GetCodeTable(hTdf, pCodeResult->szMarket[i], &pCodeTable, &nItems);
				//										 for (int nElem = 0; nElem < nItems; nElem++)
				//										 {
				//											 //Print("code:%s, name:%s\n", pCodeTable[nElem].szWindCode, pCodeTable[nElem].szCNName);
				//										 }
				//										 TDF_FreeArr(pCodeTable);
				//									 }
				//								 }
				//}
				break;
			case MSG_SYS_QUOTATIONDATE_CHANGE:
			{
				TDF_QUOTATIONDATE_CHANGE* pChange = (TDF_QUOTATIONDATE_CHANGE*)pSysMsg->pData;
				if (pChange)
				{
					loggerv2::error("Date Change, need reconnect：%s, old Date:%d, new Date：%d\n", pChange->szMarket, pChange->nOldDate, pChange->nNewDate);
				}
			}
			break;
			case MSG_SYS_MARKET_CLOSE:
			{
				TDF_MARKET_CLOSE* pCloseInfo = (TDF_MARKET_CLOSE*)pSysMsg->pData;
				if (pCloseInfo)
				{
					loggerv2::error("Market Close:market:%s, time:%d, info:%s\n", pCloseInfo->szMarket, pCloseInfo->nTime, pCloseInfo->chInfo);
				}
			}
			break;
			case MSG_SYS_HEART_BEAT:
			{
				//logger::info("Receive HeartBeat");
			}
			break;
			default:
				assert(0);
				break;
			}
		}

		void tdf_connection::OnDataMsg(THANDLE hTdf, TDF_MSG* pMsgHead)
		{


			//tdf_source* pSource = instance;
			if (instance_source!=nullptr)
			{
				//TDF_MSG* pData = new TDF_MSG;
				//memset(pData, '0', sizeof(TDF_MSG));
				//memcpy(pData, pMsgHead, sizeof(TDF_MSG));

				//pData->pAppHead = new TDF_APP_HEAD;
				//memset(pData->pAppHead, '0', sizeof(TDF_APP_HEAD));
				//memcpy(pData->pAppHead, pMsgHead->pAppHead, sizeof(TDF_APP_HEAD));
				TDF_MARKET_DATA* p1;
				TDF_INDEX_DATA* p2;
				TDF_TRANSACTION* p3;
				TDF_ORDER_QUEUE* p4;
				TDF_FUTURE_DATA* p5;
				TDF_ORDER* p6;
				switch (pMsgHead->nDataType)
				{
				case MSG_DATA_MARKET:
					//pData->pData = new TDF_MARKET_DATA[pData->pAppHead->nItemCount];
					p1 = static_cast<TDF_MARKET_DATA*>((pMsgHead->pData));
					for (int i = 0; i < pMsgHead->pAppHead->nItemCount;i++)
					{
						instance_source->market_data_queue.CopyPush(&p1[i]);
					}
					break;
				case MSG_DATA_INDEX:
					//pData->pData = new TDF_INDEX_DATA[pData->pAppHead->nItemCount];
					p2 = static_cast<TDF_INDEX_DATA*>((pMsgHead->pData));
					for (int i = 0; i < pMsgHead->pAppHead->nItemCount; i++)
					{
						instance_source->index_queue.CopyPush(&p2[i]);
					}
					break;
				case MSG_DATA_TRANSACTION:
					//pData->pData = new TDF_TRANSACTION[pData->pAppHead->nItemCount];
					p3 = static_cast<TDF_TRANSACTION*>((pMsgHead->pData));
					for (int i = 0; i < pMsgHead->pAppHead->nItemCount; i++)
					{
						instance_source->transaction_queue.CopyPush(&p3[i]);
					}
					break;
				case MSG_DATA_ORDERQUEUE:
					//pData->pData = new TDF_ORDER_QUEUE[pData->pAppHead->nItemCount];
					p4 = static_cast<TDF_ORDER_QUEUE*>((pMsgHead->pData));
					for (int i = 0; i < pMsgHead->pAppHead->nItemCount; i++)
					{
						instance_source->orderqueue_queue.CopyPush(&p4[i]);
					}

					break;
				case MSG_DATA_FUTURE:
					//pData->pData = new TDF_FUTURE_DATA[pData->pAppHead->nItemCount];
					p5 = static_cast<TDF_FUTURE_DATA*>((pMsgHead->pData));
					for (int i = 0; i < pMsgHead->pAppHead->nItemCount; i++)
					{
						instance_source->future_data_queue.CopyPush(&p5[i]);
					}

					break;
				case MSG_DATA_ORDER:
					//pData->pData = new TDF_ORDER[pData->pAppHead->nItemCount];
					p6 = static_cast<TDF_ORDER*>((pMsgHead->pData));
					for (int i = 0; i < pMsgHead->pAppHead->nItemCount; i++)
					{
						instance_source->order_queue.CopyPush(&p6[i]);
					}
					break;
				default:
				{
					//delete pData;
					return;
				}
				break;
				}
				//memset(pData->pData, '0', pData->nDataLen);
				//memcpy(pData->pData, pMsgHead->pData, pData->nDataLen);


				//instance_source->get_queue()->Push(pData);

				//delete[] pData->pData;
				//delete pData;
				//instance->process_msg(pData);
			}

		}

		//ifeed_item* tdf_source::find_item(const char* name)
		//{

		//	//std::string sCode = m_feedCode2CodeMap[name];
		//	return abstract_feed_source::find_item(name);
		//}

	}
}