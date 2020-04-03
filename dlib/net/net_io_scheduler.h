#ifndef NET_IO_SCHEDULER_h_
#define NET_IO_SCHEDULER_h_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <vector>
#include "../common/common.h"

NS_DLIB_BEGIN
namespace net{
	class CNetIoScheduler
	{
		typedef std::vector<boost::thread*>		ThreadPool;

	public:
		static CNetIoScheduler* Single() { static CNetIoScheduler _scheduler; return &_scheduler; };

		void Start(uint32_t thread_num = 4);
		void Shutdown();
		boost::asio::io_service&	GetIoservice() { return m_ioservice; };
		boost::asio::strand&		GetStrand() { return m_strand; };

	private:
		CNetIoScheduler();
		~CNetIoScheduler();

		void ThreadFunc();

		bool									m_is_running;
		ThreadPool								m_thread_pool;
		boost::asio::io_service	                m_ioservice;
		boost::asio::strand						m_strand;
	};
}
NS_DLIB_END

#endif // NET_IO_SCHEDULER_h_
