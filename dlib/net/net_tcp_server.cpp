#include "net_tcp_server.h"
#include "net_io_scheduler.h"
#include "net_server_id_mg.h"
#include "net_message_mg.h"
using namespace boost;

NS_DLIB_BEGIN
namespace net{

	CNetTcpServer::CNetTcpServer()
		: m_io(CNetIoScheduler::Single()->GetIoservice())
		, m_strand(CNetIoScheduler::Single()->GetStrand())
		, m_acceptor(m_io)
		, m_closed(true)
		, m_stop_accept(true)
        , m_session_count(0)
	{
		count_bit = 0;
	}

	CNetTcpServer::~CNetTcpServer()
	{
        Stop();
	}

	bool CNetTcpServer::Start(NetServerParam& param)
	{
        if(param.server_handle == 0)
        {
            NETLOG_ERROR("param error, server_handle==null"<<FUN_FILE_LINE);
            return false;
        }

        memcpy(&m_param, &param, sizeof(param));
        m_alloc_mg.AddAlloc(m_param.send_buff_size, m_param.max_connect);
        m_alloc_mg.AddAlloc(m_param.recv_buff_size, m_param.max_connect);

        if(m_param.bind_ip[0] == 0)
            m_endpoint.port(m_param.listen_port);
        else
        {
            boost::asio::ip::address_v4 addr;
            addr.from_string(m_param.bind_ip);
            m_endpoint = boost::asio::ip::tcp::endpoint(addr, m_param.listen_port);
        }
		
		m_acceptor.open(m_endpoint.protocol());
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
		boost::system::error_code error;
		m_acceptor.bind(m_endpoint, error);
		if(error)
		{
			NETLOG_ERROR("bind error "<<error.message()<<" port="<<m_endpoint.port()<<FUN_FILE_LINE);
			return false;
		}

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

	bool CNetTcpServer::Send( uint32_t session_id, const void* data_ptr, uint32_t data_len )
	{
		TcpSessionMapIter iter = m_map_session.find(session_id);
		if(iter == m_map_session.end())
			return false;

		return iter->second->Send(data_ptr, data_len);
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
		
		m_acceptor.async_accept( session_ptr->GetSocket(),
			boost::bind(&CNetTcpServer::HandleAccept, this, session_ptr, 
			boost::asio::placeholders::error));
	}

	CNetTcpServer::TcpSessionPtr CNetTcpServer::CreateSession()
	{
		uint32_t session_id = CNetServerIdMg::Single()->GetNewID(CNetServerIdMg::IDTYPE_SERVER_SESSION);
		if(session_id <= 0)
			return TcpSessionPtr();

		TcpSession* session_obj = (TcpSession*)m_session_pool.AllocObj();
		if(!session_obj)
			return TcpSessionPtr();

		new(session_obj) TcpSession(session_id, m_param, m_io, 
			boost::bind(&ITcpServer::OnClose, m_param.server_handle, _1),
			boost::bind(&ITcpServer::OnRecive, m_param.server_handle, _1, _2, _3),
            boost::bind(&CNetTcpServer::HandleAllocBuffer, this, _1, _2, _3));

		++m_session_count;
        m_map_session.insert(std::make_pair(session_id, session_obj));
		TcpSessionPtr session_ptr(session_obj, boost::bind(&CNetTcpServer::DestroySession, this, _1));
		return session_ptr;
	}

    bool CNetTcpServer::HandleAllocBuffer(char*& buffer, uint32_t size, bool is_alloc)
    {
        if(is_alloc)
            return m_alloc_mg.AllocBuffer(buffer, size);
        else
            return m_alloc_mg.FreeBuffer(buffer, size);
        return false;
    }

	void CNetTcpServer::HandleAccept( TcpSessionPtr session_ptr, const boost::system::error_code& error )
	{
        if(m_closed)
            return;

		bool is_ok = !error;
        CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpServer::AcceptOne, this), true);
        CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpServer::CallbackHandleAccept, this, session_ptr, is_ok));
	}

	void CNetTcpServer::HandleClose( uint32_t session_id )
	{
		CNetMessgeMg::Single()->PushMsgFun(boost::bind(&ITcpServer::OnClose, m_param.server_handle, session_id));
	}

    void CNetTcpServer::CallbackHandleAccept(TcpSessionPtr session_ptr, bool is_ok)
    {
        session_ptr->Start();
		m_param.server_handle->OnConnect(session_ptr->GetSessionId());
    }

    void CNetTcpServer::DestroySession(TcpSession* session_ptr)
    {
        if(!session_ptr)
        {
            NETLOG_ERROR("destroy session session_ptr=null");
            return;
        }
        CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpServer::CallbackDestroySession, this, session_ptr));
    }

    void CNetTcpServer::CallbackDestroySession(TcpSession* session_ptr)
    {
        if(!session_ptr)
        {
            NETLOG_ERROR("destroy session session_ptr=null");
            return;
        }
        --m_session_count;
        uint32_t session_id = session_ptr->GetSessionId();

        CNetServerIdMg::Single()->FreeID(session_id);
        m_session_pool.FreeObj(session_ptr);
        m_map_session.erase(session_id);
    }

    bool CNetTcpServer::Close(uint32_t session_id)
    {
        TcpSessionMapIter iter = m_map_session.find(session_id);
        if(iter == m_map_session.end())
            return false;
        iter->second->Close();
        return true;
    }

    void CNetTcpServer::Stop()
    {
        m_closed = true;
        m_acceptor.close();
        TcpSessionMapIter iter = m_map_session.begin();
        for (; iter != m_map_session.end(); ++iter)
        {
            iter->second->Close();
        }
    }

}

NS_DLIB_END