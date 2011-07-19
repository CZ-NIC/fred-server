#ifndef _LOGGER_CLIENT_IMPL_H_
#define _LOGGER_CLIENT_IMPL_H_

#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <src/corba/Logger.hh>
#include "corba_wrapper_decl.h"
#include "fredlib/logger_client.h"

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

    std::auto_ptr<RequestCountInfo> getRequestCountUsers(
               const boost::posix_time::ptime &from,
               const boost::posix_time::ptime &to,
               const std::string &service);

private:
    ccReg::Logger_var logger_ref;

    boost::mutex ref_mutex;
};

}
}

/*
ccReg::Logger_var logger_ref;
       logger_ref = ccReg::Logger::_narrow(CorbaContainer::get_instance()->nsresolve("Logger"));
*/

#endif // _LOGGER_CLIENT_IMPL_H_
