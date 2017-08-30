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

#ifndef POLL_REQUEST_GET_UPDATE_NSSET_DETAILS_H_FBF6E660C9A8487D836B76EBE1386F18
#define POLL_REQUEST_GET_UPDATE_NSSET_DETAILS_H_FBF6E660C9A8487D836B76EBE1386F18

#include "src/fredlib/opcontext.h"
#include "src/epp/nsset/info_nsset.h"

namespace Epp {
namespace Poll {

struct PollRequestUpdateNssetOutputData
{
    Epp::Nsset::InfoNssetOutputData old_data;
    Epp::Nsset::InfoNssetOutputData new_data;

    PollRequestUpdateNssetOutputData(
        const Epp::Nsset::InfoNssetOutputData& _old_data,
        const Epp::Nsset::InfoNssetOutputData& _new_data
    ) :
        old_data(_old_data),
        new_data(_new_data)
    {}
};

PollRequestUpdateNssetOutputData poll_request_get_update_nsset_details(
    Fred::OperationContext& _ctx,
    unsigned long long _message_id,
    unsigned long long _registrar_id);

} // namespace Epp::Poll
} // namespace Epp

#endif
