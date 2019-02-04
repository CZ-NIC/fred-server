#ifndef DUMMY_LOGGER_HH_199D82323F4A45628341FBA89D30E936
#define DUMMY_LOGGER_HH_199D82323F4A45628341FBA89D30E936

#include "src/deprecated/libfred/logger_client.hh"

namespace LibFred {
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

#endif
