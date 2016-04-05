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
        CONDITIONALLY_IDENTIFIED_CONTACT,   ///< means database `conditionallyIdentifiedContact` state
        CONTACT_FAILED_MANUAL_VERIFICATION, ///< means database `contactFailedManualVerification` state
        CONTACT_IN_MANUAL_VERIFICATION,     ///< means database `contactInManualVerification` state
        CONTACT_PASSED_MANUAL_VERIFICATION, ///< means database `contactPassedManualVerification` state
        DELETE_CANDIDATE,                   ///< means database `deleteCandidate` state
        DELETE_WARNING,                     ///< means database `deleteWarning` state
        EXPIRATION_WARNING,                 ///< means database `expirationWarning` state
        EXPIRED,                            ///< means database `expired` state
        IDENTIFIED_CONTACT,                 ///< means database `identifiedContact` state
        LINKED,                             ///< means database `linked` state
        MOJEID_CONTACT,                     ///< means database `mojeidContact` state
        NOT_VALIDATED,                      ///< means database `notValidated` state
        NSSET_MISSING,                      ///< means database `nssetMissing` state
        OUTZONE,                            ///< means database `outzone` state
        OUTZONE_UNGUARDED,                  ///< means database `outzoneUnguarded` state
        SERVER_BLOCKED,                     ///< means database `serverBlocked` state
        SERVER_DELETE_PROHIBITED,           ///< means database `serverDeleteProhibited` state
        SERVER_INZONE_MANUAL,               ///< means database `serverInzoneManual` state
        SERVER_OUTZONE_MANUAL,              ///< means database `serverOutzoneManual` state
        SERVER_REGISTRANT_CHANGE_PROHIBITED,///< means database `serverRegistrantChangeProhibited` state
        SERVER_RENEW_PROHIBITED,            ///< means database `serverRenewProhibited` state
        SERVER_TRANSFER_PROHIBITED,         ///< means database `serverTransferProhibited` state
        SERVER_UPDATE_PROHIBITED,           ///< means database `serverUpdateProhibited` state
        UNGUARDED,                          ///< means database `unguarded` state
        VALIDATED_CONTACT,                  ///< means database `validatedContact` state
        VALIDATION_WARNING1,                ///< means database `validationWarning1` state
        VALIDATION_WARNING2,                ///< means database `validationWarning2` state
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
        case Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT:    return "conditionallyIdentifiedContact";
        case Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION:  return "contactFailedManualVerification";
        case Fred::Object::State::CONTACT_IN_MANUAL_VERIFICATION:      return "contactInManualVerification";
        case Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION:  return "contactPassedManualVerification";
        case Fred::Object::State::DELETE_CANDIDATE:                    return "deleteCandidate";
        case Fred::Object::State::DELETE_WARNING:                      return "deleteWarning";
        case Fred::Object::State::EXPIRATION_WARNING:                  return "expirationWarning";
        case Fred::Object::State::EXPIRED:                             return "expired";
        case Fred::Object::State::IDENTIFIED_CONTACT:                  return "identifiedContact";
        case Fred::Object::State::LINKED:                              return "linked";
        case Fred::Object::State::MOJEID_CONTACT:                      return "mojeidContact";
        case Fred::Object::State::NOT_VALIDATED:                       return "notValidated";
        case Fred::Object::State::NSSET_MISSING:                       return "nssetMissing";
        case Fred::Object::State::OUTZONE:                             return "outzone";
        case Fred::Object::State::OUTZONE_UNGUARDED:                   return "outzoneUnguarded";
        case Fred::Object::State::SERVER_BLOCKED:                      return "serverBlocked";
        case Fred::Object::State::SERVER_DELETE_PROHIBITED:            return "serverDeleteProhibited";
        case Fred::Object::State::SERVER_INZONE_MANUAL:                return "serverInzoneManual";
        case Fred::Object::State::SERVER_OUTZONE_MANUAL:               return "serverOutzoneManual";
        case Fred::Object::State::SERVER_REGISTRANT_CHANGE_PROHIBITED: return "serverRegistrantChangeProhibited";
        case Fred::Object::State::SERVER_RENEW_PROHIBITED:             return "serverRenewProhibited";
        case Fred::Object::State::SERVER_TRANSFER_PROHIBITED:          return "serverTransferProhibited";
        case Fred::Object::State::SERVER_UPDATE_PROHIBITED:            return "serverUpdateProhibited";
        case Fred::Object::State::UNGUARDED:                           return "unguarded";
        case Fred::Object::State::VALIDATED_CONTACT:                   return "validatedContact";
        case Fred::Object::State::VALIDATION_WARNING1:                 return "validationWarning1";
        case Fred::Object::State::VALIDATION_WARNING2:                 return "validationWarning2";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Object::State::Enum");
}

template < >
inline Fred::Object::State::Enum from_db_handle< Fred::Object::State >(const std::string &db_handle)
{
    if (to_db_handle(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT) == db_handle) { return Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT; }
    if (to_db_handle(Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION) == db_handle) { return Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION; }
    if (to_db_handle(Fred::Object::State::CONTACT_IN_MANUAL_VERIFICATION) == db_handle) { return Fred::Object::State::CONTACT_IN_MANUAL_VERIFICATION; }
    if (to_db_handle(Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION) == db_handle) { return Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION; }
    if (to_db_handle(Fred::Object::State::DELETE_CANDIDATE) == db_handle) { return Fred::Object::State::DELETE_CANDIDATE; }
    if (to_db_handle(Fred::Object::State::DELETE_WARNING) == db_handle) { return Fred::Object::State::DELETE_WARNING; }
    if (to_db_handle(Fred::Object::State::EXPIRATION_WARNING) == db_handle) { return Fred::Object::State::EXPIRATION_WARNING; }
    if (to_db_handle(Fred::Object::State::EXPIRED) == db_handle) { return Fred::Object::State::EXPIRED; }
    if (to_db_handle(Fred::Object::State::IDENTIFIED_CONTACT) == db_handle) { return Fred::Object::State::IDENTIFIED_CONTACT; }
    if (to_db_handle(Fred::Object::State::LINKED) == db_handle) { return Fred::Object::State::LINKED; }
    if (to_db_handle(Fred::Object::State::MOJEID_CONTACT) == db_handle) { return Fred::Object::State::MOJEID_CONTACT; }
    if (to_db_handle(Fred::Object::State::NOT_VALIDATED) == db_handle) { return Fred::Object::State::NOT_VALIDATED; }
    if (to_db_handle(Fred::Object::State::NSSET_MISSING) == db_handle) { return Fred::Object::State::NSSET_MISSING; }
    if (to_db_handle(Fred::Object::State::OUTZONE) == db_handle) { return Fred::Object::State::OUTZONE; }
    if (to_db_handle(Fred::Object::State::OUTZONE_UNGUARDED) == db_handle) { return Fred::Object::State::OUTZONE_UNGUARDED; }
    if (to_db_handle(Fred::Object::State::SERVER_BLOCKED) == db_handle) { return Fred::Object::State::SERVER_BLOCKED; }
    if (to_db_handle(Fred::Object::State::SERVER_DELETE_PROHIBITED) == db_handle) { return Fred::Object::State::SERVER_DELETE_PROHIBITED; }
    if (to_db_handle(Fred::Object::State::SERVER_INZONE_MANUAL) == db_handle) { return Fred::Object::State::SERVER_INZONE_MANUAL; }
    if (to_db_handle(Fred::Object::State::SERVER_OUTZONE_MANUAL) == db_handle) { return Fred::Object::State::SERVER_OUTZONE_MANUAL; }
    if (to_db_handle(Fred::Object::State::SERVER_REGISTRANT_CHANGE_PROHIBITED) == db_handle) { return Fred::Object::State::SERVER_REGISTRANT_CHANGE_PROHIBITED; }
    if (to_db_handle(Fred::Object::State::SERVER_RENEW_PROHIBITED) == db_handle) { return Fred::Object::State::SERVER_RENEW_PROHIBITED; }
    if (to_db_handle(Fred::Object::State::SERVER_TRANSFER_PROHIBITED) == db_handle) { return Fred::Object::State::SERVER_TRANSFER_PROHIBITED; }
    if (to_db_handle(Fred::Object::State::SERVER_UPDATE_PROHIBITED) == db_handle) { return Fred::Object::State::SERVER_UPDATE_PROHIBITED; }
    if (to_db_handle(Fred::Object::State::UNGUARDED) == db_handle) { return Fred::Object::State::UNGUARDED; }
    if (to_db_handle(Fred::Object::State::VALIDATED_CONTACT) == db_handle) { return Fred::Object::State::VALIDATED_CONTACT; }
    if (to_db_handle(Fred::Object::State::VALIDATION_WARNING1) == db_handle) { return Fred::Object::State::VALIDATION_WARNING1; }
    if (to_db_handle(Fred::Object::State::VALIDATION_WARNING2) == db_handle) { return Fred::Object::State::VALIDATION_WARNING2; }
    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Object::State::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83
