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

#ifndef DOMAIN_OUTPUT_H_33D3AFAA0E8044C39744204635F10659
#define DOMAIN_OUTPUT_H_33D3AFAA0E8044C39744204635F10659

#include "src/fredlib/domain/info_domain_data.h"
#include "src/epp/domain/info_domain.h"
#include "src/fredlib/object_state/get_object_states.h"

#include <vector>

namespace Epp {
namespace Domain {

InfoDomainOutputData get_info_domain_output(
    const Fred::InfoDomainData& _data,
    const std::vector<Fred::ObjectStateData>& _object_state_data,
    bool _authinfopw_has_to_be_hidden);

} // namespace Epp::Domain
} // namespace Epp


#endif
