#include "logger_client_impl.h"
#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {
namespace Logger {

ccReg::RequestProperties_var corba_wrap_request_properties(
        const RequestProperties &_properties)
{
    ccReg::RequestProperties_var props(new ccReg::RequestProperties());
    props->length(_properties.size());
    for (RequestProperties::size_type i = 0; i < _properties.size(); ++i)
    {
        props[i].name = _properties[i].name.c_str();
        props[i].value = _properties[i].value.c_str();
        props[i].child = _properties[i].child;
    }
    return props;
}


ccReg::ObjectReferences_var corba_wrap_object_references(
        const ObjectReferences &_references)
{
    ccReg::ObjectReferences_var refs(new ccReg::ObjectReferences());
    refs->length(_references.size());
    for (ObjectReferences::size_type i = 0; i < _references.size(); ++i)
    {
        refs[i].type = _references[i].type.c_str();
        refs[i].id = _references[i].id;
    }
    return refs;
}




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


unsigned long long LoggerCorbaClientImpl::createRequest(
            const std::string &_src_ip,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_type,
            const unsigned long long _session_id)
{
    try
    {
        ccReg::RequestProperties_var props = corba_wrap_request_properties(_properties);
        ccReg::ObjectReferences_var refs = corba_wrap_object_references(_references);

        ccReg::RequestServiceList_var service_list = logger_ref->getServices();
        unsigned long long service_id = 0;
        for (unsigned long long i = 0; i < service_list->length(); ++i) {
            if (std::string(service_list[i].name) == _service) {
                service_id = service_list[i].id;
                break;
            }
        }
        if (service_id == 0) {
            throw std::runtime_error("unknown service");
        }
        ccReg::RequestTypeList_var request_type_list = logger_ref->getRequestTypesByService(service_id);
        unsigned long long request_type_id = 0;
        for (unsigned long long i = 0; i < request_type_list->length(); ++i) {
            if (std::string(request_type_list[i].name) == _type) {
                request_type_id = request_type_list[i].id;
                break;
            }
        }
        if (request_type_id == 0) {
            throw std::runtime_error("unknown request type");
        }

        CORBA::ULongLong rid = logger_ref->createRequest(
                _src_ip.c_str(),
                service_id,
                _content.c_str(),
                props,
                refs,
                request_type_id,
                _session_id);

        return static_cast<unsigned long long>(rid);
    }
    catch (...)
    {
        throw std::runtime_error("create request failed");
    }
}


void LoggerCorbaClientImpl::closeRequest(
            const unsigned long long _request_id,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_result,
            const unsigned long long _session_id)
{
    try
    {
        ccReg::RequestProperties_var props = corba_wrap_request_properties(_properties);
        ccReg::ObjectReferences_var refs = corba_wrap_object_references(_references);

        ccReg::RequestServiceList_var service_list = logger_ref->getServices();
        unsigned long long service_id = 0;
        for (unsigned long long i = 0; i < service_list->length(); ++i) {
            if (std::string(service_list[i].name) == _service) {
                service_id = service_list[i].id;
                break;
            }
        }
        if (service_id == 0) {
            throw std::runtime_error("unknown service");
        }

        ccReg::ResultCodeList_var result_code_list = logger_ref->getResultCodesByService(service_id);
        unsigned long long result_code_id = 0;
        for (unsigned long long i = 0; i < result_code_list->length(); ++i) {
            if (std::string(result_code_list[i].name) == _result) {
                result_code_id = result_code_list[i].result_code;
                break;
            }
        }
        if (result_code_id == 0) {
            throw std::runtime_error("unknown result code");
        }

        logger_ref->closeRequest(
                _request_id,
                _content.c_str(),
                props,
                refs,
                result_code_id,
                _session_id);


    }
    catch (...)
    {
        throw std::runtime_error("close request failed");
    }
}


}
}
