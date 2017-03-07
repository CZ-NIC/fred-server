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

#ifndef CHECK_KEYSET_H_EFE348DAB8F14531A5877D6F14C74BFD
#define CHECK_KEYSET_H_EFE348DAB8F14531A5877D6F14C74BFD

#include "src/epp/keyset/keyset_handle_registration_obstruction.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Keyset {

/**
 * @returns check results for given contact handles
 */
std::map<std::string, Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> > check_keyset(
        Fred::OperationContext& _ctx,
        const std::set<std::string>& _keyset_handles,
        unsigned long long _registrar_id);

} // namespace Epp::Keyset
} // namespace Epp

#endif
