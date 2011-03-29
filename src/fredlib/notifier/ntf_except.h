#ifndef NTF_EXCEPT_H_
#define NTF_EXCEPT_H_

#include <stdexcept>
#include <boost/format.hpp>


namespace Fred {


struct RequestNotFound : public std::runtime_error
{
    RequestNotFound(const unsigned long long &_rid)
        : std::runtime_error(str(boost::format(
            "request with id %1% not found in history; will not be notified") % _rid))
    {
    }
};


}

#endif

