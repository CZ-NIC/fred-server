/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file DEPRECATED implementation of object states. Do not use. Use src/fredlib/object/object_state.h instead.
 *  object state name
 */

#ifndef OBJECT_STATE_NAME_HH_A1D7A78BB8C24E9DBB3AC4D5D6D4A0C3
#define OBJECT_STATE_NAME_HH_A1D7A78BB8C24E9DBB3AC4D5D6D4A0C3

#include <string>

namespace LibFred
{
    /**
     * @namespace ObjectState
     * object state names string constants from db table enum_object_states.
     */
    namespace ObjectState
    {//object state names from db table enum_object_states
        const std::string SERVER_DELETE_PROHIBITED = "serverDeleteProhibited";//id 1
        const std::string SERVER_RENEW_PROHIBITED = "serverRenewProhibited";//id 2
        const std::string SERVER_TRANSFER_PROHIBITED = "serverTransferProhibited";//id 3
        const std::string SERVER_UPDATE_PROHIBITED = "serverUpdateProhibited";//id 4
        const std::string SERVER_OUTZONE_MANUAL = "serverOutzoneManual";//id 5
        const std::string SERVER_INZONE_MANUAL = "serverInzoneManual";//id 6
        const std::string SERVER_BLOCKED = "serverBlocked";//id 7
        const std::string EXPIRATION_WARNING = "expirationWarning";//id 8
        const std::string EXPIRED = "expired";//id 9
        const std::string UNGUARDED = "unguarded";//id 10
        const std::string VALIDATION_WARNING1 = "validationWarning1";//id 11
        const std::string VALIDATION_WARNING2 = "validationWarning2";//id 12
        const std::string NOT_VALIDATED = "notValidated";//id 13
        const std::string NSSET_MISSING = "nssetMissing";//id 14
        const std::string OUTZONE = "outzone";//id 15
        const std::string LINKED = "linked";//id 16
        const std::string DELETE_CANDIDATE = "deleteCandidate";//id 17
        const std::string SERVER_REGISTRANT_CHANGE_PROHIBITED = "serverRegistrantChangeProhibited";//id 18
        const std::string DELETE_WARNING = "deleteWarning";//id 19
        const std::string OUTZONE_UNGUARDED = "outzoneUnguarded";//id 20
        const std::string CONDITIONALLY_IDENTIFIED_CONTACT = "conditionallyIdentifiedContact";//id 21
        const std::string IDENTIFIED_CONTACT = "identifiedContact";//id 22
        const std::string VALIDATED_CONTACT = "validatedContact";//id 23
        const std::string MOJEID_CONTACT = "mojeidContact";//id 24
        const std::string OUTZONE_UNGUARDED_WARNING = "outzoneUnguardedWarning";//id 28
    }
} // namespace LibFred

#endif
