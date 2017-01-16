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

#ifndef DELETE_DOMAIN_LOCALIZED_H_7881B671649445C0BD9D46F87C8643B8
#define DELETE_DOMAIN_LOCALIZED_H_7881B671649445C0BD9D46F87C8643B8

#include "src/epp/impl/epp_response_success_localized.h"
#include "src/epp/impl/notification_data.h"
#include "src/epp/impl/session_data.h"

#include <string>

namespace Epp {
namespace Domain {

EppResponseSuccessLocalized delete_domain_localized(
        const std::string& _domain_fqdn,
        const SessionData& _session_data,
        const NotificationData& _notification_data);

} // namespace Epp::Domain
} // namespace Epp

#endif
