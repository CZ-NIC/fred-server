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

#ifndef KEYSET_OUTPUT_H_0B6E9FCC43D34765ADF95835FBEA9426
#define KEYSET_OUTPUT_H_0B6E9FCC43D34765ADF95835FBEA9426

#include "src/fredlib/keyset/info_keyset_data.h"
#include "src/epp/keyset/info_keyset_output_data.h"
#include "src/fredlib/object_state/get_object_states.h"

#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData get_info_keyset_output(
    const Fred::InfoKeysetData& _data,
    const std::vector<Fred::ObjectStateData>& _object_state_data,
    bool _authinfopw_has_to_be_hidden);

} // namespace Epp::Keyset
} // namespace Epp

#endif
