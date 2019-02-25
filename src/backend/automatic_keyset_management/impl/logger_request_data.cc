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
#include "src/backend/automatic_keyset_management/impl/logger_request_data.hh"

namespace Fred {
namespace Backend {
namespace AutomaticKeysetManagement {
namespace Impl {

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestObjectType::keyset>(unsigned long long _value)
{
    object_references_.push_back(
            LibFred::Logger::ObjectReference(
                    to_fred_logger_request_object_type_name<LoggerRequestObjectType::keyset>(),
                    _value));
    return *this;
}

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestObjectType::domain>(unsigned long long _value)
{
    object_references_.push_back(
            LibFred::Logger::ObjectReference(
                    to_fred_logger_request_object_type_name<LoggerRequestObjectType::domain>(),
                    _value));
    return *this;
}

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestProperty::name>(const std::string& _value)
{
    add_request_property_impl(
            to_fred_logger_request_property_name<LoggerRequestProperty::name>(),
            _value,
            !LoggerRequestProperty::is_child);
    return *this;
}

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestProperty::keyset>(const std::string& _value)
{
    add_request_property_impl(
            to_fred_logger_request_property_name<LoggerRequestProperty::keyset>(),
            _value,
            !LoggerRequestProperty::is_child);
    return *this;
}

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestProperty::old_dns_key>(const std::string& _value)
{
    add_request_property_impl(
            to_fred_logger_request_property_name<LoggerRequestProperty::old_dns_key>(),
            _value,
            !LoggerRequestProperty::is_child);
    return *this;
}

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestProperty::new_dns_key>(const std::string& _value)
{
    add_request_property_impl(
            to_fred_logger_request_property_name<LoggerRequestProperty::new_dns_key>(),
            _value,
            !LoggerRequestProperty::is_child);
    return *this;
}

template <>
LoggerRequestData& LoggerRequestData::add<LoggerRequestProperty::op_tr_id>(unsigned long long _request_id)
{
    if (_request_id != 0)
    {
        add_request_property_impl(
                to_fred_logger_request_property_name<LoggerRequestProperty::new_dns_key>(),
                Util::make_svtrid(_request_id),
                !LoggerRequestProperty::is_child);
    }
    return *this;
}

} // namespace Fred::Backend::AutomaticKeysetManagement::Impl
} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred
