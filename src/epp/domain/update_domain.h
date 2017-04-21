/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_DOMAIN_H_2D1D225B719341E0B0F4384B04E36D74
#define UPDATE_DOMAIN_H_2D1D225B719341E0B0F4384B04E36D74

#include "src/epp/contact/update_contact.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/update_domain_config_data.h"
#include "src/epp/domain/update_domain_input_data.h"
#include "src/epp/session_data.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

unsigned long long update_domain(
        Fred::OperationContext& _ctx,
        const UpdateDomainInputData& _update_domain_input_data,
        const UpdateDomainConfigData& _update_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
