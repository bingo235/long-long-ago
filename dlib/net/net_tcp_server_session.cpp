#include "net_tcp_server_session.h"

NS_DLIB_BEGIN
namespace net{

    CNetTcpServerSession::CNetTcpServerSession(uint32_t session_id,
        const NetServerParam& param,
        boost::asio::io_service&   io,
        tcp_close_callback	 onclose,
        tcp_message_callback onmessage,
        alloc_buffer_hander  alloc_hander)
        : m_param(param), 
        CNetTcpSession<CNetTcpServerSession>(session_id, 
        io, onclose, onmessage, alloc_hander, 
        param.send_buff_size, 
        param.recv_buff_size)
    {

    }

    CNetTcpServerSession::~CNetTcpServerSession()
    {

    }

}
NS_DLIB_END
