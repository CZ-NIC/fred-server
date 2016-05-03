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
*  header of Fred::Object::State class
*/
#ifndef OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83

#include "util/enum_conversion.h"

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

/**
 * Bidirectional conversions between string and enum representation of object states.
 */
class State
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
    };
};

}//Fred::Object
}//Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Fred::Object::State::Enum value)
{
    switch (value)
    {
        case Fred::Object::State::conditionally_identified_contact:    return "conditionallyIdentifiedContact";
        case Fred::Object::State::contact_failed_manual_verification:  return "contactFailedManualVerification";
        case Fred::Object::State::contact_in_manual_verification:      return "contactInManualVerification";
        case Fred::Object::State::contact_passed_manual_verification:  return "contactPassedManualVerification";
        case Fred::Object::State::delete_candidate:                    return "deleteCandidate";
        case Fred::Object::State::delete_warning:                      return "deleteWarning";
        case Fred::Object::State::expiration_warning:                  return "expirationWarning";
        case Fred::Object::State::expired:                             return "expired";
        case Fred::Object::State::identified_contact:                  return "identifiedContact";
        case Fred::Object::State::linked:                              return "linked";
        case Fred::Object::State::mojeid_contact:                      return "mojeidContact";
        case Fred::Object::State::not_validated:                       return "notValidated";
        case Fred::Object::State::nsset_missing:                       return "nssetMissing";
        case Fred::Object::State::outzone:                             return "outzone";
        case Fred::Object::State::outzone_unguarded:                   return "outzoneUnguarded";
        case Fred::Object::State::server_blocked:                      return "serverBlocked";
        case Fred::Object::State::server_delete_prohibited:            return "serverDeleteProhibited";
        case Fred::Object::State::server_inzone_manual:                return "serverInzoneManual";
        case Fred::Object::State::server_outzone_manual:               return "serverOutzoneManual";
        case Fred::Object::State::server_registrant_change_prohibited: return "serverRegistrantChangeProhibited";
        case Fred::Object::State::server_renew_prohibited:             return "serverRenewProhibited";
        case Fred::Object::State::server_transfer_prohibited:          return "serverTransferProhibited";
        case Fred::Object::State::server_update_prohibited:            return "serverUpdateProhibited";
        case Fred::Object::State::unguarded:                           return "unguarded";
        case Fred::Object::State::validated_contact:                   return "validatedContact";
        case Fred::Object::State::validation_warning1:                 return "validationWarning1";
        case Fred::Object::State::validation_warning2:                 return "validationWarning2";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Object::State::Enum");
}

template < >
inline Fred::Object::State::Enum from_db_handle< Fred::Object::State >(const std::string &db_handle)
{
    if (to_db_handle(Fred::Object::State::conditionally_identified_contact) == db_handle) { return Fred::Object::State::conditionally_identified_contact; }
    if (to_db_handle(Fred::Object::State::contact_failed_manual_verification) == db_handle) { return Fred::Object::State::contact_failed_manual_verification; }
    if (to_db_handle(Fred::Object::State::contact_in_manual_verification) == db_handle) { return Fred::Object::State::contact_in_manual_verification; }
    if (to_db_handle(Fred::Object::State::contact_passed_manual_verification) == db_handle) { return Fred::Object::State::contact_passed_manual_verification; }
    if (to_db_handle(Fred::Object::State::delete_candidate) == db_handle) { return Fred::Object::State::delete_candidate; }
    if (to_db_handle(Fred::Object::State::delete_warning) == db_handle) { return Fred::Object::State::delete_warning; }
    if (to_db_handle(Fred::Object::State::expiration_warning) == db_handle) { return Fred::Object::State::expiration_warning; }
    if (to_db_handle(Fred::Object::State::expired) == db_handle) { return Fred::Object::State::expired; }
    if (to_db_handle(Fred::Object::State::identified_contact) == db_handle) { return Fred::Object::State::identified_contact; }
    if (to_db_handle(Fred::Object::State::linked) == db_handle) { return Fred::Object::State::linked; }
    if (to_db_handle(Fred::Object::State::mojeid_contact) == db_handle) { return Fred::Object::State::mojeid_contact; }
    if (to_db_handle(Fred::Object::State::not_validated) == db_handle) { return Fred::Object::State::not_validated; }
    if (to_db_handle(Fred::Object::State::nsset_missing) == db_handle) { return Fred::Object::State::nsset_missing; }
    if (to_db_handle(Fred::Object::State::outzone) == db_handle) { return Fred::Object::State::outzone; }
    if (to_db_handle(Fred::Object::State::outzone_unguarded) == db_handle) { return Fred::Object::State::outzone_unguarded; }
    if (to_db_handle(Fred::Object::State::server_blocked) == db_handle) { return Fred::Object::State::server_blocked; }
    if (to_db_handle(Fred::Object::State::server_delete_prohibited) == db_handle) { return Fred::Object::State::server_delete_prohibited; }
    if (to_db_handle(Fred::Object::State::server_inzone_manual) == db_handle) { return Fred::Object::State::server_inzone_manual; }
    if (to_db_handle(Fred::Object::State::server_outzone_manual) == db_handle) { return Fred::Object::State::server_outzone_manual; }
    if (to_db_handle(Fred::Object::State::server_registrant_change_prohibited) == db_handle) { return Fred::Object::State::server_registrant_change_prohibited; }
    if (to_db_handle(Fred::Object::State::server_renew_prohibited) == db_handle) { return Fred::Object::State::server_renew_prohibited; }
    if (to_db_handle(Fred::Object::State::server_transfer_prohibited) == db_handle) { return Fred::Object::State::server_transfer_prohibited; }
    if (to_db_handle(Fred::Object::State::server_update_prohibited) == db_handle) { return Fred::Object::State::server_update_prohibited; }
    if (to_db_handle(Fred::Object::State::unguarded) == db_handle) { return Fred::Object::State::unguarded; }
    if (to_db_handle(Fred::Object::State::validated_contact) == db_handle) { return Fred::Object::State::validated_contact; }
    if (to_db_handle(Fred::Object::State::validation_warning1) == db_handle) { return Fred::Object::State::validation_warning1; }
    if (to_db_handle(Fred::Object::State::validation_warning2) == db_handle) { return Fred::Object::State::validation_warning2; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Object::State::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83