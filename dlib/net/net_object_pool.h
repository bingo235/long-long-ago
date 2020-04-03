#ifndef NET_OBJECT_POOL_H__
#define NET_OBJECT_POOL_H__
#include "../common/common.h"
#include "net_stack_queue.h"
NS_DLIB_BEGIN
namespace net{

    template<class T, uint32_t ALLOC_OBJECT_COUNT = 10>
    class CNetObjectPool
    {
    public:
        typedef Base_Node<T> node;

        CNetObjectPool()
        {
			m_pool_count = 0;
        }

        ~CNetObjectPool()
        {
            ClearPool();
        }

        void* AllocObj( void )
        {
            node *temp_node = 0;
            uint32_t loop = 0;
            while (!(temp_node = m_pool.Pop()))
            {
                ++loop;
                AddNewObject();
                if(loop > 100)
                    break;
            }
			
            return temp_node;
        }

        void FreeObj(void* object)
        {
            if(object)
                m_pool.Push( reinterpret_cast<node*>(object));
        }

		uint32_t Size()
		{
			return	m_pool_count;
		}

		uint32_t FreeSize()
		{
			return m_pool.Size();
		}

        T* NewObj(void)
        {
			void* object = AllocObj();

			if ( object )	
				return ( T* )( ::new(object)T() );

			return 0;
        }

        template<class P0>
        T* NewObj( P0 p0 )
        {
            void* object = AllocObj();

            if ( object )	
                return ( T* )( ::new(object)T( p0 ) );

            return 0;
        }

        template<class P0, class P1>
        T* NewObj( P0 p0, P1 p1 )
        {
            void* object = AllocObj();

            if ( object )	
                return ( T* )( ::new(object)T( p0, p1) );

            return 0;
        }

        template<class P0, class P1, class P2>
        T* NewObj( P0 p0, P1 p1, P2 p2 )
        {
            void* object = AllocObj();

            if ( object )	
                return ( T* )( ::new(object)T( p0, p1, p2) );

            return 0;
        }

        template<class P0, class P1, class P2, class P3>
        T* NewObj( P0 p0, P1 p1, P2 p2, P3 p3 )
        {
            void* object = AllocObj();

            if ( object )	
                return ( T* )( ::new(object)T( p0, p1, p2, p3) );

            return 0;
        }

        template<class P0, class P1, class P2, class P3, class P4>
        T* NewObj( P0 p0, P1 p1, P2 p2, P3 p3, P4 p4 )
        {
            void* object = AllocObj();

            if ( object )	
                return ( T* )( ::new(object)T( p0, p1, p2, p3, p4) );

            return 0;
        }

		template<class P0, class P1, class P2, class P3, class P4, class P5>
		T* NewObj( P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{
			void* object = AllocObj();

			if ( object )	
				return ( T* )( ::new(object)T( p0, p1, p2, p3, p4, p5) );

			return 0;
		}

        void DeleteObj( T* object )
        {
            if ( object )
            {
                (object)->~T( );
                FreeObj(object);
            }
        }

        void ClearPool( void )
        {
            node *temp_node = 0;
            while ((temp_node = m_pool.Pop()))
            {
                free(temp_node);
            }
			m_pool_count = 0;
        }

    private:
        void AddNewObject( void )
        {
            uint32_t count = ALLOC_OBJECT_COUNT;
            count = count > 0 ? count : 1;

            node* temp_node = 0;
            for (uint32_t i = 0; i < count; ++i)
            {
                temp_node = (node*)(malloc(sizeof(node)));
                if (temp_node)
                {
                    temp_node->pnext = 0;
                    m_pool.Push( temp_node );
					++m_pool_count;
                }
            }
        }

    private:
        CBase_Stack<T>	m_pool;
		uint32_t		m_pool_count;

		CNetObjectPool( const CNetObjectPool& );
		CNetObjectPool& operator=( const CNetObjectPool& );
    };
}

NS_DLIB_END

#endif // NET_OBJECT_POOL_H__
