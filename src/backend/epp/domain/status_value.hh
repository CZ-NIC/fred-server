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

#ifndef STATUS_VALUE_HH_BBB7905E4E284A029AC4E0CB7FA94472
#define STATUS_VALUE_HH_BBB7905E4E284A029AC4E0CB7FA94472

#include "libfred/object/object_state.hh"
#include "util/enum_conversion.hh"

#include <string>
#include <stdexcept>

namespace Epp {
namespace Domain {

/**
 * Represents RFC 5731 EPP Object Status Values of Domain
 */
class StatusValue
{
public:

    enum Enum
    {
        server_delete_prohibited,
        server_renew_prohibited,
        server_transfer_prohibited,
        server_update_prohibited,
        server_registrant_change_prohibited,
        server_blocked,
        server_outzone_manual,
        server_inzone_manual,
        expired,
        outzone,
        not_validated,
        delete_candidate,
    };

};

} // namespace Epp::Domain
} // namespace Epp

namespace Conversion {
namespace Enums {

inline std::string to_status_value_name(Epp::Domain::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Domain::StatusValue::delete_candidate:                    return "deleteCandidate";
        case Epp::Domain::StatusValue::expired:                             return "expired";
        case Epp::Domain::StatusValue::not_validated:                       return "notValidated";
        case Epp::Domain::StatusValue::outzone:                             return "outzone";
        case Epp::Domain::StatusValue::server_blocked:                      return "serverBlocked";
        case Epp::Domain::StatusValue::server_delete_prohibited:            return "serverDeleteProhibited";
        case Epp::Domain::StatusValue::server_inzone_manual:                return "serverInzoneManual";
        case Epp::Domain::StatusValue::server_outzone_manual:               return "serverOutzoneManual";
        case Epp::Domain::StatusValue::server_registrant_change_prohibited: return "serverRegistrantChangeProhibited";
        case Epp::Domain::StatusValue::server_renew_prohibited:             return "serverRenewProhibited";
        case Epp::Domain::StatusValue::server_transfer_prohibited:          return "serverTransferProhibited";
        case Epp::Domain::StatusValue::server_update_prohibited:            return "serverUpdateProhibited";
    }
    throw std::invalid_argument("value doesn't exist in Epp::Domain::StatusValue::Enum");
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_status_value_name(const std::string& staus_value_name);

template <>
inline Epp::Domain::StatusValue::Enum from_status_value_name<Epp::Domain::StatusValue>(const std::string& staus_value_name)
{
    static const Epp::Domain::StatusValue::Enum possible_results[] =
    {
        Epp::Domain::StatusValue::delete_candidate,
        Epp::Domain::StatusValue::expired,
        Epp::Domain::StatusValue::not_validated,
        Epp::Domain::StatusValue::outzone,
        Epp::Domain::StatusValue::server_blocked,
        Epp::Domain::StatusValue::server_delete_prohibited,
        Epp::Domain::StatusValue::server_inzone_manual,
        Epp::Domain::StatusValue::server_outzone_manual,
        Epp::Domain::StatusValue::server_registrant_change_prohibited,
        Epp::Domain::StatusValue::server_renew_prohibited,
        Epp::Domain::StatusValue::server_transfer_prohibited,
        Epp::Domain::StatusValue::server_update_prohibited,

    };
    return inverse_transformation(staus_value_name, possible_results, to_status_value_name);
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_fred_object_state(LibFred::Object_State::Enum value);

template <>
inline Epp::Domain::StatusValue::Enum from_fred_object_state<Epp::Domain::StatusValue>(LibFred::Object_State::Enum value)
{
    switch (value)
    {
        case LibFred::Object_State::delete_candidate:
            return Epp::Domain::StatusValue::delete_candidate;

        case LibFred::Object_State::expired:
            return Epp::Domain::StatusValue::expired;

        case LibFred::Object_State::not_validated:
            return Epp::Domain::StatusValue::not_validated;

        case LibFred::Object_State::outzone:
            return Epp::Domain::StatusValue::outzone;

        case LibFred::Object_State::server_blocked:
            return Epp::Domain::StatusValue::server_blocked;

        case LibFred::Object_State::server_delete_prohibited:
            return Epp::Domain::StatusValue::server_delete_prohibited;

        case LibFred::Object_State::server_inzone_manual:
            return Epp::Domain::StatusValue::server_inzone_manual;

        case LibFred::Object_State::server_outzone_manual:
            return Epp::Domain::StatusValue::server_outzone_manual;

        case LibFred::Object_State::server_registrant_change_prohibited:
            return Epp::Domain::StatusValue::server_registrant_change_prohibited;

        case LibFred::Object_State::server_renew_prohibited:
            return Epp::Domain::StatusValue::server_renew_prohibited;

        case LibFred::Object_State::server_transfer_prohibited:
            return Epp::Domain::StatusValue::server_transfer_prohibited;

        case LibFred::Object_State::server_update_prohibited:
            return Epp::Domain::StatusValue::server_update_prohibited;

        default:
            break;

    }
    throw std::invalid_argument("value not convertible to Epp::Domain::StatusValue::Enum");
}

inline LibFred::Object_State::Enum to_fred_object_state(Epp::Domain::StatusValue::Enum value)
{
    switch (value)
    {
        case Epp::Domain::StatusValue::delete_candidate:
            return LibFred::Object_State::delete_candidate;

        case Epp::Domain::StatusValue::expired:
            return LibFred::Object_State::expired;

        case Epp::Domain::StatusValue::not_validated:
            return LibFred::Object_State::not_validated;

        case Epp::Domain::StatusValue::outzone:
            return LibFred::Object_State::outzone;

        case Epp::Domain::StatusValue::server_blocked:
            return LibFred::Object_State::server_blocked;

        case Epp::Domain::StatusValue::server_delete_prohibited:
            return LibFred::Object_State::server_delete_prohibited;

        case Epp::Domain::StatusValue::server_inzone_manual:
            return LibFred::Object_State::server_inzone_manual;

        case Epp::Domain::StatusValue::server_outzone_manual:
            return LibFred::Object_State::server_outzone_manual;

        case Epp::Domain::StatusValue::server_registrant_change_prohibited:
            return LibFred::Object_State::server_registrant_change_prohibited;

        case Epp::Domain::StatusValue::server_renew_prohibited:
            return LibFred::Object_State::server_renew_prohibited;

        case Epp::Domain::StatusValue::server_transfer_prohibited:
            return LibFred::Object_State::server_transfer_prohibited;

        case Epp::Domain::StatusValue::server_update_prohibited:
            return LibFred::Object_State::server_update_prohibited;
    }
    throw std::invalid_argument("value doesn't exist in LibFred::Object_State::Enum");
}


} // namespace Conversion::Enums
} // namespace Conversion

#endif
