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

#include "src/fredlib/contact/check_contact.h"

#include "src/fredlib/object/check_handle.h"

namespace Fred
{

namespace Contact
{
    ContactHandleState::SyntaxValidity::Enum get_handle_syntax_validity(OperationContext& ctx, const std::string& _contact_handle) {

        if (TestHandleOf< Object_Type::contact >(_contact_handle).is_invalid_handle(ctx)) {
            return ContactHandleState::SyntaxValidity::invalid;
        }
        return ContactHandleState::SyntaxValidity::valid;
    }

    ContactHandleState::Registrability::Enum get_handle_registrability(OperationContext& ctx, const std::string& _contact_handle) {
        if (TestHandleOf< Object_Type::contact >(_contact_handle).is_registered(ctx)) {
            return ContactHandleState::Registrability::registered;
        }

        if (TestHandleOf< Object_Type::contact >(_contact_handle).is_protected(ctx)) {
            return ContactHandleState::Registrability::in_protection_period;
        }

        return ContactHandleState::Registrability::available;
    }
}

}

