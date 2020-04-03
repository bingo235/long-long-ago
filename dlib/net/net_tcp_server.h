#ifndef NET_TCP_SERVER_H__
#define NET_TCP_SERVER_H__
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include "net_common.h"
#include "net_tcp_server_session.h"
NS_DLIB_BEGIN

namespace net{

    class CNetTcpServer :  private boost::noncopyable
    {
    public:
		typedef CNetTcpServerSession	                TcpSession;
		typedef boost::shared_ptr<TcpSession>			TcpSessionPtr;
		typedef CNetObjectPool<TcpSession>				TcpSessionPool;
		typedef map<uint32_t, TcpSession*>			    TcpSessionMap;
		typedef	TcpSessionMap::iterator					TcpSessionMapIter;

        CNetTcpServer();
        ~CNetTcpServer();
        bool Start(NetServerParam& param);
        void Stop();

		bool Send(uint32_t session_id, const void* data_ptr, uint32_t data_len);
        bool Close(uint32_t session_id);
        uint32_t GetSessionCount() { return m_session_count - 1; }
        
		uint32_t count_bit;

	private:
        void            AcceptOne();
		void 			HandleAccept(TcpSessionPtr session_ptr, const boost::system::error_code& error );
        void 	        CallbackHandleAccept(TcpSessionPtr session_ptr, bool is_ok);
        void 			HandleClose( uint32_t session_id );
		
		TcpSessionPtr	CreateSession();
        void			DestroySession(TcpSession* session_ptr);
        void			CallbackDestroySession(TcpSession* session_ptr);

        bool            HandleAllocBuffer(char*& buffer, uint32_t size, bool is_alloc);

    private:
		volatile bool				    	m_closed;
		bool								m_stop_accept;
        TcpSessionPool						m_session_pool;
		TcpSessionMap						m_map_session;
        uint32_t                            m_session_count;

        boost::asio::io_service&	    	m_io;
		boost::asio::strand&				m_strand;
        boost::asio::ip::tcp::acceptor	    m_acceptor;
        boost::asio::ip::tcp::endpoint  	m_endpoint;

		TcpSession::tcp_timeout_callback	m_onoutime;
		TcpSession::tcp_close_callback		m_onclose;
		TcpSession::tcp_message_callback	m_onmessage;

        CNetAllocMg                         m_alloc_mg;
        NetServerParam                      m_param;
    };
}

NS_DLIB_END
#endif // NET_TCP_SERVER_H__
