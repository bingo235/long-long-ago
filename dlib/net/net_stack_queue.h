#ifndef NET_STACK_QUEUE_H__
#define NET_STACK_QUEUE_H__
#include "../common/common.h"
#include <boost/thread.hpp>

NS_DLIB_BEGIN
namespace net{
    //基本节点
    template<typename T>
    struct Base_Node
    {
	public:
		T							value;
		Base_Node<T>* volatile	pnext;

		Base_Node( void ) : value( ), pnext( 0 ) { }

	private:
		Base_Node( const Base_Node& );
		Base_Node &operator=( const Base_Node& );
    };

    //栈
    template<class T>
    class CBase_Stack
    {
    public:
        typedef Base_Node<T> node;

        CBase_Stack( void ) : m_phead( 0 ) , m_size(0)
        {
        }

        ~CBase_Stack( void ) 
        {
        }

        // 入栈 
        void Push( node* pnode )
        {
            boost::recursive_mutex::scoped_lock l(m_lock);

            pnode->pnext = m_phead;
            m_phead		 = pnode;
			++m_size;
        }
        // 出栈
        node* Pop( void )
        {
            boost::recursive_mutex::scoped_lock l(m_lock);

            if( m_phead==0 )	
                return 0;

            node* phead = m_phead;
            m_phead = phead->pnext;
			--m_size;
            return phead;
        }

		uint32_t Size()
		{
			return m_size;
		}

    private:
        node*	                m_phead;
		uint32_t				m_size;
        boost::recursive_mutex  m_lock;

    private:
        CBase_Stack( const CBase_Stack& );
        CBase_Stack& operator=( const CBase_Stack& );
    };

    //队列
    template<class T>
    class CBase_Queue
    {
    public:
        typedef Base_Node<T> node;

		CBase_Queue( node* head ) : m_size(0)
        {
            assert( head );
            m_ptail = m_phead = head;
        }

        ~CBase_Queue( void )
        {
            m_phead = m_ptail = 0;
        }

		uint32_t Size()
		{
			return m_size;
		}

        // 入队列
        void Push( node* pnode )
        {
            boost::recursive_mutex::scoped_lock l(m_lock);

            pnode->pnext = 0;
            m_ptail->pnext= pnode;
            m_ptail = pnode;
			++m_size;
        }
        // 出队列
        node* Pop( void )
        {
            boost::recursive_mutex::scoped_lock l(m_lock);

            if ( m_phead==m_ptail )	
                return 0;

            node* phead = m_phead;
            m_phead = phead->pnext;
            phead->pnext = 0;
			--m_size;
            return phead;
        }

    private:
        node*	                m_phead;
        node*	                m_ptail;
		uint32_t				m_size;
        boost::recursive_mutex  m_lock;

    private:
        CBase_Queue( const CBase_Queue& );
        CBase_Queue& operator=( const CBase_Queue& );
    };
}
NS_DLIB_END

#endif // NET_STACK_QUEUE_H__
