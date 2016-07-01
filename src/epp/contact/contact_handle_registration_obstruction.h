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

#ifndef EPP_CONTACT_HANDLE_REGISTRATION_OBSTRUCTION_H_03971013458
#define EPP_CONTACT_HANDLE_REGISTRATION_OBSTRUCTION_H_03971013458

#include "src/epp/exception.h"
#include "src/epp/reason.h"

#include <set>

namespace Epp {

struct ContactHandleRegistrationObstruction
{
    enum Enum
    {
        invalid_handle,
        protected_handle,
        registered_handle
    };

    static std::set< Enum > get_all_values()
    {
        static const Enum values[] =
        {
            invalid_handle,
            protected_handle,
            registered_handle
        };
        static const std::size_t number_of_values = sizeof(values) / sizeof(*values);
        static const Enum *const begin = values;
        static const Enum *const end = begin + number_of_values;
        return std::set< Enum >(begin, end);
    }

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

    static Enum from_reason(Reason::Enum value)
    {
        switch (value)
        {
            case Reason::invalid_handle:   return invalid_handle;
            case Reason::existing:         return registered_handle;
            case Reason::protected_period: return protected_handle;
            default:
                throw UnknownLocalizedDescriptionId();
        }
    }
};

/**
 * @throws MissingLocalizedDescription
 */
inline unsigned to_description_db_id(const ContactHandleRegistrationObstruction::Enum value)
{
    /**
     * XXX This is wrong - we are "reusing" descriptions of other objects. It is temporary (I've been promised) conscious hack.
     */
    return to_description_db_id(ContactHandleRegistrationObstruction::to_reason(value));
}

/**
 * @throws ExceptionMissingLocalizedDescription
 */
template < >
inline ContactHandleRegistrationObstruction::Enum from_description_db_id< ContactHandleRegistrationObstruction >(const unsigned id)
{
    /**
     * XXX This is wrong - we are "reusing" descriptions of other objects. It is temporary (I've been promised) conscious hack.
     */
    return ContactHandleRegistrationObstruction::from_reason(from_description_db_id< Reason >(id));
}

}

#endif
