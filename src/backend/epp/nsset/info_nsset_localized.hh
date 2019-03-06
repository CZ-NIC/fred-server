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
#ifndef INFO_NSSET_LOCALIZED_HH_BF05EC1AF971477EA1286FF9EF528469
#define INFO_NSSET_LOCALIZED_HH_BF05EC1AF971477EA1286FF9EF528469

#include "src/backend/epp/nsset/info_nsset_config_data.hh"
#include "src/backend/epp/nsset/info_nsset_localized_response.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Nsset {

InfoNssetLocalizedResponse info_nsset_localized(
        const std::string& _nsset_handle,
        const InfoNssetConfigData& _info_nsset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Nsset
} // namespace Epp

#endif
