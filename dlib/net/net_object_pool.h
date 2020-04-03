#ifndef NET_OBJECT_POOL_H__
#define NET_OBJECT_POOL_H__
#include "../common/common.h"
#include "net_stack_queue.h"
#include <list>
#include <boost/thread.hpp>
NS_DLIB_BEGIN
namespace net{

    template<class T, uint32_t ALLOC_OBJECT_COUNT = 10>
    class CNetObjectPool
    {
    public:
        CNetObjectPool()
        {
			m_pool_size = 0;
			m_pool_free_size = 0;
        }

        ~CNetObjectPool()
        {
            ClearPool();
        }

        void* AllocObj( void )
        {
			boost::recursive_mutex::scoped_lock l(m_lock);

            uint32_t loop = 0;
            while (m_pool.empty())
            {
                ++loop;
                AddNewObject();
                if(loop > 100)
                    break;
            }
			T* temp_obj = m_pool.front();
			m_pool.pop_front();
			--m_pool_free_size;
            return temp_obj;
        }

        void FreeObj(void* object)
        {
			boost::recursive_mutex::scoped_lock l(m_lock);

            if(object)
			{
                m_pool.push_back(reinterpret_cast<T*>(object));
				++m_pool_free_size;
			}
        }

		uint32_t Size()
		{
			return	m_pool_size;
		}

		uint32_t FreeSize()
		{
			return m_pool_free_size;
		}

        T* NewObj(void)
        {
			void* obj = AllocObj();

			if ( obj )	
				return (T*)(::new(obj)T());

			return 0;
        }

		template<class P0>
		T* NewObj( P0 p0 )
		{
			void* obj = AllocObj();

			if (obj)	
				return (T*)(::new(obj)T(p0));

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
			boost::recursive_mutex::scoped_lock l(m_lock);

            T* temp_obj = 0;
			while( !m_pool.empty() )
			{
				temp_obj = m_pool.front();
				free((void*)temp_obj);
				m_pool.pop_front();
			}
			m_pool_size = 0;
			m_pool_free_size = 0;
        }

    private:
        void AddNewObject( void )
        {
            uint32_t count = ALLOC_OBJECT_COUNT;
            count = count > 0 ? count : 1;

            T* temp_obj = 0;
            for (uint32_t i = 0; i < count; ++i)
            {
                temp_obj = (T*)(malloc(sizeof(T)));
                if (temp_obj)
                {
                    m_pool.push_back( temp_obj );
					++m_pool_size;
					++m_pool_free_size;
                }
            }
        }

    private:
		std::list<T*>	m_pool;
		uint32_t		m_pool_size;
		uint32_t		m_pool_free_size;
		boost::recursive_mutex  m_lock;

		CNetObjectPool( const CNetObjectPool& );
		CNetObjectPool& operator=( const CNetObjectPool& );
    };
}

NS_DLIB_END

#endif // NET_OBJECT_POOL_H__
