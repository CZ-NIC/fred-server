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

#include "util/static_set_of_values.h"

#include <map>
#include <stdexcept>

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
        NO_STATE = -1,                      ///< special value doesn't represent any state
    };
    /**
     * From enum value creates object having methods for conversion to its string representation.
     * @param _value enum value
     */
    explicit State(Value _value):value_(_value) { }
    /**
     * String value converts to its enum equivalent.
     * @param _str database representation of object state
     * @return its enum equivalent
     * @throw std::runtime_error if conversion is impossible
     */
    static Value from(const std::string &_str)
    {
        const StrToValue& str2val = str_to_value();
        StrToValue::const_iterator item_ptr = str2val.find(_str);
        if (item_ptr != str2val.end()) {
            return item_ptr->second;
        }
        throw std::runtime_error("Unknown object state '" + _str + "'");
    }
    /**
     * Enum value converts to its string representation.
     * @param _str where store the result
     * @return its string representation
     * @throw std::runtime_error if conversion is impossible
     */
    std::string& into(std::string &_str)const
    {
        const ValueToStr& val2str = value_to_str();
        ValueToStr::const_iterator item_ptr = val2str.find(value_);
        if (item_ptr != val2str.end()) {
            return _str = item_ptr->second;
        }
        throw std::runtime_error("Invalid object state value");
    }
    /**
     * Enum value converts to value of other type.
     * @tparam T destination type
     * @return converted value
     * @throw std::runtime_error if conversion is impossible
     */
    template < typename T >
    T into()const
    {
        T result;
        this->into(result);
        return result;
    }
    /**
     * Static set of object states intended for operations with collection of object states.
     */
    template < Value v00 = NO_STATE, Value v01 = NO_STATE, Value v02 = NO_STATE,
               Value v03 = NO_STATE, Value v04 = NO_STATE, Value v05 = NO_STATE,
               Value v06 = NO_STATE, Value v07 = NO_STATE, Value v08 = NO_STATE,
               Value v09 = NO_STATE, Value v10 = NO_STATE, Value v11 = NO_STATE,
               Value v12 = NO_STATE, Value v13 = NO_STATE, Value v14 = NO_STATE,
               Value v15 = NO_STATE, Value v16 = NO_STATE, Value v17 = NO_STATE,
               Value v18 = NO_STATE, Value v19 = NO_STATE, Value v20 = NO_STATE,
               Value v21 = NO_STATE, Value v22 = NO_STATE, Value v23 = NO_STATE,
               Value v24 = NO_STATE, Value v25 = NO_STATE, Value v26 = NO_STATE >
    struct set
    {
        /**
         * Static (at compile time) collection of object states.
         */
        typedef static_set_of_values< Value, NO_STATE,
                                      v00, v01, v02, v03, v04, v05, v06, v07, v08,
                                      v09, v10, v11, v12, v13, v14, v15, v16, v17,
                                      v18, v19, v20, v21, v22, v23, v24, v25, v26 > type;
    };
private:
    Value value_;
    typedef std::map< std::string, Value > StrToValue;
    typedef std::map< Value, std::string > ValueToStr;
    static const StrToValue& str_to_value()
    {
        static StrToValue result;
        if (result.empty()) {
            result["conditionallyIdentifiedContact"]   = CONDITIONALLY_IDENTIFIED_CONTACT;
            result["contactFailedManualVerification"]  = CONTACT_FAILED_MANUAL_VERIFICATION;
            result["contactInManualVerification"]      = CONTACT_IN_MANUAL_VERIFICATION;
            result["contactPassedManualVerification"]  = CONTACT_PASSED_MANUAL_VERIFICATION;
            result["deleteCandidate"]                  = DELETE_CANDIDATE;
            result["deleteWarning"]                    = DELETE_WARNING;
            result["expirationWarning"]                = EXPIRATION_WARNING;
            result["expired"]                          = EXPIRED;
            result["identifiedContact"]                = IDENTIFIED_CONTACT;
            result["linked"]                           = LINKED;
            result["mojeidContact"]                    = MOJEID_CONTACT;
            result["notValidated"]                     = NOT_VALIDATED;
            result["nssetMissing"]                     = NSSET_MISSING;
            result["outzone"]                          = OUTZONE;
            result["outzoneUnguarded"]                 = OUTZONE_UNGUARDED;
            result["serverBlocked"]                    = SERVER_BLOCKED;
            result["serverDeleteProhibited"]           = SERVER_DELETE_PROHIBITED;
            result["serverInzoneManual"]               = SERVER_INZONE_MANUAL;
            result["serverOutzoneManual"]              = SERVER_OUTZONE_MANUAL;
            result["serverRegistrantChangeProhibited"] = SERVER_REGISTRANT_CHANGE_PROHIBITED;
            result["serverRenewProhibited"]            = SERVER_RENEW_PROHIBITED;
            result["serverTransferProhibited"]         = SERVER_TRANSFER_PROHIBITED;
            result["serverUpdateProhibited"]           = SERVER_UPDATE_PROHIBITED;
            result["unguarded"]                        = UNGUARDED;
            result["validatedContact"]                 = VALIDATED_CONTACT;
            result["validationWarning1"]               = VALIDATION_WARNING1;
            result["validationWarning2"]               = VALIDATION_WARNING2;
        }
        return result;
    }
    static const ValueToStr& value_to_str()
    {
        static ValueToStr result;
        if (result.empty()) {
            const StrToValue &str2val = str_to_value();
            for (StrToValue::const_iterator ptr = str2val.begin(); ptr != str2val.end(); ++ptr) {
                result[ptr->second] = ptr->first;
            }
            if (str2val.size() != result.size()) {
                throw std::runtime_error("State::str_to_value() returns map with non-unique values");
            }
        }
        return result;
    }
};

}//Fred::Object
}//Fred

#endif//OBJECT_STATE_H_29361CB88732C16EBD1D73A1BE4BFD83
