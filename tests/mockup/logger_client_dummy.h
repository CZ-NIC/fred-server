#ifndef _LOGGER_CLIENT_DUMMY_H_
#define _LOGGER_CLIENT_DUMMY_H_

#include <memory>
#include <map>
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include "src/fredlib/logger_client.h"

namespace Fred {
namespace Logger {

/**
 * mockup
 */
class DummyLoggerCorbaClientImpl : public LoggerClient
{
public:
    DummyLoggerCorbaClientImpl(){}

    unsigned long long getRequestCount(
            const boost::posix_time::ptime&//from
            , const boost::posix_time::ptime&//to
            , const std::string&//service
            , const std::string&//user
            )
    {
        return 0;
    }

    std::auto_ptr<RequestCountInfo> getRequestCountUsers(
               const boost::posix_time::ptime&//from
               , const boost::posix_time::ptime&//to
               , const std::string&//service
               )
    {
        return std::auto_ptr<RequestCountInfo>(new RequestCountInfo);
    }

    unsigned long long createRequest(
            const std::string&//_src_ip
            , const std::string&//_service
            , const std::string&//_content
            , const RequestProperties&//_properties
            , const ObjectReferences&//_references
            , const std::string&//_type
            , const unsigned long long//_session_id
            )
    {
        return 1;
    }

    void closeRequest(
            const unsigned long long// _request_id
            , const std::string&//_service
            , const std::string&//_content
            , const RequestProperties&//_properties
            , const ObjectReferences&//_references
            , const std::string&//_result
            , const unsigned long long// _session_id
            )
    {}
};

}
}


#endif // _LOGGER_CLIENT_DUMMY_H_
