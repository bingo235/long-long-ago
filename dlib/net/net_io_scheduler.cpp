#include "net_io_scheduler.h"

dlib::net::CNetIoScheduler::CNetIoScheduler()
: m_is_running(false)
, m_strand(m_ioservice)
{

}

dlib::net::CNetIoScheduler::~CNetIoScheduler()
{
	Shutdown();
}

void dlib::net::CNetIoScheduler::Start( uint32_t thread_num )
{
	if (m_is_running)
		return;
	if(!m_thread_pool.empty())
		return;

	m_is_running = true;
	for(uint32_t i = 0; i < thread_num; ++i)
	{
		boost::thread* new_thread(new boost::thread( boost::bind(&CNetIoScheduler::ThreadFunc, this)));
		m_thread_pool.push_back(new_thread);
	}
}


void dlib::net::CNetIoScheduler::Shutdown()
{
	if(!m_is_running)
	{
		m_ioservice.stop();
		m_thread_pool.clear();
		return;
	}

	m_is_running = false;
	m_ioservice.stop();
	boost::thread* pthread = 0;
	for (size_t i = 0; i < m_thread_pool.size(); ++i)
	{
		pthread = m_thread_pool[i];
		pthread->join();
		delete pthread;
	}
	m_thread_pool.clear();
	
}

void dlib::net::CNetIoScheduler::ThreadFunc()
{
	do
	{
		try
		{
			if(0 == m_ioservice.run())
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}
		catch(...)
		{
			NETLOG_ERROR("[CNetIoScheduler::ThreadFunc] exception! "<<FUN_FILE_LINE);
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}while(m_is_running);
}
