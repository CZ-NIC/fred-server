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
    ContactHandleState::SyntaxValidity::Enum is_handle_valid(const std::string& _contact_handle) {

        if( TestHandle(_contact_handle).is_invalid_handle() ) {
            return ContactHandleState::SyntaxValidity::invalid;
        }
        return ContactHandleState::SyntaxValidity::valid;
    }

    ContactHandleState::Registrability::Enum is_handle_in_registry(OperationContext& ctx, const std::string& _contact_handle) {
        if( TestHandle(_contact_handle).is_registered(ctx, "contact") ) {
            return ContactHandleState::Registrability::registered;
        }

        if( TestHandle(_contact_handle).is_protected(ctx, "contact") ) {
            return ContactHandleState::Registrability::in_protection_period;
        }

        return ContactHandleState::Registrability::available;
    }
}

}

