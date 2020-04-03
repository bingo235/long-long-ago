#ifndef NET_SAFE_CONTAINER_H__
#define NET_SAFE_CONTAINER_H__
#include <queue>
#include <list>
#include <boost/thread.hpp>
#include "../common/common.h"
NS_DLIB_BEGIN
namespace net
{
	template<class T>
	class TSafeQueue
	{
		typedef std::queue<T> TQUEUE;
	public:
		TSafeQueue()
		{
		}
		~TSafeQueue()
		{
		}

		void clear()
		{
			boost::recursive_mutex::scoped_lock l(m_lock);
			while(!m_q.empty())
				m_q.pop();
		}

		bool empty()
		{
			boost::recursive_mutex::scoped_lock l(m_lock);
			return m_q.empty();
		}

		void push(const T val)
		{
			boost::recursive_mutex::scoped_lock l(m_lock);
			m_q.push( val );
		}

		T pop()
		{
			boost::recursive_mutex::scoped_lock l(m_lock);
			T item = m_q.front();
			m_q.pop();
			return item;
		}
		T& front()
		{
			boost::recursive_mutex::scoped_lock l(m_lock);
			return m_q.front();
		}

		uint32_t size()
		{
			boost::recursive_mutex::scoped_lock l(m_lock);
			return m_q.size();
		}

	private:
		boost::recursive_mutex  m_lock;
		TQUEUE					m_q;
	};

    template<class T>
    class TSafeMsgQueue
    {
        typedef std::queue<T> TQUEUE;
    public:
        TSafeMsgQueue()
            : m_read_q(&m_q1)
            , m_write_q(&m_q2)
            , m_size(0)
        {
        }
        ~TSafeMsgQueue()
        {
        }

        void clear()
        {
            boost::recursive_mutex::scoped_lock l(m_lock);
            while(!m_q1.empty())
                m_q1.pop();
            while(!m_q2.empty())
                m_q2.pop();
            m_size = 0;
        }

        bool empty()
        {
            boost::recursive_mutex::scoped_lock l(m_lock);
            return m_size == 0;
        }

        void push(const T val)
        {
            boost::recursive_mutex::scoped_lock l(m_lock);
            m_write_q->push(val);
            ++m_size;
        }

        T pop()
        {
            boost::recursive_mutex::scoped_lock l(m_lock);
            if(m_read_q->empty())
            {
                TQUEUE* tmp_q = m_write_q;
                m_write_q = m_read_q;
                m_read_q = tmp_q;
            }

            T item = m_read_q->front();
            m_read_q->pop();
            --m_size;
            return item;
        }
        T& front()
        {
            boost::recursive_mutex::scoped_lock l(m_lock);
            return m_read_q->front();
        }

        uint32_t size()
        {
            boost::recursive_mutex::scoped_lock l(m_lock);
            return m_size;
        }

    private:
        boost::recursive_mutex  m_lock;
        TQUEUE					m_q1;
        TQUEUE                  m_q2;
        TQUEUE*                 m_read_q;
        TQUEUE*                 m_write_q;
        uint32_t                m_size;
    };
}
NS_DLIB_END
#endif // NET_SAFE_CONTAINER_H__
