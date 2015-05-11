#ifndef PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D

#include <string>

namespace Fred {

class PublicRequestTypeIface
{
public:
    virtual std::string get_public_request_type()const = 0;
protected:
    virtual ~PublicRequestTypeIface() { }
};

}//namespace Fred

#endif//PUBLIC_REQUEST_TYPE_IFACE_H_E9BC2123C0A6C5F6CF12FF83939D575D
