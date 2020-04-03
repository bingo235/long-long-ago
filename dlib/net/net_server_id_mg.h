#ifndef NET_SERVER_ID_MG_h_
#define NET_SERVER_ID_MG_h_
#include "../common/common.h"
#include <map>
#include <boost/thread.hpp>

NS_DLIB_BEGIN
namespace net
{
	/*
    client_id:         [1--99999]
    */
	static const uint32_t sc_max_client_id = 99999;
	class CNetServerIdMg
	{
	public:
		static CNetServerIdMg* Single() { static CNetServerIdMg _mg; return &_mg; }

		bool        IsInvalidID(uint32_t id);
		uint32_t	GetNewID();
		void		FreeID(uint32_t id);

	private:
		CNetServerIdMg();
		~CNetServerIdMg(){};

		uint32_t						m_client_id_begin;
		std::map<uint32_t, uint32_t>	m_map_used_id;
		boost::recursive_mutex			m_lock;
	};
}
NS_DLIB_END

#endif // NET_SERVER_ID_MG_h_
