//messages_impl.cc
#include "messages_impl.h"

namespace Registry
{
namespace MessagesImpl
{


void send_sms_impl(const char* contact_handle
        , const char* phone
        , const char* content)
{

}

void send_letter_impl(const char* contact_handle
        , const PostalAddress& address
        , const ByteBuffer& file_content
        , const char* file_type)
{

}

}//namespace MessagesImpl
}//namespace Registry
