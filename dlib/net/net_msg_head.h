#ifndef NET_MSG_HEAD_h_
#define NET_MSG_HEAD_h_
#include "../common/common.h"

namespace dlib
{
	namespace net
	{
#pragma pack(push,1)
		struct NetMsgHead
		{
			int32_t op_code;
			int32_t len;
			int32_t id;
		};
#pragma pack(pop)
	}
    static const uint32_t sc_net_msg_head_len = sizeof(net::NetMsgHead);
}


#endif // NET_MSG_HEAD_h_
