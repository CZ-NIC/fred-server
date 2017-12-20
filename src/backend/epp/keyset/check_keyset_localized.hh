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

#ifndef CHECK_KEYSET_LOCALIZED_H_231A7D4B89FC4368A15ED984120F7488
#define CHECK_KEYSET_LOCALIZED_H_231A7D4B89FC4368A15ED984120F7488

#include "src/backend/epp/keyset/check_keyset_config_data.hh"
#include "src/backend/epp/keyset/check_keyset_localized_response.hh"
#include "src/backend/epp/session_data.hh"

#include <set>
#include <string>

namespace Epp {
namespace Keyset {

CheckKeysetLocalizedResponse check_keyset_localized(
        const std::set<std::string>& _keyset_handles,
        const CheckKeysetConfigData& _check_keyset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
