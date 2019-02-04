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

#ifndef CREATE_DOMAIN_LOCALIZED_HH_D9720345EB9B4BF090357A9D636B3AA3
#define CREATE_DOMAIN_LOCALIZED_HH_D9720345EB9B4BF090357A9D636B3AA3

#include "src/backend/epp/domain/create_domain_config_data.hh"
#include "src/backend/epp/domain/create_domain_input_data.hh"
#include "src/backend/epp/domain/create_domain_localized_response.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "util/optional_value.hh"

namespace Epp {
namespace Domain {

CreateDomainLocalizedResponse create_domain_localized(
        const CreateDomainInputData& _create_domain_input_data,
        const CreateDomainConfigData& _create_domain_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
