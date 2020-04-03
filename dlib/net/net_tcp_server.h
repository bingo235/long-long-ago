#ifndef NET_TCP_SERVER_H__
#define NET_TCP_SERVER_H__
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include "net_tcp_session.h"

NS_DLIB_BEGIN

namespace net{
    class CNetTcpServer :  private boost::noncopyable
    {
    public:
		typedef CNetTcpSession<CMessageProtocol_2_4>	TcpSession;
		typedef boost::shared_ptr<TcpSession>			TcpSessionPtr;
		typedef CNetObjectPool<TcpSession>				TcpSessionPool;
		typedef map<uint32_t, TcpSessionPtr>			TcpSessionMap;
		typedef	TcpSessionMap::iterator					TcpSessionMapIter;

        CNetTcpServer();
        ~CNetTcpServer();
        bool Start();
		void AcceptOne();

	private:
		void handle_accept(TcpSessionPtr session_ptr, const boost::system::error_code& error );
		
		TcpSessionPtr CreateSession();

		void OnConnect( int socketid );
		void OnClose( int socketid );
		void OnMessage( int socketid, TcpSession::MessagePtr msg);

    private:
		volatile bool				    	m_closed;
		bool								m_stop_accept;
		TcpSessionMap						m_map_session;

        boost::asio::io_service&	    	m_io;
		boost::asio::strand					m_strand;
        boost::asio::ip::tcp::acceptor	    m_acceptor;
        boost::asio::ip::tcp::endpoint  	m_endpoint;
		TcpSessionPool						m_session_pool;
		TcpSession::MessagePool				m_message_pool;

		TcpSession::tcp_timeout_callback	m_onoutime;
		TcpSession::tcp_connect_callback	m_onconnect;
		TcpSession::tcp_close_callback		m_onclose;
		TcpSession::tcp_message_callback	m_onmessage;
    };
}

NS_DLIB_END
#endif // NET_TCP_SERVER_H__
