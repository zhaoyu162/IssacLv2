// CppClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "../DataHack/structs.h"
#include <iostream>
#include "PackageExactor.h"

#include "../ZMQ/include/zmq.h"

#ifdef DEBUG
#pragma comment(lib, "libzmq-v120-mt-gd-4_0_4.lib")
#else
#pragma comment(lib, "libzmq-v120-mt-4_0_4.lib")
#endif

int main()
{
	void* zmqContext = zmq_ctx_new();
	void* zmqSocket = zmq_socket(zmqContext, ZMQ_SUB);

	zmq_connect(zmqSocket, "tcp://127.0.0.1:19908");
	zmq_setsockopt(zmqSocket, ZMQ_SUBSCRIBE, "", 0);

	for (;;)
	{
		int64_t more = 0;
		size_t more_size = sizeof(more);

		do
		{
			zmq_msg_t msgs;
			more = 0;
			zmq_msg_init_size(&msgs, 1024 * 1024);
			memset(zmq_msg_data(&msgs), 0, 1024 * 1024);

			zmq_msg_recv(&msgs, zmqSocket, 0);
			std::string str = ((char*)zmq_msg_data(&msgs));

			if (str.length() == 4) {

			}
			else
			{
				L1DataGram* msgdata = (L1DataGram*)(zmq_msg_data(&msgs));
	
				switch (msgdata->nDgType)
				{
				case 0:
					for (int n = 0; n < msgdata->nCount; n++)
					{
						if (std::string(msgdata->extData[n].cCode) == "SZ300028")
						{
							std::cout << "received " << str.c_str() << ":" << msgdata->extData[n].cName << std::endl;
						}
					}
					break;
				case 2:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					/*std::cout << "received " << str.c_str() << ":" << msgdata->extData->cName << std::endl;*/
					break;
				default:
					break;
				}
			}
			int rc = zmq_getsockopt(zmqSocket, ZMQ_RCVMORE, &more, &more_size);
			zmq_msg_close(&msgs);
		} while (more);
	}
}

