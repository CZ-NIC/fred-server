/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/automatic_keyset_management/impl/logger_request_property.hh"

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

template <>
std::string to_fred_logger_request_property_name<LoggerRequestProperty::name>()
{
    return "name";
}

template <>
std::string to_fred_logger_request_property_name<LoggerRequestProperty::keyset>()
{
    return "keyset";
}

template <>
std::string to_fred_logger_request_property_name<LoggerRequestProperty::old_dns_key>()
{
    return "old_dns_key";
}

template <>
std::string to_fred_logger_request_property_name<LoggerRequestProperty::new_dns_key>()
{
    return "new_dns_key";
}

template <>
std::string to_fred_logger_request_property_name<LoggerRequestProperty::op_tr_id>()
{
    return "opTRID";
}

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred
