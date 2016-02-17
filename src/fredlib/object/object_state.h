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
        return Conversion::Enums::operate< Value >::into_enum(_str);
    }
};

}//Fred::Object
}//Fred

namespace Conversion {
namespace Enums {

template < >
struct tools_for< Fred::Object::State::Value >
{
    static void define_enum_to_string_relation(void (*set_matching_string_counterpart)(Fred::Object::State::Value, const std::string&))
    {
        using Fred::Object::State;
        set_matching_string_counterpart(State::CONDITIONALLY_IDENTIFIED_CONTACT,    "conditionallyIdentifiedContact");
        set_matching_string_counterpart(State::CONTACT_FAILED_MANUAL_VERIFICATION,  "contactFailedManualVerification");
        set_matching_string_counterpart(State::CONTACT_IN_MANUAL_VERIFICATION,      "contactInManualVerification");
        set_matching_string_counterpart(State::CONTACT_PASSED_MANUAL_VERIFICATION,  "contactPassedManualVerification");
        set_matching_string_counterpart(State::DELETE_CANDIDATE,                    "deleteCandidate");
        set_matching_string_counterpart(State::DELETE_WARNING,                      "deleteWarning");
        set_matching_string_counterpart(State::EXPIRATION_WARNING,                  "expirationWarning");
        set_matching_string_counterpart(State::EXPIRED,                             "expired");
        set_matching_string_counterpart(State::IDENTIFIED_CONTACT,                  "identifiedContact");
        set_matching_string_counterpart(State::LINKED,                              "linked");
        set_matching_string_counterpart(State::MOJEID_CONTACT,                      "mojeidContact");
        set_matching_string_counterpart(State::NOT_VALIDATED,                       "notValidated");
        set_matching_string_counterpart(State::NSSET_MISSING,                       "nssetMissing");
        set_matching_string_counterpart(State::OUTZONE,                             "outzone");
        set_matching_string_counterpart(State::OUTZONE_UNGUARDED,                   "outzoneUnguarded");
        set_matching_string_counterpart(State::SERVER_BLOCKED,                      "serverBlocked");
        set_matching_string_counterpart(State::SERVER_DELETE_PROHIBITED,            "serverDeleteProhibited");
        set_matching_string_counterpart(State::SERVER_INZONE_MANUAL,                "serverInzoneManual");
        set_matching_string_counterpart(State::SERVER_OUTZONE_MANUAL,               "serverOutzoneManual");
        set_matching_string_counterpart(State::SERVER_REGISTRANT_CHANGE_PROHIBITED, "serverRegistrantChangeProhibited");
        set_matching_string_counterpart(State::SERVER_RENEW_PROHIBITED,             "serverRenewProhibited");
        set_matching_string_counterpart(State::SERVER_TRANSFER_PROHIBITED,          "serverTransferProhibited");
        set_matching_string_counterpart(State::SERVER_UPDATE_PROHIBITED,            "serverUpdateProhibited");
        set_matching_string_counterpart(State::UNGUARDED,                           "unguarded");
        set_matching_string_counterpart(State::VALIDATED_CONTACT,                   "validatedContact");
        set_matching_string_counterpart(State::VALIDATION_WARNING1,                 "validationWarning1");
        set_matching_string_counterpart(State::VALIDATION_WARNING2,                 "validationWarning2");
    }
};

}//namespace Conversion::Enums
}//namespace Conversion

#endif//OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83
