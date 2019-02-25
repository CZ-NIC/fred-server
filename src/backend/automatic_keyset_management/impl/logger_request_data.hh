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
#ifndef LOGGER_REQUEST_DATA_HH_BF2C07D1EDB944BF8D48EDDDF2A7BDC4
#define LOGGER_REQUEST_DATA_HH_BF2C07D1EDB944BF8D48EDDDF2A7BDC4

#include "src/backend/automatic_keyset_management/impl/logger_request_object_type.hh"
#include "src/backend/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/deprecated/libfred/logger_client.hh"
#include "src/deprecated/libfred/requests/request.hh"

#include <string>

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

class LoggerRequestData
{
public:
    template <LoggerRequestObjectType::Enum>
    LoggerRequestData& add(unsigned long long _value);

    template <LoggerRequestProperty::Enum>
    LoggerRequestData& add(const std::string& _value);

    template <LoggerRequestProperty::Enum>
    LoggerRequestData& add(unsigned long long _value);

    const Logger::ObjectReferences& get_object_references() const
    {
        return object_references_;
    }

    const Logger::RequestProperties& get_request_properties() const
    {
        return request_properties_;
    }

private:
    void add_request_property_impl(
            const std::string& _name,
            const std::string& _value,
            const bool is_child)
    {
        request_properties_.push_back(Logger::RequestProperty(_name, _value, is_child));
    }

    Logger::ObjectReferences object_references_;
    Logger::RequestProperties request_properties_;
};

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
