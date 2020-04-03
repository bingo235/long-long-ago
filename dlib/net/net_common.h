#ifndef NET_COMMON_H__
#define NET_COMMON_H__

#include "../common/common.h"
#include "net_alloc_mg.h"
#include "net_io_scheduler.h"
#include "net_msg_head.h"
#include "net_msg_protocol.h"
#include "net_safe_container.h"
#include "net_server_id_mg.h"

NS_DLIB_BEGIN
namespace net{
    
    class ITcpServer
    {
    public:
        virtual void OnConnect(uint32_t session_id) = 0;
        virtual void OnRecive(uint32_t session_id, const void* data_ptr, uint32_t data_len) = 0;
        virtual void OnClose(uint32_t session_id) = 0;
    };

    class ITcpClient
    {
    public:
        enum Status
        {
            Status_Connect_Ok = 1,
            Status_Connect_Fail,
        };
        virtual void OnConnect(uint32_t session_id, Status status) = 0;
        virtual void OnRecive(uint32_t session_id, const void* data_ptr, uint32_t data_len) = 0;
        virtual void OnClose(uint32_t session_id) = 0;
    };

    struct NetServerParam
    {
        char        bind_ip[16];
        uint32_t    listen_port;
        uint32_t    max_connect;
        uint32_t    send_buff_size;
        uint32_t    recv_buff_size;
        bool        is_cut_sendfail;
        ITcpServer* server_handle;

        NetServerParam()
        {
            memset(bind_ip, 0, sizeof(bind_ip));
            listen_port = 8898;
            max_connect = 10000;
            send_buff_size = 1024*15;
            recv_buff_size = 1024*15;
            is_cut_sendfail = true;
            server_handle = 0;
        }
    };

    struct NetClientParam
    {
        char        connect_ip[16];
        uint32_t    connect_port;
        uint32_t    send_buff_size;
        uint32_t    recv_buff_size;
        ITcpClient* client_handle;

        NetClientParam()
        {
            memset(connect_ip, 0, sizeof(connect_ip));
            connect_port = 8898;
            send_buff_size = 1024*15;
            recv_buff_size = 1024*15;
            client_handle = 0;
        }
    };
}

NS_DLIB_END
#endif // NET_COMMON_H__