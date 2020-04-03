#ifndef NET_TCP_SESSION_h_
#define NET_TCP_SESSION_h_
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include "net_msg_protocol.h"
#include "net_object_pool.h"

NS_DLIB_BEGIN
namespace net{

    template<class MessageProtocol = CMessageProtocol_2_4 >
	class CNetTcpSession : private boost::noncopyable, public boost::enable_shared_from_this<CNetTcpSession<MessageProtocol>>
	{
    public:
        typedef CNetTcpSession<MessageProtocol>							ImplSocket;
        typedef MessageProtocol								            Message;
        typedef typename boost::shared_ptr<Message>			            MessagePtr;
        typedef CNetObjectPool<Message>									MessagePool;
        typedef typename std::deque<MessagePtr>				            MessageQueue;

        // socket连接上来的通知
        typedef boost::function< void ( int ) >				tcp_connect_callback;
        // socket关闭通知，网络io不再使用本socket对象
        typedef boost::function< void ( int ) >				tcp_close_callback;
        // socket接收消息的通知
        typedef boost::function< void ( int, MessagePtr ) >	tcp_message_callback;
        // socket 定时操作，比如心跳等
        typedef boost::function< void ( int ) >				tcp_timeout_callback;

		CNetTcpSession()
		{
		}
		~CNetTcpSession()
		{
		}

        CNetTcpSession(uint32_t session_id,
            boost::asio::io_service&   io,
            MessagePool&		  msg_pool,
            tcp_connect_callback onconnect,
            tcp_close_callback	 onclose,
            tcp_message_callback onmessage)
            : m_io(io)
            , m_socket(m_io)
            , m_closed(true)
            , m_message_pool(msg_pool)
            , m_session_id(session_id)
            , m_disouttime(0)
            , m_distimer(m_io, boost::posix_time::seconds(60))
            , m_onconnect( onconnect )
            , m_onclose( onclose )
            , m_onmessage( onmessage )
        {
            boost::system::error_code error;
            boost::asio::ip::tcp::no_delay option(true);
            m_socket.set_option(option, error);
        }

		void test()
		{
			MessagePtr newmsg( m_message_pool.NewObj( m_readmsg ), 
				boost::bind( &MessagePool::DeleteObj, &m_message_pool, _1 ) );
		}

		boost::asio::ip::tcp::socket& GetSocket()
		{
			return m_socket;
		}

		uint32_t GetSessionId()
		{
			return m_session_id;
		}

		void Start()
		{
			if ( m_closed )
			{
				m_closed = false;
				m_writemsgs.clear();
				//m_onconnect( m_socketid );

				boost::asio::async_read(m_socket,
					boost::asio::buffer(m_readmsg.data(), m_readmsg.head_length()),
					boost::asio::transfer_at_least(m_readmsg.head_length()),
					boost::bind( &ImplSocket::handle_recv_head, 
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			}
		}

    private:
        void do_close( uint32_t errid )
        {
            if(m_closed)
                return;

            try
            {
                m_closed = true;
                m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                m_socket.cancel();
                m_socket.close();

                m_disouttime = 0;
                m_distimer.cancel();
            }
            catch (...)
            {
                NETLOG_ERROR("net.tcp.close异常"<<FUN_FILE_LINE);
            }
            m_onclose( m_session_id );
            m_onconnect = 0;
            m_onmessage = 0;
            m_onoutime  = 0;
            m_onclose   = 0;
        }

        void handle_recv_head( const boost::system::error_code& error, size_t bytes_transferred)
        {
            if(error)
            {
                do_close(error.value());
                return;
            }

            if(bytes_transferred != m_readmsg.head_length())
            {
                do_close(error.value());
                return;
            }

            if (m_readmsg.decode_header())
            {
                m_readmsg.session_id( m_session_id );
                if ( m_readmsg.body_length() == 0 )
                {
                    if ( !m_closed )
                    {
                        MessagePtr newmsg( m_message_pool.NewObj( m_readmsg ), 
                            boost::bind( &MessagePool::DeleteObj, &m_message_pool, _1 ) );
                        m_onmessage( m_session_id, newmsg );
                    }
                    else
                    {
                        NETLOG_ERROR( "server socket closed, but recv" );
                    }

                    boost::asio::async_read(m_socket,
                        boost::asio::buffer(m_readmsg.data(), m_readmsg.head_length()),
                        boost::asio::transfer_at_least(m_readmsg.head_length()),
                        boost::bind( &ImplSocket::handle_recv_head, 
						this->shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
                }
                else
                {
                    boost::asio::async_read(m_socket,
                        boost::asio::buffer(m_readmsg.body(), m_readmsg.body_length()),
                        boost::asio::transfer_at_least(m_readmsg.body_length()),
                        boost::bind( &ImplSocket::handle_recv_body, 
						this->shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
                }
            }
            else
            {
                do_close(error.value());
            }
        }

        void handle_recv_body( const boost::system::error_code& error, size_t bytes_transferred )
        {
            if(error)
            {
                do_close(error.value());
                return;
            }

            m_readmsg.decode_body( );
            if ( !m_closed )
            {
                MessagePtr newmsg( m_message_pool.NewObj( m_readmsg ), 
                    boost::bind( &MessagePool::DeleteObj, &m_message_pool, _1 ) );

                m_onmessage( m_session_id, newmsg );
            }
            else
            {
                NETLOG_ERROR( "server socket closed, but recv 2" );
            }

            boost::asio::async_read(m_socket,
                boost::asio::buffer(m_readmsg.data(), m_readmsg.head_length()),
                boost::asio::transfer_at_least(m_readmsg.head_length()),
                boost::bind( &ImplSocket::handle_recv_head, this->shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        }

    private:
        uint32_t                        m_session_id;
        boost::asio::io_service&	    m_io;
        boost::asio::ip::tcp::socket    m_socket;
        Message						    m_readmsg;
        MessageQueue				    m_writemsgs;
        boost::asio::deadline_timer     m_distimer;
        uint64_t					    m_disouttime;

        volatile bool				    m_closed;
        MessagePool&			        m_message_pool;

        tcp_timeout_callback		    m_onoutime;
        tcp_connect_callback		    m_onconnect;
        tcp_close_callback			    m_onclose;
        tcp_message_callback		    m_onmessage;
	};
}

NS_DLIB_END
#endif // NET_TCP_SESSION_h_
