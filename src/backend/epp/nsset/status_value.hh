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
#ifndef STATUS_VALUE_HH_E667636D77EE42A5978816D7BE901EC9
#define STATUS_VALUE_HH_E667636D77EE42A5978816D7BE901EC9

#include "libfred/object/object_state.hh"
#include "util/enum_conversion.hh"

#include <string>
#include <stdexcept>

namespace Epp {
namespace Nsset {

/**
 * Represents RFC 5731 EPP Object Status Values of Nsset
 */
class StatusValue
{
public:

    enum Enum
    {
        linked,
        server_delete_prohibited,
        server_transfer_prohibited,
        server_update_prohibited,
        delete_candidate,
    };

};

} // namespace Epp::Nsset
} // namespace Epp

namespace Conversion {
namespace Enums {

inline std::string to_status_value_name(Epp::Nsset::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Nsset::StatusValue::delete_candidate:                    return "deleteCandidate";
        case Epp::Nsset::StatusValue::linked:                              return "linked";
        case Epp::Nsset::StatusValue::server_delete_prohibited:            return "serverDeleteProhibited";
        case Epp::Nsset::StatusValue::server_transfer_prohibited:          return "serverTransferProhibited";
        case Epp::Nsset::StatusValue::server_update_prohibited:            return "serverUpdateProhibited";
    }
    throw std::invalid_argument("value doesn't exist in Epp::Nsset::StatusValue::Enum");
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_status_value_name(const std::string& staus_value_name);

template <>
inline Epp::Nsset::StatusValue::Enum from_status_value_name<Epp::Nsset::StatusValue>(const std::string& staus_value_name)
{
    static const Epp::Nsset::StatusValue::Enum possible_results[] =
    {
        Epp::Nsset::StatusValue::delete_candidate,
        Epp::Nsset::StatusValue::linked,
        Epp::Nsset::StatusValue::server_delete_prohibited,
        Epp::Nsset::StatusValue::server_transfer_prohibited,
        Epp::Nsset::StatusValue::server_update_prohibited,

    };
    return inverse_transformation(staus_value_name, possible_results, to_status_value_name);
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_fred_object_state(LibFred::Object_State::Enum value);

template <>
inline Epp::Nsset::StatusValue::Enum from_fred_object_state<Epp::Nsset::StatusValue>(LibFred::Object_State::Enum value)
{
    switch (value)
    {
        case LibFred::Object_State::delete_candidate:
            return Epp::Nsset::StatusValue::delete_candidate;

        case LibFred::Object_State::linked:
            return Epp::Nsset::StatusValue::linked;

        case LibFred::Object_State::server_delete_prohibited:
            return Epp::Nsset::StatusValue::server_delete_prohibited;

        case LibFred::Object_State::server_transfer_prohibited:
            return Epp::Nsset::StatusValue::server_transfer_prohibited;

        case LibFred::Object_State::server_update_prohibited:
            return Epp::Nsset::StatusValue::server_update_prohibited;

        default:
            break;

    }
    throw std::invalid_argument("value not convertible to Epp::Nsset::StatusValue::Enum");
}

inline LibFred::Object_State::Enum to_fred_object_state(Epp::Nsset::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Nsset::StatusValue::delete_candidate:
            return LibFred::Object_State::delete_candidate;

        case Epp::Nsset::StatusValue::linked:
            return LibFred::Object_State::linked;

        case Epp::Nsset::StatusValue::server_delete_prohibited:
            return LibFred::Object_State::server_delete_prohibited;

        case Epp::Nsset::StatusValue::server_transfer_prohibited:
            return LibFred::Object_State::server_transfer_prohibited;

        case Epp::Nsset::StatusValue::server_update_prohibited:
            return LibFred::Object_State::server_update_prohibited;
    }
    throw std::invalid_argument("value doesn't exist in LibFred::Object_State::Enum");
}


} // namespace Conversion::Enums
} // namespace Conversion

#endif
