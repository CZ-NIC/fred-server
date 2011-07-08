#include "logger_client_impl.h"
#include <boost/date_time/gregorian/gregorian.hpp>


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
    std::string c_from = boost::gregorian::to_iso_extended_string(from.date());
    std::string c_to = boost::gregorian::to_iso_extended_string(to.date());

    return logger_ref->getRequestCount(c_from.c_str(), c_to.c_str(), service.c_str(), user.c_str());
}

