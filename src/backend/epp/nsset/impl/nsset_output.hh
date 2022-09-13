/*
 * Copyright (C) 2017-2022  CZ.NIC, z. s. p. o.
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

#ifndef NSSET_OUTPUT_HH_E733861A880745A8B2CA2681046834B8
#define NSSET_OUTPUT_HH_E733861A880745A8B2CA2681046834B8

#include "libfred/registrable_object/nsset/info_nsset_data.hh"
#include "src/backend/epp/nsset/info_nsset.hh"
#include "libfred/object_state/get_object_states.hh"

namespace Epp {
namespace Nsset {

InfoNssetOutputData get_info_nsset_output(
    const LibFred::InfoNssetData& _data,
    const std::vector<LibFred::ObjectStateData>& _object_state_data);

} // namespace Epp::Nsset
} // namespace Epp

#endif
