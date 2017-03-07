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

#ifndef GET_KEYSET_INFO_H_77F8A11467AE4C6D9F92CA89267E0172
#define GET_KEYSET_INFO_H_77F8A11467AE4C6D9F92CA89267E0172

#include "src/fredlib/object/object_state.h"
#include "src/fredlib/keyset/info_keyset_data.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/epp/keyset/info_keyset.h"

#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData get_keyset_info(
    const Fred::InfoKeysetData& data,
    const std::vector<Fred::ObjectStateData>& object_states_data,
    bool authinfopw_has_to_be_hidden);

} // namespace Epp::Keyset
} // namespace Epp

#endif
