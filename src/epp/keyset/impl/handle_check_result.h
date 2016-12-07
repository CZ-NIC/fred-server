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

#ifndef HANDLE_CHECK_RESULT_H_F73D97BD86BC43568A68AD4CB1E21377
#define HANDLE_CHECK_RESULT_H_F73D97BD86BC43568A68AD4CB1E21377

#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"

namespace Epp {
namespace Keyset {

struct HandleCheckResult
{
    enum Enum
    {
        invalid_handle,
        protected_handle,
        registered_handle
    };
};

/**
 * @throws MissingLocalizedDescription
 */
inline Reason::Enum to_reason(const HandleCheckResult::Enum value)
{
    switch (value)
    {
        case HandleCheckResult::invalid_handle   : return Reason::invalid_handle;
        case HandleCheckResult::registered_handle: return Reason::existing;
        case HandleCheckResult::protected_handle : return Reason::protected_period;
    }

    throw MissingLocalizedDescription();
}

}//namespace Epp::Keyset
}//namespace Epp

#endif
