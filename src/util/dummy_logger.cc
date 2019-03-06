/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/util/dummy_logger.hh"

namespace LibFred {
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

