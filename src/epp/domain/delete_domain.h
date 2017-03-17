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

#ifndef DELETE_DOMAIN_H_E59390C3378948FBBEE42A0F7666F0E7
#define DELETE_DOMAIN_H_E59390C3378948FBBEE42A0F7666F0E7

#include "src/epp/domain/delete_domain_config_data.h"
#include "src/epp/session_data.h"
#include "src/fredlib/opcontext.h"

#include <string>

namespace Epp {
namespace Domain {

/**
 * If successful (no exception thrown), state requests of domain are performed.
 * In case of exception, behaviour is undefined and transaction should bo rolled back.
 *
 * \returns last domain history id before delete
 */
unsigned long long delete_domain(
        Fred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const DeleteDomainConfigData& _delete_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
