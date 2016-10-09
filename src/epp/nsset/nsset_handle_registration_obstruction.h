/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef EPP_NSSET_HANDLE_REGISTRATION_OBSTRUCTION_H_8f8b86441d374ace8a52657920472ff8
#define EPP_NSSET_HANDLE_REGISTRATION_OBSTRUCTION_H_8f8b86441d374ace8a52657920472ff8

#include <boost/assign/list_of.hpp>
#include <set>
#include <stdexcept>

#include "src/epp/exception.h"
#include "src/epp/reason.h"

namespace Epp
{

    struct NssetHandleRegistrationObstruction
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
    };

}

#endif
