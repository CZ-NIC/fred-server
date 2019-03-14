/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef KEYSET_OUTPUT_HH_9A6DDDEE23EE40D2A15877A699B5EE90
#define KEYSET_OUTPUT_HH_9A6DDDEE23EE40D2A15877A699B5EE90

#include "libfred/registrable_object/keyset/info_keyset_data.hh"
#include "src/backend/epp/keyset/info_keyset_output_data.hh"
#include "libfred/object_state/get_object_states.hh"

#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData get_info_keyset_output(
    const LibFred::InfoKeysetData& _data,
    const std::vector<LibFred::ObjectStateData>& _object_state_data,
    bool _info_is_for_sponsoring_registrar);

} // namespace Epp::Keyset
} // namespace Epp

#endif
