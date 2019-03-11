// MateSDK.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../DataHack/structs.h"
#include <json/json.h>

#define  ZMQ_STATIC

#include "../ZMQ/include/zmq.h"

std::map<std::string, Json::Value> __mapOfWdpk;
ATL::CComCriticalSection __csLock;

//#ifdef _WIN32
//#pragma comment(lib, "../zmq/lib/libzmq.lib")
//#else
//#pragma comment(lib, "../zmq/lib/libzmq_x64.lib")
//#endif

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

 void* ConnectServer(const char* pstrZMQServerAddress)
{
	 void* zmqContext = zmq_ctx_new();
	 void* zmqSocket = zmq_socket(zmqContext, ZMQ_PULL);
	 //int szRcvBuffSize = 1024 * 1024 * 100;
	 //zmq_setsockopt(zmqSocket, ZMQ_RCVBUF, &szRcvBuffSize, sizeof(szRcvBuffSize));
	 int szRcvHM = 10000;
	 //zmq_setsockopt(zmqSocket, ZMQ_RCVHWM, &szRcvHM, sizeof(szRcvHM));
	 zmq_setsockopt(zmqSocket, ZMQ_RCVHWM, &szRcvHM, sizeof(szRcvHM));
	 zmq_connect(zmqSocket, pstrZMQServerAddress);
	 return (void*)zmqSocket;
}

 int Subscribe(void* pzmqSocket, const char* pstrChannel)
{
	return  zmq_setsockopt(pzmqSocket, ZMQ_SUBSCRIBE, pstrChannel, strlen(pstrChannel)+1);
}

 int GetData(void* pzmqSocket, char* buffer, size_t szBuffLen)
{
	 int64_t more = 0;
	 size_t more_size = sizeof(more);
	 Json::Value jsv;
	 
	 try
	 {
		
		std::vector<zmq_msg_t> vecOfMsgs;

		while (vecOfMsgs.size() < 2)
		{
			zmq_msg_t msgs;
			more = 0;
			zmq_msg_init_size(&msgs, 1024 * 1024);
			memset(zmq_msg_data(&msgs), 0, 1024 * 1024);
			int nzmr = zmq_msg_recv(&msgs, pzmqSocket, 0);

			/*	if (nzmr == -1 && zmq_errno() == EAGAIN)
					break;*/

			if (nzmr == -1)
				return zmq_errno();

			vecOfMsgs.push_back(msgs);
		}

		if (vecOfMsgs.size() == 0)
		{
			return 100;
		}

		for (size_t sz = 0; sz < vecOfMsgs.size(); sz++)
		{
			zmq_msg_t& msgs = vecOfMsgs[sz];
			char* p = ((char*)zmq_msg_data(&msgs));

			if (nullptr == p)
				return zmq_errno();

			std::string str = p;
			if (str.length() == 4) {

			}
			else
			{
				L1DataGram* msgdata = (L1DataGram*)(zmq_msg_data(&msgs));

				if (msgdata == nullptr) {
					return -100;
				}

				switch (msgdata->nDgType)
				{
				case 0:
				{
					Json::Value jsWdPk;
					jsWdPk["QuoteType"] = "WDPK";

					for (int n = 0; n < msgdata->nCount; n++)
					{
						Json::Value stock;
						
						stock["StockCode"] = msgdata->extData[n].cCode;
						stock["StockName"] = (char*)CW2A(CA2W(msgdata->extData[n].cName, 936), CP_UTF8); 
						stock["LowLimit"] = msgdata->extData[n].nBotLtd;
						stock["HighLimit"] = msgdata->extData[n].nTopLtd;
						stock["PrevClose"] = msgdata->extData[n].nPrevClose;
						stock["TotalAmount"] = msgdata->extData[n].quote.dlbAmount;

						stock["Time"] = msgdata->extData[n].quote.dwTim32;
						stock["TotalVolume"] = msgdata->extData[n].quote.lgAllVol;
						stock["High"] = msgdata->extData[n].quote.lgHigh;
						stock["Low"] = msgdata->extData[n].quote.lgLow;

						stock["SellVolume"] = msgdata->extData[n].quote.lgNeipan;
						stock["Price"] = msgdata->extData[n].quote.lgNew;
						stock["Open"] = msgdata->extData[n].quote.lgOpen;

						for (int j = 0; j < 5; j++) {
							stock["AskPrices"].append((int)msgdata->extData[n].quote.lgPricesOfAsk[j]);
						}

						for (int j = 0; j < 5; j++) {
							stock["BidPrices"].append((int)msgdata->extData[n].quote.lgPricesOfBid[j]);
						}

						for (int j = 0; j < 5; j++) {
							stock["AskVolumes"].append((int)msgdata->extData[n].quote.lgVolumeOfAsk[j]);
						}

						for (int j = 0; j < 5; j++) {
							stock["BidVolumes"].append((int)msgdata->extData[n].quote.lgVolumeOfBid[j]);
						}

						stock["TotalDealtOrderNum"] = msgdata->extData[n].quote.lgTotalDealtNum;
						jsWdPk["Stocks"].append(stock);

						__csLock.Lock();
						__mapOfWdpk[msgdata->extData[n].cCode] = stock;
						__csLock.Unlock();
					}
					jsv.append(jsWdPk);
				}

				break;
				case 2:
					break;
				case 4:
				{
					ZhubiDataGram* msgdata = (ZhubiDataGram*)(zmq_msg_data(&msgs));
					Json::Value zbData;
					zbData["QuoteType"] = "ZBCJ";
					zbData["StockCode"] = msgdata->strCode;
					zbData["StockName"] = (char*)CW2A(CA2W(msgdata->strName, 936), CP_UTF8);
					
					for (int n = 0; n < msgdata->nCount; n++)
					{
						Json::Value zbVal;
						zbVal["Price"] = msgdata->zbDataList[n].fltPrice;
						zbVal["Time"] = msgdata->zbDataList[n].lgTime;
						zbVal["Volume"] = msgdata->zbDataList[n].lgVolume;
						zbVal["ID"] = msgdata->nOrderPackId + n;
						zbData["ZbData"].append(zbVal);
					}

					jsv.append(zbData);
				}
				break;
				case 5:
				{
					ZhubiOrderDataGram* msgdata = (ZhubiOrderDataGram*)(zmq_msg_data(&msgs));
					Json::Value wtData;
					wtData["QuoteType"] = "ZBWT";
					wtData["StockCode"] = msgdata->strCode;
					wtData["StockName"] = (char*)CW2A(CA2W(msgdata->strName, 936), CP_UTF8); 

					int nNum = msgdata->orddObj.nNumOfOrders;

					Json::Value jsLvs;
					for (int n = 0; n < nNum; )
					{
						Json::Value jsLevel;
						int price = msgdata->orddObj.nOrdersData[n] & 0xffff;
						int bos = (msgdata->orddObj.nOrdersData[n] >> 16) > 0 ? 1 : 0;
						int vols = msgdata->orddObj.nOrdersData[n + 1];
						jsLevel["Price"] = price;
						jsLevel["IsBuy"] = bos;

						Json::Value jsvVols;
						for (int j = 0; j < vols; j++) {
							jsLevel["OrderVolumes"].append(msgdata->orddObj.nOrdersData[n + 2 + j]);
						}

						n += 2 + vols;
						wtData["PriceQueue"].append(jsLevel);
					}

					jsv.append(wtData);

				}
				break;
				case 6:
				{
					HightLvQuoteDataGram* msgdata = (HightLvQuoteDataGram*)(zmq_msg_data(&msgs));
					Json::Value stock;
					stock["QuoteType"] = "SDPK";
					stock["StockCode"] = msgdata->strCode;
					stock["StockName"] = (char*)CW2A(CA2W(msgdata->strName, 936), CP_UTF8);

					for (int n = 0; n < 5; n++) {
						stock["AskPrices"].append(msgdata->quoteObj.fltAskPrices[n]);
					}

					for (int n = 0; n < 5; n++) {
						stock["BidPrices"].append(msgdata->quoteObj.fltBidPrices[n]);
					}

					for (int n = 0; n < 5; n++) {
						stock["AskVolumes"].append(msgdata->quoteObj.fltAskVolume[n]);
					}

					for (int n = 0; n < 5; n++) {
						stock["BidVolumes"].append(msgdata->quoteObj.fltBidVolume[n]);
					}
					stock["TotalAskVol"] = msgdata->quoteObj.fltTotalVolAsk;
					stock["TotalBidVol"] = msgdata->quoteObj.fltTotalVolBid;
					stock["AvgAskPrice"] = msgdata->quoteObj.fltAvgPriceAsk;
					stock["AvgBidPrice"] = msgdata->quoteObj.fltAvgPriceBid;

					//jsv = stock;
					jsv.append(stock);
				}
				break;
				case 7:
				{
					MULTI_PANKOU_DATAGRAM* msgdata = (MULTI_PANKOU_DATAGRAM*)(zmq_msg_data(&msgs));
					Json::Value bdPanKouList;
					bdPanKouList["QuoteType"] = "BDPK";
					bdPanKouList["StockCode"] = msgdata->strCode;
					bdPanKouList["StockName"] = (char*)CW2A(CA2W(msgdata->strName, 936), CP_UTF8); 

					bdPanKouList["LevelNum"] = msgdata->nLevelNum;

					for (int n = 0; n < msgdata->nLevelNum; n++)
					{
						Json::Value level;
						level["OrderNum"] = msgdata->pkData[n].nOrderNum;
						level["OrderPrice"] = msgdata->pkData[n].nPrice;
						level["OrderVolume"] = msgdata->pkData[n].nVolume;
						level["LevelID"] = msgdata->pkData[n].sLevelNum;
						bdPanKouList["Levels"].append(level);
					}

					jsv.append(bdPanKouList);
						 
				}
				break;
				case 8:
				{
					LPBIGORDERDATAGRAM msgdata = (LPBIGORDERDATAGRAM)(zmq_msg_data(&msgs));
					Json::Value jsBO;
					jsBO["QuoteType"] = "ZBDD";
					jsBO["StockCode"] = msgdata->strCode;
					jsBO["StockName"] = (char*)CW2A(CA2W(msgdata->strName, 936), CP_UTF8);

					for (int n = 0; n < msgdata->nCount; n++)
					{
						Json::Value jso;
						jso["BuyOrderID"] = (msgdata->bigOrders[n].unBuyOrderId & 0xfffffff);
						jso["BuyOrderState"] = (msgdata->bigOrders[n].unBuyOrderId >> 28);
						jso["BuyPrice"] = msgdata->bigOrders[n].unBuyPrice;
						jso["BuyVolume"] = msgdata->bigOrders[n].unBuyVol;
						jso["DealtID"] = msgdata->bigOrders[n].unOrderPackId;
						jso["SellOrderID"] = (msgdata->bigOrders[n].unSellOrderId & 0xfffffff);
						jso["SellOrderState"] = (msgdata->bigOrders[n].unSellOrderId >> 28);
						jso["SellPrice"] = msgdata->bigOrders[n].unSellPrice;
						jso["SellVolume"] = msgdata->bigOrders[n].unSellVol;
						jsBO["OrderList"].append(jso);
					}

					jsv.append(jsBO);
				}
				break;
				case 10:
				{
					QiQuanDataGram* msgdata = (QiQuanDataGram*)(zmq_msg_data(&msgs));
					Json::Value jsQQ;
					jsQQ["QuoteType"] = "SZQQ";
					for (int n = 0; n < msgdata->nCount; n++)
					{
						Json::Value jsv;
						jsv["StockCode"] = msgdata->buff[n].code;
						jsv["StockName"] = (char*)CW2A(CA2W(msgdata->buff[n].name, 936), CP_UTF8);

						jsv["BottomPrice"] = msgdata->buff[n].fBottomPrice;
						jsv["BuyPrice1"] = msgdata->buff[n].fBuyPrice1;
						jsv["BuyPrice2"] = msgdata->buff[n].fBuyPrice2;
						jsv["BuyPrice3"] = msgdata->buff[n].fBuyPrice3;
						jsv["BuyPrice4"] = msgdata->buff[n].fBuyPrice4;
						jsv["BuyPrice5"] = msgdata->buff[n].fBuyPrice5;

						jsv["BuyVol1"] = msgdata->buff[n].fBuyVol1;
						jsv["BuyVol2"] = msgdata->buff[n].fBuyVol2;
						jsv["BuyVol3"] = msgdata->buff[n].fBuyVol3;
						jsv["BuyVol4"] = msgdata->buff[n].fBuyVol4;
						jsv["BuyVol5"] = msgdata->buff[n].fBuyVol5;
						jsv["SellPrice1"] = msgdata->buff[n].fSellPrice1;
						jsv["SellPrice2"] = msgdata->buff[n].fSellPrice2;
						jsv["SellPrice3"] = msgdata->buff[n].fSellPrice3;
						jsv["SellPrice4"] = msgdata->buff[n].fSellPrice4;
						jsv["SellPrice5"] = msgdata->buff[n].fSellPrice5;

						jsv["SellVol1"] = msgdata->buff[n].fSellVol1;
						jsv["SellVol2"] = msgdata->buff[n].fSellVol2;
						jsv["SellVol3"] = msgdata->buff[n].fSellVol3;
						jsv["SellVol4"] = msgdata->buff[n].fSellVol4;
						jsv["SellVol5"] = msgdata->buff[n].fSellVol5;

						jsv["HighPrice"] = msgdata->buff[n].fHighPrice;

						jsv["LowPrice"] = msgdata->buff[n].fLowPrice;
						jsv["LastClose"] = msgdata->buff[n].fLastClosePrice;
						jsv["NewPrice"] = msgdata->buff[n].fNewPrice;

						jsv["OpenPrice"] = msgdata->buff[n].fThisOpenPrice;
						jsv["TopPrice"] = msgdata->buff[n].fTopPrice;
						jsv["TotalAmount"] = msgdata->buff[n].fTotalAmonut;
						jsv["BuyLots"] = msgdata->buff[n].fTotalBuyLots;
						jsv["TotalLots"] = msgdata->buff[n].fTotalLots;
						jsv["TotalPositions"] = msgdata->buff[n].fTotalPositions;

						jsv["Time"] = msgdata->buff[n].unTime;
						jsQQ["QQData"].append(jsv);
					}

					jsv.append(jsQQ);
				}
				break;
				default:
					break;
				}
			}
			zmq_msg_close(&msgs);
		}

		if (vecOfMsgs.size() > 0)
		{
			Json::StyledWriter sw;
			std::string str = sw.write(jsv);
			if (szBuffLen < str.length() + 1) {
				return 200;
			}

			str.copy(buffer, str.length());
		}
	 }
	 catch (...)
	 {
		 return 300;
	 }

	 return 0;
}

 int Disconnect(void* pzmqSocket, const char* pstrZMQServerAddress)
 {
	 return zmq_disconnect(pzmqSocket, pstrZMQServerAddress);
 }

 void GetErrString_ZMQ(char* buff, size_t szBuff)
 {
	 const char* pstr = zmq_strerror(zmq_errno());
	 if (nullptr != buff && szBuff > 0) {
		 buff[0] = 0;
		 if (pstr != NULL && strlen(pstr) < szBuff) {
			 strcpy_s(buff, szBuff, pstr);
		 }
	 }
 }

 int GetZMQErrNo_ZMQ()
 {
	 return zmq_errno();
 }

 int GetQuote(const char* stock_code, const char* name, int index /*= 0*/)
 {
	 int value = 0;
	 __csLock.Lock();
	 if (__mapOfWdpk.find(stock_code) != __mapOfWdpk.end()) {
		 if (index == -1)
			 value = __mapOfWdpk[stock_code][name].asInt();
		 else
			value = __mapOfWdpk[stock_code][name][index].asInt();
	 }
	 __csLock.Unlock();
	 return value;
 }
