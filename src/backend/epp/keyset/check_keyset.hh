/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CHECK_KEYSET_HH_6AF77422290D4D18870F0E59C46DF630
#define CHECK_KEYSET_HH_6AF77422290D4D18870F0E59C46DF630

#include "src/backend/epp/keyset/check_keyset_config_data.hh"
#include "src/backend/epp/keyset/keyset_handle_registration_obstruction.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"
#include "util/db/nullable.hh"

#include <map>
#include <set>
#include <string>

namespace Epp {
namespace Keyset {

/**
 * @returns check results for given contact handles
 */
std::map<std::string, Nullable<Keyset::KeysetHandleRegistrationObstruction::Enum> > check_keyset(
        LibFred::OperationContext& _ctx,
        const std::set<std::string>& _keyset_handles,
        const CheckKeysetConfigData& _check_keyset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
