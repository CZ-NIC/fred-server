/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef CREATE_NSSET_HH_7B50D7FE4BBB4019AD8D94BBD0BD688C
#define CREATE_NSSET_HH_7B50D7FE4BBB4019AD8D94BBD0BD688C

#include "src/backend/epp/nsset/create_nsset_localized.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Nsset {

struct CreateNssetResult
{
    const unsigned long long id;
    const unsigned long long create_history_id;
    // TODO guarantee non-special
    const boost::posix_time::ptime crdate;


    CreateNssetResult(
            unsigned long long _nsset_id,
            unsigned long long _create_history_id,
            const boost::posix_time::ptime& _nsset_crdate)
        : id(_nsset_id),
          create_history_id(_create_history_id),
          crdate(_nsset_crdate)
    {
    }


};

CreateNssetResult create_nsset(
        LibFred::OperationContext& _ctx,
        const CreateNssetInputData& _create_nsset_input_data,
        const CreateNssetConfigData& _create_nsset_config_data,
        const SessionData& _session_data);


} // namespace Epp::Nsset
} // namespace Epp

#endif
