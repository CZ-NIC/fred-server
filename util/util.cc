#include "util.h"
#include <boost/format.hpp>

namespace Util {

std::string make_svtrid(unsigned long long request_id)
{
    return str(boost::format("ReqID-%010d") % request_id );
}


}
