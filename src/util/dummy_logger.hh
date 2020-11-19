/*
 * Copyright (C) 2017-2020  CZ.NIC, z. s. p. o.
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
