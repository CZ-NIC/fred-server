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

#ifndef HANDLE_CHECK_RESULT_H_50B9A40CBF46B69CAACE2F8D2D34D68E//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define HANDLE_CHECK_RESULT_H_50B9A40CBF46B69CAACE2F8D2D34D68E

#include "src/epp/exception.h"
#include "src/epp/reason.h"

namespace Epp {
namespace KeySet {

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

}//namespace Epp::KeySet
}//namespace Epp

#endif//HANDLE_CHECK_RESULT_H_50B9A40CBF46B69CAACE2F8D2D34D68E