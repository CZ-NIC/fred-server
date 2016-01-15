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
 *  implementation of mojeid2 internals
 */

#include "src/mojeid/mojeid_impl_internal.h"
#include "src/mojeid/mojeid_impl_data.h"

namespace Registry {
namespace MojeIDImplInternal {

void raise(const CheckMojeIDRegistration &result)
{
    if (result.Fred::MojeID::Check::states_before_transfer_into_mojeid::mojeid_contact_present) {
        throw MojeIDImplData::AlreadyMojeidContact();
    }
    if (result.Fred::MojeID::Check::states_before_transfer_into_mojeid::server_admin_blocked) {
        throw MojeIDImplData::ObjectAdminBlocked();
    }
    if (result.Fred::MojeID::Check::states_before_transfer_into_mojeid::server_user_blocked) {
        throw MojeIDImplData::ObjectUserBlocked();
    }
    throw std::runtime_error("unexpected result of CheckMojeIDRegistration");
}

}//namespace Registry::MojeIDImplInternal
}//namespace Registry
