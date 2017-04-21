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

#ifndef STATUS_VALUE_H_25B076B14BD244B097B84D326BDFB22D
#define STATUS_VALUE_H_25B076B14BD244B097B84D326BDFB22D

#include "src/fredlib/object/object_state.h"
#include "util/enum_conversion.h"

#include <string>
#include <stdexcept>

namespace Epp {
namespace Contact {

/**
 * Represents RFC 5731 EPP Object Status Values of Contact
 */
class StatusValue
{
public:

    enum Enum
    {
        linked,
        server_transfer_prohibited,
        server_delete_prohibited,
        server_update_prohibited,
        delete_candidate,
        conditionally_identified_contact,
        identified_contact,
        validated_contact,
        mojeid_contact,
    };

};

} // namespace Epp::Contact
} // namespace Epp

namespace Conversion {
namespace Enums {

inline std::string to_status_value_name(Epp::Contact::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Contact::StatusValue::conditionally_identified_contact:    return "conditionallyIdentifiedContact";
        case Epp::Contact::StatusValue::delete_candidate:                    return "deleteCandidate";
        case Epp::Contact::StatusValue::identified_contact:                  return "identifiedContact";
        case Epp::Contact::StatusValue::linked:                              return "linked";
        case Epp::Contact::StatusValue::mojeid_contact:                      return "mojeidContact";
        case Epp::Contact::StatusValue::server_delete_prohibited:            return "serverDeleteProhibited";
        case Epp::Contact::StatusValue::server_transfer_prohibited:          return "serverTransferProhibited";
        case Epp::Contact::StatusValue::server_update_prohibited:            return "serverUpdateProhibited";
        case Epp::Contact::StatusValue::validated_contact:                   return "validatedContact";
    }
    throw std::invalid_argument("value doesn't exist in Epp::Contact::StatusValue::Enum");
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_status_value_name(const std::string& staus_value_name);

template <>
inline Epp::Contact::StatusValue::Enum from_status_value_name<Epp::Contact::StatusValue>(const std::string& staus_value_name)
{
    static const Epp::Contact::StatusValue::Enum possible_results[] =
    {
        Epp::Contact::StatusValue::conditionally_identified_contact,
        Epp::Contact::StatusValue::delete_candidate,
        Epp::Contact::StatusValue::identified_contact,
        Epp::Contact::StatusValue::linked,
        Epp::Contact::StatusValue::mojeid_contact,
        Epp::Contact::StatusValue::server_delete_prohibited,
        Epp::Contact::StatusValue::server_transfer_prohibited,
        Epp::Contact::StatusValue::server_update_prohibited,
        Epp::Contact::StatusValue::validated_contact,

    };
    return inverse_transformation(staus_value_name, possible_results, to_status_value_name);
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_fred_object_state(Fred::Object_State::Enum value);

template <>
inline Epp::Contact::StatusValue::Enum from_fred_object_state<Epp::Contact::StatusValue>(Fred::Object_State::Enum value)
{
    switch (value)
    {
        case Fred::Object_State::conditionally_identified_contact:
            return Epp::Contact::StatusValue::conditionally_identified_contact;

        case Fred::Object_State::delete_candidate:
            return Epp::Contact::StatusValue::delete_candidate;

        case Fred::Object_State::identified_contact:
            return Epp::Contact::StatusValue::identified_contact;

        case Fred::Object_State::linked:
            return Epp::Contact::StatusValue::linked;

        case Fred::Object_State::mojeid_contact:
            return Epp::Contact::StatusValue::mojeid_contact;

        case Fred::Object_State::server_delete_prohibited:
            return Epp::Contact::StatusValue::server_delete_prohibited;

        case Fred::Object_State::server_transfer_prohibited:
            return Epp::Contact::StatusValue::server_transfer_prohibited;

        case Fred::Object_State::server_update_prohibited:
            return Epp::Contact::StatusValue::server_update_prohibited;

        case Fred::Object_State::validated_contact:
            return Epp::Contact::StatusValue::validated_contact;

        default:
            break;

    }
    throw std::invalid_argument("value not convertible to Epp::Contact::StatusValue::Enum");
}

inline Fred::Object_State::Enum to_fred_object_state(Epp::Contact::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Contact::StatusValue::conditionally_identified_contact:
            return Fred::Object_State::conditionally_identified_contact;

        case Epp::Contact::StatusValue::delete_candidate:
            return Fred::Object_State::delete_candidate;

        case Epp::Contact::StatusValue::identified_contact:
            return Fred::Object_State::identified_contact;

        case Epp::Contact::StatusValue::linked:
            return Fred::Object_State::linked;

        case Epp::Contact::StatusValue::mojeid_contact:
            return Fred::Object_State::mojeid_contact;

        case Epp::Contact::StatusValue::server_delete_prohibited:
            return Fred::Object_State::server_delete_prohibited;

        case Epp::Contact::StatusValue::server_transfer_prohibited:
            return Fred::Object_State::server_transfer_prohibited;

        case Epp::Contact::StatusValue::server_update_prohibited:
            return Fred::Object_State::server_update_prohibited;

        case Epp::Contact::StatusValue::validated_contact:
            return Fred::Object_State::validated_contact;
    }
    throw std::invalid_argument("value doesn't exist in Fred::Object_State::Enum");
}


} // namespace Conversion::Enums
} // namespace Conversion

#endif
