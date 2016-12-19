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

/**
 *  @file domain_check.h
 *  <++>
 */

#ifndef DOMAIN_CHECK_H_465105F022A041DC84D07A0A30AF568C
#define DOMAIN_CHECK_H_465105F022A041DC84D07A0A30AF568C

#include "src/epp/impl/action.h"
#include "src/epp/domain/domain_check_localization.h"
#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/epp/impl/response_localized.h"
#include "src/epp/impl/session_lang.h"

#include <set>
#include <map>

namespace Epp {

namespace Domain {

struct DomainCheckResponse {
    const LocalizedSuccessResponse localized_success_response;
    const DomainFqdnToDomainLocalizedRegistrationObstruction domain_fqdn_to_domain_localized_registration_obstruction;

    DomainCheckResponse(
        const LocalizedSuccessResponse& localized_success_response,
        const DomainFqdnToDomainLocalizedRegistrationObstruction& domain_fqdn_to_domain_localized_registration_obstruction
    ) :
        localized_success_response(localized_success_response),
        domain_fqdn_to_domain_localized_registration_obstruction(domain_fqdn_to_domain_localized_registration_obstruction)
    { }
};

DomainCheckResponse domain_check(
    const std::set<std::string>& _domain_fqdns,
    unsigned long long _registrar_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle
);

}

}

#endif
