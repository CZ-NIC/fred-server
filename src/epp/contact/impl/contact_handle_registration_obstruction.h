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
 */

#ifndef CONTACT_HANDLE_REGISTRATION_OBSTRUCTION_H_61A92CEC0BE84C5EBED69DC29CD3BDA0
#define CONTACT_HANDLE_REGISTRATION_OBSTRUCTION_H_61A92CEC0BE84C5EBED69DC29CD3BDA0

#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"

#include <set>

namespace Epp {
namespace Contact {

struct ContactHandleRegistrationObstruction
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
    static Reason::Enum to_reason(Enum value)
    {
        switch (value)
        {
            case invalid_handle:    return Reason::invalid_handle;
            case registered_handle: return Reason::existing;
            case protected_handle:  return Reason::protected_period;
        }
        throw MissingLocalizedDescription();
    }

    /**
     * @throws MissingLocalizedDescription
     */
    static Enum from_reason(Reason::Enum value)
    {
        switch (value)
        {
            case Reason::invalid_handle:   return invalid_handle;
            case Reason::existing:         return registered_handle;
            case Reason::protected_period: return protected_handle;
            default:
                throw MissingLocalizedDescription();
                break;
        }
    }
};

} // namespace Epp::Contact
} // namespace Epp

#endif
