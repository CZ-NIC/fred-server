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

#ifndef NSSET_OUTPUT_H_23CC782463CD425BB913B83178B77AC6
#define NSSET_OUTPUT_H_23CC782463CD425BB913B83178B77AC6

#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/epp/nsset/info_nsset.h"
#include "src/fredlib/object_state/get_object_states.h"

namespace Epp {
namespace Nsset {

InfoNssetOutputData get_info_nsset_output(
    const Fred::InfoNssetData& _data,
    const std::vector<Fred::ObjectStateData>& _object_state_data,
    bool _info_is_for_sponsored_registrar);

} // namespace Epp::Nsset
} // namespace Epp

#endif
