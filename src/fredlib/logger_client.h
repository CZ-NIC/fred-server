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

    virtual std::unique_ptr<RequestCountInfo> getRequestCountUsers(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service) = 0;

    virtual unsigned long long createRequest(
            const std::string &_src_ip,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_type,
            const unsigned long long _session_id) = 0;

    virtual void closeRequest(
            const unsigned long long _request_id,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_result,
            const unsigned long long _session_id) = 0;

};

}
}

#endif // _LOGGER_CLIENT_H_
