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

#ifndef UPDATE_DOMAIN_LOCALIZED_H_F4E3294590164F499765EEE0244A5196
#define UPDATE_DOMAIN_LOCALIZED_H_F4E3294590164F499765EEE0244A5196

#include "src/epp/domain/update_domain_input_data.h"
#include "src/epp/epp_response_success_localized.h"
#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Epp {
namespace Domain {

EppResponseSuccessLocalized update_domain_localized(
        const UpdateDomainInputData& _update_domain_input_data,
        const SessionData& _session_data,
        const NotificationData& _notification_data,
        const Optional<unsigned long long>& _logd_request_id,
        bool _rifd_epp_update_domain_keyset_clear);


} // namespace Epp::Domain
} // namespace Epp

#endif
