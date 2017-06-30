#include "util/dummy_logger.hh"

namespace Fred {
namespace Logger {


DummyLoggerImpl::DummyLoggerImpl()
{
}


unsigned long long DummyLoggerImpl::getRequestCount(
    const boost::posix_time::ptime &from,
    const boost::posix_time::ptime &to,
    const std::string &service,
    const std::string &user)
{
    return 0;
}


std::unique_ptr<RequestCountInfo> DummyLoggerImpl::getRequestCountUsers(
    const boost::posix_time::ptime &from,
    const boost::posix_time::ptime &to,
    const std::string &service)
{
    return std::unique_ptr<RequestCountInfo>();
}


unsigned long long DummyLoggerImpl::createRequest(
    const std::string &_src_ip,
    const std::string &_service,
    const std::string &_content,
    const RequestProperties &_properties,
    const ObjectReferences &_references,
    const std::string &_type,
    const unsigned long long _session_id)
{
    return 0;
}


void DummyLoggerImpl::closeRequest(
    const unsigned long long _request_id,
    const std::string &_service,
    const std::string &_content,
    const RequestProperties &_properties,
    const ObjectReferences &_references,
    const std::string &_result,
    const unsigned long long _session_id)
{
}


}
}

