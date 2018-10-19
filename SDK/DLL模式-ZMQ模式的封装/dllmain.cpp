// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

void* ConnectServer(const char* pstrZMQServerAddress);
int GetData(void* pzmqSocket, char* buffer, size_t szBuffLen);
int Subscribe(void* pzmqSocket,const char* pstrChannel);

extern CComCriticalSection __csLock;

//#include "MateSDK.h"
unsigned __stdcall ThreadProc_Recv(void *p)
{
	char buffer[1024 * 1024] = { 0 };
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		//GetData(buffer, sizeof(buffer));
		/*if (strlen(buffer) > 0) {

		}*/
	}

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
		__csLock.Init();
		//ConnectServer("tcp://127.0.0.1:19908");
		//Subscribe("WDPK");
		//_beginthreadex(nullptr, 0, ThreadProc_Recv, nullptr, 0, NULL);
	}break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

