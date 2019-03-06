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
#ifndef POLL_REQUEST_GET_UPDATE_KEYSET_DETAILS_HH_C0DB9B2BCE0043E2825A69494B3331FE
#define POLL_REQUEST_GET_UPDATE_KEYSET_DETAILS_HH_C0DB9B2BCE0043E2825A69494B3331FE

#include "libfred/opcontext.hh"
#include "src/backend/epp/keyset/info_keyset_output_data.hh"

namespace Epp {
namespace Poll {

struct PollRequestUpdateKeysetOutputData
{
    Epp::Keyset::InfoKeysetOutputData old_data;
    Epp::Keyset::InfoKeysetOutputData new_data;
};

PollRequestUpdateKeysetOutputData poll_request_get_update_keyset_details(
    LibFred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
