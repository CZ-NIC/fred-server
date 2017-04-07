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

#ifndef POLL_REQUEST_GET_UPDATE_KEYSET_DETAILS_H_0C634699015A4D1B82588E2B54A52D87
#define POLL_REQUEST_GET_UPDATE_KEYSET_DETAILS_H_0C634699015A4D1B82588E2B54A52D87

#include "src/fredlib/opcontext.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/epp/keyset/info_keyset_output_data.h"

namespace Epp {
namespace Poll {

struct PollRequestUpdateKeysetOutputData
{
    Epp::Keyset::InfoKeysetOutputData old_data;
    Epp::Keyset::InfoKeysetOutputData new_data;
};

PollRequestUpdateKeysetOutputData poll_request_get_update_keyset_details(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
