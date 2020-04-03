#ifndef NET_MESSAGE_MG_H__
#define NET_MESSAGE_MG_H__
#include "../common/common.h"
#include "net_safe_container.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>

NS_DLIB_BEGIN
namespace net
{
	typedef boost::function<void()>		NetMsgFun;
	typedef TSafeMsgQueue<NetMsgFun>    NetMsgFunQueue;

	class CNetMessgeMg
	{
	public:
		static CNetMessgeMg* Single() { static CNetMessgeMg _mg; return &_mg; }

		void		PushMsgFun(NetMsgFun msg_fun, bool is_hiht_level = false);
		uint32_t	Run(uint32_t run_count);

	private:
		CNetMessgeMg();
		~CNetMessgeMg();

		uint32_t	RunHighQueue(uint32_t run_count);
		uint32_t	RunNormalQueue(uint32_t run_count);
		
		NetMsgFunQueue m_high_level_queue;
		NetMsgFunQueue m_normal_queue;
	};
}
NS_DLIB_END
#endif // NET_MESSAGE_MG_H__
