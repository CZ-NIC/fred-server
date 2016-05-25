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

#ifndef HANDLE_STATE_7739859411
#define HANDLE_STATE_7739859411

namespace Fred
{

namespace NssetHandleState {
    struct InRegistry {
        enum Enum {
            registered,
            in_protection_period,
            unregistered,
        };
    };

    struct SyntaxValidity {
        enum Enum {
            valid,
            invalid,
        };
    };
}

}
#endif
