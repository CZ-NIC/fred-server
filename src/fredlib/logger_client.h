#ifndef _LOGGER_CLIENT_H_
#define _LOGGER_CLIENT_H_

namespace Fred {

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

};

}

#endif // _LOGGER_CLIENT_H_
