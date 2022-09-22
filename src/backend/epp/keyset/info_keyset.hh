/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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
#ifndef INFO_KEYSET_HH_0985D216320F4D00A6F848634F3AE6D9
#define INFO_KEYSET_HH_0985D216320F4D00A6F848634F3AE6D9

#include "src/backend/epp/keyset/info_keyset_config_data.hh"
#include "src/backend/epp/keyset/info_keyset_output_data.hh"
#include "src/backend/epp/password.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Keyset {

/**
 * @throws NonexistentHandle
 */
InfoKeysetOutputData info_keyset(
        LibFred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        const InfoKeysetConfigData& _info_keyset_config_data,
        const Password& _authinfopw,
        const SessionData& _session_data);


} // namespace Epp::Keyset
} // namespace Epp

#endif
