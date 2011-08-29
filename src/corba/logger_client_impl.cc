#include "logger_client_impl.h"
#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {
namespace Logger {

LoggerCorbaClientImpl::LoggerCorbaClientImpl()
{
    boost::mutex::scoped_lock lock (ref_mutex);

    logger_ref = ccReg::Logger::_narrow(CorbaContainer::get_instance()->nsresolve("Logger"));
}

unsigned long long LoggerCorbaClientImpl::getRequestCount(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service,
            const std::string &user)
{
    std::string c_from = boost::posix_time::to_iso_string(from);
    std::string c_to = boost::posix_time::to_iso_string(to);

    return logger_ref->getRequestCount(c_from.c_str(), c_to.c_str(), service.c_str(), user.c_str());
}


std::auto_ptr<RequestCountInfo> LoggerCorbaClientImpl::getRequestCountUsers(
           const boost::posix_time::ptime &from,
           const boost::posix_time::ptime &to,
           const std::string &service)
{
    std::string c_from = boost::posix_time::to_iso_string(from);
    std::string c_to = boost::posix_time::to_iso_string(to);

    ccReg::RequestCountInfo_var info = logger_ref->getRequestCountUsers(c_from.c_str(), c_to.c_str(), service.c_str());

    std::auto_ptr<RequestCountInfo> ret(new RequestCountInfo);

    // conversion
    for (unsigned i=0; i<info->length(); ++i) {
        ret->insert(std::make_pair(info[i].user_handle, info[i].count));
    }

    return ret;

}

}
}
