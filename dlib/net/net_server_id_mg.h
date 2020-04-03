#ifndef NET_SERVER_ID_MG_h_
#define NET_SERVER_ID_MG_h_
#include "../common/common.h"
#include <map>
#include <boost/thread.hpp>

NS_DLIB_BEGIN
namespace net
{
	/*
    server_session_id:          [1--99999]
    client_session_id:          [100001--109999]
    */

	class CNetServerIdMg
	{
    public:
        static const uint32_t sc_min_server_session_id = 1;
        static const uint32_t sc_max_server_session_id = 99999;
        static const uint32_t sc_min_client_session_id = 100001;
        static const uint32_t sc_max_client_session_id = 109999;

        enum emIDType
        {
            IDTYPE_SERVER_SESSION = 1,
            IDTYPE_CLIENT_SESSION,
        };

		static CNetServerIdMg* Single() { static CNetServerIdMg _mg; return &_mg; }

		bool        IsInvalidID(uint32_t id);
		uint32_t	GetNewID(emIDType type);
		void		FreeID(uint32_t id);

	private:
		CNetServerIdMg();
		~CNetServerIdMg(){};

		uint32_t						m_server_session_id_begin;
        uint32_t						m_client_session_id_begin;
		std::map<uint32_t, uint32_t>	m_map_used_id;
		boost::recursive_mutex			m_lock;
	};
}
NS_DLIB_END

#endif // NET_SERVER_ID_MG_h_
