/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#ifndef LOGGER_CLIENT_IMPL_HH_08E0527D90D74431B6E79639226FDB14
#define LOGGER_CLIENT_IMPL_HH_08E0527D90D74431B6E79639226FDB14

#include <memory>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <src/bin/corba/Logger.hh>
#include "src/util/corba_wrapper_decl.hh"
#include "src/deprecated/libfred/logger_client.hh"

namespace LibFred {
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

#endif
