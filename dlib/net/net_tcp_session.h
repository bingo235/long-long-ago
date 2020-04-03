#ifndef NET_TCP_SESSION_h_
#define NET_TCP_SESSION_h_
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include "net_msg_protocol.h"
#include "net_object_pool.h"
#include "net_safe_container.h"
#include "net_ring_buffer.h"
#include "net_message_mg.h"

NS_DLIB_BEGIN
namespace net{

    template< class T >
	class CNetTcpSession : private boost::noncopyable, public boost::enable_shared_from_this<T>
	{
    public:
        typedef CNetTcpSession<T>							            ImplSession;
        typedef boost::function<void(uint32_t)>							tcp_close_callback;
		typedef boost::function<void(uint32_t,const void*,uint32_t)>	tcp_message_callback;
        typedef boost::function<void(uint32_t) >						tcp_timeout_callback;
        typedef boost::recursive_mutex::scoped_lock                     boost_scoped_lock;

		CNetTcpSession()
		{
		}
		virtual ~CNetTcpSession()
		{
		}

        CNetTcpSession(uint32_t session_id,
            boost::asio::io_service&   io,
            tcp_close_callback	 onclose,
            tcp_message_callback onmessage,
            alloc_buffer_hander  alloc_hander,
            uint32_t             send_buff_size,
            uint32_t             recv_buff_size
            )
            : m_io(io)
            , m_socket(m_io)
            , m_closed(true)
            , m_session_id(session_id)
            , m_disouttime(0)
            , m_distimer(m_io, boost::posix_time::seconds(60))
            , m_onclose( onclose )
            , m_onmessage( onmessage )
            , m_send_buffer(alloc_hander, send_buff_size)
            , m_recv_buffer(alloc_hander, recv_buff_size)
        {
            boost::system::error_code error;
            boost::asio::ip::tcp::no_delay option(true);
            m_socket.set_option(option, error);
            m_is_stop_recv = false;
        }

		void test()
		{
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
            if(!m_socket.is_open())
                return;
			if ( m_closed )
			{
				m_closed = false;

                recv_head();
			}
		}

		void Close()
		{
			if (m_closed)		
				return;
			m_io.post(boost::bind(&ImplSession::do_close, this->shared_from_this(), 0));
		}

		bool Send(const void* data_ptr, uint32_t data_len)
		{
			if(m_closed)
				return false;

            if(data_len >= m_send_buffer.GetMaxLen())
            {
                NETLOG_ERROR("[net.TcpSession.Send]发送失败，发送长度超过缓冲区长度 sessionid="<<m_session_id<<
                    "; data_len="<<data_len);
                return false;
            }

            bool is_empty = m_send_buffer.IsEmpty();
            if(!m_send_buffer.Push(data_ptr, data_len))
            {
                NETLOG_ERROR("[net.TcpSession.Send]发送失败，发送缓冲区满 sessionid="<<m_session_id<<";"
                    <<m_send_buffer.GetDebugInfo());

                NETLOG_ERROR("[net.TcpSession.Send]关闭连接，发送缓冲区满 sessionid="<<m_session_id<<";"
                    <<m_send_buffer.GetDebugInfo());
                do_close(0);
                return false;
            }

            if(is_empty)
                m_io.post(boost::bind(&ImplSession::send_one, this->shared_from_this()));

			return true;
		}

    private:
        void on_close()
        {
            m_onclose(m_session_id);
        }

        bool run()
        {
            if(m_closed)
                return false;

            uint32_t data_len = 0;
            void* data_ptr = m_recv_buffer.Front(data_len);
            if(data_len < sc_net_msg_head_len || data_len > m_recv_buffer.GetMaxLen())
            {
                do_close(0);
                return false;
            }

            m_onmessage(m_session_id, data_ptr, data_len);

            m_recv_buffer.Pop();
            if(m_is_stop_recv)
                recv_body();

            return true;
        }

        void recv_head()
        {
            try
            {
                boost::asio::async_read(m_socket,
                    boost::asio::buffer(&m_recv_head, sc_net_msg_head_len),
                    boost::asio::transfer_at_least(sc_net_msg_head_len),
                    boost::bind( &ImplSession::handle_recv_head, 
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
            catch (...)
            {
                NETLOG_ERROR("net.tcp.recv_head异常"<<FUN_FILE_LINE);
            }
        }

        bool recv_body()
        {
            uint32_t body_len = m_recv_head.len - sc_net_msg_head_len;
            void* buffer_ptr = m_recv_buffer.OpenForRecv(m_recv_head.len);
            if(!buffer_ptr)
            {
                m_is_stop_recv = true;
                return false;
            }
            m_is_stop_recv = false;
            memcpy(buffer_ptr, &m_recv_head, sc_net_msg_head_len);
			if(body_len == 0)
			{
				if(!m_recv_buffer.FinishRecv(m_recv_head.len))
				{
					do_close(0);
					return false;
				}
				CNetMessgeMg::Single()->PushMsgFun(boost::bind(&ImplSession::run, this->shared_from_this()));
				recv_head();
				return true;
			}

            try
            {
                boost::asio::async_read(m_socket,
                    boost::asio::buffer((char*)buffer_ptr+sc_net_msg_head_len, body_len),
                    boost::asio::transfer_at_least(body_len),
                    boost::bind( &ImplSession::handle_recv_body, 
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
            catch (...)
            {
                NETLOG_ERROR("net.tcp.recv_body异常"<<FUN_FILE_LINE);
            }
            return true;
        }

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

            CNetMessgeMg::Single()->PushMsgFun(boost::bind(&ImplSession::on_close, this->shared_from_this()));
        }

        bool send_one()
        {
            if(m_closed)
				return false;

            uint32_t send_data_len = 0;
            void*    send_data_ptr = m_send_buffer.Front(send_data_len);

            if(send_data_len <= 0)
                return true;

            NetMsgHead* msg_head = (NetMsgHead*)send_data_ptr;
            if(msg_head->len >= m_send_buffer.GetMaxLen() || 
                msg_head->len < sc_net_msg_head_len)
            {
                NETLOG_ERROR("[net.TcpSession.Send]关闭连接，包头校验失败 sessionid="<<m_session_id
                    <<" msg_head->len="<<msg_head->len);
                do_close(0);
                return false;
            }

            try
            {
                boost::asio::async_write(m_socket,
                    boost::asio::buffer(send_data_ptr, send_data_len),
                    boost::asio::transfer_at_least(send_data_len),
                    boost::bind(&ImplSession::handle_send, this->shared_from_this(),
                    boost::asio::placeholders::error));

            }
            catch (...)
            {
                NETLOG_ERROR("net.tcp.send_one异常"<<FUN_FILE_LINE);
            }
            return true;
        }

		void handle_send( const boost::system::error_code& error)
		{
            if(error)
            {
                do_close(error.value());
                return;
            }

            m_send_buffer.Pop();
            send_one();
		}

        void handle_recv_head( const boost::system::error_code& error, size_t bytes_transferred)
        {
            if(m_closed)
                return;

            if(error)
            {
                do_close(error.value());
                return;
            }

            if(bytes_transferred != sc_net_msg_head_len
				|| m_recv_head.len < sc_net_msg_head_len)
            {
                do_close(error.value());
                return;
            }
			recv_body();
        }

        void handle_recv_body( const boost::system::error_code& error, size_t bytes_transferred )
        {
            if(m_closed)
                return;

            if(error)
            {
                do_close(error.value());
                return;
            }
            uint32_t body_len = m_recv_head.len - sc_net_msg_head_len;
            if(bytes_transferred != body_len)
            {
                do_close(error.value());
                return;
            }
            
            if(!m_recv_buffer.FinishRecv(m_recv_head.len))
            {
                do_close(error.value());
                return;
            }

			CNetMessgeMg::Single()->PushMsgFun(boost::bind(&ImplSession::run, this->shared_from_this()));
            recv_head();
        }

	protected:
        uint32_t                        m_session_id;
        boost::asio::io_service&	    m_io;
        boost::asio::ip::tcp::socket    m_socket;
        boost::asio::deadline_timer     m_distimer;
        uint64_t					    m_disouttime;
        volatile bool				    m_closed;

        tcp_timeout_callback		    m_onoutime;
        tcp_close_callback			    m_onclose;
        tcp_message_callback		    m_onmessage;

        CNetSendBuffer                  m_send_buffer;
        CNetRecvBuffer                  m_recv_buffer;
        NetMsgHead	                    m_recv_head;
        bool		                    m_is_stop_recv;
	};
}

NS_DLIB_END
#endif // NET_TCP_SESSION_h_
