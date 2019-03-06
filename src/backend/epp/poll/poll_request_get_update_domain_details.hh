/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_HH_E2E18DD99B3A4FD3B9AE6E5DDFAB2DB8
#define POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_HH_E2E18DD99B3A4FD3B9AE6E5DDFAB2DB8

#include "libfred/opcontext.hh"

#include "src/backend/epp/domain/info_domain.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateDomainOutputData
{
    Epp::Domain::InfoDomainOutputData old_data;
    Epp::Domain::InfoDomainOutputData new_data;
};

PollRequestUpdateDomainOutputData poll_request_get_update_domain_details(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
