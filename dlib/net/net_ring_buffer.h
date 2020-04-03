#ifndef net_ring_buffer_h__
#define net_ring_buffer_h__
#include <boost/function.hpp>
#include <string>
#include "../common/common.h"
#include "net_msg_head.h"

namespace dlib
{
	namespace net
	{
		static const uint32_t  sc_default_buffer_szie = 1024*15;
		typedef boost::function<bool (char*& buffer, uint32_t size, bool is_alloc)> alloc_buffer_hander;

		class CNetRingBuffer
		{
		protected:
			CNetRingBuffer(alloc_buffer_hander hander, uint32_t size);
			virtual ~CNetRingBuffer();

			inline void Reset();
			char* GetDataHeadPtr(){ return (char*)(m_data + m_pos_head); }
			char* GetDataTailPtr(){ return (char*)(m_data + m_pos_tail); }
			virtual void* GetDataPtrByLen(uint32_t data_len);

		public:
			uint32_t	GetDataLen();
			uint32_t	GetMaxLen()		{ return m_buffer_size; }
			bool		IsEmpty()		{ return m_pos_head == m_pos_tail; }
			void		Pop();
			std::string	GetDebugInfo();

		protected:
			char*                    	m_data;
			uint32_t					m_pos_head;			//头部
			uint32_t					m_pos_tail;			//当前的尾部
			uint32_t					m_pos_last_tail;	//掉头时的尾部
			uint32_t					m_head_size;
			uint32_t					m_buffer_size;
			alloc_buffer_hander			m_alloc_hander;
		};

		class CNetSendBuffer : public CNetRingBuffer
		{
		public:
			CNetSendBuffer(alloc_buffer_hander hander, uint32_t size = sc_default_buffer_szie);
			~CNetSendBuffer(){};

			bool  Push(const void *data_ptr, uint32_t data_len);
			void* Front(uint32_t &data_len);
		};

		class CNetRecvBuffer : public CNetRingBuffer
		{
		public:
			CNetRecvBuffer(alloc_buffer_hander hander, uint32_t size = sc_default_buffer_szie);
			~CNetRecvBuffer(){}

			void*	OpenForRecv(uint32_t data_len);
			bool	FinishRecv(uint32_t data_len);
			void*	Front(uint32_t &data_len);

			bool	is_stop_recv() { return m_is_stop_recv; }
			bool	is_head_read() { return m_is_head_read; }
			void*	recv_head()    { return &m_recv_head; }	

		private:
			bool		m_is_stop_recv;
			bool		m_is_head_read;
			NetMsgHead	m_recv_head;
		};
	}
}

#endif // net_ring_buffer_h__
