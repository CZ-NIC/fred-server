/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/corba/logger_client_impl.hh"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <utility>

namespace LibFred {
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
    logger_request_count_ref = ccReg::LoggerRequestCount::_narrow(CorbaContainer::get_instance()->nsresolve("LoggerRequestCount"));

    try {
        ccReg::RequestServiceList_var service_list = logger_ref->getServices();
        for (unsigned long long i = 0; i < service_list->length(); ++i) {
            std::string s_name = std::string(service_list[i].name);
            unsigned long long s_id = service_list[i].id;

            service_map.insert(std::make_pair(s_name, s_id));

            ccReg::RequestTypeList_var request_type_list = logger_ref->getRequestTypesByService(s_id);
            for (unsigned long long j = 0; j < request_type_list->length(); ++j) {
                service_request_map[s_name][std::string(request_type_list[j].name)] = request_type_list[j].id;
            }

            ccReg::ResultCodeList_var result_code_list = logger_ref->getResultCodesByService(s_id);
            for (unsigned long long j = 0; j < result_code_list->length(); ++j) {
                service_result_code_map[s_name][std::string(result_code_list[j].name)] = result_code_list[j].result_code;
            }
        }
    }
    catch (...) {
        throw std::runtime_error("unable to initialize logger client");
    }


}

unsigned long long LoggerCorbaClientImpl::getRequestCount(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service,
            const std::string &user)
{
    std::string c_from = boost::posix_time::to_iso_string(from);
    std::string c_to = boost::posix_time::to_iso_string(to);

    return logger_request_count_ref->getRequestCount(c_from.c_str(), c_to.c_str(), service.c_str(), user.c_str());
}


std::unique_ptr<RequestCountInfo> LoggerCorbaClientImpl::getRequestCountUsers(
           const boost::posix_time::ptime &from,
           const boost::posix_time::ptime &to,
           const std::string &service)
{
    std::string c_from = boost::posix_time::to_iso_string(from);
    std::string c_to = boost::posix_time::to_iso_string(to);

    ccReg::RequestCountInfo_var info = logger_request_count_ref->getRequestCountUsers(c_from.c_str(), c_to.c_str(), service.c_str());

    std::unique_ptr<RequestCountInfo> ret(new RequestCountInfo);

    // conversion
    for (unsigned i=0; i<info->length(); ++i) {
      ret->insert(std::make_pair(std::string(info[i].user_handle), info[i].count));
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

        std::map<std::string, unsigned long long>::const_iterator s_it = service_map.find(_service);
        if (s_it == service_map.end())
        {
            throw std::runtime_error("unknown service type");
        }
        std::map<std::string, std::map<std::string, unsigned long long> >::const_iterator r_map_it
            = service_request_map.find(_service);
        if (r_map_it == service_request_map.end())
        {
            throw std::runtime_error("no request type for given service");
        }
        std::map<std::string, unsigned long long>::const_iterator r_it = (r_map_it->second).find(_type);
        if (r_it == (r_map_it->second).end())
        {
            throw std::runtime_error("unknown request type");
        }

        CORBA::ULongLong rid = logger_ref->createRequest(
                _src_ip.c_str(),
                s_it->second,
                _content.c_str(),
                props,
                refs,
                r_it->second,
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


        std::map<std::string, unsigned long long>::const_iterator s_it = service_map.find(_service);
        if (s_it == service_map.end())
        {
            throw std::runtime_error("unknown service");
        }
        std::map<std::string, std::map<std::string, unsigned long long> >::const_iterator r_map_it
            = service_result_code_map.find(_service);
        if (r_map_it == service_request_map.end())
        {
            throw std::runtime_error("no result code for given service");
        }
        std::map<std::string, unsigned long long>::const_iterator r_it = (r_map_it->second).find(_result);
        if (r_it == (r_map_it->second).end())
        {
            throw std::runtime_error("unknown request type");
        }

        logger_ref->closeRequest(
                _request_id,
                _content.c_str(),
                props,
                refs,
                r_it->second,
                _session_id);
    }
    catch (...)
    {
        throw std::runtime_error("close request failed");
    }
}


}
}
