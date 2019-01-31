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

#ifndef DOMAIN_OUTPUT_HH_185F3D29A6954423AF5335D867A82AFD
#define DOMAIN_OUTPUT_HH_185F3D29A6954423AF5335D867A82AFD

#include "libfred/registrable_object/domain/info_domain_data.hh"
#include "src/backend/epp/domain/info_domain.hh"
#include "libfred/object_state/get_object_states.hh"

#include <vector>

namespace Epp {
namespace Domain {

InfoDomainOutputData get_info_domain_output(
    const LibFred::InfoDomainData& _data,
    const std::vector<LibFred::ObjectStateData>& _object_state_data,
    bool _info_is_for_sponsoring_registrar);

} // namespace Epp::Domain
} // namespace Epp


#endif
