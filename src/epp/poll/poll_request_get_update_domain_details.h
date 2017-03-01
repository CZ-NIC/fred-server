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

#ifndef POLL_POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_H_EE5157F1F74A49D28A6B8F4CC22BB363
#define POLL_POLL_REQUEST_GET_UPDATE_DOMAIN_DETAILS_H_EE5157F1F74A49D28A6B8F4CC22BB363

#include "src/fredlib/opcontext.h"
#include "src/fredlib/domain/info_domain.h"

namespace Epp {
namespace Poll {

struct PollRequestUpdateDomainOutputData
{
    Fred::InfoDomainOutput old_data;
    Fred::InfoDomainOutput new_data;
};

PollRequestUpdateDomainOutputData poll_request_get_update_domain_details(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
