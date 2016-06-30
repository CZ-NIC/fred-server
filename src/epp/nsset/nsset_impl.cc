#include "src/epp/nsset/nsset_impl.h"

namespace Epp {

    bool is_unspecified_ip_addr(const boost::asio::ip::address& ipaddr)
    {
        if(ipaddr.is_v6()
            && ipaddr.to_v6().is_unspecified())
        {
             return true;
        }
        else if(ipaddr.is_v4()
            && (ipaddr.to_v4().to_ulong() == 0ul))
        {
             return true;
        }
        return false;
    }

}
