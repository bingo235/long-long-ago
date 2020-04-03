#ifndef NET_TCP_CLIENT_H__
#define NET_TCP_CLIENT_H__
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include "net_common.h"
#include "net_tcp_client_session.h"
NS_DLIB_BEGIN

namespace net{

    class CNetTcpClient :  private boost::noncopyable
    {
    public:
		typedef CNetTcpClientSession	                TcpSession;
		typedef boost::shared_ptr<TcpSession>			TcpSessionPtr;
		typedef CNetObjectPool<TcpSession>				TcpSessionPool;
		typedef map<uint32_t, TcpSession*>			    TcpSessionMap;
		typedef	TcpSessionMap::iterator					TcpSessionMapIter;

        CNetTcpClient();
        ~CNetTcpClient();

        void        Start();
        void        Stop();

        uint32_t    AddClient(NetClientParam& param);
		bool        Send(uint32_t session_id, const void* data_ptr, uint32_t data_len);
        bool        Close(uint32_t session_id);
        uint32_t    GetSessionCount() { return m_session_count - 1; }
        
		uint32_t count_bit;

	private:
		TcpSessionPtr	CreateSession(NetClientParam& param);
        void			DestroySession(TcpSession* session_ptr);
        void			CallbackDestroySession(TcpSession* session_ptr);
        bool            HandleAllocBuffer(char*& buffer, uint32_t size, bool is_alloc);

    private:
        TcpSessionPool						m_session_pool;
		TcpSessionMap						m_map_session;
        uint32_t                            m_session_count;

        boost::asio::io_service&	    	m_io;
		boost::asio::strand&				m_strand;
    };
}

NS_DLIB_END
#endif // NET_TCP_CLIENT_H__
