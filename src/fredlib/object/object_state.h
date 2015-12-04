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
    enum Value
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
    /**
     * String value converts to its enum equivalent.
     * @param _str database representation of object state
     * @return its enum equivalent
     * @throw std::invalid_argument if conversion is impossible
     */
    static Value from(const std::string &_str)
    {
        return Conversion::Enums::into_from< Value >::into_enum_from(_str);
    }
};

}//Fred::Object
}//Fred

namespace Conversion {
namespace Enums {

template < >
struct tools_for< Fred::Object::State::Value >
{
    static void enum_to_other_init(void (*enum_to_other_set)(Fred::Object::State::Value, const std::string&))
    {
        using Fred::Object::State;
        enum_to_other_set(State::CONDITIONALLY_IDENTIFIED_CONTACT,    "conditionallyIdentifiedContact");
        enum_to_other_set(State::CONTACT_FAILED_MANUAL_VERIFICATION,  "contactFailedManualVerification");
        enum_to_other_set(State::CONTACT_IN_MANUAL_VERIFICATION,      "contactInManualVerification");
        enum_to_other_set(State::CONTACT_PASSED_MANUAL_VERIFICATION,  "contactPassedManualVerification");
        enum_to_other_set(State::DELETE_CANDIDATE,                    "deleteCandidate");
        enum_to_other_set(State::DELETE_WARNING,                      "deleteWarning");
        enum_to_other_set(State::EXPIRATION_WARNING,                  "expirationWarning");
        enum_to_other_set(State::EXPIRED,                             "expired");
        enum_to_other_set(State::IDENTIFIED_CONTACT,                  "identifiedContact");
        enum_to_other_set(State::LINKED,                              "linked");
        enum_to_other_set(State::MOJEID_CONTACT,                      "mojeidContact");
        enum_to_other_set(State::NOT_VALIDATED,                       "notValidated");
        enum_to_other_set(State::NSSET_MISSING,                       "nssetMissing");
        enum_to_other_set(State::OUTZONE,                             "outzone");
        enum_to_other_set(State::OUTZONE_UNGUARDED,                   "outzoneUnguarded");
        enum_to_other_set(State::SERVER_BLOCKED,                      "serverBlocked");
        enum_to_other_set(State::SERVER_DELETE_PROHIBITED,            "serverDeleteProhibited");
        enum_to_other_set(State::SERVER_INZONE_MANUAL,                "serverInzoneManual");
        enum_to_other_set(State::SERVER_OUTZONE_MANUAL,               "serverOutzoneManual");
        enum_to_other_set(State::SERVER_REGISTRANT_CHANGE_PROHIBITED, "serverRegistrantChangeProhibited");
        enum_to_other_set(State::SERVER_RENEW_PROHIBITED,             "serverRenewProhibited");
        enum_to_other_set(State::SERVER_TRANSFER_PROHIBITED,          "serverTransferProhibited");
        enum_to_other_set(State::SERVER_UPDATE_PROHIBITED,            "serverUpdateProhibited");
        enum_to_other_set(State::UNGUARDED,                           "unguarded");
        enum_to_other_set(State::VALIDATED_CONTACT,                   "validatedContact");
        enum_to_other_set(State::VALIDATION_WARNING1,                 "validationWarning1");
        enum_to_other_set(State::VALIDATION_WARNING2,                 "validationWarning2");
    }
};

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83
