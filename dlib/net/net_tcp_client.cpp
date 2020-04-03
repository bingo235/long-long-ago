#include "net_tcp_client.h"
#include "net_io_scheduler.h"
#include "net_server_id_mg.h"
#include "net_message_mg.h"
using namespace boost;

NS_DLIB_BEGIN
namespace net{

    CNetTcpClient::CNetTcpClient()
        : m_io(CNetIoScheduler::Single()->GetIoservice())
        , m_strand(CNetIoScheduler::Single()->GetStrand())
        , m_session_count(0)
    {

    }

    CNetTcpClient::~CNetTcpClient()
    {
        Stop();
    }

    void CNetTcpClient::Stop()
    {

    }

    uint32_t CNetTcpClient::AddClient(NetClientParam& param)
    {
        TcpSessionPtr session_ptr = CreateSession(param);
        if(NULL == session_ptr)
        {
            NETLOG_ERROR("[CNetTcpClient::AddClient()]  CreateSession fail."<<FUN_FILE_LINE);
            return 0;
        }

        session_ptr->StartConnect();
        return session_ptr->GetSessionId();
    }

    bool CNetTcpClient::Send(uint32_t session_id, const void* data_ptr, uint32_t data_len)
    {
        TcpSessionMapIter iter = m_map_session.find(session_id);
        if(iter == m_map_session.end())
            return false;

        return iter->second->Send(data_ptr, data_len);
    }

    bool CNetTcpClient::Close(uint32_t session_id)
    {
        TcpSessionMapIter iter = m_map_session.find(session_id);
        if(iter == m_map_session.end())
            return false;
        iter->second->Close();
        return true;
    }

    CNetTcpClient::TcpSessionPtr CNetTcpClient::CreateSession(NetClientParam& param)
    {
        uint32_t session_id = CNetServerIdMg::Single()->GetNewID(CNetServerIdMg::IDTYPE_CLIENT_SESSION);
        if(session_id <= 0)
            return TcpSessionPtr();

        TcpSession* session_obj = (TcpSession*)m_session_pool.AllocObj();
        if(!session_obj)
            return TcpSessionPtr();

        new(session_obj) TcpSession(session_id, param, m_io, 
            boost::bind(&ITcpClient::OnClose, param.client_handle, _1),
            boost::bind(&ITcpClient::OnRecive, param.client_handle, _1, _2, _3),
            boost::bind(&CNetTcpClient::HandleAllocBuffer, this, _1, _2, _3));

        ++m_session_count;
        m_map_session.insert(std::make_pair(session_id, session_obj));
        TcpSessionPtr session_ptr(session_obj, boost::bind(&CNetTcpClient::DestroySession, this, _1));
        return session_ptr;
    }

    void CNetTcpClient::DestroySession(TcpSession* session_ptr)
    {
        if(!session_ptr)
        {
            NETLOG_ERROR("destroy session session_ptr=null");
            return;
        }
        CNetMessgeMg::Single()->PushMsgFun(boost::bind(&CNetTcpClient::CallbackDestroySession, this, session_ptr));
    }

    void CNetTcpClient::CallbackDestroySession(TcpSession* session_ptr)
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

    bool CNetTcpClient::HandleAllocBuffer(char*& buffer, uint32_t size, bool is_alloc)
    {
        if(is_alloc)
        {
            buffer = new char[size];
            return buffer != 0;
        }
        else
        {
            if(buffer)
            {
                delete[] buffer;
                buffer = 0;
            }
        }
        return false;
    }
}
NS_DLIB_END