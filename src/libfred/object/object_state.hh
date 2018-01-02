/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

/**
*  @file
*  header of LibFred::Object_State class
*/
#ifndef OBJECT_STATE_HH_A7838AEAF86D4E8D9E6B3269351FA976
#define OBJECT_STATE_HH_A7838AEAF86D4E8D9E6B3269351FA976

#include "src/util/enum_conversion.hh"

/// Fred matters
namespace LibFred {

/**
 * Bidirectional conversions between string and enum representation of object states.
 */
class Object_State
{
public:
    /**
     * Names of particular object states.
     */
    enum Enum//enum_object_states table
    {
        conditionally_identified_contact,   ///< means database `conditionallyIdentifiedContact` state
        contact_failed_manual_verification, ///< means database `contactFailedManualVerification` state
        contact_in_manual_verification,     ///< means database `contactInManualVerification` state
        contact_passed_manual_verification, ///< means database `contactPassedManualVerification` state
        delete_candidate,                   ///< means database `deleteCandidate` state
        delete_warning,                     ///< means database `deleteWarning` state
        expiration_warning,                 ///< means database `expirationWarning` state
        expired,                            ///< means database `expired` state
        identified_contact,                 ///< means database `identifiedContact` state
        linked,                             ///< means database `linked` state
        mojeid_contact,                     ///< means database `mojeidContact` state
        not_validated,                      ///< means database `notValidated` state
        nsset_missing,                      ///< means database `nssetMissing` state
        outzone,                            ///< means database `outzone` state
        outzone_unguarded,                  ///< means database `outzoneUnguarded` state
        server_blocked,                     ///< means database `serverBlocked` state
        server_delete_prohibited,           ///< means database `serverDeleteProhibited` state
        server_inzone_manual,               ///< means database `serverInzoneManual` state
        server_outzone_manual,              ///< means database `serverOutzoneManual` state
        server_registrant_change_prohibited,///< means database `serverRegistrantChangeProhibited` state
        server_renew_prohibited,            ///< means database `serverRenewProhibited` state
        server_transfer_prohibited,         ///< means database `serverTransferProhibited` state
        server_update_prohibited,           ///< means database `serverUpdateProhibited` state
        unguarded,                          ///< means database `unguarded` state
        validated_contact,                  ///< means database `validatedContact` state
        validation_warning1,                ///< means database `validationWarning1` state
        validation_warning2,                ///< means database `validationWarning2` state
        outzone_unguarded_warning,          ///< means database `outzoneUnguardedWarning` state
    };
};

}//Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(LibFred::Object_State::Enum value)
{
    switch (value)
    {
        case LibFred::Object_State::conditionally_identified_contact:    return "conditionallyIdentifiedContact";
        case LibFred::Object_State::contact_failed_manual_verification:  return "contactFailedManualVerification";
        case LibFred::Object_State::contact_in_manual_verification:      return "contactInManualVerification";
        case LibFred::Object_State::contact_passed_manual_verification:  return "contactPassedManualVerification";
        case LibFred::Object_State::delete_candidate:                    return "deleteCandidate";
        case LibFred::Object_State::delete_warning:                      return "deleteWarning";
        case LibFred::Object_State::expiration_warning:                  return "expirationWarning";
        case LibFred::Object_State::expired:                             return "expired";
        case LibFred::Object_State::identified_contact:                  return "identifiedContact";
        case LibFred::Object_State::linked:                              return "linked";
        case LibFred::Object_State::mojeid_contact:                      return "mojeidContact";
        case LibFred::Object_State::not_validated:                       return "notValidated";
        case LibFred::Object_State::nsset_missing:                       return "nssetMissing";
        case LibFred::Object_State::outzone:                             return "outzone";
        case LibFred::Object_State::outzone_unguarded:                   return "outzoneUnguarded";
        case LibFred::Object_State::server_blocked:                      return "serverBlocked";
        case LibFred::Object_State::server_delete_prohibited:            return "serverDeleteProhibited";
        case LibFred::Object_State::server_inzone_manual:                return "serverInzoneManual";
        case LibFred::Object_State::server_outzone_manual:               return "serverOutzoneManual";
        case LibFred::Object_State::server_registrant_change_prohibited: return "serverRegistrantChangeProhibited";
        case LibFred::Object_State::server_renew_prohibited:             return "serverRenewProhibited";
        case LibFred::Object_State::server_transfer_prohibited:          return "serverTransferProhibited";
        case LibFred::Object_State::server_update_prohibited:            return "serverUpdateProhibited";
        case LibFred::Object_State::unguarded:                           return "unguarded";
        case LibFred::Object_State::validated_contact:                   return "validatedContact";
        case LibFred::Object_State::validation_warning1:                 return "validationWarning1";
        case LibFred::Object_State::validation_warning2:                 return "validationWarning2";
        case LibFred::Object_State::outzone_unguarded_warning:           return "outzoneUnguardedWarning";
    }
    throw std::invalid_argument("value doesn't exist in LibFred::Object_State::Enum");
}

template < >
inline LibFred::Object_State::Enum from_db_handle< LibFred::Object_State >(const std::string &db_handle)
{
    if (to_db_handle(LibFred::Object_State::conditionally_identified_contact) == db_handle) { return LibFred::Object_State::conditionally_identified_contact; }
    if (to_db_handle(LibFred::Object_State::contact_failed_manual_verification) == db_handle) { return LibFred::Object_State::contact_failed_manual_verification; }
    if (to_db_handle(LibFred::Object_State::contact_in_manual_verification) == db_handle) { return LibFred::Object_State::contact_in_manual_verification; }
    if (to_db_handle(LibFred::Object_State::contact_passed_manual_verification) == db_handle) { return LibFred::Object_State::contact_passed_manual_verification; }
    if (to_db_handle(LibFred::Object_State::delete_candidate) == db_handle) { return LibFred::Object_State::delete_candidate; }
    if (to_db_handle(LibFred::Object_State::delete_warning) == db_handle) { return LibFred::Object_State::delete_warning; }
    if (to_db_handle(LibFred::Object_State::expiration_warning) == db_handle) { return LibFred::Object_State::expiration_warning; }
    if (to_db_handle(LibFred::Object_State::expired) == db_handle) { return LibFred::Object_State::expired; }
    if (to_db_handle(LibFred::Object_State::identified_contact) == db_handle) { return LibFred::Object_State::identified_contact; }
    if (to_db_handle(LibFred::Object_State::linked) == db_handle) { return LibFred::Object_State::linked; }
    if (to_db_handle(LibFred::Object_State::mojeid_contact) == db_handle) { return LibFred::Object_State::mojeid_contact; }
    if (to_db_handle(LibFred::Object_State::not_validated) == db_handle) { return LibFred::Object_State::not_validated; }
    if (to_db_handle(LibFred::Object_State::nsset_missing) == db_handle) { return LibFred::Object_State::nsset_missing; }
    if (to_db_handle(LibFred::Object_State::outzone) == db_handle) { return LibFred::Object_State::outzone; }
    if (to_db_handle(LibFred::Object_State::outzone_unguarded) == db_handle) { return LibFred::Object_State::outzone_unguarded; }
    if (to_db_handle(LibFred::Object_State::server_blocked) == db_handle) { return LibFred::Object_State::server_blocked; }
    if (to_db_handle(LibFred::Object_State::server_delete_prohibited) == db_handle) { return LibFred::Object_State::server_delete_prohibited; }
    if (to_db_handle(LibFred::Object_State::server_inzone_manual) == db_handle) { return LibFred::Object_State::server_inzone_manual; }
    if (to_db_handle(LibFred::Object_State::server_outzone_manual) == db_handle) { return LibFred::Object_State::server_outzone_manual; }
    if (to_db_handle(LibFred::Object_State::server_registrant_change_prohibited) == db_handle) { return LibFred::Object_State::server_registrant_change_prohibited; }
    if (to_db_handle(LibFred::Object_State::server_renew_prohibited) == db_handle) { return LibFred::Object_State::server_renew_prohibited; }
    if (to_db_handle(LibFred::Object_State::server_transfer_prohibited) == db_handle) { return LibFred::Object_State::server_transfer_prohibited; }
    if (to_db_handle(LibFred::Object_State::server_update_prohibited) == db_handle) { return LibFred::Object_State::server_update_prohibited; }
    if (to_db_handle(LibFred::Object_State::unguarded) == db_handle) { return LibFred::Object_State::unguarded; }
    if (to_db_handle(LibFred::Object_State::validated_contact) == db_handle) { return LibFred::Object_State::validated_contact; }
    if (to_db_handle(LibFred::Object_State::validation_warning1) == db_handle) { return LibFred::Object_State::validation_warning1; }
    if (to_db_handle(LibFred::Object_State::validation_warning2) == db_handle) { return LibFred::Object_State::validation_warning2; }
    if (to_db_handle(LibFred::Object_State::outzone_unguarded_warning) == db_handle) { return LibFred::Object_State::outzone_unguarded_warning; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to LibFred::Object_State::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif
