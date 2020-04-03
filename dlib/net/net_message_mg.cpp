#include "net_message_mg.h"

dlib::net::CNetMessgeMg::CNetMessgeMg()
{
	
}

dlib::net::CNetMessgeMg::~CNetMessgeMg()
{

}

dlib::uint32_t dlib::net::CNetMessgeMg::RunHighQueue( uint32_t run_count )
{
	uint32_t count = 0;
	while(count < run_count)
	{
		if(m_high_level_queue.size() <= 0)
			break;
		++count;
		NetMsgFun msg_fun = m_high_level_queue.pop();
		msg_fun();
		//m_high_level_queue.pop();
	}
	return count;
}

dlib::uint32_t dlib::net::CNetMessgeMg::RunNormalQueue( uint32_t run_count )
{
	uint32_t count = 0;
	while(count < run_count)
	{
		if(m_normal_queue.size() <= 0)
			break;
		++count;
		NetMsgFun msg_fun = m_normal_queue.pop();
		msg_fun();
		//m_normal_queue.pop();
	}
	return count;
}

dlib::uint32_t dlib::net::CNetMessgeMg::Run( uint32_t run_count )
{
	uint32_t count = 0;

	count = RunHighQueue(run_count);
	count += RunNormalQueue(run_count);

	return count;
}

void dlib::net::CNetMessgeMg::PushMsgFun( NetMsgFun msg_fun, bool is_hiht_level /*= false*/ )
{
	if(is_hiht_level)
		m_high_level_queue.push(msg_fun);
	else
		m_normal_queue.push(msg_fun);
}

