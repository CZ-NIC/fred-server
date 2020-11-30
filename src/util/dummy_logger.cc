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
#include "src/util/dummy_logger.hh"

namespace LibFred {
namespace Logger {


DummyLoggerImpl::DummyLoggerImpl()
{
}


std::unique_ptr<RequestCountInfo> DummyLoggerImpl::getRequestCountUsers(
    const boost::posix_time::ptime &,
    const boost::posix_time::ptime &,
    const std::string &)
{
    return std::unique_ptr<RequestCountInfo>();
}


unsigned long long DummyLoggerImpl::createRequest(
    const std::string &,
    const std::string &,
    const std::string &,
    const RequestProperties &,
    const ObjectReferences &,
    const std::string &,
    const unsigned long long )
{
    return 0;
}


void DummyLoggerImpl::closeRequest(
    const unsigned long long ,
    const std::string &,
    const std::string &,
    const RequestProperties &,
    const ObjectReferences &,
    const std::string &,
    const unsigned long long )
{
}


}
}

