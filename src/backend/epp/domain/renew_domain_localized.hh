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

#ifndef RENEW_DOMAIN_LOCALIZED_H_AADAB1519F8C4D6BAA4E0766858D8E49
#define RENEW_DOMAIN_LOCALIZED_H_AADAB1519F8C4D6BAA4E0766858D8E49

#include "src/backend/epp/domain/renew_domain_config_data.hh"
#include "src/backend/epp/domain/renew_domain_input_data.hh"
#include "src/backend/epp/domain/renew_domain_localized_response.hh"
#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "src/util/optional_value.hh"

namespace Epp {
namespace Domain {

RenewDomainLocalizedResponse renew_domain_localized(
        const RenewDomainInputData& _renew_domain_input_data,
        const RenewDomainConfigData& _renew_domain_config_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
