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
#ifndef LOGGER_CLIENT_HH_6295BB642CBC4154A1BA7AEC088EF657
#define LOGGER_CLIENT_HH_6295BB642CBC4154A1BA7AEC088EF657

#include "src/deprecated/libfred/requests/request_manager.hh"

namespace LibFred {
namespace Logger {

class LoggerClient
{
public:
    virtual ~LoggerClient() {
    }

    virtual std::unique_ptr<RequestCountInfo> getRequestCountUsers(
            const boost::posix_time::ptime &from,
            const boost::posix_time::ptime &to,
            const std::string &service) = 0;

    virtual unsigned long long createRequest(
            const std::string &_src_ip,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_type,
            const unsigned long long _session_id) = 0;

    virtual void closeRequest(
            const unsigned long long _request_id,
            const std::string &_service,
            const std::string &_content,
            const RequestProperties &_properties,
            const ObjectReferences &_references,
            const std::string &_result,
            const unsigned long long _session_id) = 0;

};

}
}

#endif
