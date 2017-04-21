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

#ifndef KEYSET_HANDLE_REGISTRATION_OBSTRUCTION_H_07D209223EC644388C037073F58ED928
#define KEYSET_HANDLE_REGISTRATION_OBSTRUCTION_H_07D209223EC644388C037073F58ED928

#include "src/epp/exception.h"
#include "src/epp/reason.h"

namespace Epp {
namespace Keyset {

struct KeysetHandleRegistrationObstruction
{
    enum Enum
    {
        invalid_handle,
        protected_handle,
        registered_handle

    };

    /**
     * @throws MissingLocalizedDescription
     */
    static Reason::Enum to_reason(const KeysetHandleRegistrationObstruction::Enum value)
    {
        switch (value)
        {
            case KeysetHandleRegistrationObstruction::invalid_handle:
                return Reason::invalid_handle;

            case KeysetHandleRegistrationObstruction::registered_handle:
                return Reason::existing;

            case KeysetHandleRegistrationObstruction::protected_handle:
                return Reason::protected_period;
        }
        throw MissingLocalizedDescription();
    }


    /**
     * @throws MissingLocalizedDescription
     */
    static KeysetHandleRegistrationObstruction::Enum from_reason(const Reason::Enum value)
    {
        switch (value)
        {
            case Reason::invalid_handle:
                return KeysetHandleRegistrationObstruction::invalid_handle;

            case Reason::existing:
                return KeysetHandleRegistrationObstruction::registered_handle;

            case Reason::protected_period:
                return KeysetHandleRegistrationObstruction::protected_handle;

            default:
                throw MissingLocalizedDescription();
        }
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
