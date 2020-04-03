#ifndef net_ring_buffer_h__
#define net_ring_buffer_h__
#include "../common/common.h"
#include "net_msg_head.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <string>
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
			bool		IsEmpty();
			void		Pop();
			std::string	GetDebugInfo();
            bool        IsAllocOk()     { return 0 != m_data; }

		protected:
			char*                    	m_data;
			uint32_t					m_pos_head;			//头部
			uint32_t					m_pos_tail;			//当前的尾部
			uint32_t					m_pos_last_tail;	//掉头时的尾部
			uint32_t					m_head_size;
			uint32_t					m_buffer_size;
			alloc_buffer_hander			m_alloc_hander;
			boost::recursive_mutex      m_lock;
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
		};
	}
}

#endif // net_ring_buffer_h__
