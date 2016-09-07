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
 *  @file domain_info.h
 *  <++>
 */

#ifndef SRC_EPP_DOMAIN_DOMAIN_INFO_H
#define SRC_EPP_DOMAIN_DOMAIN_INFO_H

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"

namespace Epp {

namespace Domain {

struct DomainInfoOutputData {
    DomainInfoOutputData() { }
};

struct DomainInfoResponse {
    const LocalizedSuccessResponse localized_success_response;
    const DomainInfoOutputData domain_info_output_data;

    DomainInfoResponse(
        const LocalizedSuccessResponse& localized_success_response,
        const DomainInfoOutputData& domain_info_output_data
    ) :
        localized_success_response(localized_success_response),
        domain_info_output_data(domain_info_output_data)
    { }
};

DomainInfoResponse domain_info(
    const std::string& domain_fqdn,
    unsigned long long registrar_id,
    SessionLang::Enum lang,
    const std::string& server_transaction_handle
);

}

}

#endif
