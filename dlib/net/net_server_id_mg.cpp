#include "net_server_id_mg.h"

dlib::net::CNetServerIdMg::CNetServerIdMg()
{
	m_client_id_begin = 1;
}


bool dlib::net::CNetServerIdMg::IsInvalidID( uint32_t id )
{
	boost::recursive_mutex::scoped_lock l(m_lock);
	return m_map_used_id.find(id) != m_map_used_id.end();
}

dlib::uint32_t dlib::net::CNetServerIdMg::GetNewID()
{
	boost::recursive_mutex::scoped_lock l(m_lock);

	uint32_t new_id = 0;;
	uint32_t loop_count = 0;
	while(loop_count < sc_max_client_id)
	{
		new_id = m_client_id_begin++;

		if( m_client_id_begin >= sc_max_client_id )
			m_client_id_begin = 1;

		if(!IsInvalidID(new_id))
		{
			m_map_used_id[new_id] = 1;
			return new_id;
		}
		++loop_count;
	}

	NETLOG_ERROR("[CNetServerIdMg::GetNewID] Id”√ÕÍ¡À..."<<FUN_FILE_LINE);
	return 0;
}

void dlib::net::CNetServerIdMg::FreeID( uint32_t id )
{
	boost::recursive_mutex::scoped_lock l(m_lock);
	m_map_used_id.erase(id);
}

