#include "net_ring_buffer.h"
#include "../dcommon.h"
#include <sstream>

dlib::net::CNetRingBuffer::CNetRingBuffer( alloc_buffer_hander hander, uint32_t size )
{
	m_data				= 0; 
	m_alloc_hander		= hander;
	m_head_size			= sizeof(NetMsgHead);
	m_buffer_size		= size;
	m_alloc_hander(m_data, m_buffer_size, true);
	Reset();
}

dlib::net::CNetRingBuffer::~CNetRingBuffer()
{
	if( m_data )
		m_alloc_hander(m_data, m_buffer_size, false);
	m_data = 0; 
}

void dlib::net::CNetRingBuffer::Reset()
{
	m_pos_head = 0;
	m_pos_tail = 0;
	m_pos_last_tail = 0;
}

void* dlib::net::CNetRingBuffer::GetDataPtrByLen( uint32_t data_len )
{
	if(m_buffer_size <= data_len)
		return 0;

	if(0 != m_pos_head && IsEmpty())
		Reset();

	if(m_pos_tail > m_pos_head)
	{
		uint32_t tail_free_len = m_buffer_size - m_pos_tail; //Î²²¿¿ÕÏÐ
		uint32_t head_free_len = m_pos_head;
		if(tail_free_len > data_len)
		{
		}
		else if(head_free_len > data_len) //µôÍ·
		{
			m_pos_last_tail = m_pos_tail;
			m_pos_tail = 0;
		}
		else
			return 0;
	}
	else if (m_pos_tail < m_pos_head)
	{
		if(m_pos_head - m_pos_tail <= data_len)
			return 0;
	}
	return (char*)m_data + m_pos_tail;
}

void dlib::net::CNetRingBuffer::Pop()
{
	if(IsEmpty())
		return;

	NetMsgHead* header = (NetMsgHead*)(m_data + m_pos_head);
	m_pos_head += header->len;
	if(m_pos_head > m_pos_tail && m_pos_head == m_pos_last_tail)
	{
		m_pos_head = 0;
		m_pos_last_tail = 0;
	}
}


dlib::uint32_t dlib::net::CNetRingBuffer::GetDataLen()
{
	uint32_t data_len = 0;
	if(m_pos_tail > m_pos_head)
	{
		data_len = m_pos_tail - m_pos_head;
	}
	else if(m_pos_tail < m_pos_head)
	{
		data_len = m_head_size - (m_pos_head - m_pos_tail);
	}
	return data_len;
}


std::string dlib::net::CNetRingBuffer::GetDebugInfo()
{
	std::ostringstream ss;
	ss<<"head="<<m_pos_head<<" tail="<<m_pos_tail<<" lasttail="<<m_pos_last_tail
		<<" len="<<GetDataLen()<<" size="<<m_buffer_size;

	return ss.str();
}

////////////////////////
dlib::net::CNetSendBuffer::CNetSendBuffer( alloc_buffer_hander hander, uint32_t size )
: CNetRingBuffer(hander, size)
{

}

bool dlib::net::CNetSendBuffer::Push( const void *data_ptr, uint32_t data_len )
{
	if( data_len <= 0 )
		return true;

	void* buffer_ptr = GetDataPtrByLen(data_len);
	if( data_ptr )
	{
		memcpy((char*)buffer_ptr, data_ptr, data_len);
		m_pos_tail += data_len;
		return true;
	}
	return false;
}

void* dlib::net::CNetSendBuffer::Front( uint32_t &data_len )
{
	if(IsEmpty())
	{
		data_len = 0;
		return 0;
	}
	NetMsgHead* header = (NetMsgHead*)(m_data + m_pos_head);
	data_len = header->len;
	return (void*)header;
}

/////////////////////////
dlib::net::CNetRecvBuffer::CNetRecvBuffer( alloc_buffer_hander hander, uint32_t size )
: CNetRingBuffer(hander, size)
{
	m_is_stop_recv = false;
	m_is_head_read = false;
}

void* dlib::net::CNetRecvBuffer::OpenForRecv(uint32_t data_len)
{
	void *data_ptr = GetDataPtrByLen( data_len );
	if( data_ptr )
	{
		m_is_stop_recv = false;
		return data_ptr;
	}
	m_is_stop_recv = true;
	return 0;
}

bool dlib::net::CNetRecvBuffer::FinishRecv( uint32_t data_len )
{
	if(data_len <= 0)
		return true;

	NetMsgHead* header = (NetMsgHead*)(m_data + m_pos_tail);
	if(header->len = m_recv_head.len && 
		header->len == data_len)
	{
		m_pos_tail += data_len;
		return true;
	}
	else
	{
		NETLOG_ERROR("CNetRecvBuffer::FinishRecv len error"<<FUN_FILE_LINE);
		return false;
	}
}

void* dlib::net::CNetRecvBuffer::Front( uint32_t &data_len )
{
	data_len = 0;
	if(IsEmpty())
		return 0;

	NetMsgHead* header = (NetMsgHead*)(m_data + m_pos_head);
	data_len = header->len;
	if(data_len >= m_buffer_size)
	{
		data_len = 0;
		return 0;
	}
	return header;
}
