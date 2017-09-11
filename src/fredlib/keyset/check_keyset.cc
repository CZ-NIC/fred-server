/*
 * Copyright (C) 2016 CZ.NIC, z.s.p.o.
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

#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object/check_handle.h"

namespace Fred {
namespace Keyset {

HandleState::SyntaxValidity get_handle_syntax_validity(OperationContext &_ctx, const std::string &_keyset_handle)
{
    if (TestHandleOf< Object_Type::keyset >(_keyset_handle).is_invalid_handle(_ctx)) {
        return HandleState::invalid;
    }
    return HandleState::valid;
}

HandleState::Registrability get_handle_registrability(OperationContext &_ctx,
                                                      const std::string &_keyset_handle)
{
    if (TestHandleOf< Object_Type::keyset >(_keyset_handle).is_registered(_ctx)) {
        return HandleState::registered;
    }

    if (TestHandleOf< Object_Type::keyset >(_keyset_handle).is_protected(_ctx)) {
        return HandleState::in_protection_period;
    }

    return HandleState::available;
}

}//namespace Fred::Keyset
}//namespace Fred
