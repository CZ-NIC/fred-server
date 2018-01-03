/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef INFO_KEYSET_LOCALIZED_HH_B61D8FD3EC2A4DA3B3FAC9BB98C79F45
#define INFO_KEYSET_LOCALIZED_HH_B61D8FD3EC2A4DA3B3FAC9BB98C79F45

#include "src/backend/epp/keyset/info_keyset_config_data.hh"
#include "src/backend/epp/keyset/info_keyset_localized_response.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Keyset {

InfoKeysetLocalizedResponse info_keyset_localized(
        const std::string& _keyset_handle,
        const InfoKeysetConfigData& _info_keyset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
