#ifndef _LOGGER_CLIENT_IMPL_H_
#define _LOGGER_CLIENT_IMPL_H_

#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <src/corba/Logger.hh>
#include "util/corba_wrapper_decl.h"
#include "src/fredlib/logger_client.h"

namespace Fred {
namespace Logger {


class LoggerCorbaClientImpl : public LoggerClient
{
public:
    LoggerCorbaClientImpl();

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


private:
    ccReg::Logger_var logger_ref;
    boost::mutex ref_mutex;

    /* { service name => service id } */
    std::map<std::string, unsigned long long> service_map;
    /* { service name => { request name => request_id } } */
    std::map<std::string, std::map<std::string, unsigned long long> > service_request_map;
    /* { service name => { result status name => result code } } */
    std::map<std::string, std::map<std::string, unsigned long long> > service_result_code_map;
};

}
}

/*
ccReg::Logger_var logger_ref;
       logger_ref = ccReg::Logger::_narrow(CorbaContainer::get_instance()->nsresolve("Logger"));
*/

#endif // _LOGGER_CLIENT_IMPL_H_
