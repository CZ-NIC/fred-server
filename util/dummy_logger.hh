#ifndef DUMMY_LOGGER_HH_62EAF1D9E458668CA63929BEC2A6DD13//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define DUMMY_LOGGER_HH_62EAF1D9E458668CA63929BEC2A6DD13

#include "src/fredlib/logger_client.h"

namespace Fred {
namespace Logger {


class DummyLoggerImpl : public LoggerClient
{
public:
    DummyLoggerImpl();

    unsigned long long getRequestCount(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service,
            const std::string &user);

    std::unique_ptr<RequestCountInfo> getRequestCountUsers(
               const boost::posix_time::ptime &from,
               const boost::posix_time::ptime &to,
               const std::string &service);

    unsigned long long createRequest(
            const std::string &_src_ip,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_type,
            const unsigned long long _session_id);

    void closeRequest(
            const unsigned long long _request_id,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_result,
            const unsigned long long _session_id);

};


}
}

#endif//DUMMY_LOGGER_HH_62EAF1D9E458668CA63929BEC2A6DD13
