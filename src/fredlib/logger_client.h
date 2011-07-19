#ifndef _LOGGER_CLIENT_H_
#define _LOGGER_CLIENT_H_

#include "requests/request_manager.h"

namespace Fred {
namespace Logger {

class LoggerClient
{
public:
    virtual ~LoggerClient() {
    }

    virtual unsigned long long getRequestCount(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service,
            const std::string &user) = 0;

    virtual std::auto_ptr<RequestCountInfo> getRequestCountUsers(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service) = 0;

};

}
}

#endif // _LOGGER_CLIENT_H_
