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

#ifndef OBJECT_STATE_H_2AF4B8DFD68B456AB2BEC5DAED31D509
#define OBJECT_STATE_H_2AF4B8DFD68B456AB2BEC5DAED31D509

#include "src/fredlib/object/object_state.h"
#include "util/enum_conversion.h"

namespace Epp {

/**
 * Bidirectional conversions between string and enum representation of object states.
 */
class Object_State
{
public:
    /**
     * Names of particular object states.
     */
    enum Enum
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

} // namespace Epp

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Epp::Object_State::Enum value)
{
    switch (value)
    {
        case Epp::Object_State::conditionally_identified_contact:    return "conditionallyIdentifiedContact";
        case Epp::Object_State::contact_failed_manual_verification:  return "contactFailedManualVerification";
        case Epp::Object_State::contact_in_manual_verification:      return "contactInManualVerification";
        case Epp::Object_State::contact_passed_manual_verification:  return "contactPassedManualVerification";
        case Epp::Object_State::delete_candidate:                    return "deleteCandidate";
        case Epp::Object_State::delete_warning:                      return "deleteWarning";
        case Epp::Object_State::expiration_warning:                  return "expirationWarning";
        case Epp::Object_State::expired:                             return "expired";
        case Epp::Object_State::identified_contact:                  return "identifiedContact";
        case Epp::Object_State::linked:                              return "linked";
        case Epp::Object_State::mojeid_contact:                      return "mojeidContact";
        case Epp::Object_State::not_validated:                       return "notValidated";
        case Epp::Object_State::nsset_missing:                       return "nssetMissing";
        case Epp::Object_State::outzone:                             return "outzone";
        case Epp::Object_State::outzone_unguarded:                   return "outzoneUnguarded";
        case Epp::Object_State::server_blocked:                      return "serverBlocked";
        case Epp::Object_State::server_delete_prohibited:            return "serverDeleteProhibited";
        case Epp::Object_State::server_inzone_manual:                return "serverInzoneManual";
        case Epp::Object_State::server_outzone_manual:               return "serverOutzoneManual";
        case Epp::Object_State::server_registrant_change_prohibited: return "serverRegistrantChangeProhibited";
        case Epp::Object_State::server_renew_prohibited:             return "serverRenewProhibited";
        case Epp::Object_State::server_transfer_prohibited:          return "serverTransferProhibited";
        case Epp::Object_State::server_update_prohibited:            return "serverUpdateProhibited";
        case Epp::Object_State::unguarded:                           return "unguarded";
        case Epp::Object_State::validated_contact:                   return "validatedContact";
        case Epp::Object_State::validation_warning1:                 return "validationWarning1";
        case Epp::Object_State::validation_warning2:                 return "validationWarning2";
        case Epp::Object_State::outzone_unguarded_warning:           return "outzoneUnguardedWarning";
    }
    throw std::invalid_argument("value doesn't exist in Epp::Object_State::Enum");
}

template <>
inline Epp::Object_State::Enum from_db_handle<Epp::Object_State>(const std::string& db_handle)
{
    if (to_db_handle(Epp::Object_State::conditionally_identified_contact) == db_handle) {
              return Epp::Object_State::conditionally_identified_contact; }

    if (to_db_handle(Epp::Object_State::contact_failed_manual_verification) == db_handle) {
              return Epp::Object_State::contact_failed_manual_verification; }

    if (to_db_handle(Epp::Object_State::contact_in_manual_verification) == db_handle) {
              return Epp::Object_State::contact_in_manual_verification; }

    if (to_db_handle(Epp::Object_State::contact_passed_manual_verification) == db_handle) {
              return Epp::Object_State::contact_passed_manual_verification; }

    if (to_db_handle(Epp::Object_State::delete_candidate) == db_handle) {
              return Epp::Object_State::delete_candidate; }

    if (to_db_handle(Epp::Object_State::delete_warning) == db_handle) {
              return Epp::Object_State::delete_warning; }

    if (to_db_handle(Epp::Object_State::expiration_warning) == db_handle) {
              return Epp::Object_State::expiration_warning; }

    if (to_db_handle(Epp::Object_State::expired) == db_handle) {
              return Epp::Object_State::expired; }

    if (to_db_handle(Epp::Object_State::identified_contact) == db_handle) {
              return Epp::Object_State::identified_contact; }

    if (to_db_handle(Epp::Object_State::linked) == db_handle) {
              return Epp::Object_State::linked; }

    if (to_db_handle(Epp::Object_State::mojeid_contact) == db_handle) {
              return Epp::Object_State::mojeid_contact; }

    if (to_db_handle(Epp::Object_State::not_validated) == db_handle) {
              return Epp::Object_State::not_validated; }

    if (to_db_handle(Epp::Object_State::nsset_missing) == db_handle) {
              return Epp::Object_State::nsset_missing; }

    if (to_db_handle(Epp::Object_State::outzone) == db_handle) {
              return Epp::Object_State::outzone; }

    if (to_db_handle(Epp::Object_State::outzone_unguarded) == db_handle) {
              return Epp::Object_State::outzone_unguarded; }

    if (to_db_handle(Epp::Object_State::server_blocked) == db_handle) {
              return Epp::Object_State::server_blocked; }

    if (to_db_handle(Epp::Object_State::server_delete_prohibited) == db_handle) {
              return Epp::Object_State::server_delete_prohibited; }

    if (to_db_handle(Epp::Object_State::server_inzone_manual) == db_handle) {
              return Epp::Object_State::server_inzone_manual; }

    if (to_db_handle(Epp::Object_State::server_outzone_manual) == db_handle) {
              return Epp::Object_State::server_outzone_manual; }

    if (to_db_handle(Epp::Object_State::server_registrant_change_prohibited) == db_handle) {
              return Epp::Object_State::server_registrant_change_prohibited; }

    if (to_db_handle(Epp::Object_State::server_renew_prohibited) == db_handle) {
              return Epp::Object_State::server_renew_prohibited; }

    if (to_db_handle(Epp::Object_State::server_transfer_prohibited) == db_handle) {
              return Epp::Object_State::server_transfer_prohibited; }

    if (to_db_handle(Epp::Object_State::server_update_prohibited) == db_handle) {
              return Epp::Object_State::server_update_prohibited; }

    if (to_db_handle(Epp::Object_State::unguarded) == db_handle) {
              return Epp::Object_State::unguarded; }

    if (to_db_handle(Epp::Object_State::validated_contact) == db_handle) {
              return Epp::Object_State::validated_contact; }

    if (to_db_handle(Epp::Object_State::validation_warning1) == db_handle) {
              return Epp::Object_State::validation_warning1; }

    if (to_db_handle(Epp::Object_State::validation_warning2) == db_handle) {
              return Epp::Object_State::validation_warning2; }

    if (to_db_handle(Epp::Object_State::outzone_unguarded_warning) == db_handle) {
              return Epp::Object_State::outzone_unguarded_warning; }

    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Epp::Object_State::Enum");
}

template <typename ENUM_HOST_TYPE>
inline typename ENUM_HOST_TYPE::Enum from_fred_object_state(Fred::Object_State::Enum value);

template <>
inline Epp::Object_State::Enum from_fred_object_state<Epp::Object_State>(Fred::Object_State::Enum value)
{
    switch (value)
    {
        case Fred::Object_State::conditionally_identified_contact:
       return Epp::Object_State::conditionally_identified_contact;

        case Fred::Object_State::contact_failed_manual_verification:
       return Epp::Object_State::contact_failed_manual_verification;

        case Fred::Object_State::contact_in_manual_verification:
       return Epp::Object_State::contact_in_manual_verification;

        case Fred::Object_State::contact_passed_manual_verification:
       return Epp::Object_State::contact_passed_manual_verification;

        case Fred::Object_State::delete_candidate:
       return Epp::Object_State::delete_candidate;

        case Fred::Object_State::delete_warning:
       return Epp::Object_State::delete_warning;

        case Fred::Object_State::expiration_warning:
       return Epp::Object_State::expiration_warning;

        case Fred::Object_State::expired:
       return Epp::Object_State::expired;

        case Fred::Object_State::identified_contact:
       return Epp::Object_State::identified_contact;

        case Fred::Object_State::linked:
       return Epp::Object_State::linked;

        case Fred::Object_State::mojeid_contact:
       return Epp::Object_State::mojeid_contact;

        case Fred::Object_State::not_validated:
       return Epp::Object_State::not_validated;

        case Fred::Object_State::nsset_missing:
       return Epp::Object_State::nsset_missing;

        case Fred::Object_State::outzone:
       return Epp::Object_State::outzone;

        case Fred::Object_State::outzone_unguarded:
       return Epp::Object_State::outzone_unguarded;

        case Fred::Object_State::server_blocked:
       return Epp::Object_State::server_blocked;

        case Fred::Object_State::server_delete_prohibited:
       return Epp::Object_State::server_delete_prohibited;

        case Fred::Object_State::server_inzone_manual:
       return Epp::Object_State::server_inzone_manual;

        case Fred::Object_State::server_outzone_manual:
       return Epp::Object_State::server_outzone_manual;

        case Fred::Object_State::server_registrant_change_prohibited:
       return Epp::Object_State::server_registrant_change_prohibited;

        case Fred::Object_State::server_renew_prohibited:
       return Epp::Object_State::server_renew_prohibited;

        case Fred::Object_State::server_transfer_prohibited:
       return Epp::Object_State::server_transfer_prohibited;

        case Fred::Object_State::server_update_prohibited:
       return Epp::Object_State::server_update_prohibited;

        case Fred::Object_State::unguarded:
       return Epp::Object_State::unguarded;

        case Fred::Object_State::validated_contact:
       return Epp::Object_State::validated_contact;

        case Fred::Object_State::validation_warning1:
       return Epp::Object_State::validation_warning1;

        case Fred::Object_State::validation_warning2:
       return Epp::Object_State::validation_warning2;

        case Fred::Object_State::outzone_unguarded_warning:
       return Epp::Object_State::outzone_unguarded_warning;

    }
    throw std::invalid_argument("value doesn't exist in Fred::Object_State::Enum");
}


} // namespace Conversion::Enums
} // namespace Conversion

#endif
