#include <stdio.h>
#include <iostream>
#include "dcommon.h"
#include "net/net_tcp_server.h"
#include "net/net_io_scheduler.h"
using namespace std;
using namespace dlib;
using namespace dlib::net;
int main()
{
	int32_t i = 0;
	printf("start...\n");

	CNetIoScheduler::Single()->Start();

	CNetTcpServer server;

	server.Start();

	Sleep(1111111111);
	return 0;
}