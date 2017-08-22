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

#ifndef LOGGER_REQUEST_DATA_HH_BF2C07D1EDB944BF8D48EDDDF2A7BDC4
#define LOGGER_REQUEST_DATA_HH_BF2C07D1EDB944BF8D48EDDDF2A7BDC4

#include "src/automatic_keyset_management/impl/logger_request_object_type.hh"
#include "src/automatic_keyset_management/impl/logger_request_property.hh"
#include "src/fredlib/logger_client.h"
#include "src/fredlib/requests/request.h"

#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {

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

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
