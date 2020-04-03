#ifndef NET_TCP_SERVER_SESSION_H__
#define NET_TCP_SERVER_SESSION_H__
#include "net_common.h"
#include "net_tcp_session.h"

NS_DLIB_BEGIN
namespace net{

    class CNetTcpServerSession : public CNetTcpSession<CNetTcpServerSession>
    {
    public:
        CNetTcpServerSession(uint32_t session_id,
            const NetServerParam& param,
            boost::asio::io_service&   io,
            tcp_close_callback	 onclose,
            tcp_message_callback onmessage,
            alloc_buffer_hander  alloc_hander);

        ~CNetTcpServerSession();

    private:
        const NetServerParam& m_param;
    };
}

NS_DLIB_END
#endif // NET_TCP_SERVER_SESSION_H__
