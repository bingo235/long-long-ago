#include "net_tcp_client_session.h"
#include <boost/format.hpp>
#include "net_message_mg.h"
NS_DLIB_BEGIN
namespace net{


    CNetTcpClientSession::CNetTcpClientSession(uint32_t session_id, 
        const NetClientParam& param, 
        boost::asio::io_service& io, 
        tcp_close_callback onclose, 
        tcp_message_callback onmessage, 
        alloc_buffer_hander alloc_hander)
		: m_resolver(io), 
		CNetTcpSession<CNetTcpClientSession>(session_id, 
		io, onclose, onmessage, alloc_hander, 
		param.send_buff_size, 
		param.recv_buff_size)
    {
		m_param = param;
    }

    CNetTcpClientSession::~CNetTcpClientSession()
    {

    }

	void CNetTcpClientSession::StartConnect()
	{
		tcp::resolver::query query(string(m_param.connect_ip), 
			 boost::str(boost::format("%d")%m_param.connect_port));

		m_resolver.async_resolve(query,
			boost::bind(&CNetTcpClientSession::handle_resolve, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));
	}

	void CNetTcpClientSession::handle_resolve( const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator )
	{
		if(!error)
		{
			tcp::endpoint endpoint = *endpoint_iterator;
			m_socket.async_connect(endpoint, 
				boost::bind(&CNetTcpClientSession::handle_connect, shared_from_this(),
				boost::asio::placeholders::error, ++endpoint_iterator));
		}
		else
		{
            CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpClientSession::on_connect, 
                this->shared_from_this(), ITcpClient::Status_Connect_Fail));
		}
	}

	void CNetTcpClientSession::handle_connect( const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator )
	{
		if(!error)
		{
            CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpClientSession::on_connect, 
                this->shared_from_this(), ITcpClient::Status_Connect_Ok));
			Start();
		}
		else if(endpoint_iterator != tcp::resolver::iterator())
		{
			//async_connect next
			m_socket.close();
			tcp::endpoint endpoint = *endpoint_iterator;
			m_socket.async_connect(endpoint, 
				boost::bind(&CNetTcpClientSession::handle_connect, shared_from_this(),
				boost::asio::placeholders::error, ++endpoint_iterator));
		}
		else
		{
            CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpClientSession::on_connect, 
                this->shared_from_this(), ITcpClient::Status_Connect_Fail));
		}
	}

    void CNetTcpClientSession::on_connect(ITcpClient::Status status)
    {
        m_param.client_handle->OnConnect(m_session_id, status);
    }

}
NS_DLIB_END