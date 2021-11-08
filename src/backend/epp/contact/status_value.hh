/*
 * Copyright (C) 2017-2021  CZ.NIC, z. s. p. o.
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

#ifndef STATUS_VALUE_HH_6D608A4E0A6C41ED88EDBA158AD8F801
#define STATUS_VALUE_HH_6D608A4E0A6C41ED88EDBA158AD8F801

#include "libfred/object/object_state.hh"
#include "util/enum_conversion.hh"

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
        server_contact_name_change_prohibited,
        server_contact_organization_change_prohibited,
        server_contact_ident_change_prohibited,
        server_contact_permanent_address_change_prohibited,
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
        case Epp::Contact::StatusValue::server_contact_name_change_prohibited:              return "serverContactNameChangeProhibited";
        case Epp::Contact::StatusValue::server_contact_organization_change_prohibited:      return "serverContactOrganizationChangeProhibited";
        case Epp::Contact::StatusValue::server_contact_ident_change_prohibited:             return "serverContactIdentChangeProhibited";
        case Epp::Contact::StatusValue::server_contact_permanent_address_change_prohibited: return "serverContactPermanentAddressChangeProhibited";
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
        Epp::Contact::StatusValue::server_contact_name_change_prohibited,
        Epp::Contact::StatusValue::server_contact_organization_change_prohibited,
        Epp::Contact::StatusValue::server_contact_ident_change_prohibited,
        Epp::Contact::StatusValue::server_contact_permanent_address_change_prohibited
    };
    return inverse_transformation(staus_value_name, possible_results, to_status_value_name);
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_fred_object_state(LibFred::Object_State::Enum value);

template <>
inline Epp::Contact::StatusValue::Enum from_fred_object_state<Epp::Contact::StatusValue>(LibFred::Object_State::Enum value)
{
    switch (value)
    {
        case LibFred::Object_State::conditionally_identified_contact:
            return Epp::Contact::StatusValue::conditionally_identified_contact;

        case LibFred::Object_State::delete_candidate:
            return Epp::Contact::StatusValue::delete_candidate;

        case LibFred::Object_State::identified_contact:
            return Epp::Contact::StatusValue::identified_contact;

        case LibFred::Object_State::linked:
            return Epp::Contact::StatusValue::linked;

        case LibFred::Object_State::mojeid_contact:
            return Epp::Contact::StatusValue::mojeid_contact;

        case LibFred::Object_State::server_delete_prohibited:
            return Epp::Contact::StatusValue::server_delete_prohibited;

        case LibFred::Object_State::server_transfer_prohibited:
            return Epp::Contact::StatusValue::server_transfer_prohibited;

        case LibFred::Object_State::server_update_prohibited:
            return Epp::Contact::StatusValue::server_update_prohibited;

        case LibFred::Object_State::validated_contact:
            return Epp::Contact::StatusValue::validated_contact;

        case LibFred::Object_State::server_contact_name_change_prohibited:
            return Epp::Contact::StatusValue::server_contact_name_change_prohibited;

        case LibFred::Object_State::server_contact_organization_change_prohibited:
            return Epp::Contact::StatusValue::server_contact_organization_change_prohibited;

        case LibFred::Object_State::server_contact_ident_change_prohibited:
            return Epp::Contact::StatusValue::server_contact_ident_change_prohibited;

        case LibFred::Object_State::server_contact_permanent_address_change_prohibited:
            return Epp::Contact::StatusValue::server_contact_permanent_address_change_prohibited;

        default:
            break;
    }
    throw std::invalid_argument("value not convertible to Epp::Contact::StatusValue::Enum");
}

inline LibFred::Object_State::Enum to_fred_object_state(Epp::Contact::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Contact::StatusValue::conditionally_identified_contact:
            return LibFred::Object_State::conditionally_identified_contact;

        case Epp::Contact::StatusValue::delete_candidate:
            return LibFred::Object_State::delete_candidate;

        case Epp::Contact::StatusValue::identified_contact:
            return LibFred::Object_State::identified_contact;

        case Epp::Contact::StatusValue::linked:
            return LibFred::Object_State::linked;

        case Epp::Contact::StatusValue::mojeid_contact:
            return LibFred::Object_State::mojeid_contact;

        case Epp::Contact::StatusValue::server_delete_prohibited:
            return LibFred::Object_State::server_delete_prohibited;

        case Epp::Contact::StatusValue::server_transfer_prohibited:
            return LibFred::Object_State::server_transfer_prohibited;

        case Epp::Contact::StatusValue::server_update_prohibited:
            return LibFred::Object_State::server_update_prohibited;

        case Epp::Contact::StatusValue::validated_contact:
            return LibFred::Object_State::validated_contact;

        case Epp::Contact::StatusValue::server_contact_name_change_prohibited:
            return LibFred::Object_State::server_contact_name_change_prohibited;

        case Epp::Contact::StatusValue::server_contact_organization_change_prohibited:
            return LibFred::Object_State::server_contact_organization_change_prohibited;

        case Epp::Contact::StatusValue::server_contact_ident_change_prohibited:
            return LibFred::Object_State::server_contact_ident_change_prohibited;

        case Epp::Contact::StatusValue::server_contact_permanent_address_change_prohibited:
            return LibFred::Object_State::server_contact_permanent_address_change_prohibited;
    }
    throw std::invalid_argument("value doesn't exist in LibFred::Object_State::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif
