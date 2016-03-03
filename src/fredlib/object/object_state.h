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
    static const Fred::Object::State::Enum values[] =
    {
        Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT,
        Fred::Object::State::CONTACT_FAILED_MANUAL_VERIFICATION,
        Fred::Object::State::CONTACT_IN_MANUAL_VERIFICATION,
        Fred::Object::State::CONTACT_PASSED_MANUAL_VERIFICATION,
        Fred::Object::State::DELETE_CANDIDATE,
        Fred::Object::State::DELETE_WARNING,
        Fred::Object::State::EXPIRATION_WARNING,
        Fred::Object::State::EXPIRED,
        Fred::Object::State::IDENTIFIED_CONTACT,
        Fred::Object::State::LINKED,
        Fred::Object::State::MOJEID_CONTACT,
        Fred::Object::State::NOT_VALIDATED,
        Fred::Object::State::NSSET_MISSING,
        Fred::Object::State::OUTZONE,
        Fred::Object::State::OUTZONE_UNGUARDED,
        Fred::Object::State::SERVER_BLOCKED,
        Fred::Object::State::SERVER_DELETE_PROHIBITED,
        Fred::Object::State::SERVER_INZONE_MANUAL,
        Fred::Object::State::SERVER_OUTZONE_MANUAL,
        Fred::Object::State::SERVER_REGISTRANT_CHANGE_PROHIBITED,
        Fred::Object::State::SERVER_RENEW_PROHIBITED,
        Fred::Object::State::SERVER_TRANSFER_PROHIBITED,
        Fred::Object::State::SERVER_UPDATE_PROHIBITED,
        Fred::Object::State::UNGUARDED,
        Fred::Object::State::VALIDATED_CONTACT,
        Fred::Object::State::VALIDATION_WARNING1,
        Fred::Object::State::VALIDATION_WARNING2,
    };
    return from_db_handle_impl(db_handle, values, "Fred::Object::State::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83
