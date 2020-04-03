#ifndef NET_MSG_PROTOCOL_h_
#define NET_MSG_PROTOCOL_h_
#include "../common/common.h"

NS_DLIB_BEGIN
namespace net{

    class CMessageProtocol_2_4
    {
    public:
        enum 
        {
            HEADLEN			= 6,
            MAXBODYLEN		= 512
        };
    public:
        CMessageProtocol_2_4( void )
            : m_session_id(0)
            , m_body_length(0)
            , m_msg_id(0)
        {
            memset( m_data, 0, sizeof(m_data) );
        }

        CMessageProtocol_2_4( const CMessageProtocol_2_4& message )
        {
            m_session_id  = message.m_session_id;
            m_body_length = message.m_body_length;
            m_msg_id	  = message.m_msg_id;
            memcpy( m_data, message.data(), message.length() );
        }

        const char* data( void ) const
        {
            return m_data;
        }
        char* data( void )
        {
            return m_data;
        }

        uint32_t size( void ) const
        {
            return HEADLEN+MAXBODYLEN;
        }

        uint32_t length( void ) const
        {
            return HEADLEN+m_body_length;
        }

        uint16_t head_length( void ) const
        {
            return HEADLEN;
        }

        const char* body( void ) const
        {
            return m_data+HEADLEN;
        }
        char* body( void )
        {
            return m_data+HEADLEN;
        }

        uint16_t body_length( void ) const
        {
            return m_body_length;
        }

        uint32_t msg_id( void ) const
        {
            return m_msg_id;
        }

        void body_length( uint16_t length )
        {
            m_body_length = length;
        }

        void msg_id( uint32_t msg_id )
        {
            m_msg_id = msg_id;
        }

        bool decode_header( void )
        {
            memcpy(&m_body_length, m_data, 2);
            memcpy(&m_msg_id, m_data + 2, 4);

            if ( m_body_length > MAXBODYLEN )
            {
                m_body_length = 0;
                return false;
            }
            return true;
        }

        bool encode_header( void )
        {
            if ( m_body_length > MAXBODYLEN ) 
                return false;
            memcpy(m_data, &m_body_length, 2);
            memcpy(m_data + 2, &m_msg_id, 4);
            return true;
        }

        void decode_body( void )
        {
            // 可以做解密、解压缩等事情
        }

        void encode_body( void )
        {
            // 可以做加密、压缩body部分等事情
        }

        void session_id( uint32_t id )
        {
            m_session_id = id;
        }

        uint32_t session_id( void ) const 
        {
            return m_session_id;
        }

    private:
        uint32_t	    m_session_id;
        uint32_t	    m_msg_id;
        char			m_data[HEADLEN+MAXBODYLEN];
        uint16_t	    m_body_length;
        CMessageProtocol_2_4& operator=( const CMessageProtocol_2_4& );
    };
}
NS_DLIB_END

#endif // NET_MSG_PROTOCOL_h_
