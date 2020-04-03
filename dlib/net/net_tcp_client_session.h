#ifndef NET_TCP_CLIENT_SESSION_H__
#define NET_TCP_CLIENT_SESSION_H__
#include "net_common.h"
#include "net_tcp_session.h"

NS_DLIB_BEGIN
namespace net{

	using namespace boost::asio::ip;
    class CNetTcpClientSession : public CNetTcpSession<CNetTcpClientSession>
    {
    public:
        CNetTcpClientSession(uint32_t session_id,
            const NetClientParam&   param,
            boost::asio::io_service& io,
            tcp_close_callback	 onclose,
            tcp_message_callback onmessage,
            alloc_buffer_hander  alloc_hander);

        ~CNetTcpClientSession();

		void StartConnect();

	private:
		void handle_resolve(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
		void handle_connect(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);

        void on_connect(ITcpClient::Status status);

	private:
		NetClientParam m_param;
		tcp::resolver m_resolver;
    };
}
NS_DLIB_END
#endif // NET_TCP_CLIENT_SESSION_H__
