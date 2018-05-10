/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef UNDISCLOSE_ADDRESS_HH_E7C08A53071943B6894428F2E9EE37BF
#define UNDISCLOSE_ADDRESS_HH_E7C08A53071943B6894428F2E9EE37BF

#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace Contact {

void undisclose_address(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string& _registrar_handle);

void undisclose_address_async(
        unsigned long long _contact_id,
        const std::string& _registrar_handle);

} // namespace LibFred::Contact
} // namespace LibFred

#endif
