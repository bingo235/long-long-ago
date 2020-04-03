#include <stdio.h>
#include <iostream>
#include "common/common.h"
#include "net/net_common.h"
#include "net/net_tcp_server.h"
#include "net/net_io_scheduler.h"
#include "net/net_message_mg.h"
#include <boost/thread.hpp>
#include "net/net_tcp_client.h"
using namespace std;
using namespace dlib;
using namespace dlib::net;

//#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")

struct NetMsgBase
{
    int32_t op_code;
    int32_t len;
    int32_t id;
    NetMsgBase(uint32_t code, uint16_t l):op_code(code),len(l){}
};

struct NetMsgTest : public NetMsgBase
{
	char cmd[256];
    NetMsgTest() : NetMsgBase(101, sizeof(NetMsgTest))
    {
    }
};

class TcpServer : public ITcpServer
{
public:
	TcpServer()
	{
		count_bit = 0;
	}
    void OnConnect(uint32_t session_id)
    {
		cout<<"[TcpServer] OnConnect session_id="<<session_id<<endl;
		NetMsgTest test;
		//server.Send(session_id, &test, test.len);
    }
    void OnRecive(uint32_t session_id, const void* data_ptr, uint32_t data_len)
    {
		cout<<"[TcpServer] OnRecive data_len="<<data_len<<" index"<<count_bit<<endl;
		count_bit++;
        NetMsgTest test;
        server.Send(session_id, data_ptr, data_len);
    }
    void OnClose(uint32_t session_id)
    {
		count_bit = 0;
        cout<<"[TcpServer] OnClose session_id="<<session_id<<endl;
    }
    void Start()
    {
        NetServerParam param;
        param.server_handle = this;
        server.Start(param);
    }

    CNetTcpServer server;
	uint32_t count_bit;
};

char cmd[256] = {0};
char ip[32] = {0};

class TcpClient : public ITcpClient
{
public:
    TcpClient()
    {
        count_bit = 0;
    }

    void OnConnect(uint32_t session_id, Status status)
    {
        //cout<<"[TcpClient] OnConnect session_id="<<session_id<<endl;
		NetMsgTest test;
		strcpy(test.cmd, cmd);
		client.Send(session_id, &test, test.len);
    }
    void OnRecive(uint32_t session_id, const void* data_ptr, uint32_t data_len)
    {
        //cout<<"OnRecive session_id="<<session_id<<" data_len="<<data_len<<endl;
    }
    void OnClose(uint32_t session_id)
    {
        //cout<<"[TcpClient] OnClose session_id="<<session_id<<endl;
    }

    void Start()
    {
        NetClientParam param;
        strcpy(param.connect_ip, ip);
        param.connect_port = 8123;
        param.client_handle = this;
        //for (uint32_t i = 0; i < 100; ++i)
        {
            client.AddClient(param);
        }
    }

    CNetTcpClient client;
    int count_bit;
};

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cout<<"²ÎÊý´íÎó¡£client.exe ip ÃüÁî";
		return 0;
	}
	strcpy(ip, "127.0.0.1");
	int32_t i = 0;
	CNetIoScheduler::Single()->Start(2);
    
	/*TcpServer server;
	server.Start();*/

	strcpy(ip, argv[1]);
	strcpy(cmd, argv[2]);

    TcpClient client;
    client.Start();

    int last_print = GetTickCount();
    int loop = 0;
	int loop_count = 0;
	int speed = 0;
    while(1)
    {
        loop = CNetMessgeMg::Single()->Run(1000);
        if(loop == 0)
            Sleep(1);

		/*loop_count += loop;
        if(GetTickCount() - last_print > 3000)
        {
            last_print = GetTickCount();
            printf("count = %d  speed=%dMbps  loop_count=%d\n", 
				server.server.GetSessionCount(), (server.count_bit - speed)*8/1024/1024/3, 
				loop_count/3);
			speed = server.count_bit;
			loop_count = 0;
        }*/

   //     /*if(GetTickCount() - last_print > 3000)
   //     {
   //         last_print = GetTickCount();
   //         printf("count = %d  speed=%dMbps  loop_count=%d\n", 
   //             client.client.GetSessionCount(), (client.count_bit - speed)*8/1024/1024/3, 
   //             loop_count/3);
   //         speed = client.count_bit;
   //         loop_count = 0;
   //     }*/
    }

    Sleep(111111111);

	return 0;
}
