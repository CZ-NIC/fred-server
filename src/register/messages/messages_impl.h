//messages_impl.h
#ifndef MESSAGES_IMPL_H_
#define MESSAGES_IMPL_H_

#include <string>
#include <vector>

namespace Registry
{
namespace MessagesImpl
{


void send_sms_impl(const char* contact_handle
        , const char* phone
        , const char* content);

struct PostalAddress
{
    std::string name;
    std::string org;
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string state;
    std::string code;
    std::string county;
};//struct PostalAddress

typedef std::vector<char> ByteBuffer;

void send_letter_impl(const char* contact_handle
        , const PostalAddress& address
        , const ByteBuffer& file_content
        , const char* file_type);

}//namespace MessagesImpl
}//namespace Registry
#endif //MESSAGES_IMPL_H_
