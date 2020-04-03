#include "net_tcp_server.h"
#include "net_io_scheduler.h"
#include "net_server_id_mg.h"
using namespace boost;

NS_DLIB_BEGIN
namespace net{

	CNetTcpServer::CNetTcpServer()
		: m_io(CNetIoScheduler::Single()->GetIoservice())
		, m_acceptor(m_io)
		, m_strand(m_io)
		, m_closed(true)
		, m_stop_accept(true)
	{

	}

	CNetTcpServer::~CNetTcpServer()
	{

	}

	bool CNetTcpServer::Start()
	{
		m_endpoint.port(8898);
		m_acceptor.open(m_endpoint.protocol());
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));

		boost::system::error_code	error;

		//	bind endpoint
		m_acceptor.bind(m_endpoint,error);
		if(error)
		{
			NETLOG_ERROR("bind error "<<error.message()<<" port="<<m_endpoint.port()<<FUN_FILE_LINE);
			return false;
		}

		//	start listen
		m_acceptor.listen(boost::asio::socket_base::max_connections,error);
		if(error)
		{
			NETLOG_ERROR("listen error "<<error.message()<<" port="<<m_endpoint.port()<<FUN_FILE_LINE);
			return false;
		}

		m_stop_accept = false;
		m_closed = false;
		AcceptOne();
		return true;
	}

	void CNetTcpServer::AcceptOne()
	{
		if(m_closed)
			return;

		TcpSessionPtr session_ptr = CreateSession();
		if(NULL == session_ptr)
		{
			m_stop_accept = true;
			NETLOG_ERROR("[CNetTcpServer::AcceptOne()] stop accept."<<FUN_FILE_LINE);
			return;
		}
		
		m_map_session.insert(std::make_pair(session_ptr->GetSessionId(), session_ptr));
		m_acceptor.async_accept( session_ptr->GetSocket(),
			m_strand.wrap(boost::bind(&CNetTcpServer::handle_accept, this, session_ptr, 
			boost::asio::placeholders::error)));
	}

	void CNetTcpServer::handle_accept( TcpSessionPtr session_ptr, const boost::system::error_code& error )
	{
		cout<<"on accept id="<<session_ptr->GetSessionId()<<endl;
		session_ptr->Start();
		AcceptOne();
	}

	CNetTcpServer::TcpSessionPtr CNetTcpServer::CreateSession()
	{
		uint32_t session_id = CNetServerIdMg::Single()->GetNewID();
		if(session_id <= 0)
			return TcpSessionPtr();

		TcpSession* session_obj = (TcpSession*)m_session_pool.AllocObj();
		if(!session_obj)
			return TcpSessionPtr();

		new(session_obj) TcpSession(session_id, m_io, m_message_pool, 
			m_onconnect, boost::bind(&CNetTcpServer::OnClose, this, _1), 
			boost::bind(&CNetTcpServer::OnMessage, this, _1, _2));
		
		TcpSessionPtr session_ptr(session_obj, boost::bind(&TcpSessionPool::DeleteObj, &m_session_pool, _1));
		return session_ptr;
	}

	void CNetTcpServer::OnMessage( int socketid, TcpSession::MessagePtr msg )
	{
		cout<<"OnMessage id="<<socketid<<" msgid="<<msg->msg_id()<<endl;
	}	

	void CNetTcpServer::OnClose( int socketid )
	{
		cout<<"OnClose id="<<socketid<<endl;
	}

}

NS_DLIB_END