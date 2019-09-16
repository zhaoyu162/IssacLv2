#pragma once

//EXTERN_C int ConnectServer(const char* pstrZMQServerAddress);
//EXTERN_C int GetData(char* buffer, size_t szBuffLen);

#ifndef IMPORT_MATESDK
#define MATESDK_STYLE __declspec(dllexport)
#else
#define MATESDK_STYLE __declspec(dllimport)
#endif
#ifdef __cplusplus
extern "C"
{
#endif

	//************************************
	// Method:    ConnectServer
	// FullName:  ConnectServer
	// Access:    public 
	// Returns:   MATESDK_STYLE ,void *， socket ptr to use in other functions!
	// Qualifier: 链接一个ZMQ 服务器，提供连接字串即可，如：tcp://127.0.0.1:19908
	// Parameter: const char * pstrZMQServerAddress
	//************************************
	MATESDK_STYLE void* ConnectServer(const char* pstrZMQServerAddress);
	//************************************
	// Method:    Subscribe
	// FullName:  Subscribe
	// Access:    public 
	// Returns:   MATESDK_STYLE int
	// Qualifier: 订阅某种行情数据，如ZBCJ,WDPK...
	// 空字符串("")表示订阅所有行情数据
	// Parameter: void* pzmqSocket, zmq socket ptr from ConnectServer returns.
	// Parameter: const char * pstrChannel
	//************************************
	MATESDK_STYLE int Subscribe(void* pzmqSocket, const char* pstrChannel);
	//************************************
	// Method:    GetData
	// FullName:  GetData
	// Access:    public 
	// Returns:   MATESDK_STYLE int， 0表示成功，非0是错误码，表示失败，请重新调用ConnectServer.
	// Qualifier: 放在一个无限循环中调用，不断地接收行情，buffer用来接收一个JSON串
	// Parameter: void* pzmqSocket, zmq socket ptr from ConnectServer returns.
	// Parameter: char * buffer
	// Parameter: size_t szBuffLen
	//************************************
	MATESDK_STYLE int GetData(void* pzmqSocket, char* buffer, size_t szBuffLen);

	//************************************
	// Method:    Disconnect
	// FullName:  Disconnect
	// Access:    public 
	// Returns:   MATESDK_STYLE int
	// Qualifier: 断开与ZMQ SERVER的链接
	// Parameter: void* pzmqSocket, zmq socket ptr from ConnectServer returns.
	// Parameter: const char * pstrZMQServerAddress：zmq链接字符串
	//************************************
	MATESDK_STYLE int Disconnect(void* pzmqSocket, const char* pstrZMQServerAddress);

	//************************************
	// Method:    GetErrString_ZMQ
	// FullName:  GetErrString_ZMQ
	// Access:    public 
	// Returns:   MATESDK_STYLE void
	// Qualifier: 获取ZMQ错误描述
	// Parameter: char * buff
	// Parameter: size_t szBuff
	//************************************
	MATESDK_STYLE void GetErrString_ZMQ(char* buff, size_t szBuff);

	//************************************
	// Method:    GetZMQErrNo_ZMQ
	// FullName:  GetZMQErrNo_ZMQ
	// Access:    public 
	// Returns:   MATESDK_STYLE int
	// Qualifier: 获取ZMQ错误码
	//************************************
	MATESDK_STYLE int GetZMQErrNo_ZMQ();

	MATESDK_STYLE int GetQuote(const char* stock_code, const char* name, int index = 0);
	//MATESDK_STYLE int GetQuote(const char* stock_code, const char* name);

#ifdef __cplusplus
}
#endif